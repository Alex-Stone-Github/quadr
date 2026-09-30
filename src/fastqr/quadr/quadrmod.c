#include "contour.h"
#include "kernel.h"
#include "square.h"
#include "pipe.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

typedef struct {
  PyObject_HEAD
  struct QuadPipeline pipe;
} QuadPipeObject;
static PyObject *QuadPipe_new(PyTypeObject *type, PyObject *args,
                              PyObject *kwds) {
    QuadPipeObject* self = NULL;
    self = (QuadPipeObject*)type->tp_alloc(type, 0);
    if (self != NULL) {
        size_t width, height;
        int w, h;
        if (!PyArg_ParseTuple(args, "ii", &w, &h)) {
            return NULL; // Return NULL if parsing fails to raise a TypeError
        }
        width = w; height = h;
        if (width < 50 || height < 50) {
            PyErr_SetString(PyExc_ValueError,
            "Image should have at least a width and height of 50 pixels");
            return NULL;
        }
        QuadPipeline_init(width, height, &self->pipe);
    }
    return (PyObject*)self;
}
static void QuadPipe_dealloc(QuadPipeObject* self) {
    QuadPipeline_deinit(&self->pipe);
    Py_TYPE(self)->tp_free((PyObject*)self);
}
static PyObject* QuadPipe_print_stats(PyObject *self,
                                     PyObject *Py_UNUSED(ignored)) {
    QuadPipeObject* self_pipe = (QuadPipeObject*)self;
    printf("QuadPipe(%d, %d)\n", self_pipe->pipe.width, self_pipe->pipe.height);
    Py_RETURN_NONE;
}
static PyObject* QuadPipe_process(PyObject *self, PyObject *args) {
    // Extract Self
    QuadPipeObject* self_pipe = (QuadPipeObject*)self;
    assert(self_pipe);

    // Get the np array input
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

    // Image imput size validation
    int ndims = PyArray_NDIM(input_array);
    npy_intp* dims = PyArray_DIMS(input_array);
    if (!(ndims == 2 || ndims == 3)) {
        PyErr_SetString(PyExc_ValueError,
        "Image must have 2 or 3 dimensions");
        return NULL;
    }
    size_t height = dims[0];
    size_t width = dims[1];
    if (width != self_pipe->pipe.width || height != self_pipe->pipe.height) {
        PyErr_SetString(PyExc_ValueError,
        "Image dimensions should match pipeline input dimensions");
        return NULL;
    }
    size_t nchannels = 1;
    if (ndims == 3) nchannels = dims[2];
    if (!(nchannels == 3 || nchannels == 1)) {
        PyErr_SetString(PyExc_ValueError,
        "Image pixel format should be grayscale or rgb");
        return NULL;
    }

    // Extract data from np array
    npy_intp size = PyArray_SIZE(input_array);
    double* data = (double*)PyArray_DATA(input_array);
    struct BitmapInfo bitmap_info = {data, nchannels, size / nchannels};

    // Process some stuff
    QuadPipeline_process(&self_pipe->pipe, &bitmap_info);


    // Memcpy out to a return numpy array
    int view_ndims = 3;
    npy_intp view_dims[3];
    view_dims[0] = self_pipe->pipe.squares.squares_length;
    view_dims[1] = 4; // 4 pts a square
    view_dims[2] = 2; // 2 pt
    PyObject* squares_view = PyArray_SimpleNewFromData(view_ndims, view_dims,
        NPY_FLOAT32, (void*)self_pipe->pipe.squares.squares);
    PyObject* squares_ret = PyArray_NewCopy((PyArrayObject*)squares_view, NPY_CORDER);
    Py_DECREF(squares_view);

    return squares_ret;
}
static PyMethodDef QuadPipe_methods[] = {
    {"process", QuadPipe_process, METH_VARARGS, "Detect quads in an image"},
    {"print_stats", QuadPipe_print_stats, METH_NOARGS, "Print pipe statistics"},
    {NULL, NULL, 0, NULL},
};
static PyTypeObject QuadPipeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "quadr.QuadPipe",
    .tp_doc = PyDoc_STR("Quad detection pipeline"),
    .tp_basicsize = sizeof(QuadPipeObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = QuadPipe_new,
    .tp_dealloc = (destructor)QuadPipe_dealloc,
    .tp_methods = QuadPipe_methods,
};


static PyObject* quadr_naive(PyObject *self, PyObject *args) {
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
    struct QuadPipeline pipe;
    QuadPipeline_init(width, height, &pipe);

    // Perform the processing
    npy_intp size = PyArray_SIZE(input_array);
    double* data = (double*)PyArray_DATA(input_array);
    struct BitmapInfo bitmap_info = {data, nchannels, size / nchannels};
    QuadPipeline_process(&pipe, &bitmap_info);

    // Memcpy out to a return numpy array
    int view_ndims = 3;
    npy_intp view_dims[3];
    view_dims[0] = pipe.squares.squares_length;
    view_dims[1] = 4; // 4 pts a square
    view_dims[2] = 2; // 2 pt
    PyObject* squares_view = PyArray_SimpleNewFromData(view_ndims, view_dims,
        NPY_FLOAT32, (void*)pipe.squares.squares);
    PyObject *squares_ret = PyArray_NewCopy((PyArrayObject *)squares_view,
                                            NPY_CORDER);
    Py_DECREF(squares_view);

    // Cleanup
    QuadPipeline_deinit(&pipe);

    return squares_ret;
}


static PyMethodDef MyModuleMethods[] = {
    {"naive", quadr_naive, METH_VARARGS, "Naive detect quads in an image"},
    {NULL, NULL, 0, NULL}  // Sentinel element marking the end of the array
};

static struct PyModuleDef quadr_module_def = {
    PyModuleDef_HEAD_INIT,
    "quadr",
    "Fast quad detection library interfacing with numpy",
    -1,
    MyModuleMethods
};

PyMODINIT_FUNC PyInit_quadr(void) {
    // Verify types
    if (PyType_Ready(&QuadPipeType) < 0) {
        PyErr_Print();
        return NULL;
    }

    // Create the module
    PyObject* m = PyModule_Create(&quadr_module_def);
    if (m == NULL) return NULL;

    // Register types into the module namespace
    Py_INCREF(&QuadPipeType);
    if (PyModule_AddObject(m, "QuadPipe", (PyObject *)&QuadPipeType) < 0) {
        printf("Could not register QuadPipe type with python\n");
        Py_DECREF(&QuadPipeType);
        Py_DECREF(m);
        return NULL;
    }

    // Initializes the numpy api 
    import_array();
    return m;
}
