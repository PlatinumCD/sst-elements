# Project Golem

This project includes emulated/CrossSim arrays, Vanadis-RoCC interfaces, and testing for analog matrix-vector multiplication (MVM) applications.

## Directory Structure

### `golem/array`
Contains specifications for various compute arrays.

- **ComputeArray**
  - **EmulatedComputeArray**
    - **EmulatedFloatArray**
    - **EmulatedIntArray**
  - **CrossSimComputeArray**
    - **CrossSimFloatArray**

### `golem/rocc`
Contains analog specifications for RoCC.

- **RoCCAnalog**
  - **RoCCAnalogFloat**
  - **RoCCAnalogInt**

### `golem/tests`
Includes tests for different MVM applications. Below is the structure of planned and completed tests.

- **multi_array_mvm**: Planned but not yet completed.
- **multi_core_mvm**: Planned but not yet completed.
- **multi_array_multi_core_mvm**: Planned but not yet completed.
- **basic_mvm**: Contains basic MVM tests on three compute arrays:
  - **crossSimArray**
    - `analog_library_example`: MVM example using CrossSimArray and analog library.
    - `intrinsics_example`: MVM example using CrossSimArray.
  - **floatArray**
    - `analog_library_example`: MVM example using EmulatedFloatArray and analog library.
    - `intrinsics_example`: MVM example using EmulatedFloatArray.
  - **intArray**
    - `analog_library_example_int`: MVM example using EmulatedIntArray and analog library.
    - `analog_library_example_float`: MVM example using EmulatedIntArray with quantized float values.
    - `intrinsics_example`: MVM example using EmulatedIntArray.

### Files

- **Makefile.am**: Build file for subcomponents.
- **golem.cc**: Main file for including subcomponents.
