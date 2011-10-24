// Copyright (c) 2011 Michael McCandless. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Python.h>

#define CLD_WINDOWS

#include "encodings/compact_lang_det/compact_lang_det.h"
#include "encodings/compact_lang_det/ext_lang_enc.h"
#include "base/string_util.h"
#include "cld_encodings.h"

static PyObject *CLDError;

static bool EncodingFromName(const char *name, Encoding *answer) {
  for (int encIDX=0;encIDX<NUM_ENCODINGS;encIDX++) {
    if (!base::strcasecmp(name, cld_encoding_info[encIDX].name)) {
      *answer = cld_encoding_info[encIDX].encoding;
      return true;
    }
  }
  *answer = UNKNOWN_ENCODING;

  return false;
}

static PyObject *
detect(PyObject *self, PyObject *args, PyObject *kwArgs) {
  char *bytes;
  int numBytes;

  int isPlainText = 0;
  int pickSummaryLanguage = 0;
  int removeWeakLanguages = 1;
  int includeExtendedLanguages = 1;

  // "id" boosts Indonesian;
  const char* hintTopLevelDomain = NULL;

  // ITALIAN boosts it
  const char* hintLanguageCode = NULL;

  // SJS boosts Japanese
  const char* hintEncoding = NULL;

  static const char *kwList[] = {"utf8Bytes",
                                 "isPlainText",
                                 "includeExtendedLanguages",
                                 "hintTopLevelDomain",
                                 "hintLanguageCode",
                                 "hintEncoding",
                                 "pickSummaryLanguage",
                                 "removeWeakLanguages",
                                 NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwArgs, "s#|iizzzii",
                                   (char **) kwList,
                                   &bytes, &numBytes,
                                   &isPlainText,
                                   &includeExtendedLanguages,
                                   &hintTopLevelDomain,
                                   &hintLanguageCode,
                                   &hintEncoding,
                                   &pickSummaryLanguage,
                                   &removeWeakLanguages)) {
    return NULL;
  }

  Language hintLanguageEnum;
  if (hintLanguageCode == NULL) {
    // no hint
    hintLanguageEnum = UNKNOWN_LANGUAGE;
  } else if (!LanguageFromCode(hintLanguageCode, &hintLanguageEnum)) {
    // TODO: maybe LookupError?
    PyErr_Format(CLDError, "Unrecognized language hint code (got '%s'); see cld.LANGUAGES for recognized language codes (note that currently external languages cannot be hinted)", hintLanguageCode);
    return NULL;
  }

  Encoding hintEncodingEnum;
  if (hintEncoding == NULL) {
    // no hint
    hintEncodingEnum = UNKNOWN_ENCODING;
  } else if (!EncodingFromName(hintEncoding, &hintEncodingEnum)) {
    PyErr_Format(CLDError, "Unrecognized encoding hint code (got '%s'); see cld.ENCODINGS for recognized encodings", hintEncoding);
    return NULL;
  }
    
  bool isReliable;
  Language language3[3];
  int percent3[3];
  double normalized_score3[3];
  int textBytesFound;
  Language sumLang;
  Py_BEGIN_ALLOW_THREADS
  sumLang = CompactLangDet::DetectLanguage(0,
                                           bytes, numBytes,
                                           isPlainText != 0,
                                           includeExtendedLanguages != 0,
                                           pickSummaryLanguage != 0,
                                           removeWeakLanguages != 0,
                                           hintTopLevelDomain,
                                           hintEncodingEnum,
                                           hintLanguageEnum,
                                           language3,
                                           percent3,
                                           normalized_score3,
                                           &textBytesFound,
                                           &isReliable);
  Py_END_ALLOW_THREADS

  PyObject *details = PyList_New(0);
  for(int idx=0;idx<3;idx++) {
    Language lang = language3[idx];
    if (lang == UNKNOWN_LANGUAGE) {
      break;
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
                                   ExtLanguageName(sumLang),
                                   ExtLanguageCode(sumLang),
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
  PyObject* m = Py_InitModule("cld", CLDMethods);
  if (m == NULL) {
    return;
  }

  // Set module-global ENCODINGS tuple:
  PyObject* pyEncs = PyTuple_New(NUM_ENCODINGS);
  for(int encIDX=0;encIDX<NUM_ENCODINGS;encIDX++) {
    PyTuple_SET_ITEM(pyEncs, encIDX, PyString_FromString(cld_encoding_info[encIDX].name));
  }
  // Steals ref:
  PyModule_AddObject(m, "ENCODINGS", pyEncs);

  // Set module-global LANGUAGES tuple:
  PyObject* pyLangs = PyTuple_New(NUM_LANGUAGES);
  for(int langIDX=0;langIDX<NUM_LANGUAGES;langIDX++) {
    PyObject* pyLang = Py_BuildValue("(zz)",
                                     LanguageName((Language) langIDX),
                                     LanguageCode((Language) langIDX));
    PyTuple_SET_ITEM(pyLangs, langIDX, pyLang);
  }
  // Steals ref:
  PyModule_AddObject(m, "LANGUAGES", pyLangs);

  // Set module-global EXTERNAL_LANGUAGES tuple:
  const int numExtLangs = EXT_NUM_LANGUAGES - EXT_LANGUAGE_BASE; // see ext_lang_enc.h
  PyObject* pyExtLangs = PyTuple_New(numExtLangs);
  for(int langIDX=EXT_LANGUAGE_BASE;langIDX<EXT_NUM_LANGUAGES;langIDX++) {
    PyObject* pyLang = Py_BuildValue("(zz)",
                                     ExtLanguageName((Language) langIDX),
                                     ExtLanguageCode((Language) langIDX));
    PyTuple_SET_ITEM(pyExtLangs, langIDX - EXT_LANGUAGE_BASE, pyLang);
  }
  // Steals ref:
  PyModule_AddObject(m, "EXTERNAL_LANGUAGES", pyExtLangs);
  
  CLDError = PyErr_NewException((char *) "cld.error", NULL, NULL);
  // Steals ref:
  PyModule_AddObject(m, "error", CLDError);
}
