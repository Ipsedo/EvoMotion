#include <iostream>
#include <cstring>
#include "tests/arma_test.h"
#include "tests/coco_test.h"
#include "tests/bullet_test.h"
#include "tests/opengl_test.h"

int main(int argc, char *argv[]) {

    auto err_msg = "Need specify test case :\n"
                   "- coco\n"
                   "- bullet\n"
                   "- armadillo\n"
                   "- opengl\n"
                   "Example (launch COCO test) : ./build/EvoMotion coco\n";

    if (argc <= 1) {
        std::cout << err_msg;
        return 1;
    }

    auto arg = std::string(argv[1]);

    if (arg == "coco" ) test_coco("bbob", "dimensions: 2,3,5,10,20", "bbob", "result_folder: CMA_on_bbob");
    else if (arg == "armadillo") test_armadillo();
    else if (arg == "bullet") test_bullet();
    else if (arg == "opengl") test_opengl();
    else {
        std::cout << "Unrecognized arg : " << arg << std::endl;
        std::cout << err_msg;
        return 1;
    }

    return 0;
}