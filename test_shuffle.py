import re
import cld2

reOneLine = re.compile('^Samp ([^] ]+) /(.*?)/ (.*)$')
lineCount = 0

# NOTE: some lines have \r in them and we want to NOT make a new line for that:
def readlines(f):
  buf = bytearray()
  while True:
    c = f.read(1)
    if len(c) == 0:
      return
    buf.append(c[0])
    if c == b'\n': 
      yield buf.decode('utf-8')
      buf = bytearray()

count = 0
with open('../cld2/internal/test_shuffle_1000_48_666.utf8', 'rb') as f:
  for lineCount, line in enumerate(readlines(f)):
    m = reOneLine.match(line)
    if m is None:
      raise RuntimeError('malformed line %d: %s' % (lineCount, line))
    lang = m.group(1)
    source = m.group(2)
    text = m.group(3)
    count += 1
    #print('%s: %s' % (lang, source))
print('%d lines' % count)
