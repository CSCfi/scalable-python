#include "Python.h"
#include "parallel_stdio.h"

PyObject* disable_c(PyObject *self, PyObject *args)
{
	disable_io_wrappers();
	Py_RETURN_NONE;
}

static PyMethodDef functions[] = {
	{"disable", disable_c, METH_VARARGS, 0},
	{0, 0, 0, 0}
};

PyMODINIT_FUNC initiowrapper(void)
{
	(void) Py_InitModule("iowrapper", functions);
}

