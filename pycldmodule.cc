#include <Python.h>

#define CLD_WINDOWS

#include "encodings/compact_lang_det/compact_lang_det.h"
#include "encodings/compact_lang_det/ext_lang_enc.h"

static PyObject *CLDError;

static PyObject *
detectFromUTF8(PyObject *self, PyObject *args) {
  char *bytes;
  int numBytes;
  int isPlainText;

  if (!PyArg_ParseTuple(args, "s#i", &bytes, &numBytes, &isPlainText)) {
    return NULL;
  }

  bool isReliable;
  Language lang = CompactLangDet::DetectLanguage(0, bytes, numBytes,
                                                 isPlainText != 0,
                                                 &isReliable);

  return Py_BuildValue("(ssi)", ExtLanguageCode(lang), ExtLanguageName(lang), isReliable ? 1 : 0);
}

static PyMethodDef CLDMethods[] = {
    {"detectFromUTF8",  detectFromUTF8, METH_VARARGS,
     "Detect language from a UTF8 string."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initcld() {
  PyObject *m = Py_InitModule("cld", CLDMethods);
  if (m == NULL) {
    return;
  }
  
  CLDError = PyErr_NewException("cld.error", NULL, NULL);
  Py_INCREF(CLDError);
  PyModule_AddObject(m, "error", CLDError);
}
