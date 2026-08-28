#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

static PyObject* quadr_takeInNumpy(PyObject *self, PyObject *args) {
    long a, b;

    // Parse two positional integer arguments ("ll" for long long)
    if (!PyArg_ParseTuple(args, "ll", &a, &b)) {
        return NULL; // Return NULL if parsing fails to raise a TypeError
    }


    // Perform the addition and return a Python long object
    return PyLong_FromLong(a + b);
}

// 1. Core function logic
static PyObject* my_module_add(PyObject* self, PyObject* args) {
    long a, b;

    // Parse two positional integer arguments ("ll" for long long)
    if (!PyArg_ParseTuple(args, "ll", &a, &b)) {
        return NULL; // Return NULL if parsing fails to raise a TypeError
    }


    // Perform the addition and return a Python long object
    return PyLong_FromLong(a + b);
}

// 2. Method definition array
static PyMethodDef MyModuleMethods[] = {
    {"takeIn", quadr_takeInNumpy, METH_VARARGS, "Take in numpy arr"},
    {"add", my_module_add, METH_VARARGS, "Add two integers together."},
    {NULL, NULL, 0, NULL}  // Sentinel element marking the end of the array
};

// 3. Module definition structure
static struct PyModuleDef my_module = {
    PyModuleDef_HEAD_INIT,
    "quadr",         // Name of the module
    "A simple C extension module.", // Module documentation
    -1,                  // Size of per-interpreter state (-1 if global state)
    MyModuleMethods      // Methods array link
};

// 4. Initialization function called on "import my_module"
PyMODINIT_FUNC PyInit_quadr(void) {
    return PyModule_Create(&my_module);
}
