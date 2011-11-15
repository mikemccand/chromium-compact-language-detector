// Copyright (c) 2011 Michael McCandless. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Python.h>

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

  // Set module-global DETECTED_LANGUAGES tuple:
  // MKM: NOTE I reverse engineered this list from the unit
  // test!!  It has all languages ever detected by the test

  PyObject* pyDetLangs = PyTuple_New(75);
  PyTuple_SET_ITEM(pyDetLangs, 0, PyString_FromString("AFRIKAANS"));
  PyTuple_SET_ITEM(pyDetLangs, 1, PyString_FromString("ALBANIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 2, PyString_FromString("AMHARIC"));
  PyTuple_SET_ITEM(pyDetLangs, 3, PyString_FromString("ARABIC"));
  PyTuple_SET_ITEM(pyDetLangs, 4, PyString_FromString("ARMENIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 5, PyString_FromString("AZERBAIJANI"));
  PyTuple_SET_ITEM(pyDetLangs, 6, PyString_FromString("BASQUE"));
  PyTuple_SET_ITEM(pyDetLangs, 7, PyString_FromString("BELARUSIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 8, PyString_FromString("BENGALI"));
  PyTuple_SET_ITEM(pyDetLangs, 9, PyString_FromString("BULGARIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 10, PyString_FromString("BURMESE"));
  PyTuple_SET_ITEM(pyDetLangs, 11, PyString_FromString("CATALAN"));
  PyTuple_SET_ITEM(pyDetLangs, 12, PyString_FromString("CHEROKEE"));
  PyTuple_SET_ITEM(pyDetLangs, 13, PyString_FromString("CROATIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 14, PyString_FromString("CZECH"));
  PyTuple_SET_ITEM(pyDetLangs, 15, PyString_FromString("Chinese"));
  PyTuple_SET_ITEM(pyDetLangs, 16, PyString_FromString("ChineseT"));
  PyTuple_SET_ITEM(pyDetLangs, 17, PyString_FromString("DANISH"));
  PyTuple_SET_ITEM(pyDetLangs, 18, PyString_FromString("DHIVEHI"));
  PyTuple_SET_ITEM(pyDetLangs, 19, PyString_FromString("DUTCH"));
  PyTuple_SET_ITEM(pyDetLangs, 20, PyString_FromString("ENGLISH"));
  PyTuple_SET_ITEM(pyDetLangs, 21, PyString_FromString("ESTONIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 22, PyString_FromString("FINNISH"));
  PyTuple_SET_ITEM(pyDetLangs, 23, PyString_FromString("FRENCH"));
  PyTuple_SET_ITEM(pyDetLangs, 23, PyString_FromString("GALICIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 24, PyString_FromString("GEORGIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 25, PyString_FromString("GERMAN"));
  PyTuple_SET_ITEM(pyDetLangs, 26, PyString_FromString("GREEK"));
  PyTuple_SET_ITEM(pyDetLangs, 27, PyString_FromString("GUJARATI"));
  PyTuple_SET_ITEM(pyDetLangs, 28, PyString_FromString("HAITIAN_CREOLE"));
  PyTuple_SET_ITEM(pyDetLangs, 29, PyString_FromString("HEBREW"));
  PyTuple_SET_ITEM(pyDetLangs, 30, PyString_FromString("HINDI"));
  PyTuple_SET_ITEM(pyDetLangs, 31, PyString_FromString("HUNGARIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 32, PyString_FromString("ICELANDIC"));
  PyTuple_SET_ITEM(pyDetLangs, 33, PyString_FromString("INDONESIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 34, PyString_FromString("INUKTITUT"));
  PyTuple_SET_ITEM(pyDetLangs, 35, PyString_FromString("IRISH"));
  PyTuple_SET_ITEM(pyDetLangs, 36, PyString_FromString("ITALIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 37, PyString_FromString("Japanese"));
  PyTuple_SET_ITEM(pyDetLangs, 38, PyString_FromString("KANNADA"));
  PyTuple_SET_ITEM(pyDetLangs, 39, PyString_FromString("KHMER"));
  PyTuple_SET_ITEM(pyDetLangs, 40, PyString_FromString("Korean"));
  PyTuple_SET_ITEM(pyDetLangs, 41, PyString_FromString("LAOTHIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 42, PyString_FromString("LATVIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 43, PyString_FromString("LITHUANIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 44, PyString_FromString("MACEDONIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 45, PyString_FromString("MALAY"));
  PyTuple_SET_ITEM(pyDetLangs, 46, PyString_FromString("MALAYALAM"));
  PyTuple_SET_ITEM(pyDetLangs, 47, PyString_FromString("MALTESE"));
  PyTuple_SET_ITEM(pyDetLangs, 48, PyString_FromString("NORWEGIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 49, PyString_FromString("ORIYA"));
  PyTuple_SET_ITEM(pyDetLangs, 50, PyString_FromString("PERSIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 51, PyString_FromString("POLISH"));
  PyTuple_SET_ITEM(pyDetLangs, 52, PyString_FromString("PORTUGUESE"));
  PyTuple_SET_ITEM(pyDetLangs, 53, PyString_FromString("PUNJABI"));
  PyTuple_SET_ITEM(pyDetLangs, 54, PyString_FromString("ROMANIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 55, PyString_FromString("RUSSIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 56, PyString_FromString("SERBIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 57, PyString_FromString("SINHALESE"));
  PyTuple_SET_ITEM(pyDetLangs, 58, PyString_FromString("SLOVAK"));
  PyTuple_SET_ITEM(pyDetLangs, 59, PyString_FromString("SLOVENIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 60, PyString_FromString("SPANISH"));
  PyTuple_SET_ITEM(pyDetLangs, 61, PyString_FromString("SWAHILI"));
  PyTuple_SET_ITEM(pyDetLangs, 62, PyString_FromString("SWEDISH"));
  PyTuple_SET_ITEM(pyDetLangs, 63, PyString_FromString("SYRIAC"));
  PyTuple_SET_ITEM(pyDetLangs, 64, PyString_FromString("TAGALOG"));
  PyTuple_SET_ITEM(pyDetLangs, 65, PyString_FromString("TAMIL"));
  PyTuple_SET_ITEM(pyDetLangs, 66, PyString_FromString("TELUGU"));
  PyTuple_SET_ITEM(pyDetLangs, 67, PyString_FromString("THAI"));
  PyTuple_SET_ITEM(pyDetLangs, 68, PyString_FromString("TIBETAN"));
  PyTuple_SET_ITEM(pyDetLangs, 69, PyString_FromString("TURKISH"));
  PyTuple_SET_ITEM(pyDetLangs, 70, PyString_FromString("UKRAINIAN"));
  PyTuple_SET_ITEM(pyDetLangs, 71, PyString_FromString("URDU"));
  PyTuple_SET_ITEM(pyDetLangs, 72, PyString_FromString("VIETNAMESE"));
  PyTuple_SET_ITEM(pyDetLangs, 73, PyString_FromString("WELSH"));
  PyTuple_SET_ITEM(pyDetLangs, 74, PyString_FromString("YIDDISH"));

  // Steals ref:
  PyModule_AddObject(m, "DETECTED_LANGUAGES", pyDetLangs);
  
  CLDError = PyErr_NewException((char *) "cld.error", NULL, NULL);
  // Steals ref:
  PyModule_AddObject(m, "error", CLDError);
}
