from colored import fg, bg, attr
import sys 
import argparse

parser = argparse.ArgumentParser(prog='Compare', description='compares two files')
parser.add_argument('file1', type=str)
parser.add_argument('file2', type=str)
args = parser.parse_args()

with open(args.file1, 'rb') as f: f1_data = f.read()
with open(args.file2, 'rb') as f: f2_data = f.read()

for i in range(0, min(len(f1_data), len(f2_data)), 16):

  c1, c2  = f1_data[i:i+16], f2_data[i:i+16]
  text = ''.join([chr(j) if j < 128 and j > 32 else '.' for j in c1])

  color = lambda x: f'{fg("white")}{bg(x[1])} {hex(x[0])[2:].ljust(2, "0")} {attr("reset")}'
  r16 = map(color, [(ch1, 'red') if ch1 != ch2 else (ch1, 'green') for ch1, ch2 in zip(c1, c2)])
  l16 = map(color, [(ch2, 'red') if ch1 != ch2 else (ch2, 'green') for ch1, ch2 in zip(c1, c2)])

  output = f'{i:#08x}: {"".join(r16)} | {"".join(l16)}  {"  " + "    " * (16-len(c1)) + text} '
  print(output)