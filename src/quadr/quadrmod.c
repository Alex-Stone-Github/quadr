#include "contour.h"
#include "kernel.h"
#include "square.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

static PyObject* quadr_takeInNumpy(PyObject *self, PyObject *args) {
    printf("We are receiving the array form c!\n");
    // Get a numpy array as input for an image representation
    PyObject* input_object = NULL;
    if (!PyArg_ParseTuple(args, "O", &input_object)) {
        return NULL; // Return NULL if parsing fails to raise a TypeError
    }
    PyArrayObject* input_array = (PyArrayObject*)PyArray_FROM_OTF(
        input_object,
        NPY_DOUBLE,
        NPY_ARRAY_IN_ARRAY);
    if (input_array == NULL) {
        PyErr_SetString(PyExc_TypeError, "Image must be a numpy array");
        return NULL;
    }

    // Image imput validation
    int ndims = PyArray_NDIM(input_array);
    npy_intp* dims = PyArray_DIMS(input_array);
    if (!(ndims == 2 || ndims == 3)) {
        PyErr_SetString(PyExc_ValueError,
        "Image must have 2 or 3 dimensions");
        return NULL;
    }
    size_t height = dims[0];
    size_t width = dims[1];
    if (width < 50 || height < 50) {
        PyErr_SetString(PyExc_ValueError,
        "Image should have at least a width and height of 50 pixels");
        return NULL;
    }
    size_t nchannels = 1;
    if (ndims == 3) nchannels = dims[2];
    if (!(nchannels == 3 || nchannels == 1)) {
        PyErr_SetString(PyExc_ValueError,
        "Image pixel format should be grayscale or rgb");
        return NULL;
    }

    // Create a bitmap info struct
    struct Substrate src;
    Substrate_init(width, height, &src);
    struct Substrate dst;
    Substrate_init(width - 2, height - 2, &dst);
    struct Contours contours;
    Contours_initFromSubstrate(&src, &contours);

    npy_intp size = PyArray_SIZE(input_array);
    double* data = (double*)PyArray_DATA(input_array);
    struct BitmapInfo bitmap_info = {data, nchannels, size / nchannels};
    Substrate_updateBitmap(&src, &bitmap_info);

    printf("We have a substrate of (%ld, %ld)\n", src.width, src.height);

    printf("Beginning convolution!\n");
    convolute(&src, &dst, &edge_detection_kernel);
    for (size_t i = 0; i < dst.width * dst.height; i++) {
        dst.data[i] = dst.data[i] > 0.3f ? 1.0f : 0.0f;
    }
    printf("Convolution finished!\n");

    Contours_findContoursConsumeSubstrate(&contours, &dst);
    printf("We have %ld points in the dest image!\n", contours.points_length);

    // Contours_deinit(&contours); // Purposefully leak and construct an array out of
    Substrate_deinit(&dst); 
    Substrate_deinit(&src); 

    // BTW lets find some squares
    printf("Finding squares\n");
    struct Squares squares;
    Squares_initFromContours(&contours, &squares);

    printf("Converting to np array for return!\n");
    int view_ndims = 3;
    npy_intp view_dims[3] = {squares.squares_length, 4, 2};
    PyObject* contours_view = PyArray_SimpleNewFromData(view_ndims, view_dims,
        NPY_FLOAT32, (void*)squares.squres);


    return contours_view;
}

static PyObject* my_module_add(PyObject* self, PyObject* args) {
    long a, b;
    if (!PyArg_ParseTuple(args, "ll", &a, &b)) {
        return NULL; // Return NULL if parsing fails to raise a TypeError
    }
    return PyLong_FromLong(a + b);
}

static PyMethodDef MyModuleMethods[] = {
    {"takeIn", quadr_takeInNumpy, METH_VARARGS, "Detect features of an image"},
    {"add", my_module_add, METH_VARARGS, "Add two integers together."},
    {NULL, NULL, 0, NULL}  // Sentinel element marking the end of the array
};

static struct PyModuleDef quadr_module_def = {
    PyModuleDef_HEAD_INIT,
    "quadr",
    "This is a quick polygon detection library for python working with numpy arrays focused on speed",
    -1,
    MyModuleMethods
};

PyMODINIT_FUNC PyInit_quadr(void) {
    // Create the module
    PyObject* m = PyModule_Create(&quadr_module_def);

    // Initializes the numpy api 
    import_array();

    return m;
}
