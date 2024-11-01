#ifndef _CROSSSIMCOMPUTEARRAY_H
#define _CROSSSIMCOMPUTEARRAY_H

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <sst/elements/golem/array/computeArray.h>
#include <Python.h>
#include "numpy/arrayobject.h"
#include <string>
#include <type_traits>

namespace SST {
namespace Golem {

template<typename T>
class CrossSimComputeArray : public ComputeArray {
public:
    SST_ELI_REGISTER_SUBCOMPONENT_DERIVED_API(
            CrossSimComputeArray<T>, 
            SST::Golem::ComputeArray,
            TimeConverter*,
            Event::HandlerBase*
    )

    SST_ELI_DOCUMENT_PARAMS(
        {"CrossSimJSONParameters", "JSON configuration for CrossSim", ""}
    )

    CrossSimComputeArray(ComponentId_t id, Params& params,
                         TimeConverter* tc,
                         Event::HandlerBase* handler)
        : ComputeArray(id, params, tc, handler) {
        CrossSimJSON = params.find<std::string>("CrossSimJSONParameters", std::string());

        // Configure selfLink
        selfLink = configureSelfLink("Self", tc, new Event::Handler<CrossSimComputeArray>(this, &CrossSimComputeArray::handleSelfEvent));
        selfLink->setDefaultTimeBase(latencyTC);

        // Initialize Python
        Py_Initialize();
        import_array1();

        // Initialize arrays to hold Python objects
        pyMatrix = new PyObject*[numArrays];
        npMatrix = new PyArrayObject*[numArrays];
        pyArrayIn = new PyObject*[numArrays];
        npArrayIn = new PyArrayObject*[numArrays];
        pyArrayOut = new PyObject*[numArrays];
        npArrayOut = new PyArrayObject*[numArrays];
        cores = new PyObject*[numArrays];
        setMatrixFunction = new PyObject*[numArrays];
        computeMVM = new PyObject*[numArrays];

        inputVectors.resize(numArrays);
        outputVectors.resize(numArrays);
        for (uint32_t i = 0; i < numArrays; i++) {
            inputVectors[i].resize(inputArraySize, T());
            outputVectors[i].resize(outputArraySize, T());
        }
    }

    virtual ~CrossSimComputeArray() {
        // Clean up Python objects and arrays
        for (uint32_t i = 0; i < numArrays; i++) {
            Py_DECREF(pyMatrix[i]);
            Py_DECREF(pyArrayIn[i]);
            Py_DECREF(pyArrayOut[i]);
            Py_DECREF(cores[i]);
            Py_DECREF(setMatrixFunction[i]);
            Py_DECREF(computeMVM[i]);
        }
        delete[] pyMatrix;
        delete[] npMatrix;
        delete[] pyArrayIn;
        delete[] npArrayIn;
        delete[] pyArrayOut;
        delete[] npArrayOut;
        delete[] cores;
        delete[] setMatrixFunction;
        delete[] computeMVM;

        Py_DECREF(crossSim);
        Py_DECREF(paramsConstructor);
        Py_DECREF(AnalogCoreConstructor);
        Py_DECREF(crossSim_params);

        Py_Finalize();
    }

