#include "albcexample.h"
extern "C"
{
#include "c_albcexample.h"
}

#include <iostream>

int main(int argc, char *argv[])
{
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