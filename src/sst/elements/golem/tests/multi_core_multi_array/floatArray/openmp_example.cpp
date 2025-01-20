#include <omp.h>
#include <stdio.h>
#include <sched.h>
#include <algorithm>

#define NUM_THREADS 2

typedef float in_type;
typedef float out_type;

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

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mat_A0[i * cols + j] = 1;
            mat_A1[i * cols + j] = 2;
            mat_B0[i * cols + j] = 3;
            mat_B1[i * cols + j] = 4;
        }
    }

    for (int i = 0; i < cols; i++) {
        vec_A0[i] = 1;
        vec_A1[i] = 2;
        vec_B0[i] = 3;
        vec_B1[i] = 4;

    }

    int status_flag;
    int tile_id;
    
    omp_set_num_threads(NUM_THREADS);

    // Load Matrices
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        bind_thread_to_core(thread_id); // Bind thread to its respective core

        int core_id = sched_getcpu(); // Verify the core ID
        printf("Thread %d running on core %d\n", thread_id, core_id);

        if (thread_id == 0) {

            // Load Matrix core 0, array 0
            asm volatile (
                "mvm.set %0, %1, %2"
              : "=r" (status_flag)
              : "r"(mat_A0), "r"(0)
              : "memory"
            );

            // Load Matrix core 0, array 1
            asm volatile (
                "mvm.set %0, %1, %2"
              : "=r" (status_flag)
              : "r"(mat_A1), "r"(1)
              : "memory"
            );

            #pragma omp barrier

        } else if (thread_id == 1) {

            // Load Matrix core 1, array 0
            asm volatile (
                "mvm.set %0, %1, %2"
              : "=r" (status_flag)
              : "r"(mat_B0), "r"(0)
              : "memory"
            );

            // Load Matrix core 1, array 1
            asm volatile (
                "mvm.set %0, %1, %2"
              : "=r" (status_flag)
              : "r"(mat_B1), "r"(1)
              : "memory"
            );

            #pragma omp barrier
        }
    }

    int rounds = 2;
    int mini_rounds = 2;

    // Ping-pong processing
    for (int round = 0; round < rounds; round++) {
        #pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            bind_thread_to_core(thread_id); // Bind thread to its respective core

            int core_id = sched_getcpu(); // Verify the core ID
            printf("Thread %d running on core %d\n", thread_id, core_id);

            if (thread_id == 0) {
//                printf("Thread %d running on core %d\n", thread_id, core_id);

                // Load vector core 0, array 0
                asm volatile (
                    "mvm.l %0, %1, %2"
                  : "=r" (status_flag)
                  : "r"(vec_A0), "r"(0)
                  : "memory"
                );

                // Load vector core 0, array 1
                asm volatile (
                    "mvm.l %0, %1, %2"
                  : "=r" (status_flag)
                  : "r"(vec_A1), "r"(1)
                  : "memory"
                );

                for (int mini_round = 0; mini_round < mini_rounds; mini_round++) {

                    // mvm on vector core 0, array 0
                    asm volatile (
                        "mvm %0, %1, x0"
                        : "=r" (status_flag)
                        : "r"(0)
                    );

                    // mvm on vector core 0, array 1
                    asm volatile (
                        "mvm %0, %1, x0"
                        : "=r" (status_flag)
                        : "r"(1)
                    );

                    // move output vector core 0, array 0 to core 0, array 1
                    asm volatile (
                        "mvm.mv %0, %1, %2"
                      : "=r" (status_flag)
                      : "r"(0), "r"(1)
                    );

                    // move output vector core 0, array 1 to core 0, array 0
                    asm volatile (
                        "mvm.mv %0, %1, %2"
                      : "=r" (status_flag)
                      : "r"(1), "r"(0)
                    );
                }

                asm volatile (
                    "mvm.s %0, %1, %2"
                    : "=r" (status_flag)
                    : "r"(out_A0), "r"(0)
                    : "memory"
                );

                asm volatile (
                    "mvm.s %0, %1, %2"
                    : "=r" (status_flag)
                    : "r"(out_A1), "r"(1)
                    : "memory"
                );

                #pragma omp barrier


            } else if (thread_id == 1) {
//                printf("Thread %d running on core %d\n", thread_id, core_id);

                // Load vector core 1, array 0
                asm volatile (
                    "mvm.l %0, %1, %2"
                  : "=r" (status_flag)
                  : "r"(vec_B0), "r"(0)
                  : "memory"
                );

                // Load vector core 1, array 1
                asm volatile (
                    "mvm.l %0, %1, %2"
                  : "=r" (status_flag)
                  : "r"(vec_B1), "r"(1)
                  : "memory"
                );

                for (int mini_round = 0; mini_round < mini_rounds; mini_round++) {

                    // mvm on vector core 1, array 0
                    asm volatile (
                        "mvm %0, %1, x0"
                        : "=r" (status_flag)
                        : "r"(0)
                    );

                    // mvm on vector core 1, array 1
                    asm volatile (
                        "mvm %0, %1, x0"
                        : "=r" (status_flag)
                        : "r"(1)
                    );

                    // move output vector core 1, array 0 to core 1, array 1
                    asm volatile (
                        "mvm.mv %0, %1, %2"
                      : "=r" (status_flag)
                      : "r"(0), "r"(1)
                    );

                    // move output vector core 1, array 1 to core 1, array 0
                    asm volatile (
                        "mvm.mv %0, %1, %2"
                      : "=r" (status_flag)
                      : "r"(1), "r"(0)
                    );
                }

                asm volatile (
                    "mvm.s %0, %1, %2"
                    : "=r" (status_flag)
                    : "r"(out_B0), "r"(0)
                    : "memory"
                );

                asm volatile (
                    "mvm.s %0, %1, %2"
                    : "=r" (status_flag)
                    : "r"(out_B1), "r"(1)
                    : "memory"
                );

                #pragma omp barrier

            }
        }


        printf("out_A0 (round: %d): ", round);
        for (int i = 0; i < rows; i++) {
            printf("%f ", out_A0[i]);
        }
        printf("\n");

        printf("out_A1 (round: %d): ", round);
        for (int i = 0; i < rows; i++) {
            printf("%f ", out_A1[i]);
        }
        printf("\n");

        printf("out_B0 (round: %d): ", round);
        for (int i = 0; i < rows; i++) {
            printf("%f ", out_B0[i]);
        }
        printf("\n");

        printf("out_B1 (round: %d): ", round);
        for (int i = 0; i < rows; i++) {
            printf("%f ", out_B1[i]);
        }

        printf("\n\n");

        if (!(round == rounds - 1)) {
            std::swap(out_A0, vec_B0);
            std::swap(out_A1, vec_B1);
            std::swap(out_B0, vec_A0);
            std::swap(out_B1, vec_A1);
        }
    }

    printf("Final out_A0: ");
    for (int i = 0; i < rows; i++) {
        printf("%f ", out_A0[i]);
    }
    printf("\n");

    printf("Final out_A1: ");
    for (int i = 0; i < rows; i++) {
        printf("%f ", out_A1[i]);
    }
    printf("\n");

    printf("Final out_B0: ");
    for (int i = 0; i < rows; i++) {
        printf("%f ", out_B0[i]);
    }
    printf("\n");

    printf("Final out_B1: ");
    for (int i = 0; i < rows; i++) {
        printf("%f ", out_B1[i]);
    }
    printf("\n");

    return 0;
}
