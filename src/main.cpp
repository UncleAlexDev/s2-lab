/**
  \file   main.cpp
  \brief  Визуализация данных по протокоам Атлас, APC, ...
  \author Aleksey Lekhtin
  \date   27.11.2024
*/

#include "mw.h"
#include "tr_TR.h"

#include <QApplication>

// #ifdef _WIN32
// #include <windows.h>
// #endif

//static void SetupConsoleOutput();

int main(int argc, char *argv[])
{
    //SetupConsoleOutput();
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":app_logo.png"));
    TRANSLATIONS tr;
    MW mainwindow;
    mainwindow.show();
    const int rc = QApplication::exec();
    return rc;
}

#if 0
//---------------------------------------------------------
/// Спец - дрочево для Windows \n
/// Функция для привязки потоков ввода-вывода к консоли
static void SetupConsoleOutput() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD fileType = GetFileType(hOut);

    // Если мы внутри CLion (Run или Debug)
    if (fileType == FILE_TYPE_PIPE) {
        SetConsoleOutputCP(CP_UTF8);
        std::setvbuf(stdout, nullptr, _IONBF, 0);
        std::setvbuf(stderr, nullptr, _IONBF, 0);

        // --- УВЕЛИЧИВАЕМ ШИРИНУ ВЫВОДА В CLION ---
        // Получаем дескриптор скрытой консоли winpty, которую создал CLion
        HANDLE hPtyOut = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        if (hPtyOut != INVALID_HANDLE_VALUE) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(hPtyOut, &csbi)) {
                // Оставляем высоту прежней, а ширину жестко ставим в 500 символов (вместо 120)
                COORD newSize;
                newSize.X = 500;
                newSize.Y = csbi.dwSize.Y;
                SetConsoleScreenBufferSize(hPtyOut, newSize);
            }
            CloseHandle(hPtyOut);
        }
        // -----------------------------------------
        return;
    }

    // Обычный запуск из cmd / powershell
    if (AttachConsole(static_cast<unsigned long>(-1))) {
        std::freopen("CONOUT$", "w", stdout);
        std::freopen("CONOUT$", "w", stderr);
        std::freopen("CONIN$", "r", stdin);
        std::ios_base::sync_with_stdio();
        SetConsoleOutputCP(CP_UTF8);
    }
#endif
}

#endif