    virtual void init(unsigned int phase) override {
        if (phase == 0) {
            uint64_t inputSize = inputArraySize;
            uint64_t outputSize = outputArraySize;

            // Matrix Dimensions
            int matrixNumDims = 2;
            npy_intp matrixDims[matrixNumDims] = {static_cast<npy_intp>(outputSize), static_cast<npy_intp>(inputSize)};

            int arrayInNumDims = 1;
            npy_intp arrayInDim[arrayInNumDims] {static_cast<npy_intp>(inputSize)};

            int numpyType = getNumpyType();

            // Create Numpy matrices/arrays
            for (uint32_t i = 0; i < numArrays; i++) {
                pyMatrix[i] = PyArray_SimpleNew(matrixNumDims, matrixDims, numpyType);
                npMatrix[i] = reinterpret_cast<PyArrayObject*>(pyMatrix[i]);

                pyArrayIn[i] = PyArray_SimpleNew(arrayInNumDims, arrayInDim, numpyType);
                npArrayIn[i] = reinterpret_cast<PyArrayObject*>(pyArrayIn[i]);
            }

            // Import CrossSim module
            crossSim = PyImport_ImportModule("simulator");
            if (!crossSim) {
                out.fatal(CALL_INFO, -1, "Import CrossSim failed\n");
                PyErr_Print();
            }

            paramsConstructor = PyObject_GetAttrString(crossSim, "CrossSimParameters");
            if (!paramsConstructor) {
                out.fatal(CALL_INFO, -1, "Get CrossSimParameters constructor failed\n");
                PyErr_Print();
            }

            AnalogCoreConstructor = PyObject_GetAttrString(crossSim, "AnalogCore");
            if (!AnalogCoreConstructor) {
                out.fatal(CALL_INFO, -1, "Get AnalogCore constructor failed\n");
                PyErr_Print();
            }

            if (CrossSimJSON.empty()) {
                crossSim_params = PyObject_CallFunction(paramsConstructor, NULL);
            } else {
                PyObject* paramsConstructorJSON = PyObject_GetAttrString(paramsConstructor, "from_json");
                PyObject* jsonArgs = Py_BuildValue("(s)", CrossSimJSON.c_str());
                crossSim_params = PyObject_CallObject(paramsConstructorJSON, jsonArgs);
                Py_DECREF(paramsConstructorJSON);
                Py_DECREF(jsonArgs);
            }

            if (!crossSim_params) {
                out.fatal(CALL_INFO, -1, "Call to CrossSimParameters constructor failed\n");
                PyErr_Print();
            }

            // Create the cores
            for (uint32_t i = 0; i < numArrays; i++) {
                cores[i] = PyObject_CallFunctionObjArgs(AnalogCoreConstructor, pyMatrix[i], crossSim_params, NULL);
                if (!cores[i]) {
                    out.fatal(CALL_INFO, -1, "Call to AnalogCore failed\n");
                    PyErr_Print();
                }

                setMatrixFunction[i] = PyObject_GetAttrString(cores[i], "set_matrix");
                if (!setMatrixFunction[i]) {
                    out.fatal(CALL_INFO, -1, "Get core.set_matrix failed\n");
                    PyErr_Print();
                }

                computeMVM[i] = PyObject_GetAttrString(cores[i], "matvec");
                if (!computeMVM[i]) {
                    out.fatal(CALL_INFO, -1, "Get core.matvec failed\n");
                    PyErr_Print();
                }
            }
        }
    }

    virtual void beginComputation(uint32_t arrayID) override {
        SimTime_t latency = getArrayLatency(arrayID);
        ArrayEvent* ev = new ArrayEvent(arrayID);
        selfLink->send(latency, ev);
    }

    virtual void handleSelfEvent(Event* ev) override {
        ArrayEvent* aev = static_cast<ArrayEvent*>(ev);
        uint32_t arrayID = aev->getArrayID();

        compute(arrayID);

        (*tileHandler)(ev);
    }

    virtual void setMatrixItem(int32_t arrayID, int32_t index, double value) override {
        T val = static_cast<T>(value);
        T* data = reinterpret_cast<T*>(PyArray_DATA(npMatrix[arrayID]));
        data[index] = val;

        if (index == inputArraySize * outputArraySize - 1) {
            PyObject* status = PyObject_CallFunctionObjArgs(setMatrixFunction[arrayID], npMatrix[arrayID], NULL);
            if (!status) {
                out.fatal(CALL_INFO, -1, "Call to core.set_matrix failed\n");
                PyErr_Print();
            }
        }
    }

    virtual void setVectorItem(int32_t arrayID, int32_t index, double value) override {
        T val = static_cast<T>(value);
        T* data = reinterpret_cast<T*>(PyArray_DATA(npArrayIn[arrayID]));
        data[index] = val;
    }

