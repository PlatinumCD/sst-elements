import numpy as np

# Define types
in_type = np.int16
out_type = np.int16

# Initialize dimensions
rows = 4
cols = 4

# Allocate matrices and vectors
mat_A0 = np.full((rows, cols), 1, dtype=in_type)
mat_A1 = np.full((rows, cols), 2, dtype=in_type)
mat_B0 = np.full((rows, cols), 3, dtype=in_type)
mat_B1 = np.full((rows, cols), 4, dtype=in_type)

vec_A0 = np.full(cols, 1, dtype=in_type)
vec_A1 = np.full(cols, 2, dtype=in_type)
vec_B0 = np.full(cols, 3, dtype=in_type)
vec_B1 = np.full(cols, 4, dtype=in_type)

out_A0 = np.zeros(rows, dtype=out_type)
out_A1 = np.zeros(rows, dtype=out_type)
out_B0 = np.zeros(rows, dtype=out_type)
out_B1 = np.zeros(rows, dtype=out_type)

cam = mat_B0.dot(mat_B1.dot(mat_A1.dot(mat_A0.dot(vec_A0))))

rounds = 2
mini_rounds = 2

# Ping-pong processing
for round_idx in range(rounds):
    # Core 0 operations
    for _ in range(mini_rounds):
        # Simulate matrix-vector multiplication
        out_A0 = np.dot(mat_A0, vec_A0)
        out_A1 = np.dot(mat_A1, vec_A1)
        # Swap operations (ping-pong)
        vec_A0, vec_A1 = out_A1, out_A0
    
    # Core 1 operations
    for _ in range(mini_rounds):
        # Simulate matrix-vector multiplication
        out_B0 = np.dot(mat_B0, vec_B0)
        out_B1 = np.dot(mat_B1, vec_B1)
        # Swap operations (ping-pong)
        vec_B0, vec_B1 = out_B1, out_B0

    print(f"out_A0 (round {round_idx}): {out_A0}")
    print(f"out_A1 (round {round_idx}): {out_A1}")
    print(f"out_B0 (round {round_idx}): {out_B0}")
    print(f"out_B1 (round {round_idx}): {out_B1}")

    if round_idx != rounds - 1:
        # Swap outputs and vectors for the next round
        vec_B0 = out_A0
        vec_B1 = out_A1
        vec_A0 = out_B0
        vec_A1 = out_B1

# Final outputs
print(f"Final out_A0: {out_A0}")
print(f"Final out_A1: {out_A1}")
print(f"Final out_B0: {out_B0}")
print(f"Final out_B1: {out_B1}")
