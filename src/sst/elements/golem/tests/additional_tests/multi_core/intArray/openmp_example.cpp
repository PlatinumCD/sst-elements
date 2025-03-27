#include <omp.h>
#include <stdio.h>
#include <sched.h>
#include <algorithm>

#define NUM_THREADS 2

typedef int16_t in_type;
typedef int16_t out_type;

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

    int rows = 4;
    int cols = 4;

    in_type* mat_A = new in_type[rows * cols];
    in_type* mat_B = new in_type[rows * cols];

    in_type* vec_A = new in_type[cols];
    in_type* vec_B = new in_type[cols];

    out_type* out_A = new out_type[rows];
    out_type* out_B = new out_type[rows];

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat_A[i * cols + j] = 1;
            mat_B[i * cols + j] = 2;
        }
    }

    for (int i = 0; i < cols; i++) {
        vec_A[i] = 3;
        vec_B[i] = 4;
    }


    int status_flag;
    int tile_id = 0;
    
    omp_set_num_threads(NUM_THREADS);

    // Load Matrices
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        bind_thread_to_core(thread_id); // Bind thread to its respective core

        int core_id = sched_getcpu(); // Verify the core ID
        printf("Thread %d running on core %d\n", thread_id, core_id);

        if (thread_id == 0) {
            int core_id = sched_getcpu(); // Get the core ID the thread is running on
//            printf("Thread %d running on core %d\n", thread_id, core_id);

            // Load Matrix
            asm volatile (
                "mvm.set %0, %1, %2"
              : "=r" (status_flag)
              : "r"(mat_A), "r"(tile_id)
              : "memory"
            );

            #pragma omp barrier

        } else if (thread_id == 1) {
            int core_id = sched_getcpu(); // Get the core ID the thread is running on
//            printf("Thread %d running on core %d\n", thread_id, core_id);

            // Load Matrix
            asm volatile (
                "mvm.set %0, %1, %2"
              : "=r" (status_flag)
              : "r"(mat_B), "r"(tile_id)
              : "memory"
            );

            #pragma omp barrier
        }
    }

    int rounds = 3;

    // Ping-pong processing
    for (int round = 0; round < rounds; round++) {
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            bind_thread_to_core(thread_id); // Bind thread to its respective core

            int core_id = sched_getcpu(); // Verify the core ID
            printf("Thread %d running on core %d\n", thread_id, core_id);


            if (thread_id == 0) {
                int core_id = sched_getcpu(); // Get the core ID the thread is running on
//                printf("Thread %d running on core %d\n", thread_id, core_id);

                asm volatile (
                    "mvm.l %0, %1, %2"
                  : "=r" (status_flag)
                  : "r"(vec_A), "r"(tile_id)
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
                    : "r"(out_A), "r"(tile_id)
                    : "memory"
                );

                #pragma omp barrier


            } else if (thread_id == 1) {
                int core_id = sched_getcpu(); // Get the core ID the thread is running on
//                printf("Thread %d running on core %d\n", thread_id, core_id);

                asm volatile (
                    "mvm.l %0, %1, %2"
                  : "=r" (status_flag)
                  : "r"(vec_B), "r"(tile_id)
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
                    : "r"(out_B), "r"(tile_id)
                    : "memory"
                );

                #pragma omp barrier
            }

        }



        printf("out_A (round: %d): ", round);
        for (int i = 0; i < rows; i++) {
            printf("%d ", out_A[i]);
        }
        printf("\n");

        printf("out_B (round: %d): ", round);
        for (int i = 0; i < rows; i++) {
            printf("%d ", out_B[i]);
        }
        printf("\n\n");

        if (!(round == rounds - 1)) {
            std::swap(out_B, vec_A);
            std::swap(out_A, vec_B);
        }
    }

    printf("Final out_A: ");
    for (int i = 0; i < rows; i++) {
        printf("%d ", out_A[i]);
    }
    printf("\n");

    printf("Final out_B: ");
    for (int i = 0; i < rows; i++) {
        printf("%d ", out_B[i]);
    }
    printf("\n");


    return 0;
}
