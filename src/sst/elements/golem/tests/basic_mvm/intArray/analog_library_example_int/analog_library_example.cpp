// File: analog_library_example.cpp
#include <cstdint>
#include <cstdlib> // For malloc and free (if needed)
#include <iostream>
#include "analog.h"

typedef int8_t in_type;
typedef int32_t out_type;

int main() {
    // Dimensions for matrix and vector
    const int rows = 3;
    const int cols = 4;

    // Allocate memory for the matrix and vectors using new[]
    in_type* mat = new in_type[rows * cols];
    in_type* vec = new in_type[cols];
    out_type* out = new out_type[rows];

    // Fill the matrix
    printf("Matrix:\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat[i * cols + j] = 1;
            printf("%d ", mat[i * cols + j]);
        }
        printf("\n");
    }

    // Initialize vector
    printf("\nVector:\n");
    for (int i = 0; i < cols; i++) {
        vec[i] = 2;
        printf("%d ", vec[i]);
    }
    printf("\n\n");

    // Specify number of arrays for AnalogContext
    const int num_arrays = 1;
    AnalogContext ctx(num_arrays);

    // Create analog matrix and vectors using the 1D matrix and 1D vector arrays
    AnalogMatrix<in_type> analog_mat(mat, rows, cols);
    AnalogVector<in_type> analog_vec(vec, cols);
    AnalogVector<out_type> analog_vec_out(out, rows); 

    // Set matrix in the analog context and load the vector
    const int array_id = 0;
    mvm_set_matrix(ctx, analog_mat, array_id);
    mvm_load_vector(ctx, analog_vec, array_id);

    // Print the matrix and vector
    analog_mat.print();
    analog_vec.print();

    // Perform matrix-vector multiplication
    mvm_compute(ctx, array_id);

    // Store the resulting vector
    mvm_store_vector(ctx, analog_vec_out, array_id);

    printf("Output Vector:\n");
    for (int i = 0; i < rows; i++) {
        printf("%d ", out[i]);
    }
    printf("\n\n");

    // Print the result
    analog_vec_out.print();

    // Clean up dynamically allocated memory
    delete[] mat;
    delete[] vec;
    delete[] out;

    return 0;
}
