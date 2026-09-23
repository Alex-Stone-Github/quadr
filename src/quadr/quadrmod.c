#include "contour.h"
#include "kernel.h"
#include "numpy/ndarraytypes.h"
#include "square.h"
#include "tupleobject.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

struct Color {double r, g, b;};
static struct Color red = {255, 0, 0};
static void drawPixel3(double *buffer, size_t w, size_t h, size_t x, size_t y,
                      struct Color *color) {
    assert(w > 0 && h > 0);
    assert(x < w && y < h); // Make sure our sizes are valid
    size_t index = 3 * (y * w + x);
    memcpy(&buffer[index], color, 12);
}

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
    struct Substrate src, edges, skelx, skely, or, ret;
    Substrate_init(width, height, &src);
    Substrate_init(width - 2, height - 2, &edges);
    Substrate_init(width - 4, height - 4, &skelx);
    Substrate_init(width - 4, height - 4, &skely);
    Substrate_init(width - 4, height - 4, &or);
    Substrate_init(width - 4, height - 4, &ret);

    // Memcpy into the source
    npy_intp size = PyArray_SIZE(input_array);
    double* data = (double*)PyArray_DATA(input_array);
    struct BitmapInfo bitmap_info = {data, nchannels, size / nchannels};
    Substrate_updateBitmap(&src, &bitmap_info);
    printf("We have a substrate of (%ld, %ld)\n", src.width, src.height);

    // Process the arrays
    // TODO: Magic constants
    float edge_thresh = 0.1f;
    float erode_thresh = 0.1f;
    printf("Beginning convolution!\n");
    convolute(&src, &edges, &edge_detection_kernel);
    Substrate_stepPixels(&edges, edge_thresh);
    convolute(&edges, &skelx, &edge_erosion_kernel);
    Substrate_stepPixels(&skelx, erode_thresh);
    // could be a copy instead from erode
    Substrate_copyFrom(&skely, &skelx);
    Substrate_skeletonizeX(&skelx);
    Substrate_skeletonizeY(&skely);
    Substrate_or(&or, &skelx, &skely);
    Substrate_copyFrom(&ret, &or); // skely must have a bad input
    printf("Convolution finished!\n");

    // Finding contours and squares
    printf("We are finding contours now!\n");
    struct Contours contours;
    Contours_initFromSubstrate(&src, &contours);
    Contours_findContoursConsumeSubstrate(&contours, &or);
    printf("We have %ld points in the dest image and some contours!\n", contours.points_length);
    printf("Finding squares form contours\n");
    struct Squares squares;
    Squares_initFromContours(&contours, &squares);
    printf("We found some contours");

    // Memcpy out to a return numpy array
    printf("Converting ret to np array for return!\n");
    int view_ndims = 2;
    npy_intp view_dims[2] = {ret.height, ret.width};
    PyObject* ret_view = PyArray_SimpleNewFromData(view_ndims, view_dims,
        NPY_FLOAT32, (void*)ret.data);
    printf("Converting points to np array for return!\n");
    view_dims[0] = squares.corners_length;
    view_dims[1] = 2;
    PyObject* corners_view = PyArray_SimpleNewFromData(view_ndims, view_dims,
        NPY_FLOAT32, (void*)squares.corners);

    // Cleanup
    Substrate_deinit(&src); 
    Substrate_deinit(&edges); 
    //Substrate_deinit(&erode); 
    //Substrate_deinit(&ret); 
    // Contours_deinit(&contours); // Purposefully leak and construct an array out of
    // potentially deinit squares

    // Make a white line
    for (size_t i = 0; i < 50; i++) {
        *Substrate_getPixel(&ret, i, i) = 0.5f;
    }

    // Beware no error checking
    PyObject* tuple = PyTuple_New(2);
    PyTuple_SetItem(tuple, 0, ret_view);
    PyTuple_SetItem(tuple, 1, corners_view);
    return tuple;
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
