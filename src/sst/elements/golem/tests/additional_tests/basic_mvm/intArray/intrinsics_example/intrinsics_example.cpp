#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

typedef int8_t in_type;
typedef int16_t out_type;

int main() {
    // Dimensions for matrix and vector
    int rows = 16;
    int cols = 3;

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

    int status_flag;
    int tile_id = 0;

    // Perform MVM
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

    asm volatile (
        "mvm %0, %1, x0"
        : "=r" (status_flag)
        : "r"(tile_id)
    );

    asm volatile (
        "mvm.s %0, %1, %2"
        : "=r" (status_flag)
        : "r"(out), "r"(tile_id)
        : "memory"
    );

    printf("\n\nOutput Vector:\n");
    for (int i = 0; i < rows; i++) {
        printf("%d ", out[i]);
    }

    // Free allocated memory
    free(mat);
    free(vec);
    free(out);
}
