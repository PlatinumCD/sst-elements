#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// Convert a cpu_set_t to a binary string representation.
// 'out' must be allocated by the caller with at least nprocs+1 bytes.
void cpuset_to_string(cpu_set_t *set, long nprocs, char *out) {
    for (int i = 0; i < nprocs; i++) {
        out[i] = CPU_ISSET(i, set) ? '1' : '0';
    }
    out[nprocs] = '\0';
}

// Build the expected mask string for the case when only 'active_cpu' is set.
void expected_mask_string(long nprocs, int active_cpu, char *out) {
    for (int i = 0; i < nprocs; i++) {
        out[i] = (i == active_cpu) ? '1' : '0';
    }
    out[nprocs] = '\0';
}

// Test 1: For each CPU, set affinity to that CPU and compare expected vs. actual mask.
int test_basic_affinity(long nprocs) {
    int errors = 0;
    printf("=== Test 1: Basic Affinity Test ===\n");
    for (int i = 0; i < nprocs; i++) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);

        if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
            printf("Error: Failed to set affinity for CPU %d: %s\n", i, strerror(errno));
            errors++;
            continue;
        }

        cpu_set_t getset;
        CPU_ZERO(&getset);
        if (sched_getaffinity(0, sizeof(getset), &getset) == -1) {
            printf("Error: Failed to get affinity for CPU %d: %s\n", i, strerror(errno));
            errors++;
            continue;
        }

        char actual[nprocs+1];
        char expected[nprocs+1];
        cpuset_to_string(&getset, nprocs, actual);
        expected_mask_string(nprocs, i, expected);

        int cpu_number = sched_getcpu();
        if (cpu_number == -1) {
            printf("Error: Failed to get current CPU number: %s\n", strerror(errno));
            errors++;
        }

        if (strcmp(actual, expected) != 0) {
            printf("Error: For CPU %d, expected mask: %s, got: %s\n", i, expected, actual);
            errors++;
        } else {
            printf("Affinity correctly set to CPU %d. Mask: %s. Currently running on CPU: %d\n", i, actual, cpu_number);
        }
    }
    printf("\n");
    return errors;
}

// Test 2: Set the full affinity mask (all CPUs) and compare.
int test_full_affinity(long nprocs) {
    int errors = 0;
    printf("=== Test 2: Full Affinity Test ===\n");
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (int i = 0; i < nprocs; i++) {
        CPU_SET(i, &cpuset);
    }
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
        printf("Error: Failed to set full affinity mask: %s\n", strerror(errno));
        return 1;
    }

    cpu_set_t getset;
    CPU_ZERO(&getset);
    if (sched_getaffinity(0, sizeof(getset), &getset) == -1) {
        printf("Error: Failed to get full affinity mask: %s\n", strerror(errno));
        return 1;
    }

    char actual[nprocs+1];
    char expected[nprocs+1];
    cpuset_to_string(&getset, nprocs, actual);
    for (int i = 0; i < nprocs; i++) {
        expected[i] = '1';
    }
    expected[nprocs] = '\0';

    int cpu_number = sched_getcpu();
    if (cpu_number == -1) {
        printf("Error: Failed to get current CPU number: %s\n", strerror(errno));
        errors++;
    }

    if (strcmp(actual, expected) != 0) {
        printf("Error: Full affinity mask mismatch. Expected: %s, Got: %s\n", expected, actual);
        errors++;
    } else {
        printf("Full affinity mask set correctly. Mask: %s. Currently Running on CPU: %d\n", actual, cpu_number);
    }
    printf("\n");
    return errors;
}

// Test 3: Round-robin: cycle through all CPUs for a given number of rounds.
int test_round_robin(long nprocs, int rounds) {
    int errors = 0;
    printf("=== Test 3: Round-Robin Affinity Switching (%d rounds) ===\n", rounds);
    for (int round = 0; round < rounds; round++) {
        printf("  -- Round %d --\n", round + 1);
        for (int i = 0; i < nprocs; i++) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(i, &cpuset);
            if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1) {
                printf("Error: Failed to set affinity for CPU %d: %s\n", i, strerror(errno));
                errors++;
                continue;
            }

            cpu_set_t getset;
            CPU_ZERO(&getset);
            if (sched_getaffinity(0, sizeof(getset), &getset) == -1) {
                printf("Error: Failed to get affinity for CPU %d: %s\n", i, strerror(errno));
                errors++;
                continue;
            }

            char actual[nprocs+1];
            char expected[nprocs+1];
            cpuset_to_string(&getset, nprocs, actual);
            expected_mask_string(nprocs, i, expected);

            int cpu_number = sched_getcpu();
            if (cpu_number == -1) {
                printf("Error: Failed to get current CPU number: %s\n", strerror(errno));
                errors++;
            }

            if (strcmp(actual, expected) != 0) {
                printf("Error: Round %d, CPU %d: expected mask: %s, got: %s\n", round + 1, i, expected, actual);
                errors++;
            } else {
                printf("Affinity correctly set to CPU %d. Mask: %s. Currently running on CPU: %d\n", i, actual, cpu_number);
            }
        }
        printf("\n");
    }
    return errors;
}

int main() {
    long nprocs = sysconf(_SC_NPROCESSORS_CONF);
    if (nprocs < 1) {
        printf("Error: Could not determine number of processors.\n");
        return EXIT_FAILURE;
    }
    printf("Number of processors detected: %ld\n\n", nprocs);

    int total_errors = 0;
    total_errors += test_basic_affinity(nprocs);
    total_errors += test_full_affinity(nprocs);
    int rounds = 2;  // Adjust number of rounds if needed.
    total_errors += test_round_robin(nprocs, rounds);

    if (total_errors == 0) {
        printf("=== All tests passed successfully! ===\n");
        return EXIT_SUCCESS;
    } else {
        printf("=== Some tests failed. Total errors: %d ===\n", total_errors);
        return EXIT_FAILURE;
    }
}

