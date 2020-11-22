#ifndef DEBUG_
#include <python3.7m/Python.h>
#endif

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <stdlib.h>

#define MAX_EVENTS 1024  /* Maximum number of events to process*/
#define LEN_NAME 16  /* Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  sizeof(struct inotify_event) /*size of one event*/
#define BUF_LEN     MAX_EVENTS * (EVENT_SIZE + LEN_NAME)



void tail_(const char *filename, void *py_cb_func)
{
    int fd, file_fd;
    char watch_dir[1024];
    int length;
#ifndef DEBUG_
    PyObject *cb_arglist;
#endif

    strcpy(watch_dir, filename);
    dirname(watch_dir);

    fd = inotify_init();
    if (inotify_add_watch(fd, watch_dir, IN_MODIFY) < 0) {
        printf("pytail watch error: %s\n", watch_dir);
        return;
    }

    file_fd = open(filename, O_RDONLY);
    lseek(file_fd, 0, SEEK_END);

    while (1) {
        int  i = 0;
        char buffer[BUF_LEN];
        char buffer2[1024];

        length = read(fd, buffer, BUF_LEN);

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & IN_MODIFY) {
                    while (read(file_fd, buffer2, 1024)) {
                        #ifndef DEBUG_
                        cb_arglist = Py_BuildValue("(s)", buffer2);
                        PyObject *pv = PyObject_CallObject((PyObject *)py_cb_func, cb_arglist);
                        Py_CLEAR(pv);
                        #else
                        printf("%s", buffer2);
                        #endif
                    }
                    lseek(file_fd, 0, SEEK_END);
                }
            }

            i += EVENT_SIZE + event->len;
        }

    }
}


#ifndef DEBUG_

// Function 1: A simple 'hello world' function
static PyObject *tail(PyObject *self, PyObject *args)
{
    char *file_name;
    PyObject *callback_func;
    // PyObject *cb_arglist;

    if(!PyArg_ParseTuple(args, "sO", &file_name, &callback_func)) {
        printf("file name should be string\n");
        return Py_None;
    }

    if (!PyCallable_Check(callback_func)) {
        PyErr_SetString(PyExc_TypeError, "callable argument is required");
        return 0;
    }


    /* now file_name has the file to watch */
    tail_(file_name, (void *)callback_func);

    return Py_None;
}

// Our Module's Function Definition struct
// We require this `NULL` to signal the end of our method
// definition
static PyMethodDef pytail_methods[] = {
    { "tail", tail, METH_VARARGS, "Tail given file" },
    { NULL, NULL, 0, NULL }
};

// Our Module Definition struct
static struct PyModuleDef pytail = {
    PyModuleDef_HEAD_INIT,
    "pytail",
    "Python Tail module",
    -1,
    pytail_methods
};

// Initializes our module using our above struct
PyMODINIT_FUNC PyInit_pytail(void)
{
    return PyModule_Create(&pytail);
}

#endif


/* main function call to test functionality */
#ifdef DEBUG_
int main(int argc, char *argv[])
{
    tail_(argv[1]);

    return 0;
}
#endif
