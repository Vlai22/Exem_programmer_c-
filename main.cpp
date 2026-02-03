#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>

void WriteLineColor(HANDLE hConsloe, int x, int y, std::string  line, WORD color){
    COORD pos = {(SHORT) x, (SHORT) y};
    DWORD written;    
    WriteConsoleOutputCharacter(hConsloe, line.c_str(), (DWORD)line.length(), pos, &written);
    WORD* color_att = new WORD[line.length()];
    for(int i=0; i < line.length(); i++){ 
        color_att[i] = color;
    }
    WriteConsoleOutputAttribute(hConsloe, color_att, line.length(), pos, &written);
    delete[] color_att;
}
void ClearPole(HANDLE hConsole, int x, int y, int w, int h){
    COORD pos = { (SHORT) x, (SHORT) y};
    DWORD written;
    DWORD count = w * h;

    // Стираем символы (Unicode версия)
    FillConsoleOutputCharacterW(hConsole, ' ', count, pos, &written);
    
    // Стираем цвета (ставим белый текст на черном фоне)
    WORD defaultAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    FillConsoleOutputAttribute(hConsole, defaultAttr, count, pos, &written);
}
struct answer{
    std::string answer_str;
    bool answer_click = false;
};
struct quetion{
    std::string title;
    int id;
    bool answer_arr;
    std::vector <answer> answers;
    bool answer_status;
    std::vector <std::string> answer_true;
    bool done = false;
};

