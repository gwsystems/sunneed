#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int
main(void)
{
    PyObject *module, *func, *py_args, *py_kwargs;
    PyGILState_STATE state;

    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("sys.path.append(os.getcwd()+'/ext/SunneeD_dev_drivers/PiCamDriver')");

    if ( (module = PyImport_Import(PyUnicode_FromString("picam_driver"))) == NULL) {
        PyErr_Print();
        abort();
    }
    func = PyObject_GetAttrString(module, "read_pipe");

    state = PyGILState_Ensure();

    if (!PyCallable_Check(func)) {
        fprintf(stderr, "Function not callable\n");
        goto fail;
    }

    py_args = NULL;
    py_kwargs = NULL;

    printf("calling function\n");
    PyObject_Call(func, py_args, py_kwargs);

    Py_DECREF(py_args);
    Py_XDECREF(py_kwargs);

    if(PyErr_Occurred()) {
        PyErr_Print();
        goto fail;
    }

    PyGILState_Release(state);

    Py_Finalize();

    return 0;

fail:
    PyGILState_Release(state);
    abort();
}