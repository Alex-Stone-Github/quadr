#include "contour.h"
#include "kernel.h"
#include "square.h"
#include "pipe.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

static PyObject* quadr_takeInNumpy(PyObject *self, PyObject *args) {
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

    // Create Esential Structures
    struct Substrate src, edges, skelx, skely, or;
    Substrate_init(width, height, &src);
    Substrate_init(width - 2, height - 2, &edges);
    Substrate_init(width - 4, height - 4, &skelx);
    Substrate_init(width - 4, height - 4, &skely);
    Substrate_init(width - 4, height - 4, &or);

    // Memcpy into the source
    npy_intp size = PyArray_SIZE(input_array);
    double* data = (double*)PyArray_DATA(input_array);
    struct BitmapInfo bitmap_info = {data, nchannels, size / nchannels};
    Substrate_updateBitmap(&src, &bitmap_info);

    // Perform convolutions and image processing
    float edge_thresh = 0.1f;
    float erode_thresh = 0.1f;
    convolute(&src, &edges, &edge_detection_kernel);
    Substrate_stepPixels(&edges, edge_thresh);
    convolute(&edges, &skelx, &edge_erosion_kernel);
    Substrate_stepPixels(&skelx, erode_thresh);
    // could be a copy instead from erode
    Substrate_copyFrom(&skely, &skelx);
    Substrate_skeletonizeX(&skelx);
    Substrate_skeletonizeY(&skely);
    Substrate_or(&or, &skelx, &skely);

    // Finding contours and squares
    struct Contours contours;
    Contours_initFromSubstrate(&src, &contours);
    Contours_findContoursConsumeSubstrate(&contours, &or);
    struct Squares squares;
    Squares_initFromContours(&contours, &squares);

    // Memcpy out to a return numpy array
    int view_ndims = 3;
    npy_intp view_dims[3];
    view_dims[0] = squares.squares_length;
    view_dims[1] = 4; // 4 pts a square
    view_dims[2] = 2; // 2 pt
    PyObject* squares_view = PyArray_SimpleNewFromData(view_ndims, view_dims,
        NPY_FLOAT32, (void*)squares.squares);
    PyObject* squares_ret = PyArray_NewCopy((PyArrayObject*)squares_view, NPY_CORDER);
    Py_DECREF(squares_view);

    // Cleanup
    Substrate_deinit(&src);
    Substrate_deinit(&edges);
    Substrate_deinit(&skelx);
    Substrate_deinit(&skely);
    Substrate_deinit(&or);
    Contours_deinit(&contours);

    // Beware no error checking

    return squares_ret;
}


static PyMethodDef MyModuleMethods[] = {
    {"takeIn", quadr_takeInNumpy, METH_VARARGS, "Detect features of an image"},
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
