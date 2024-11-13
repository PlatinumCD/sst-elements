// File: analog_library_example.cpp
#include <cstdint>
#include <cstdlib> // For malloc and free (if needed)
#include <iostream>
#include "analog.h"

int main() {
    // Dimensions for matrix and vector
    const int rows = 3;
    const int cols = 4;

    // Allocate memory for the matrix and vectors using new[]
    float* mat = new float[rows * cols];
    float* vec = new float[cols];
    float* out = new float[rows];

    // Fill the matrix
    printf("Matrix:\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat[i * cols + j] = 1.0;
            printf("%.1f ", mat[i * cols + j]);
        }
        printf("\n");
    }

    // Initialize vector
    printf("\nVector:\n");
    for (int i = 0; i < cols; i++) {
        vec[i] = 2.0f;
        printf("%.1f ", vec[i]);
    }
    printf("\n\n");

    // Specify number of arrays for AnalogContext
    const int num_arrays = 1;
    AnalogContext ctx(num_arrays);

    // Create analog matrix and vectors using the 1D matrix and 1D vector arrays
    AnalogMatrix<float, int8_t> analog_mat(mat, rows, cols);
    AnalogVector<float, int8_t> analog_vec(vec, cols);
    AnalogVector<float, int32_t> analog_vec_out(out, rows); 

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
        printf("%.1f ", out[i]);
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