int main() {
    //выставляем кодировку консоли в utf-8
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    //переменные работы с Windows API консоли
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    COORD coordScreen = {0, 0}; //создание всех необходимых переменых и получения текущей консоли
    DWORD cCharsWritten;
    DWORD eventsRead;
    INPUT_RECORD ir;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;
    //переменные работы с запуском и прекращением программы
    bool run = true;
    bool exit = false;
    //рандом
    std::random_device rd; 
    std::mt19937 gen(rd()); // rd() возвращает число, которое становится сидом для gen
    //переменные работы с меню
    int menu = 1;//меню 1 главное меню | меню 2 админ меню | меню 3 меню ввода имени | меню 4 меню вопросов | меню 0 меню ошибка выхода
    int menu_input = 1;
    int menu_size = 2;
    int menu_quetion = 0;
    // ненбольшие определения для упрощение кода не слишком вдавался в подробности хочеться побыстрее закончить а не разбираться 
    using clock_t = std::chrono::steady_clock;
    using time_point_t = std::chrono::time_point<clock_t>;
    //время с начала экзамена
    time_point_t timer_exam;
    //запуск таймера после захода в вопросы выходить из вопросов нельзя
    bool timer_exam_start = false;
    //переменная начала таймера 
    time_point_t event_start;
    //конец таймера
    time_point_t event_end;
    std::chrono::minutes duration(1);
    //пременные данных пользователя
    std::string name_admin;
    std::string name_user;
    //вектор с вопросами
    std::vector <quetion> ques;
    //массив вопросов
    int arr_ques[10];
    //получение разамеров текущей консоли
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return 0;
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    //  залитие всего простарнства пробелами
    if (!FillConsoleOutputCharacterW(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten)) return 0;
    // сброс всех аттрибутов 
    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return 0;
    //подгрузка вопросов 
    std::fstream file;
    file.open("quetion.csv", std::ios::in);
    if(file.is_open()){
        std::string line;
        while(std::getline(file, line)){
            if(line.empty()) continue; // Пропуск пустых строк
            std::vector<std::string> arr;
            std::stringstream ss(line);
            std::string segment;
            // Разбиваем строку по ';'
            while(std::getline(ss, segment, ';')) {
                arr.push_back(segment);
            }
            // Проверяем, что считали хотя бы базовые поля (id, title, flag)
            if(arr.size() < 3) continue; 
            quetion que;
            que.id = std::stoi(arr.at(0));
            que.title = arr.at(1);
            que.answer_arr = (arr.at(2) == "true");
            int i = 3;
            // Собираем ответы, пока не встретим true/false или не кончится массив
            while(i < arr.size() && arr.at(i) != "true" && arr.at(i) != "false") {
                que.answers.push_back(answer(arr.at(i), false));
                i++;
            }
            // Обработка оставшихся элементов
            if(i < arr.size()) {
                std::string status = arr.at(i);
                que.answer_status = (status == "true");
                i++; 
                while(i < arr.size()) {
                    que.answer_true.push_back(arr.at(i));
                    i++;
                }
            }
            ques.push_back(que);
        }
    }
    file.close();
    std::uniform_int_distribution<> dist_q(0, ques.size()-2);
    int a = 0;//получаем массив уникальных номеров вопросов рандомных
    while(a < 10){
        bool new_sid;
        int sid = 0;
        do{
            new_sid = true;
            sid = dist_q(gen)+1;
            for(int i=0;i<10;i++){
                if (arr_ques[i] == sid){
                    new_sid = false;
                    break;
                }
            }
        }while(!new_sid);
        arr_ques[a] = sid;
        a++;
    }
    while(run){
        //отключаем CTRL+C что бы не выключалось приложение
        SetConsoleCtrlHandler(NULL, TRUE);
        //системные никогда не стрираемые атрибуты
        WriteLineColor(hConsole, 10, 0, "Программа для прохождения письменного экзамена", FOREGROUND_RED);
        WriteLineColor(hConsole, 0, 20, "Перед началом экзамена и приступания к вопросам перейдите в первое меню и введите ваше ФИО. Затем зайдите в экзамен второй пункт главного меню. После входа в экзамен начнётся 30 минутный таймер по истечению вас выкинет в главное меню. Если вы не ввели ФИО введите его. Управление: стрелки вверx и вниз для выбора параметров, Enter для выбора единичного или множественного ответа, для выхода в главное меню используйте Esc, для полного выхода используйте CTRL+Q, CTRL+D следующий вопрос, CTRL+X предыдущий вопрос", FOREGROUND_RED | FOREGROUND_GREEN);
        //отрисовка меню 
        if(menu == 1){
            ClearPole(hConsole, 0, 2, 80, 30);
            WriteLineColor(hConsole, 10, 2, "Вписать своё имя и фамилию", menu_input == 1 ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
            WriteLineColor(hConsole, 10, 3, "Перейти к экзамену", menu_input == 2 ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
        }else if(menu == 0){
            ClearPole(hConsole, 0, 2, 80, 30);
            WriteLineColor(hConsole, 0, 19, "Вы не можожете выйти до тех пор пока не введёте корректно своё имя и фамилию или не прорешаете экзамен!!", FOREGROUND_RED);
        }else if(menu == 3){
            ClearPole(hConsole, 0, 2, 80, 30);
            WriteLineColor(hConsole, 0, 3, name_user , FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            WriteLineColor(hConsole, 0, 19, "Введите своё имя и фамилию!!!", FOREGROUND_RED);
        }else if(menu == 4 && !exit){
            ClearPole(hConsole, 0, 2, 80, 30);
            //вывод вопросов осуществляется из общего списка по номеру который был выбран уникально рандомом, также что бы переключаться между 
            //вопросами была добавленна паременная menu_quetion что бы мы были на рандомно выбранном вопросе первом втором и так далее
            std::string que_value;
            if(ques.at(arr_ques[menu_quetion]-1).answer_status){
                que_value = (ques.at(arr_ques[menu_quetion]-1).answer_arr) ? "Введите неверные ответы на вопрос." : "Введите неверный ответ на вопрос.";
            }else{
                que_value = (ques.at(arr_ques[menu_quetion]-1).answer_arr) ? "Введите верные ответы на вопрос." : "Введите верный ответ на вопрос.";
            }
            WriteLineColor(hConsole, 0, 3, ques.at(arr_ques[menu_quetion]-1).title + que_value , FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            menu_size = ques.at(arr_ques[menu_quetion]-1).answers.size();
            for(int i =0; i<ques.at(arr_ques[menu_quetion]-1).answers.size(); i++){
                WriteLineColor(hConsole, 0, i+4, std::to_string(i) + ") " + ques.at(arr_ques[menu_quetion]-1).answers.at(i).answer_str , 
                (menu_input == i + 1) ? (FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE) // Курсор на кнопке (белый текст на синем)
                : (ques.at(arr_ques[menu_quetion] - 1).answers.at(i).answer_click) 
                ? (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | BACKGROUND_GREEN) // Выбрано (белый текст на зеленом)
                : (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN) // Обычное состояние (просто белый текст)
            );
            }

        }else if(menu == 2){
            ClearPole(hConsole, 0, 2, 80, 15);
            WriteLineColor(hConsole, 0, 3, name_admin , FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            WriteLineColor(hConsole, 0, 19, "Введите имя студента которого вы туда посадили!!", FOREGROUND_RED);
        }
        //ввод в консоль
        ReadConsoleInput(hInput, &ir, 1, &eventsRead);
        //нажатие
        if(ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown){
            DWORD modifiers  = ir.Event.KeyEvent.dwControlKeyState;
            WORD keyCode = ir.Event.KeyEvent.wVirtualKeyCode;
            wchar_t unicodeChar = ir.Event.KeyEvent.uChar.UnicodeChar;
            //сочетание клавиш ctrl+q ctrl+s
            bool ctrlPress = false;
            if((modifiers & LEFT_CTRL_PRESSED) || (modifiers & RIGHT_CTRL_PRESSED)){
                ctrlPress = true;
            }else{
                ctrlPress = false;
            }
            if(ctrlPress){
                if(keyCode == 'Q'){
                    if(name_user != ""){
                        exit = true;
                    }
                    if(exit){
                        run = false;
                        file.open("C:/Users/Public/exam.txt", std::ios::out);
                        file << "Имя фамилия введённая админом: " << name_admin << "\n";
                        file << "Имя фамилия введённая учеником: " << name_user << "\n";
                        file << "Балы студента: ";
                        int value_ball = 0;
                        int max_ball = 0;
                        for(int i=0;i<10;i++){
                            max_ball += ques.at(arr_ques[i]-1).answer_true.size();
                            for(int k=0;k<ques.at(arr_ques[i]-1).answers.size();k++){
                                for(int y=0;y<ques.at(arr_ques[i]-1).answer_true.size();y++){
                                    if(ques.at(arr_ques[i]-1).answers.at(k).answer_str == ques.at(arr_ques[i]-1).answer_true.at(y) && ques.at(arr_ques[i]-1).answers.at(k).answer_click){
                                        value_ball++;
                                        break;
                                    }
                                }
                            }
                        }
                        file << std::to_string(max_ball) << "/" << std::to_string(value_ball) << "\n";
                        for(int i=0;i<10;i++){
                            file << "Номер вопроса ученика: " << std::to_string(ques.at(arr_ques[i]).id) << "\nОтветы студента: ";
                            for(int k=0;k<ques.at(arr_ques[i]-1).answers.size();k++){
                                if(ques.at(arr_ques[i]-1).answers.at(k).answer_click){
                                    file <<  ques.at(arr_ques[i]-1).answers.at(k).answer_str << " | ";
                                }
                            }
                            file << "\n" << "Верные ответы: ";
                            for(int k=0;k<ques.at(arr_ques[i]-1).answer_true.size();k++){
                                file << ques.at(arr_ques[i]-1).answer_true.at(k) << " | ";
                            }
                            file << "\n";
                        }
                    }else{
                        menu = 0;
                    }
                }else if(keyCode == 'S'){
                    menu = 2;
                }else if(keyCode == 'D' && menu_quetion < 9 && !exit){
                    menu_quetion += 1;
                }else if(keyCode == 'D' && menu_quetion >= 9 && !exit){
                    menu_quetion = 0;
                }else if(keyCode == 'X' && menu_quetion > 0 && !exit){
                    menu_quetion -= 1;
                }else if(keyCode == 'X' && menu_quetion <= 0 && !exit){
                    menu_quetion = 8;
                }
            }
            if(unicodeChar != 0 && menu == 3 && unicodeChar != 13){
                name_user += unicodeChar;
            }else if(unicodeChar != 0 && menu == 2 && unicodeChar != 13){
                name_admin += unicodeChar;
            }
            if(keyCode == VK_RETURN){
                if(menu == 1){
                    if(menu_input == 1){
                        menu = 3;
                    }else if(menu_input == 2 && !exit){
                        menu = 4;
                        timer_exam_start = true;
                        event_start = std::chrono::steady_clock::now();
                        event_end = event_start + duration;
                    }
                }else if(menu == 2){
                    menu = 1;
                }else if(menu == 3){
                    menu = 1;
                }else if(menu == 4 && !exit){
                    ques.at(arr_ques[menu_quetion]-1).answers.at(menu_input-1).answer_click = !ques.at(arr_ques[menu_quetion]-1).answers.at(menu_input-1).answer_click;
                }
            }else if(keyCode == VK_ESCAPE){
                if(menu != 1){
                    menu = 1;
                    menu_quetion = 1;
                }
            }else if(keyCode == VK_UP){
                menu_input = (menu_input > 1) ? menu_input - 1 : menu_size;
            }else if(keyCode == VK_DOWN){
                menu_input = (menu_input < menu_size) ? menu_input + 1 : 1;
            }else if(keyCode == VK_BACK){
                if(menu == 2){
                    name_admin = name_admin.substr(0, name_admin.length()-2);
                }else if(menu == 3){
                    name_user = name_user.substr(0, name_user.length()-2);
                }
            }
        }  
        if(timer_exam_start){
            timer_exam = std::chrono::steady_clock::now();
        }
        if(timer_exam > event_end){
            exit = true;
        }
    }
    return 0;
}