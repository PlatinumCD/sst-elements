#include <omp.h>
#include <stdio.h>
#include <sched.h>
#include <algorithm>
#include <stdlib.h>
#include <stdint.h>

#define NUM_THREADS 3   // Now using 3 threads: core 0 builds data, cores 1 & 2 do ping-pong

typedef int32_t in_type;
typedef int32_t out_type;

void bind_thread_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
        perror("sched_setaffinity failed");
        exit(EXIT_FAILURE);
    }
}

int main() {
    // Dimensions
    int rows = 4, cols = 4;

    // Allocate
    in_type* mat_A0 = new in_type[rows * cols];
    in_type* mat_A1 = new in_type[rows * cols];
    in_type* mat_B0 = new in_type[rows * cols];
    in_type* mat_B1 = new in_type[rows * cols];

    in_type* vec_A0 = new in_type[cols];
    in_type* vec_A1 = new in_type[cols];
    in_type* vec_B0 = new in_type[cols];
    in_type* vec_B1 = new in_type[cols];

    out_type* out_A0 = new out_type[rows];
    out_type* out_A1 = new out_type[rows];
    out_type* out_B0 = new out_type[rows];
    out_type* out_B1 = new out_type[rows];

    // Core 0 (thread 0) creates/initializes data (sequentially, no parallel needed)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat_A0[i * cols + j] = 1;
            mat_A1[i * cols + j] = 2;
            mat_B0[i * cols + j] = 3;
            mat_B1[i * cols + j] = 4;
        }
    }
    for (int i = 0; i < cols; i++) {
        vec_A0[i] = 1;  vec_A1[i] = 2;
        vec_B0[i] = 3;  vec_B1[i] = 4;
    }

    // Prepare for parallel
    omp_set_num_threads(NUM_THREADS);
    int status_flag;

    // Load matrices onto cores 1 & 2 only
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        bind_thread_to_core(thread_id);
        if (thread_id == 1) {
            // Load Matrix on "core 1"
            asm volatile ("mvm.set %0, %1, %2" : "=r" (status_flag) : "r"(mat_A0), "r"(0) : "memory");
            asm volatile ("mvm.set %0, %1, %2" : "=r" (status_flag) : "r"(mat_A1), "r"(1) : "memory");
        }
        else if (thread_id == 2) {
            // Load Matrix on "core 2"
            asm volatile ("mvm.set %0, %1, %2" : "=r" (status_flag) : "r"(mat_B0), "r"(0) : "memory");
            asm volatile ("mvm.set %0, %1, %2" : "=r" (status_flag) : "r"(mat_B1), "r"(1) : "memory");
        }
        // Thread 0 does nothing here, just waits
        #pragma omp barrier
    }

    int rounds = 2, mini_rounds = 2;

    // Ping-Pong between cores 1 & 2
    for (int round = 0; round < rounds; round++) {
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            bind_thread_to_core(thread_id);

            if (thread_id == 1) {
                // Load vector on core 1
                asm volatile ("mvm.l %0, %1, %2" : "=r"(status_flag) : "r"(vec_A0), "r"(0) : "memory");
                asm volatile ("mvm.l %0, %1, %2" : "=r"(status_flag) : "r"(vec_A1), "r"(1) : "memory");

                for (int mini_round = 0; mini_round < mini_rounds; mini_round++) {
                    asm volatile ("mvm %0, %1, x0" : "=r"(status_flag) : "r"(0));
                    asm volatile ("mvm %0, %1, x0" : "=r"(status_flag) : "r"(1));
                    asm volatile ("mvm.mv %0, %1, %2" : "=r"(status_flag) : "r"(0), "r"(1));
                    asm volatile ("mvm.mv %0, %1, %2" : "=r"(status_flag) : "r"(1), "r"(0));
                }
                // Store results
                asm volatile ("mvm.s %0, %1, %2" : "=r"(status_flag) : "r"(out_A0), "r"(0) : "memory");
                asm volatile ("mvm.s %0, %1, %2" : "=r"(status_flag) : "r"(out_A1), "r"(1) : "memory");
            }
            else if (thread_id == 2) {
                // Load vector on core 2
                asm volatile ("mvm.l %0, %1, %2" : "=r"(status_flag) : "r"(vec_B0), "r"(0) : "memory");
                asm volatile ("mvm.l %0, %1, %2" : "=r"(status_flag) : "r"(vec_B1), "r"(1) : "memory");

                for (int mini_round = 0; mini_round < mini_rounds; mini_round++) {
                    asm volatile ("mvm %0, %1, x0" : "=r"(status_flag) : "r"(0));
                    asm volatile ("mvm %0, %1, x0" : "=r"(status_flag) : "r"(1));
                    asm volatile ("mvm.mv %0, %1, %2" : "=r"(status_flag) : "r"(0), "r"(1));
                    asm volatile ("mvm.mv %0, %1, %2" : "=r"(status_flag) : "r"(1), "r"(0));
                }
                // Store results
                asm volatile ("mvm.s %0, %1, %2" : "=r"(status_flag) : "r"(out_B0), "r"(0) : "memory");
                asm volatile ("mvm.s %0, %1, %2" : "=r"(status_flag) : "r"(out_B1), "r"(1) : "memory");
            }
            // Thread 0 idle
            #pragma omp barrier
        }

        // Print partial results
        printf("out_A0 (round %d): ", round);
        for (int i = 0; i < rows; i++) printf("%d ", out_A0[i]);
        printf("\n");

        printf("out_A1 (round %d): ", round);
        for (int i = 0; i < rows; i++) printf("%d ", out_A1[i]);
        printf("\n");

        printf("out_B0 (round %d): ", round);
        for (int i = 0; i < rows; i++) printf("%d ", out_B0[i]);
        printf("\n");

        printf("out_B1 (round %d): ", round);
        for (int i = 0; i < rows; i++) printf("%d ", out_B1[i]);
        printf("\n\n");

        // Swap to feed outputs back in as inputs for next round (ping-pong effect)
        if (round != rounds - 1) {
            std::swap(out_A0, vec_B0);
            std::swap(out_A1, vec_B1);
            std::swap(out_B0, vec_A0);
            std::swap(out_B1, vec_A1);
        }
    }

    // Final results
    printf("Final out_A0: "); for (int i = 0; i < rows; i++) printf("%d ", out_A0[i]);  printf("\n");
    printf("Final out_A1: "); for (int i = 0; i < rows; i++) printf("%d ", out_A1[i]);  printf("\n");
    printf("Final out_B0: "); for (int i = 0; i < rows; i++) printf("%d ", out_B0[i]);  printf("\n");
    printf("Final out_B1: "); for (int i = 0; i < rows; i++) printf("%d ", out_B1[i]);  printf("\n");

    // Cleanup
    delete[] mat_A0;  delete[] mat_A1;  delete[] mat_B0;  delete[] mat_B1;
    delete[] vec_A0;  delete[] vec_A1;  delete[] vec_B0;  delete[] vec_B1;
    delete[] out_A0;  delete[] out_A1;  delete[] out_B0;  delete[] out_B1;

    return 0;
}
