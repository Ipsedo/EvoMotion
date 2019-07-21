#include <iostream>
#include "tests/vector_test.h"
#include "tests/coco_test.h"
#include "tests/matrix_test.h"
#include "tests/arma_test.h"

int main() {

    /*tests_vector_operator();
    test_matrix();
    test_armadillo();*/
    //test_armadillo();
    example_experiment("bbob", "dimensions: 2,3,5,10", "bbob", "result_folder: CMA_on_bbob");
    return 0;
}