//
// Created by samuel on 19/07/19.
//
// From COCO (https://github.com/numbbo/coco) random search example

#include <cstdio>
#include "../algos/evolution/CMA_ES.h"
#include "coco_test.h"


static coco_problem_t *PROBLEM;

static timing_data_t *timing_data_initialize(coco_suite_t *suite);
static void timing_data_time_problem(timing_data_t *timing_data, coco_problem_t *problem);
static void timing_data_finalize(timing_data_t *timing_data);

static const long INDEPENDENT_RESTARTS = 1e5;

static const unsigned int BUDGET_MULTIPLIER = 2;

void test_coco(const char *suite_name,
               const char *suite_options,
               const char *observer_name,
               const char *observer_options) {

    coco_random_state_t *random_generator = coco_random_new(12345);

    /* Change the log level to "warning" to get less output */
    coco_set_log_level("info");

    size_t run;
    coco_suite_t *suite;
    coco_observer_t *observer;
    timing_data_t *timing_data;

    /* Initialize the suite and observer. */
    suite = coco_suite(suite_name, "", suite_options);
    observer = coco_observer(observer_name, observer_options);

    /* Initialize timing */
    timing_data = timing_data_initialize(suite);

    /* Iterate over all problems in the suite */
    while ((PROBLEM = coco_suite_get_next_problem(suite, observer)) != NULL) {

        size_t dimension = coco_problem_get_dimension(PROBLEM);

        /* Run the algorithm at least once */
        for (run = 1; run <= 1 + INDEPENDENT_RESTARTS; run++) {

            long evaluations_done = (long) (coco_problem_get_evaluations(PROBLEM) +
                                            coco_problem_get_evaluations_constraints(PROBLEM));
            long evaluations_remaining = (long) (dimension * BUDGET_MULTIPLIER) - evaluations_done;

            /* Break the loop if the target was hit or there are no more remaining evaluations */
            if ((coco_problem_final_target_hit(PROBLEM) &&
                 coco_problem_get_number_of_constraints(PROBLEM) == 0)
                || (evaluations_remaining <= 0))
                break;

            /* Call the optimization algorithm for the remaining number of evaluations */
            CMA_ES cma = CMA_ES(PROBLEM);
            for (int i = 0; i < 200 && cma.step(); i++) {}


            /* Break the loop if the algorithm performed no evaluations or an unexpected thing happened */
            if (coco_problem_get_evaluations(PROBLEM) == evaluations_done) {
                printf("WARNING: Budget has not been exhausted (%lu/%lu evaluations done)!\n",
                       (unsigned long) evaluations_done, (unsigned long) dimension * BUDGET_MULTIPLIER);
                break;
            }
            else if (coco_problem_get_evaluations(PROBLEM) < evaluations_done)
                coco_error("Something unexpected happened - function evaluations were decreased!");
        }

        /* Keep track of time */
        timing_data_time_problem(timing_data, PROBLEM);
    }

    /* Output and finalize the timing data */
    timing_data_finalize(timing_data);

    coco_observer_free(observer);
    coco_suite_free(suite);

}

/**
 * Allocates memory for the timing_data_t object and initializes it.
 */
static timing_data_t *timing_data_initialize(coco_suite_t *suite) {

    timing_data_t *timing_data = (timing_data_t *) coco_allocate_memory(sizeof(*timing_data));
    size_t function_idx, dimension_idx, instance_idx, i;

    /* Find out the number of all dimensions */
    coco_suite_decode_problem_index(suite, coco_suite_get_number_of_problems(suite) - 1, &function_idx,
                                    &dimension_idx, &instance_idx);
    timing_data->number_of_dimensions = dimension_idx + 1;
    timing_data->current_idx = 0;
    timing_data->output = (char **) coco_allocate_memory(timing_data->number_of_dimensions * sizeof(char *));
    for (i = 0; i < timing_data->number_of_dimensions; i++) {
        timing_data->output[i] = NULL;
    }
    timing_data->previous_dimension = 0;
    timing_data->cumulative_evaluations = 0;
    time(&timing_data->start_time);
    time(&timing_data->overall_start_time);

    return timing_data;
}

/**
 * Keeps track of the total number of evaluations and elapsed time. Produces an output string when the
 * current problem is of a different dimension than the previous one or when NULL.
 */
static void timing_data_time_problem(timing_data_t *timing_data, coco_problem_t *problem) {

    double elapsed_seconds = 0;

    if ((problem == NULL) || (timing_data->previous_dimension != coco_problem_get_dimension(problem))) {

        /* Output existing timing information */
        if (timing_data->cumulative_evaluations > 0) {
            time_t now;
            time(&now);
            elapsed_seconds = difftime(now, timing_data->start_time) / (double) timing_data->cumulative_evaluations;
            timing_data->output[timing_data->current_idx++] = coco_strdupf("d=%lu done in %.2e seconds/evaluation\n",
                                                                           timing_data->previous_dimension, elapsed_seconds);
        }

        if (problem != NULL) {
            /* Re-initialize the timing_data */
            timing_data->previous_dimension = coco_problem_get_dimension(problem);
            timing_data->cumulative_evaluations = coco_problem_get_evaluations(problem);
            time(&timing_data->start_time);
        }

    } else {
        timing_data->cumulative_evaluations += coco_problem_get_evaluations(problem);
    }
}

/**
 * Outputs and finalizes the given timing data.
 */
static void timing_data_finalize(timing_data_t *timing_data) {

    /* Record the last problem */
    timing_data_time_problem(timing_data, NULL);

    if (timing_data) {
        size_t i;
        double elapsed_seconds;
        time_t now;
        int hours, minutes, seconds;

        time(&now);
        elapsed_seconds = difftime(now, timing_data->overall_start_time);

        printf("\n");
        for (i = 0; i < timing_data->number_of_dimensions; i++) {
            if (timing_data->output[i]) {
                printf("%s", timing_data->output[i]);
                coco_free_memory(timing_data->output[i]);
            }
        }
        hours = (int) elapsed_seconds / 3600;
        minutes = ((int) elapsed_seconds % 3600) / 60;
        seconds = (int)elapsed_seconds - (hours * 3600) - (minutes * 60);
        printf("Total elapsed time: %dh%02dm%02ds\n", hours, minutes, seconds);

        coco_free_memory(timing_data->output);
        coco_free_memory(timing_data);
    }
}