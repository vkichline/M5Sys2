#!/usr/bin/env python3
import struct

"""
This program can be used to verify the idx file generated by make_index,
ensuring that each index in the idx file points to a record with 12 tabs
and that the id and value entries expressed match expectations.
"""

# Create the indexes in memory
by_bin  = {}
by_id   = {}
index = open("hexagrams.idx", "r+b")
struct_fmt    = '=ii'
struct_len    = struct.calcsize(struct_fmt)
struct_unpack = struct.Struct(struct_fmt).unpack_from
for i in range(0, 64):
  data = index.read(struct_len)
  s = struct_unpack(data)
  by_bin[i]   = s[0]
  by_id[i+1]  = s[1]
index.close()

# Test by_id:
# Important: id is radix 1, bin is radix 0
data = open("hexagrams.isam", "r")
for i in range(1, 65):
  offset    = by_id[i]
  data.seek(offset)
  record    = data.readline()
  values    = record.split('\t')
  num_tabs  = len(values) - 1
  id        = int(values[1])
  valid     = (12 == num_tabs) and (id == i)
  print("%d\tOff = %d\tSiz = %d\tValid = %s" % (i, offset, len(record), "true" if valid else "false"))
data.close()

print()

# Test by_bin:
# Important: id is radix 1, bin is radix 0
data = open("hexagrams.isam", "r")
for i in range(0, 64):
  offset    = by_bin[i]
  data.seek(by_bin[i])
  record    = data.readline()
  values    = record.split('\t')
  num_tabs  = len(values) - 1
  bin       = int(values[0])
  valid     = (12 == num_tabs) and (bin == i)
  print("%d\tOff = %d\tSiz = %d\tValid = %s" % (i, offset, len(record), "true" if valid else "false"))
data.close()
