#include <windows.h>
#include <iostream>
#include <string>
void WriteLineColor(HANDLE hConsloe, int x, int y, std::wstring  line, WORD color){
    COORD pos = {(SHORT) x, (SHORT) y};
    DWORD written;    
    WriteConsoleOutputCharacterW(hConsloe, line.c_str(), (DWORD)line.length(), pos, &written);
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
    FillConsoleOutputCharacterW(hConsole, L' ', count, pos, &written);
    
    // Стираем цвета (ставим белый текст на черном фоне)
    WORD defaultAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    FillConsoleOutputAttribute(hConsole, defaultAttr, count, pos, &written);
}
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

    //переменные работы с меню
    int menu = 1;//меню 1 главное меню | меню 2 админ меню | меню 3 меню ввода имени | меню 4 меню вопросов | меню 0 меню ошибка выхода
    int menu_input = 1;
    int menu_size = 2;
    int menu_quetion = 0;
    //работа со временем
    int timer = 0;
    int timer_exam = 0;
    //ввод с клавиатуры системный
    std::string input;

    //пременные данных пользователя
    std::string name_admin;
    std::string name_user;
    //получение разамеров текущей консоли
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return 0;
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    //  залитие всего простарнства пробелами
    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten)) return 0;
    // сброс всех аттрибутов 
    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten)) return 0;
    while(run){
        //системные никогда не стрираемые атрибуты
        WriteLineColor(hConsole, 10, 0, L"Программа для прохождения письменного экзамена", FOREGROUND_RED);
        WriteLineColor(hConsole, 0, 20, L"Управление: стрелки вверx и вниз для выбора параметров, Enter для выбора единичного или множественного, для выхода в главное меню используйте Esc, для полного выхода используйте CTRL+Q", FOREGROUND_RED | FOREGROUND_GREEN);
        //отрисовка меню 
        if(menu == 1){
            WriteLineColor(hConsole, 10, 2, L"Вписать своё имя и фамилию", menu_input == 1 ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
            WriteLineColor(hConsole, 10, 3, L"Перейти к экзамену", menu_input == 2 ? FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | BACKGROUND_BLUE : FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN);
        }else if(menu == 0){
            ClearPole(hConsole, 0, 2, 80, 15);
            WriteLineColor(hConsole, 0, 19, L"Вы не можожете выйти до тех пор пока не введёте корректно своё имя и фамилию или не прорешаете экзамен!!", FOREGROUND_RED);
        }else if(menu == 3){
            ClearPole(hConsole, 0, 2, 80, 15);
        }else if(menu == 4){
            ClearPole(hConsole, 0, 2, 80, 15);
        }else if(menu == 2){
            ClearPole(hConsole, 0, 2, 80, 15);
        }
        //ввод в консоль
        ReadConsoleInput(hInput, &ir, 1, &eventsRead);
        //нажатие
        if(ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown){
            DWORD modifiers  = ir.Event.KeyEvent.dwControlKeyState;
            WORD keyCode = ir.Event.KeyEvent.wVirtualKeyCode;
            //сочетание клавиш ctrl+q ctrl+s
            bool ctrlPress = false;
            if((modifiers & LEFT_CTRL_PRESSED) || (modifiers & RIGHT_CTRL_PRESSED)){
                ctrlPress = true;
            }else{
                ctrlPress = false;
            }
            if(keyCode == VK_RETURN){
                if(menu == 1){
                    if(menu_input == 1){
                        menu = 3;
                    }else if(menu_input == 2){
                        menu = 4;
                    }
                }else if(menu == 2){
                    name_admin = input;
                }
            }else if(keyCode == VK_ESCAPE){
                if(menu != 1){
                    menu = 1;
                }
            }else if(keyCode == VK_UP){
                menu_input = (menu_input > 1) ? menu_input - 1 : menu_size;
            }else if(keyCode == VK_DOWN){
                menu_input = (menu_input < menu_size) ? menu_input + 1 : 1;
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
                    WriteLineColor(hConsole, 0, 19, L"Введите имя студента которого вы туда посадили!!", FOREGROUND_RED);
                }
            }

        }
    }
    return 0;
}