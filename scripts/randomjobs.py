#!/usr/bin/python
#
# randomjobs.py
#
# Nov 8, 2010 -- Derrick Stolee
#
# Take the current list of jobs and randomly select a given number of them.
# 
# Consider these jobs to be the "expanded" jobs.
#

import os
import sys
import random

folder = "./"
targetfolder = None;

GROUP_JOBS = 1
RANDOM_COUNT = 50;

#get folder, max jobs, and group jobs from args
for arg in sys.argv:
	if arg.startswith('-F'):
		folder = arg[2:];
	if arg.startswith('-T'):
		targetfolder = arg[2:];
	if arg.startswith('-G'):
		GROUP_JOBS = int(arg[2:]);
	if arg.startswith('-R'):
		RANDOM_COUNT = int(arg[2:]);

if targetfolder == None:
	targetfolder = folder;

jobfile = open(folder + 'alljobs.txt','r')
outjobs = open(targetfolder + 'backjobs.txt','w')


TOTAL_COUNT = 0;
for line in jobfile.xreadlines():
	TOTAL_COUNT = TOTAL_COUNT + 1;	
jobfile.close()

if TOTAL_COUNT < RANDOM_COUNT:
	print "There are only " + str(TOTAL_COUNT) + " jobs but you are trying to select " + str(RANDOM_COUNT) + " random jobs";
	exit(1);

# now, select randomly 
random_list = [];
random.seed();
for i in range(0, RANDOM_COUNT):
	r = int(random.uniform(0,TOTAL_COUNT));
	while ( r in random_list ):
		r = int(random.uniform(0,TOTAL_COUNT));
	random_list.append(r);
	

random_list.sort();

job = 0;
rjob = 0;
rjobfile = 0;
outjobcount = 0;
outfile = None;

jobfile = open(folder + 'alljobs.txt','r')
for line in jobfile.xreadlines():
	if (line[0] == 'J' or line[0] == 'P'):
		if job in random_list:
			if ( GROUP_JOBS == 1 or (rjob % GROUP_JOBS) == 0 ):	
				outfile = open(targetfolder + 'in.' + str(rjobfile), 'w');
				rjobfile = rjobfile + 1;
			# in the first MAX_JOB jobs
			outfile.write(line)
			
			if ( GROUP_JOBS == 1 or (rjob % GROUP_JOBS) == GROUP_JOBS - 1 or rjob == RANDOM_COUNT - 1 ):	
				outfile.close()
				
			rjob = rjob + 1;
		else:
			# too many jobs queued already!
			outjobs.write(line)
			outjobcount = outjobcount + 1
		job = job + 1		
jobfile.close()

print "Expanded " + str(rjob) + " jobs in " + str(rjobfile) + " files, leaving " + str(outjobcount) + " queued."

# set up the submission script
os.system('cat ' + str(folder) + 'condorsubmit.sub.tmp | sed s/\\$QUEUE_SIZE/' + str(rjobfile) + '/ > ' + targetfolder + 'condorsubmit.sub')

 
