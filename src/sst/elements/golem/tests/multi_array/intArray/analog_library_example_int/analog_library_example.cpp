// File: analog_library_example.cpp
#include <cstdint>
#include <cstdlib> // For malloc and free (if needed)
#include <iostream>
#include "analog.h"

typedef int8_t in_type;
typedef int32_t out_type;

void fill_array(in_type* arr, in_type value, uint32_t size) {
    for (int i = 0; i < size; i++) {
        arr[i] = value;
    }
}

void print_matrix(in_type* mat, uint32_t rows, uint32_t cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", mat[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void print_vector(in_type* vec, uint32_t cols) {
    for (int i = 0; i < cols; i++) {
        printf("%d ", vec[i]);
    }
    printf("\n\n");
}

void print_vector(out_type* vec, uint32_t cols) {
    for (int i = 0; i < cols; i++) {
        printf("%d ", vec[i]);
    }
    printf("\n\n");
}

int main() {
    // Dimensions for matrix and vector
    int rows = 5;
    int cols = 5;
    int num_ping_pongs = 2;

    in_type* matA = new in_type[rows * cols];
    in_type* vecA = new in_type[cols];
    out_type* outA = new out_type[rows];

    in_type* matB = new in_type[rows * cols];
    in_type* vecB = new in_type[cols];
    out_type* outB = new out_type[rows];


    // Fill the matrices and vectors
    fill_array(matA, 3, rows*cols);
    fill_array(matB, 2, rows*cols);
    fill_array(vecA, 1, cols);
    fill_array(vecB, 4, cols);

    // Specify number of arrays for AnalogContext
    const int num_arrays = 2;
    AnalogContext ctx(num_arrays);

    // Create analog matrix and vectors using the 1D matrix and 1D vector arrays
    AnalogMatrix<in_type> analog_matA(matA, rows, cols);
    AnalogVector<in_type> analog_vecA(vecA, cols);
    AnalogVector<out_type> analog_vecA_out(outA, rows); 

    AnalogMatrix<in_type> analog_matB(matB, rows, cols);
    AnalogVector<in_type> analog_vecB(vecB, cols);
    AnalogVector<out_type> analog_vecB_out(outB, rows); 

    // Set matrix in the analog context and load the vector
    mvm_set_matrix(ctx, analog_matA, 0);
    mvm_load_vector(ctx, analog_vecA, 0);

    mvm_set_matrix(ctx, analog_matB, 1);
    mvm_load_vector(ctx, analog_vecB, 1);

    // Print the matrix and vector
    analog_matA.print();
    analog_vecA.print();

    analog_matB.print();
    analog_vecB.print();

    // Perform matrix-vector multiplication
    mvm_compute(ctx, 0);
    mvm_compute(ctx, 1);

    for (int i = 0; i < num_ping_pongs; i++) {
        mvm_move_vector(ctx, 0, 1);
        mvm_move_vector(ctx, 1, 0);

        mvm_compute(ctx, 0);
        mvm_compute(ctx, 1);
    }

    mvm_store_vector(ctx, analog_vecA_out, 0);
    mvm_store_vector(ctx, analog_vecB_out, 1);

    print_vector(outA, cols);
    print_vector(outB, cols);

    // Clean up dynamically allocated memory
    delete[] matA;
    delete[] vecA;
    delete[] outA;

    delete[] matB;
    delete[] vecB;
    delete[] outB;


    return 0;
}
