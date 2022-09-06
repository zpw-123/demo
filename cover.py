#
#
# Convert Input file from edge list format (source destination weight) 
# to (source destination weight destination weight ... ) 
# Note: weight is optional
#
#


import os
import sys

from os import listdir
from os.path import isfile, join

given_dir = sys.argv[1]
to_dir = sys.argv[2]
print("From_Dir: ", given_dir)
print("To_Dir: ", to_dir)
#extension = '.xs1' # input and output file extension 
has_weight = False

#if given_dir[-2] == 'W':
#	has_weight = True 

files = [ f for f in listdir(given_dir) if isfile(join(given_dir, f))]
files = sorted(files) # sorted not require

for f in files:
	dat = {}
	in_length = 0
	out_length = 0

	# read section
	print(f)
	f_open = open(given_dir + f, "r")
	for line in f_open:
		if line[0] is not '#' and line[0] is not '\n' and line[0] is not ' ' and line[0] is not '\t':
			in_length = in_length + 1
			src = int(line.split()[0])
			dst = int(line.split()[1])
			if src in dat:
				dat[src].append(dst)
			else: 
				dat[src] = [dst]

			if has_weight: # contain weight
				w = float(line.split()[2])
				dat[src].append(w)

	# write section
	outFnm = to_dir + f
	#outFnm = to_dir + f.split('.')[0] + extension
	f_write = open(outFnm, 'a+')
	key = sorted(dat)
	for k in key:
		if has_weight:
			out_length = out_length + (len(dat[k]) // 2)
		else:
			out_length = out_length + len(dat[k])

		f_write.write("%s " % str(k))
		for v in dat[k]:
			if isinstance(v, int):
				f_write.write("%s " % str(v))
			elif isinstance(v, float):
				f_write.write("%0.6f " % v)
			else:
				print("...NOT DIGIT...")
		f_write.write("\n")

	if in_length == out_length: # (weak) input/output validation
		print("...PASSED...", outFnm)
	else:
		print("...FAILED...", outFnm)

