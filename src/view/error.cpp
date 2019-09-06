//
// Created by samuel on 11/08/19.
//

#include "error.h"
#include <iostream>

void error_callback(int error, const char *description) {
	fputs(description, stderr);
}