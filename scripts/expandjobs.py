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
import sys

folder = "./"
targetfolder = None;

MAX_JOBS = 1000
GROUP_JOBS = 1

#get folder, max jobs, and group jobs from args
for arg in sys.argv:
	if arg.startswith('-F'):
		folder = arg[2:];
	if arg.startswith('-T'):
		targetfolder = arg[2:];
	if arg.startswith('-M'):
		MAX_JOBS = int(arg[2:]);
	if arg.startswith('-G'):
		GROUP_JOBS = int(arg[2:]);

if targetfolder == None:
	targetfolder = folder;


jobfile = open(folder + 'alljobs.txt','r')
outjobs = open(targetfolder + 'backjobs.txt','w')

job = 0
outjobcount = 0 
outfile = None;

for line in jobfile.xreadlines():
	if (line[0] == 'J' or line[0] == 'P'):
		if  job < MAX_JOBS:
			if ( GROUP_JOBS == 1 or (job % GROUP_JOBS) == 0 ):	
				outfile = open(targetfolder + 'in.' + str(job), 'w')
			# in the first MAX_JOB jobs
			outfile.write(line)
			
			if ( GROUP_JOBS == 1 or (job % GROUP_JOBS) == GROUP_JOBS - 1 or job == MAX_JOBS - 1 ):	
				outfile.close()
				
			job = job + 1
		else:
			# too many jobs queued already!
			outjobs.write(line)
			outjobcount = outjobcount + 1
			
jobfile.close()

print "Expanded " + str(job) + " jobs, leaving " + str(outjobcount) + " queued."

# set up the submission script
os.system('cat ' + str(folder) + 'condorsubmit.sub.tmp | sed s/\\$QUEUE_SIZE/' + str(job) + '/ > ' + targetfolder + 'condorsubmit.sub')

 
