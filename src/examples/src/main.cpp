#include "albcexample.h"
extern "C"
{
#include "c_albcexample.h"
}

#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#undef ERROR
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout <<
        "Enter a number: \n"
        "1: C++\n"
        "2: C\n"
        "3: Exit\n"
        ">";

    int choice;
    std::cin >> choice;
    switch (choice)
    {
        case 1:
            albc_example_main();
            break;
        case 2:
            c_albc_example_main();
            break;
        case 3:
            return 0;
        default:
            std::cout << "Invalid choice\n";
            break;
    }
}