/*
 *      libwmfs.c
 *      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "libwmfs.h"

/* Set Methods */
static PyMethodDef WmfsMethods[] =
{
     {"init",               wmfs_init,           METH_VARARGS, "Init wmfs module"},
     {"statustext",         wmfs_statustext,     METH_VARARGS, "Wrote in the statustext"},
     {"tag_set",            wmfs_tag_set,        METH_VARARGS, "Set a tag"},
     {"screen_set",         wmfs_screen_set,     METH_VARARGS, "Set the selected screen"},
     {"screen_get_current", wmfs_screen_get_cur, METH_VARARGS, "Get the current screen number"},
     {"screen_count",       wmfs_screen_count,   METH_VARARGS, "Get how many screen there are"},
     {"spawn",              wmfs_spawn,          METH_VARARGS, "Execute a command"},
     /* Sentinel */
     {NULL, NULL, 0, NULL}
};

void
send_client_message(char* atom_name, long data_l[5])
{
     XEvent ev;
     int i;

     ev.xclient.type = ClientMessage;
     ev.xclient.serial = 0;
     ev.xclient.send_event = True;
     ev.xclient.message_type = ATOM(atom_name);
     ev.xclient.window = ROOT;
     ev.xclient.format = 32;

     for(i = 0; i < 6; ++i)
          ev.xclient.data.l[i] = data_l[i];

     XSendEvent(dpy, ROOT, False, SubstructureRedirectMask|SubstructureNotifyMask, &ev);
     XSync(dpy, False);

     return;
}

PyObject*
wmfs_init(PyObject *self, PyObject *args)
{
     Atom rt;
     int rf;
     unsigned long ir, il;
     unsigned char *ret;

     PyArg_ParseTuple(args, "");

     if(!(dpy = XOpenDisplay(NULL)))
     {
          fprintf(stderr, "wmfs-control: cannot open X server.\n");
          exit(EXIT_FAILURE);
     }

     /* Check if wmfs is running */
     XGetWindowProperty(dpy, ROOT, ATOM("_WMFS_RUNNING"), 0L, 4096,
                        False, XA_CARDINAL, &rt, &rf, &ir, &il, &ret);

     /* Else, exit. */
     if(!ret)
     {
          fprintf(stderr, "WMFS is not running.\n");
          XFree(ret);
          exit(EXIT_FAILURE);
     }
     XFree(ret);

     inited = True;

     Py_INCREF(Py_None);

     return Py_None;
}


PyObject*
wmfs_statustext(PyObject *self, PyObject *args)
{
     char *arg;
     long data[5];

     if(!PyArg_ParseTuple(args, "s", &arg))
          return NULL;

     data[4] = True;

     XChangeProperty(dpy, ROOT, ATOM("_WMFS_STATUSTEXT"), ATOM("UTF8_STRING"),
                     8, PropModeReplace, (unsigned char*)arg, strlen(arg));

     send_client_message("_WMFS_STATUSTEXT", data);

     Py_INCREF(Py_None);

     return Py_None;
}

PyObject*
wmfs_spawn(PyObject *self, PyObject *args)
{
     char *arg, *sh;

     Py_INCREF(Py_None);

     if(!PyArg_ParseTuple(args, "s", &arg))
          return NULL;

     if(!(sh = getenv("SHELL")))
          sh = "/bin/sh";
     if(!strlen(arg))
          return Py_None; /* Error ? */

     if(fork() == 0)
     {
          if(fork() == 0)
          {
               setsid();
               execl(sh, sh, "-c", arg, (char*)NULL);
          }
          exit(EXIT_SUCCESS);
     }


     return Py_None;
}


PyMODINIT_FUNC
initwmfs(void)
{
     PyObject *m = Py_InitModule("wmfs", WmfsMethods);

     if(m == NULL)
          return;

     WmfsInitError = PyErr_NewException("wmfs.error", NULL, NULL);

     Py_INCREF(WmfsInitError);
     PyModule_AddObject(m, "error", WmfsInitError);
}

int
main(int argc, char **argv)
{
     Py_SetProgramName(argv[0]);

     /* Initialize the Python interpreter.  Required. */
     Py_Initialize();

     /* Init Module */
     initwmfs();

     return 0;
}
