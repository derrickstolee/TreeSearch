#!/usr/bin/python
#
# statistics.py
#
# Collect and collate statistic files into CSVs.
#
# Derrick Stolee - April 1, 2010
#

import os
import sys

vars = [ ]
cols = { }
maxrow = 1

for line in sys.stdin.xreadlines():
	# split by whitespace
	linedata = line.strip().split(" ")
	
	if len(linedata) < 4:
		continue
		
	# third entry is variable name, fourth value
	name, value = linedata[2], linedata[3]
	
	
	atloc = name.find("_AT_")
	
	if atloc > 0:
		truename = name[0:atloc]
		index = int(name[atloc+4:len(name)]) 
		
		if truename not in vars:
			vars.append(truename)
			cols[truename] = { 0:truename }
		
		cols[truename][index] = value
		
		if index > maxrow:
			maxrow = index 
	
	else:
		if name not in vars:
			vars.append(name)
			cols[name] = { 0:name }
		cols[name][1] = value
		
header = "DEPTH,"
for var in vars:
	header = header + var + ","
	
print header

for i in range(1, maxrow+1):
	row = str(i) + ","
	for var in vars:
		if i in cols[var]:
			row = row + str(cols[var][i]) + ","
		else:
			row = row + ","
	print row

