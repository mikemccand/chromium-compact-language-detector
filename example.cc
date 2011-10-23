#include <stdio.h>
#include "encodings/compact_lang_det/compact_lang_det.h"
#include "encodings/compact_lang_det/ext_lang_enc.h"
#include "encodings/compact_lang_det/unittest_data.h"
#include "encodings/proto/encodings.pb.h"

// gcc -I. -L. -o example example.cc -lcld

const char* kTeststr_en =
  "confiscation of goods is assigned as the penalty part most of the courts "
  "consist of members and when it is necessary to bring public cases before a "
  "jury of members two courts combine for the purpose the most important cases "
  "of all are brought jurors or";

// UTF8 constants. Use a UTF-8 aware editor for this file
const char* kTeststr_ks =
  "नेपाल एसिया "
  "मंज अख मुलुक"
  " राजधानी काठ"
  "माडौं नेपाल "
  "अधिराज्य पेर"
  "ेग्वाय "
  "दक्षिण अमेरि"
  "का महाद्वीपे"
  " मध् यक्षेत्"
  "रे एक देश अस"
  "् ति फणीश्वर"
  " नाथ रेणु "
  "फिजी छु दक्ष"
  "िण प्रशान् त"
  " महासागर मंज"
  " अख देश बहाम"
  "ास छु केरेबि"
  "यन मंज "
  "अख मुलुख राज"
  "धानी नसौ सम्"
  " बद्घ विषय ब"
  "ुरुंडी अफ्री"
  "का महाद्वीपे"
  " मध् "
  "यक्षेत्रे दे"
  "श अस् ति सम्"
  " बद्घ विषय";

int main(int argc, char **argv) {
    bool is_plain_text = true;
    bool do_allow_extended_languages = true;
    bool do_pick_summary_language = false;
    bool do_remove_weak_matches = false;
    bool is_reliable;
    Language plus_one = UNKNOWN_LANGUAGE;
    const char* tld_hint = NULL;
    int encoding_hint = UNKNOWN_ENCODING;
    Language language_hint = UNKNOWN_LANGUAGE;

    double normalized_score3[3];
    Language language3[3];
    int percent3[3];
    int text_bytes;

    const char* src = kTeststr_en;
    Language lang;
    lang = CompactLangDet::DetectLanguage(0,
                                          src, strlen(src),
                                          is_plain_text,
                                          do_allow_extended_languages,
                                          do_pick_summary_language,
                                          do_remove_weak_matches,
                                          tld_hint,
                                          encoding_hint,
                                          language_hint,
                                          language3,
                                          percent3,
                                          normalized_score3,
                                          &text_bytes,
                                          &is_reliable);
    printf("LANG=%s\n", LanguageName(lang));

    src = kTeststr_ks;
    lang = CompactLangDet::DetectLanguage(0,
                                          src, strlen(src),
                                          is_plain_text,
                                          do_allow_extended_languages,
                                          do_pick_summary_language,
                                          do_remove_weak_matches,
                                          tld_hint,
                                          encoding_hint,
                                          language_hint,
                                          language3,
                                          percent3,
                                          normalized_score3,
                                          &text_bytes,
                                          &is_reliable);
    printf("LANG=%s\n", LanguageName(lang));
}
