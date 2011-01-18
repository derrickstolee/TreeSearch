#!/usr/bin/python

#
# convertoutput.py
#
# Nov 3, 2009 -- Derrick Stolee
#
# Take the current list of output files and create a new set of 
# jobs.  To prevent repeating the job creation, files of the form
# out.* are moved to analyzed.*
#

import os

folder = "jobs/"

# remove all stalled/canceled jobs
os.system('ls ' + folder + ' | grep running > filelisting.txt')

files = open('filelisting.txt','r')
solfile = open(folder + 'allsols.txt','a')

job = 0

runningfilelines = files.readlines()

files.close()

for f in runningfilelines:
	fjob = f.strip()[8:]
	os.system('mv ' + folder + 'running.' + fjob + ' ' + folder + 'in.' + fjob)
	os.system('rm -rf ' + folder + 'out.' + fjob)


# find the current job number, if there exist some already
os.system('ls ' + folder + ' | grep out > filelisting.txt')

files = open('filelisting.txt','r')

job = 0

filelines = files.readlines()

files.close()

for f in filelines:
	fjob = int(f[4:])
	if fjob > job:
		job = fjob

# now test the infiles job number, too
os.system('ls ' + folder + ' | grep in. > filelisting.txt')

files = open('filelisting.txt','r')

job = 0

for f in files.xreadlines():
	fjob = int(f[3:])
	if fjob > job:
		job = fjob

files.close()
os.system('rm -rf filelisting.txt')

job = job + 1

initjob = job

# read all files, if a line starts with J or P, it is a new job!
for f in filelines:
	f = f.strip()
	
	# open this file out.*
	thefile = open(folder + f,"r")
	
	# read all lines
	for line in thefile.xreadlines():
		# if a full job or partial job
		if line[0] == 'J' or line[0] == 'P':
			# output as a new job in in.?
			outfile = open(folder + "in."+str(job),"w")
			outfile.write("J " + line[2:])
			outfile.close()
			job = job + 1
		if line[0] == 'S':
			solfile.write(line)
	thefile.close()
	
	# convert the out.* to analyzed.*
	os.system('mv ' + folder + f + ' ' + folder + 'analyzed.' + f[4:])
	
	# delete the associated input file
	os.system('rm ' + folder + 'in.' + f[4:])


solfile.close()

print "Created " + str(job - initjob) + " new jobs"