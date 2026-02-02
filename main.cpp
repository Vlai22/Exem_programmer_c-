#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <random>

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

struct quetion{
    std::string title;
    int id;
    bool answer_arr;
    std::vector <std::string> answer;
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
    // Чтобы получить число, используйте распределение:


    //переменные работы с меню
    int menu = 1;//меню 1 главное меню | меню 2 админ меню | меню 3 меню ввода имени | меню 4 меню вопросов | меню 0 меню ошибка выхода
    int menu_input = 1;
    int menu_size = 2;
    int menu_quetion = 0;
    //работа со временем
    int timer = 0;
    //время с начала экзамена
    int timer_exam = 0;
    //запуск таймера после захода в вопросы выходить из вопросов нельзя
    bool timer_exam_start = false;
    //переменная для посчёта милессекунд
    double interval = 0;
    //переменная частоты необходима для подсчёта задержки
    LARGE_INTEGER frequency;
    //переменные начала и конца отчёта
    LARGE_INTEGER start, end;
    //ввод с клавиатуры системный
    std::string input;
    //пременные данных пользователя
    std::string name_admin;
    std::string name_user;
    //вектор с вопросами
    std::vector <quetion> ques;
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
                que.answer.push_back(arr.at(i));
                i++; // НЕ ЗАБЫВАЕМ ИНКРЕМЕНТ
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
    std::uniform_int_distribution<> dist_q(1, ques.size());
    QueryPerformanceFrequency(&frequency);
    while(run){
        //частота работы кода 
        QueryPerformanceCounter(&start);
        //системные никогда не стрираемые атрибуты
        WriteLineColor(hConsole, 10, 0, "Программа для прохождения письменного экзамена", FOREGROUND_RED);
        WriteLineColor(hConsole, 0, 20, "Управление: стрелки вверx и вниз для выбора параметров, Enter для выбора единичного или множественного, для выхода в главное меню используйте Esc, для полного выхода используйте CTRL+Q", FOREGROUND_RED | FOREGROUND_GREEN);
        //отрисовка меню 
        if(menu == 1){
            ClearPole(hConsole, 0, 2, 80, 15);
            WriteLineColor(hConsole, 10, 2, "Вписать своё имя и фамилию", menu_input == 1 ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
            WriteLineColor(hConsole, 10, 3, "Перейти к экзамену", menu_input == 2 ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
        }else if(menu == 0){
            ClearPole(hConsole, 0, 2, 80, 15);
            WriteLineColor(hConsole, 0, 19, "Вы не можожете выйти до тех пор пока не введёте корректно своё имя и фамилию или не прорешаете экзамен!!", FOREGROUND_RED);
        }else if(menu == 3){
            ClearPole(hConsole, 0, 2, 80, 15);
            WriteLineColor(hConsole, 0, 3, input , FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
            WriteLineColor(hConsole, 0, 19, "Введите своё имя и фамилию!!!", FOREGROUND_RED);
        }else if(menu == 4){

        }else if(menu == 2){
            ClearPole(hConsole, 0, 2, 80, 15);
            WriteLineColor(hConsole, 0, 3, input , FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
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
                    if(exit){
                        run = false;
                    }else{
                        menu = 0;
                    }
                }else if(keyCode == 'S'){
                    menu = 2;
                    input = "";
                }
            }
            if(unicodeChar != 0 && (menu == 2 || menu == 3) && unicodeChar != 13){
                input += unicodeChar;
            }
            if(keyCode == VK_RETURN){
                if(menu == 1){
                    if(menu_input == 1){
                        menu = 3;
                        input = "";
                    }else if(menu_input == 2){
                        menu = 4;
                    }
                }else if(menu == 2){
                    name_admin = input;
                    menu = 1;
                }else if(menu == 3){
                    name_user = input;
                    menu = 1;
                }else if(menu == 4){
                    ClearPole(hConsole, 0, 2, 80, 15);
                    int sid = dist_q(gen);
                    std::string que_value;
                    if(ques.at(sid).answer_status){
                        que_value = (ques.at(sid).answer_arr) ? "Введите верные ответы на вопрос." : "Введите верный ответ на вопрос.";
                    }else{
                        que_value = (ques.at(sid).answer_arr) ? "Введите неверные ответы на вопрос." : "Введите неверный ответ на вопрос.";
                    }
                    WriteLineColor(hConsole, 0, 3, ques.at(sid).title + que_value , FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
                    for(int i =0; i<ques.at(sid).answer.size(); i++){
                        WriteLineColor(hConsole, 0, i+4, std::to_string(i) + ") " + ques.at(sid).answer.at(i) , (menu_input == i+1) ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
                    }
                }
            }else if(keyCode == VK_ESCAPE){
                if(menu != 1 && menu != 4){
                    menu = 1;
                }
            }else if(keyCode == VK_UP){
                menu_input = (menu_input > 1) ? menu_input - 1 : menu_size;
            }else if(keyCode == VK_DOWN){
                menu_input = (menu_input < menu_size) ? menu_input + 1 : 1;
            }else if(keyCode == VK_BACK){
                input = input.substr(0, input.length()-2);
            }
        }  
        //вичисляем за сколько выполняется основной код для работы разных задержек
        QueryPerformanceCounter(&end);
        //получаем частоту в мс для нашего кода
        interval = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart * 1000;
    }
    return 0;
}