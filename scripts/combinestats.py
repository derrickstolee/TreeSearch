#!/usr/bin/python

#
# combinestats.py
#
# Oct 11, 2010 -- Derrick Stolee
#
# Take the allstats.txt file and 
# convert the variables into a CSV where
# the rows are the variable type
# the columns are the depth in the tree
# 
# ROW0: Labels of depth.
# COL0: Label of variable.
# COL1: TOTAL of variable.
# COLi: Value at depth i-2 (for i >= 2)
#

import os

folder = "./"

#import folder
for arg in sys.argv:
    if arg.startswith('-F'):
        folder = arg[2:];



statfile = open(folder + 'allstats.txt','r')

variable_names = [];
variable_types = {};
variable_values = {};

max_index = 0;

for line in statfile.xreadlines():
	if line[0] == 'T':
		linearr = line.split(" ")
		# FORMAT: T TYPE VAR_NAME(_#) VALUE
		var_type = linearr[1];
		var_name = linearr[2];
		var_value = float(linearr[3]);
		
		# Attempt to extract an index from the variable name
		_at_ = var_name.find("_AT_");
		
		index = -1;
		full_name = var_name;
		
		if _at_ > 0:
			index = int(var_name[_at_+4:len(var_name)]); #WORD
			full_name = var_name[0:_at_];
			if index > max_index:
				max_index = index;
		else:
			full_name = var_name;
			
		if full_name not in variable_names :
			variable_names.append(full_name);
			variable_values[full_name] = {};
			variable_types[full_name] = var_type;
			variable_values[full_name][-1] = 0;
		
		# index -1 is the aggregated value
		variable_values[full_name][index] = var_value;
	# end if
# end for

# output to a CSV!
line = "Variable,Aggregate,";

for i in range(0,max_index+1):
	line += str(i) + ",";

print line;

for var_name in variable_names:
	val_list = variable_values[var_name];
	line = var_name + "," + str(val_list[-1]) + ",";
	
	for i in range(0,max_index+1):
		if i in val_list.keys():
			line += str(val_list[i]);
		line += ",";
	
	print line;
# end for var_name

print "\n";
