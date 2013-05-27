// Copyright (c) 2011 Michael McCandless. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the COPYING file.

#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#include "compact_lang_det.h"
#include "ext_lang_enc.h"
#include "base/string_util.h"
#include "cld_encodings.h"

struct PYCLDState {
  PyObject *error;
};

#ifdef IS_PY3K
#define GETSTATE(m) ((struct PYCLDState*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct PYCLDState _state;
#endif

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
  int removeWeakMatches = 1;
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
                                 "removeWeakMatches",
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
                                   &removeWeakMatches)) {
    return NULL;
  }
  PyObject *CLDError = GETSTATE(self)->error;
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
                                           removeWeakMatches != 0,
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
  {NULL, NULL}        /* Sentinel */
};

#ifdef IS_PY3K

static int cld_traverse(PyObject *m, visitproc visit, void *arg) {
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int cld_clear(PyObject *m) {
  Py_CLEAR(GETSTATE(m)->error);
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "cld",
  NULL,
  sizeof(struct PYCLDState),
  CLDMethods,
  NULL,
  cld_traverse,
  cld_clear,
  NULL
};

#define INITERROR return NULL

//PyObject *
PyMODINIT_FUNC
PyInit_cld(void)

#else
#define INITERROR return

PyMODINIT_FUNC
initcld()
#endif
{

#ifdef IS_PY3K
  PyObject *m = PyModule_Create(&moduledef);
#else
  PyObject* m = Py_InitModule("cld", CLDMethods);
#endif

  if (m == NULL) {
    INITERROR;
  }

  struct PYCLDState *st = GETSTATE(m);

  // Set module-global ENCODINGS tuple:
  PyObject* pyEncs = PyTuple_New(NUM_ENCODINGS);
  for(int encIDX=0;encIDX<NUM_ENCODINGS;encIDX++) {
    PyTuple_SET_ITEM(pyEncs, encIDX, PyUnicode_FromString(cld_encoding_info[encIDX].name));
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

  // Set module-global DETECTED_LANGUAGES tuple:
  // MKM: NOTE I reverse engineered this list from the unit
  // test!!  It has all languages ever detected by the test

  PyObject* pyDetLangs = PyTuple_New(75);
  PyTuple_SET_ITEM(pyDetLangs, 0, PyUnicode_FromString("AFRIKAANS"));
  PyTuple_SET_ITEM(pyDetLangs, 1, PyUnicode_FromString("ALBANIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 2, PyUnicode_FromString("AMHARIC"));
  PyTuple_SET_ITEM(pyDetLangs, 3, PyUnicode_FromString("ARABIC"));
  PyTuple_SET_ITEM(pyDetLangs, 4, PyUnicode_FromString("ARMENIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 5, PyUnicode_FromString("AZERBAIJANI"));
  PyTuple_SET_ITEM(pyDetLangs, 6, PyUnicode_FromString("BASQUE"));
  PyTuple_SET_ITEM(pyDetLangs, 7, PyUnicode_FromString("BELARUSIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 8, PyUnicode_FromString("BENGALI"));
  PyTuple_SET_ITEM(pyDetLangs, 9, PyUnicode_FromString("BULGARIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 10, PyUnicode_FromString("BURMESE"));
  PyTuple_SET_ITEM(pyDetLangs, 11, PyUnicode_FromString("CATALAN"));
  PyTuple_SET_ITEM(pyDetLangs, 12, PyUnicode_FromString("CHEROKEE"));
  PyTuple_SET_ITEM(pyDetLangs, 13, PyUnicode_FromString("CROATIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 14, PyUnicode_FromString("CZECH"));
  PyTuple_SET_ITEM(pyDetLangs, 15, PyUnicode_FromString("Chinese"));
  PyTuple_SET_ITEM(pyDetLangs, 16, PyUnicode_FromString("ChineseT"));
  PyTuple_SET_ITEM(pyDetLangs, 17, PyUnicode_FromString("DANISH"));
  PyTuple_SET_ITEM(pyDetLangs, 18, PyUnicode_FromString("DHIVEHI"));
  PyTuple_SET_ITEM(pyDetLangs, 19, PyUnicode_FromString("DUTCH"));
  PyTuple_SET_ITEM(pyDetLangs, 20, PyUnicode_FromString("ENGLISH"));
  PyTuple_SET_ITEM(pyDetLangs, 21, PyUnicode_FromString("ESTONIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 22, PyUnicode_FromString("FINNISH"));
  PyTuple_SET_ITEM(pyDetLangs, 23, PyUnicode_FromString("FRENCH"));
  PyTuple_SET_ITEM(pyDetLangs, 23, PyUnicode_FromString("GALICIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 24, PyUnicode_FromString("GEORGIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 25, PyUnicode_FromString("GERMAN"));
  PyTuple_SET_ITEM(pyDetLangs, 26, PyUnicode_FromString("GREEK"));
  PyTuple_SET_ITEM(pyDetLangs, 27, PyUnicode_FromString("GUJARATI"));
  PyTuple_SET_ITEM(pyDetLangs, 28, PyUnicode_FromString("HAITIAN_CREOLE"));
  PyTuple_SET_ITEM(pyDetLangs, 29, PyUnicode_FromString("HEBREW"));
  PyTuple_SET_ITEM(pyDetLangs, 30, PyUnicode_FromString("HINDI"));
  PyTuple_SET_ITEM(pyDetLangs, 31, PyUnicode_FromString("HUNGARIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 32, PyUnicode_FromString("ICELANDIC"));
  PyTuple_SET_ITEM(pyDetLangs, 33, PyUnicode_FromString("INDONESIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 34, PyUnicode_FromString("INUKTITUT"));
  PyTuple_SET_ITEM(pyDetLangs, 35, PyUnicode_FromString("IRISH"));
  PyTuple_SET_ITEM(pyDetLangs, 36, PyUnicode_FromString("ITALIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 37, PyUnicode_FromString("Japanese"));
  PyTuple_SET_ITEM(pyDetLangs, 38, PyUnicode_FromString("KANNADA"));
  PyTuple_SET_ITEM(pyDetLangs, 39, PyUnicode_FromString("KHMER"));
  PyTuple_SET_ITEM(pyDetLangs, 40, PyUnicode_FromString("Korean"));
  PyTuple_SET_ITEM(pyDetLangs, 41, PyUnicode_FromString("LAOTHIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 42, PyUnicode_FromString("LATVIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 43, PyUnicode_FromString("LITHUANIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 44, PyUnicode_FromString("MACEDONIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 45, PyUnicode_FromString("MALAY"));
  PyTuple_SET_ITEM(pyDetLangs, 46, PyUnicode_FromString("MALAYALAM"));
  PyTuple_SET_ITEM(pyDetLangs, 47, PyUnicode_FromString("MALTESE"));
  PyTuple_SET_ITEM(pyDetLangs, 48, PyUnicode_FromString("NORWEGIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 49, PyUnicode_FromString("ORIYA"));
  PyTuple_SET_ITEM(pyDetLangs, 50, PyUnicode_FromString("PERSIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 51, PyUnicode_FromString("POLISH"));
  PyTuple_SET_ITEM(pyDetLangs, 52, PyUnicode_FromString("PORTUGUESE"));
  PyTuple_SET_ITEM(pyDetLangs, 53, PyUnicode_FromString("PUNJABI"));
  PyTuple_SET_ITEM(pyDetLangs, 54, PyUnicode_FromString("ROMANIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 55, PyUnicode_FromString("RUSSIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 56, PyUnicode_FromString("SERBIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 57, PyUnicode_FromString("SINHALESE"));
  PyTuple_SET_ITEM(pyDetLangs, 58, PyUnicode_FromString("SLOVAK"));
  PyTuple_SET_ITEM(pyDetLangs, 59, PyUnicode_FromString("SLOVENIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 60, PyUnicode_FromString("SPANISH"));
  PyTuple_SET_ITEM(pyDetLangs, 61, PyUnicode_FromString("SWAHILI"));
  PyTuple_SET_ITEM(pyDetLangs, 62, PyUnicode_FromString("SWEDISH"));
  PyTuple_SET_ITEM(pyDetLangs, 63, PyUnicode_FromString("SYRIAC"));
  PyTuple_SET_ITEM(pyDetLangs, 64, PyUnicode_FromString("TAGALOG"));
  PyTuple_SET_ITEM(pyDetLangs, 65, PyUnicode_FromString("TAMIL"));
  PyTuple_SET_ITEM(pyDetLangs, 66, PyUnicode_FromString("TELUGU"));
  PyTuple_SET_ITEM(pyDetLangs, 67, PyUnicode_FromString("THAI"));
  PyTuple_SET_ITEM(pyDetLangs, 68, PyUnicode_FromString("TIBETAN"));
  PyTuple_SET_ITEM(pyDetLangs, 69, PyUnicode_FromString("TURKISH"));
  PyTuple_SET_ITEM(pyDetLangs, 70, PyUnicode_FromString("UKRAINIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 71, PyUnicode_FromString("URDU"));
  PyTuple_SET_ITEM(pyDetLangs, 72, PyUnicode_FromString("VIETNAMESE"));
  PyTuple_SET_ITEM(pyDetLangs, 73, PyUnicode_FromString("WELSH"));
  PyTuple_SET_ITEM(pyDetLangs, 74, PyUnicode_FromString("YIDDISH"));

  // Steals ref:
  PyModule_AddObject(m, "DETECTED_LANGUAGES", pyDetLangs);
  
  st->error = PyErr_NewException((char *) "cld.error", NULL, NULL);
  if (st->error == NULL) {
    Py_DECREF(m);
    INITERROR;
  }

  // Steals ref:
  PyModule_AddObject(m, "error", st->error);
#ifdef IS_PY3K
  return m;
#endif
}
