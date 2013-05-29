// Copyright (c) 2013 Igalia S.L. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using CCLD;

namespace CCLDTest
{
    class Program
    {
        static void Main(string[] args)
        {
                // create a LanguageDetector instance
                LanguageDetector detector = new LanguageDetector();

                // call detect() method
                DetectionResult result = detector.detect("Hello world",
                                                         true,
                                                         null,
                                                         CCLD.Encoding.UNKNOWN_ENCODING,
                                                         CCLD.LanguageId.UNKNOWN_LANGUAGE);

                Console.WriteLine ("\nDetected language: " + result.detectedLang.name() +
                                   " (reliable: " + result.isReliable + ")");

                for (byte i=0; i<3; i++) {
                  Console.WriteLine (result.languages[i].name() +
                                     " (" + result.languages[i].code() + ")" +
                                     " (" + result.languages[i].percent + "%)" +
                                     " (score: " + result.languages[i].normalizedScore + ")");
                }
        }
    }
}
