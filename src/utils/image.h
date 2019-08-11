//
// Created by samuel on 11/08/19.
//

#ifndef EVOMOTION_IMAGE_H
#define EVOMOTION_IMAGE_H

#include <string>

struct img_rgb {
    int width;
    int height;
    unsigned char *colors;
};

img_rgb load_image(std::string image_file);

#endif //EVOMOTION_IMAGE_H
