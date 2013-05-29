// Copyright (c) 2013 Igalia S.L. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <msclr/marshal.h>

#include "encodings/compact_lang_det/compact_lang_det.h"
#include "encodings/compact_lang_det/ext_lang_enc.h"
#include "encodings/compact_lang_det/unittest_data.h"
#include "encodings/proto/encodings.pb.h"

#include "ccld.h"

using namespace CCLD;
using namespace msclr::interop;

#define DEFAULT_ALLOW_EXTENDED_LANGUAGES true
#define DEFAULT_PICK_SUMMARY_LANGUAGE    false
#define DEFAULT_REMOVE_WEAK_MATCHES      false

// CCLD:LanguageDetector
LanguageDetector::LanguageDetector(void)
{
  allowExtendedLanguages = DEFAULT_ALLOW_EXTENDED_LANGUAGES;
  pickSummaryLanguage = DEFAULT_PICK_SUMMARY_LANGUAGE;
  removeWeakMatches = DEFAULT_REMOVE_WEAK_MATCHES;
}

DetectionResult^
LanguageDetector::detect(String^        source,
                         Boolean        isPlainText,
                         String^        tldHint,
                         CCLD::Encoding encodingHint,
                         LanguageId     langHint)
{
  marshal_context ^ context = gcnew marshal_context();
  const char *src = context->marshal_as<const char*>(source);
  const char *tld_hint = context->marshal_as<const char*>(tldHint);

  bool is_plain_text = static_cast<bool>(isPlainText);
  int encoding_hint = static_cast<int>(encodingHint);
  Language language_hint = static_cast<Language>(langHint);

  double normalized_score3[3];
  ::Language language3[3];
  int percent3[3];
  int text_bytes;
  bool is_reliable;

  DetectionResult^ result;
  CCLD::Lang^ detectedLang;
  array<DetectedLang^>^ languages;

  Language lang;
  lang = CompactLangDet::DetectLanguage(0,
                                        src, strlen (src),
                                        is_plain_text,
                                        allowExtendedLanguages,
                                        pickSummaryLanguage,
                                        removeWeakMatches,
                                        tld_hint,
                                        encoding_hint,
                                        language_hint,
                                        language3,
                                        percent3,
                                        normalized_score3,
                                        &text_bytes,
                                        &is_reliable);

  detectedLang = gcnew CCLD::Lang(static_cast<LanguageId>(lang));

  languages = gcnew array<DetectedLang^>(3);

  for (int i=0; i<3; i++) {
    LanguageId lang_id = static_cast<LanguageId>(language3[i]);

    languages[i] = gcnew DetectedLang(lang_id,
                                      percent3[i],
                                      normalized_score3[i]);
  }

  result = gcnew DetectionResult(detectedLang,
                                 text_bytes,
                                 is_reliable,
                                 languages);

  delete context;

  return result;
}

DetectionResult^
LanguageDetector::detectSimple(String^ source)
{
  return detect(source,
                true,
                "",
                Encoding::UNKNOWN_ENCODING,
                LanguageId::UNKNOWN_LANGUAGE);
}

// CCLD:Language
CCLD::Lang::Lang(LanguageId id)
{
  this->id = id;
}

String^
CCLD::Lang::name(void)
{
  const char *name;

  name = LanguageName(static_cast<::Language>(id));

  return gcnew String(name);
}

String^
CCLD::Lang::code(void)
{
  const char *code;

  code = LanguageCode(static_cast<::Language>(id));

  return gcnew String(code);
}

// DetectionResult
DetectionResult::DetectionResult(CCLD::Lang^           detectedLang,
                                 Int32                 textBytes,
                                 Boolean               isReliable,
                                 array<DetectedLang^>^ languages)
{
  this->detectedLang = detectedLang;
  this->textBytes = textBytes;
  this->isReliable = isReliable;
  this->languages = languages;
}

// DetectedLang
DetectedLang::DetectedLang(LanguageId id,
                           int        percent,
                           double     normalizedScore):
  Lang(id)
{
  this->percent = percent;
  this->normalizedScore = normalizedScore;
}
