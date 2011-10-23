# Copyright (c) 2011 Michael McCandless. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import cld
import sys
import time

# Download corpus at http://code.google.com/p/language-detection/downloads/detail?name=europarl-test.zip

# For a description of the corpus see http://shuyo.wordpress.com/2011/09/29/langdetect-is-updatedadded-profiles-of-estonian-lithuanian-latvian-slovene-and-so-on/

# /x/tmp3/europarl.test

corpusFile = sys.argv[1]
f = open(corpusFile)
count = 0
fail = 0
byLang = {}
bytes = 0
startTime = time.time()
for line in f.readlines():
  count += 1
  answer, text = line.split('\t')
  if answer not in byLang:
    byLang[answer] = {}
  bytes += len(text)
  name, code, reliable, numBytes, details = cld.detect(text, isPlainText=True, removeWeakLanguages=False, pickSummaryLanguage=False)
  byLang[answer][code] = 1 + byLang[answer].get(code, 0)
  if code != answer:
    #print 'WRONG: %d: %s (%s) vs %s [reliable=%s numBytes=%s details=%s]' % (count, code, name, answer, reliable, numBytes, details)
    fail += 1
elapsedSec = time.time() - startTime
# print 'FAIL %d count %d' % (fail, count)
l = byLang.items()
l.sort()

totFail = 0
for lang, hits in l:
  sum = 0
  for k, ct in hits.items():
    sum += ct
    if k != lang:
      totFail += ct
  correctCT = hits.get(lang)
  l2 = [(ct, lang2) for lang2, ct in hits.items()]
  l2.sort(reverse=True)
  s0 = '%s=%4d' % (l2[0][1], l2[0][0])
  print '%s %5.1f%%: %s %s' % (lang, 100.0*correctCT/sum, s0, ' '.join('%s=%d' % (lang2, ct) for ct, lang2 in l2[1:]))

print
print 'Total %.2f%% (= %d/%d); %d bytes in %.1f msec (%.1f MB/sec)' % (100.0*(count-fail)/count, count-fail, count, bytes, 1000.0*elapsedSec, bytes/1024./1024./elapsedSec)

assert totFail == fail
