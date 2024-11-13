#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstdint>

typedef int8_t in_type;
typedef int16_t out_type;

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

void setup_array(in_type *mat, in_type* vec, uint32_t tile_id) {
    int status_flag = 0;

    asm volatile (
        "mvm.set %0, %1, %2"
      : "=r" (status_flag)
      : "r"(mat), "r"(tile_id)
      : "memory"
    );

    asm volatile (
        "mvm.l %0, %1, %2"
      : "=r" (status_flag)
      : "r"(vec), "r"(tile_id)
      : "memory"
    );
}

void execute_mvm(uint32_t tile_id) {
    int status_flag = 0;

    asm volatile (
        "mvm %0, %1, x0"
        : "=r" (status_flag)
        : "r"(tile_id)
    );
}

void store_vector(out_type* out, uint32_t tile_id) { 
    int status_flag = 0;

    asm volatile (
        "mvm.s %0, %1, %2"
        : "=r" (status_flag)
        : "r"(out), "r"(tile_id)
        : "memory"
    );
}

void move_vector(uint32_t tile_id, uint32_t tile_id_new) {
    int status_flag = 0;

    asm volatile (
        "mvm.mv %0, %1, %2"
        : "=r" (status_flag)
        : "r"(tile_id), "r"(tile_id_new)
        : "memory"
    );
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

    // Print matrices and vectors
    printf("Matrix A:\n");
    print_matrix(matA, rows, cols);

    printf("Matrix B:\n");
    print_matrix(matB, rows, cols);

    printf("Vector A:\n");
    print_vector(vecA, cols);

    printf("Vector B:\n");
    print_vector(vecB, cols);

    // Set up array
    setup_array(matA, vecA, 0);
    setup_array(matB, vecB, 1);

    // Run mvm
    execute_mvm(0);
    execute_mvm(1);

    for (int i = 0; i < num_ping_pongs; i++) { 

        // Move vectors
        move_vector(1, 0);
        move_vector(0, 1);

        // Run mvm
        execute_mvm(0);
        execute_mvm(1);
    }

    // Store vector
    store_vector(outA, 0);
    store_vector(outB, 1);

    // Print output vector
    printf("Output Vector A:\n");
    print_vector(outA, cols);
    
    printf("Output Vector B:\n");
    print_vector(outB, cols);

    // Free allocated memory
    free(matA);
    free(vecA);
    free(outA);

    free(matB);
    free(vecB);
    free(outB);
}
