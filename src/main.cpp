/* 
 * File:   main.cpp
 * Author: Julius Higiro
 *
 * Created on January 9, 2018, 2:06 PM
 */

#include "ImageSearchAlgorithm.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::printf("[SPECIFY]: %s %s", argv[0], "<Mask image> <Search image>"
        " <Output image> <Percentage> <Tolerance>\n");
        return 1;
    }
    ImageSearchAlgorithm program(argv[1], argv[2], argv[3],
            (argc > 4 ? std::atoi(argv[4]) : 75),
            (argc > 5 ? std::atoi(argv[5]) : 32));
    program.search();
    program.print();

    return 0;
}

