//
// Created by samuel on 11/08/19.
//

#include "image.h"
#include <SOIL/SOIL.h>
#include <iostream>

img_rgb load_image(std::string image_file) {
    int width, height;
    unsigned char *img = SOIL_load_image(image_file.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    return {width, height, img};
}