#include <iostream>
#include <cstring>
#include "tests/bullet_test.h"
#include "tests/opengl_test.h"
#include "tests/rl_test.h"

int main(int argc, char *argv[]) {

	auto err_msg = "Need specify test case :\n"
	               "- bullet\n"
	               "- opengl\n"
	               "- rl\n"
	               "Example (launch COCO test) : ./build/EvoMotion coco\n";

	if (argc <= 1) {
		std::cout << err_msg;
		return 1;
	}

	auto arg = std::string(argv[1]);

	if (arg == "bullet") test_bullet();
	else if (arg == "opengl") test_opengl();
	else if (arg == "rl") test_reinforcement_learning();
	else {
		std::cout << "Unrecognized arg : " << arg << std::endl;
		std::cout << err_msg;
		return 1;
	}

	return 0;
}