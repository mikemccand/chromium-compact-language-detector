// Copyright (c) 2013 Igalia S.L. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _CCLD_H_
#define _CCLD_H_

#pragma once

#if _MSC_VER >= 1600
#include <stdint.h>
#else
#include "stdint.h"
#endif

using namespace System;

#pragma managed

namespace CCLD {

#include "languages.h"
#include "encodings.h"

  // Lang
  public ref class Lang
  {
  public:
    Lang(LanguageId id);

    String^ name(void);
    String^ code(void);

    LanguageId id;
  };

  // DetectedLang
  public ref class DetectedLang: Lang
  {
  public:
    DetectedLang(LanguageId id,
                 int        percent,
                 double     normalizedScore);

    Int32 percent;
    Double normalizedScore;
  };

  // DetectionResult
  public ref class DetectionResult
  {
  public:
    DetectionResult(Lang^                 detectedLang,
                    Int32                 text_bytes,
                    Boolean               isReliable,
                    array<DetectedLang^>^ languages);

    Lang^ detectedLang;
    Int32 textBytes;
    Boolean isReliable;
    array<DetectedLang^>^ languages;
  };

  // LanguageDetector
  public ref class LanguageDetector
  {
  public:
    LanguageDetector(void);

    DetectionResult^ detect(String^        source,
                            Boolean        isPlainText,
                            String^        tldHint,
                            CCLD::Encoding encodingHint,
                            LanguageId     langHint);

    DetectionResult^ detectSimple(String^ source);

    Boolean allowExtendedLanguages;
    Boolean pickSummaryLanguage;
    Boolean removeWeakMatches;
  };

}

#endif // _CCLD_H_
