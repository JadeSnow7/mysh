#include "core/shell.h"
#include <iostream>

int main(int argc, char** argv)
{
    try {
        Shell shell;
        return shell.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