    virtual void compute(uint32_t arrayID) override {
        // Call MVM
        pyArrayOut[arrayID] = PyObject_CallFunctionObjArgs(computeMVM[arrayID], npArrayIn[arrayID], NULL);
        if (!pyArrayOut[arrayID]) {
            out.fatal(CALL_INFO, -1, "Run MVM Call Failed\n");
            PyErr_Print();
        }
        npArrayOut[arrayID] = reinterpret_cast<PyArrayObject*>(pyArrayOut[arrayID]);
        T* outputVector = reinterpret_cast<T*>(PyArray_DATA(npArrayOut[arrayID]));
        int len = PyArray_SIZE(npArrayOut[arrayID]);
        outputVectors[arrayID].resize(len);
        std::copy(outputVector, outputVector + len, outputVectors[arrayID].begin());

        T* inputVector = reinterpret_cast<T*>(PyArray_DATA(npArrayIn[arrayID]));
        T* matrix = reinterpret_cast<T*>(PyArray_DATA(npMatrix[arrayID]));

        out.verbose(CALL_INFO, 2, 0, "CrossSim MVM on array %u:\n", arrayID);
        for (uint32_t col = 0; col < inputArraySize; col++) {
            printValue(inputVector[col]);
        }
        out.output("\n\n");

        for (uint32_t row = 0; row < outputArraySize; row++) {
            for (uint32_t col = 0; col < inputArraySize; col++) {
                printValue(matrix[row * inputArraySize + col]);
            }
            out.output("  ");
            printValue(outputVector[row]);
            out.output("\n");
        }
        out.output("\n\n");
    }

    virtual SimTime_t getArrayLatency(uint32_t arrayID) override {
        return 1;
    }

    virtual void moveOutputToInput(uint32_t srcArrayID, uint32_t destArrayID) override {
        // Move output to input in NumPy arrays
        T* srcData = reinterpret_cast<T*>(PyArray_DATA(npArrayOut[srcArrayID]));
        T* destData = reinterpret_cast<T*>(PyArray_DATA(npArrayIn[destArrayID]));
        std::copy(srcData, srcData + outputArraySize, destData);
    }

    virtual void* getInputVector(uint32_t arrayID) {
        // Convert NumPy array data to std::vector (if needed)
        T* data = reinterpret_cast<T*>(PyArray_DATA(npArrayIn[arrayID]));
        int len = PyArray_SIZE(npArrayIn[arrayID]);
        inputVectors[arrayID].resize(len);
        std::copy(data, data + len, inputVectors[arrayID].begin());
        return static_cast<void*>(&inputVectors[arrayID]);
    }

    virtual void* getOutputVector(uint32_t arrayID) {
        return static_cast<void*>(&outputVectors[arrayID]);
    }

protected:
    std::string CrossSimJSON;

    PyObject* crossSim = nullptr;
    PyObject* paramsConstructor = nullptr;
    PyObject* AnalogCoreConstructor = nullptr;
    PyObject* crossSim_params = nullptr;

    PyObject** pyMatrix = nullptr;
    PyArrayObject** npMatrix = nullptr;
    PyObject** pyArrayIn = nullptr;
    PyArrayObject** npArrayIn = nullptr;
    PyObject** pyArrayOut = nullptr;
    PyArrayObject** npArrayOut = nullptr;
    PyObject** cores = nullptr;
    PyObject** setMatrixFunction = nullptr;
    PyObject** computeMVM = nullptr;

    std::vector<std::vector<T>> inputVectors;
    std::vector<std::vector<T>> outputVectors;

    int getNumpyType() {
        if constexpr (std::is_same<T, int64_t>::value) {
            return NPY_INT64;
        } else if constexpr (std::is_same<T, float>::value) {
            return NPY_FLOAT32;
        } else {
            static_assert(!sizeof(T*), "Unsupported data type for CrossSimComputeArray.");
        }
    }

    void printValue(const T& value) {
        if constexpr (std::is_same<T, int64_t>::value) {
            out.output("%ld ", value);
        } else if constexpr (std::is_same<T, float>::value) {
            out.output("%f ", value);
        }
    }
};

} // namespace Golem
} // namespace SST

#endif /* _CROSSSIMCOMPUTEARRAY_H */
