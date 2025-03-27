# Variables
EXAMPLES = openmp_example.cpp
EXE_OUTPUTS = $(basename $(EXAMPLES))

# Define paths
COMPILER = $(BUILD_DEST)/llvm/bin/clang++
TARGET = riscv64-unknown-linux-musl
TOOLCHAIN = $(BUILD_DEST)/riscv
SYSROOT = $(BUILD_DEST)/riscv/sysroot

# Compiler and flags
CC = $(COMPILER) --target=$(TARGET) --gcc-toolchain=$(TOOLCHAIN) --sysroot=$(SYSROOT) -march=rv64g
CXX_FLAGS = -static -fopenmp 


# Targets
all: $(EXE_OUTPUTS)

%: %.cpp
	$(CC) $(CXX_FLAGS) $< -o $@

# Clean rule to remove the executables and log files
clean:
	rm -f $(EXE_OUTPUTS) stderr-100 stdout-100 
