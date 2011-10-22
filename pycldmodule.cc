#include <Python.h>

#define CLD_WINDOWS

#include "encodings/compact_lang_det/compact_lang_det.h"
#include "encodings/compact_lang_det/ext_lang_enc.h"
#include "encodings/proto/encodings.pb.h"

static PyObject *CLDError;

static PyObject *
detect(PyObject *self, PyObject *args, PyObject *kwArgs) {
  char *bytes;
  int numBytes;

  int isPlainText = 0;
  int includeExtendedLanguages = 1;

  // "id" boosts Indonesian;
  const char* hintTopLevelDomain = NULL;

  // ITALIAN boosts it
  const char* hintLanguage = NULL;

  // SJS boosts Japanese
  const char* hintEncoding = NULL;

  static const char *kwList[] = {"utf8Bytes",
                                 "isPlainText",
                                 "includeExtendedLanguages",
                                 "hintTopLevelDomain",
                                 "hintLanguage",
                                 "hintEncoding",
                                 NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwArgs, "s#|iizzz",
                                   (char **)kwList,
                                   &bytes, &numBytes,
                                   &isPlainText,
                                   &includeExtendedLanguages,
                                   &hintTopLevelDomain,
                                   &hintLanguage,
                                   &hintEncoding)) {
    return NULL;
  }

  Language hintLanguageEnum;
  if (hintLanguage == NULL) {
    // no hint
    hintLanguageEnum = UNKNOWN_LANGUAGE;
  } else if (!LanguageFromCode(hintLanguage, &hintLanguageEnum)) {
    // TODO: maybe LookupError?
    PyErr_Format(CLDError, "Unrecognized language hint code (got '%s'); note that currently external languages cannot be hinted", hintLanguage);
    return NULL;
  }

  int hintEncodingEnum;
  if (hintEncoding == NULL) {
    // no hint
    hintEncodingEnum = UNKNOWN_ENCODING;
  } else {
    // TODO: looks like the fwd map is in
    // encodings/proto/encodings.pb.h but no function to
    // look up the reverse...?
    PyErr_Format(CLDError, "Cannot handle encoding hints yet (got '%s')", hintEncoding);
    return NULL;
  }
    
  bool isReliable;
  Language language3[3];
  int percent3[3];
  double normalized_score3[3];
  int textBytesFound;
  Py_BEGIN_ALLOW_THREADS
  if (includeExtendedLanguages != 0) {
     CompactLangDet::ExtDetectLanguageSummary(0,
                                              bytes, numBytes,
                                              isPlainText != 0,
                                              hintTopLevelDomain,
                                              hintEncodingEnum,
                                              hintLanguageEnum,
                                              language3,
                                              percent3,
                                              normalized_score3,
                                              &textBytesFound,
                                              &isReliable);
  } else {
    // TODO: apparently you cannot get normalized_score3?
    CompactLangDet::DetectLanguageSummary(0,
                                          bytes, numBytes,
                                          isPlainText != 0,
                                          hintTopLevelDomain,
                                          hintEncodingEnum,
                                          hintLanguageEnum,
                                          language3,
                                          percent3,
                                          &textBytesFound,
                                          &isReliable);
  }
  Py_END_ALLOW_THREADS

  PyObject *details = PyList_New(0);
  Language topLang = UNKNOWN_LANGUAGE;
  for(int idx=0;idx<3;idx++) {
    Language lang = language3[idx];
    if (lang == UNKNOWN_LANGUAGE) {
      break;
    }
    if (topLang == UNKNOWN_LANGUAGE) {
      topLang = lang;
    }

    PyObject *oneDetail = Py_BuildValue("(ssif)",
                                        ExtLanguageName(lang),
                                        ExtLanguageCode(lang),
                                        percent3[idx],
                                        normalized_score3[idx]);
    PyList_Append(details, oneDetail);
    Py_DECREF(oneDetail);
  }

  PyObject *result = Py_BuildValue("(ssOiO)",
                                   ExtLanguageName(topLang),
                                   ExtLanguageCode(topLang),
                                   isReliable ? Py_True : Py_False,
                                   textBytesFound,
                                   details);
  Py_DECREF(details);
  return result;
}

static PyMethodDef CLDMethods[] = {
  {"detect",  (PyCFunction) detect, METH_VARARGS | METH_KEYWORDS,
   "Detect language from a UTF8 string."},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initcld() {
  PyObject *m = Py_InitModule("cld", CLDMethods);
  if (m == NULL) {
    return;
  }
  
  CLDError = PyErr_NewException((char *) "cld.error", NULL, NULL);
  Py_INCREF(CLDError);
  PyModule_AddObject(m, "error", CLDError);
}
