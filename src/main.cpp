#include "ATM.h"
#include <iostream>

int main() {
    try {
        ATM atm;
        atm.run();
    } catch (const std::exception& ex) {
        std::cerr << "An unexpected error occurred: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
