#include <iostream>
#include <cstring>
#include "tests/arma_test.h"
#include "tests/coco_test.h"
#include "tests/bullet_test.h"

int main(int argc, char *argv[]) {
    // Test program args
    if (argc <= 1) {
        std::cout << "Need specify test case :" << std::endl;
        std::cout << "- coco" << std::endl << "- bullet" << std::endl << "- armadillo" << std::endl;
        std::cout << "Example (launch COCO test) : ./build/EvoMotion coco" << std::endl;
        return 1;
    }

    auto arg = std::string(argv[1]);

    if (arg == "coco" ) test_coco("bbob", "dimensions: 2,3,5,10,20", "bbob", "result_folder: CMA_on_bbob");
    else if (arg == "armadillo") test_armadillo();
    else if (arg == "bullet") test_bullet();

    return 0;
}