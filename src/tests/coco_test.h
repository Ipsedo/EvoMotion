//
// Created by samuel on 19/07/19.
//
// From COCO (https://github.com/numbbo/coco) random search example

#ifndef EVOMOTION_COCO_TEST_H
#define EVOMOTION_COCO_TEST_H


#include <time.h>
#include "../coco.h"

typedef struct {
	size_t number_of_dimensions;
	size_t current_idx;
	char **output;
	size_t previous_dimension;
	size_t cumulative_evaluations;
	time_t start_time;
	time_t overall_start_time;
} timing_data_t;


void test_coco(const char *suite_name,
               const char *suite_options,
               const char *observer_name,
               const char *observer_options);

#endif //EVOMOTION_COCO_TEST_H
