# Copyright (c) 2011 Michael McCandless. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import time
import codecs
import subprocess
import cld
import unicodedata

# Download corpus at http://code.google.com/p/language-detection/downloads/detail?name=europarl-test.zip

# For a description of the corpus see http://shuyo.wordpress.com/2011/09/29/langdetect-is-updatedadded-profiles-of-estonian-lithuanian-latvian-slovene-and-so-on/

# /x/tmp3/europarl.test

class DetectTika:
  TIKA_COMMAND = 'java -cp /home/mike/src/util:/lucene/tika.clean/tika-app/target/tika-app-1.0-SNAPSHOT.jar TikaDetectLanguageEmbedded'
  name = 'Tika'
  def __init__(self):
    if os.system('javac -cp /lucene/tika.clean/tika-app/target/tika-app-1.0-SNAPSHOT.jar /home/mike/src/util/TikaDetectLanguageEmbedded.java'):
      raise RuntimeError('compile failed')

    # talk to Tika via pipes
    self.tika = subprocess.Popen(self.TIKA_COMMAND,
                                 shell=True,
                                 stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE)

  def detect(self, utf8):
    self.tika.stdin.write('%7d' % len(utf8))
    self.tika.stdin.write(utf8)
    size = int(self.tika.stdout.read(7).strip())
    lang, reliable = self.tika.stdout.read(size).split(' ')
    reliable = reliable == 'true'
    return lang, reliable

class DetectCLD:
  name = 'CLD'

  allLangs = set()

  def detect(self, utf8):
    name, code, reliable, numBytes, details = cld.detect(utf8, isPlainText=True, removeWeakMatches=False, pickSummaryLanguage=False)
    for tup in details:
      self.allLangs.add(tup[0])
    return code, reliable

class DetectLD:
  name = 'LangDetect'
  
  LD_COMMAND = 'java -cp /home/mike/src/langdetect/lib/langdetect.jar:/home/mike/src/langdetect/lib/jsonic-1.2.0.jar:/home/mike/src/util LDDetectEmbedded'
  def __init__(self):
    if os.system('javac -cp /home/mike/src/langdetect/lib/langdetect.jar /home/mike/src/util/LDDetectEmbedded.java'):
      raise RuntimeError('compile failed')

    # talk to Tika via pipes
    self.ld = subprocess.Popen(self.LD_COMMAND,
                               shell=True,
                               stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE)

  def detect(self, utf8):
    self.ld.stdin.write('%7d' % len(utf8))
    self.ld.stdin.write(utf8)
    size = int(self.ld.stdout.read(7).strip())
    lang = self.ld.stdout.read(size)
    reliable = True
    return lang, reliable

def loadTestData(corpusFile):
  f = open(corpusFile)
  testData = []
  bytes = 0
  for line in f.readlines():
    line = line.strip()
    answer, text = line.split('\t')
    # nocommit
    if False and answer in ('bg', 'cs', 'lt', 'lv'):
      continue
    bytes += len(text)
    if 0:
      # This corpus never differs
      uText = codecs.getdecoder('UTF-8')(text)[0]
      text2 = unicodedata.normalize('NFC', uText)
      text2 = codecs.getencoder('UTF-8')(text2)[0]
      if text2 != text:
        print 'DIFFS'
    testData.append((answer, text))
  return testData, bytes

def accTest():
  if len(sys.argv) == 1:
    corpusFile = '/x/tmp3/europarl.test'
  else:
    corpusFile = sys.argv[1]
    
  testData = loadTestData(corpusFile)[0]

  tika = DetectTika()
  cld = DetectCLD()
  ld = DetectLD()

  allResults = {}
  for detector in (cld, ld, tika):
    print '  run %s...' % detector.name
    results = allResults[detector.name] = []
    startTime = time.time()
    for answer, text in testData:
      results.append(detector.detect(text))
    print '    %.3f sec' % (time.time()-startTime)
    if detector == cld:
      l = list(cld.allLangs)
      l.sort()
      print '%d langs: %s' % (len(l), l)

  processResults(testData, allResults)

def perfTest():
  # NOTE: not really fair because cld is embedded in python but the others communicate through pipes
  testData, totBytes = loadTestData(sys.argv[1])

  tika = DetectTika()
  cld = DetectCLD()
  ld = DetectLD()

  for detector in (cld, ld, tika):
    print '  run %s...' % detector.name
    best = None
    for iter in xrange(10):
      print '    iter %s...' % iter
      tStart = time.time()
      for answer, text in testData:
        if answer in ('bg', 'cs', 'lt', 'lv'):
          continue
        detector.detect(text)
      elapsedSec = time.time() - tStart
      print '      %.3f sec' % elapsedSec
      if best is None or elapsedSec < best:
        print '      **'
        best = elapsedSec
    print '    best %.1f sec %.3f MB/sec' % (best, totBytes/1024./1024./best)

def processResults(testData, allResults):

  byDetector = {}
  for name in allResults.keys():
    byDetector[name] = {}
    
  for idx in xrange(len(testData)):
    answer, text = testData[idx]
    if False:
      print '%s: %d bytes' % (answer, len(text))
    for name, results in allResults.items():
      byLang = byDetector[name]
      if answer not in byLang:
        byLang[answer] = {}
      hits = byLang[answer]
      detected = results[idx][0]
      hits[detected] = 1+hits.get(detected, 0)
      if False and detected != answer:
        print '  %s: wrong (got %s)' % (name, detected)

  count = len(testData)
  
  for name, byLang in byDetector.items():
    l = byLang.items()
    l.sort()

    print
    print '%s results:' % name
    totFail = 0
    for lang, hits in l:
      sum = 0
      for k, ct in hits.items():
        sum += ct
        if k != lang:
          totFail += ct
      correctCT = hits.get(lang, 0)
      l2 = [(ct, lang2) for lang2, ct in hits.items()]
      l2.sort(reverse=True)
      s0 = '%s=%4d' % (l2[0][1], l2[0][0])
      print '  %s %5.1f%%: %s %s' % (lang, 100.0*correctCT/sum, s0, ' '.join('%s=%d' % (lang2, ct) for ct, lang2 in l2[1:]))

    print '  total %.2f%% (= %d/%d)' % (100.0*(count-totFail)/count, count-totFail, count)

if __name__ == '__main__':
  accTest()
  #perfTest()
