/*
 * treesearch.h
 *
 *  Created on: Oct 8, 2009
 *      Author: derrickstolee
 */

#ifndef TREESEARCH_H_
#define TREESEARCH_H_

#include <stdio.h>

#define JOB_RUNNING 		0
#define JOB_ERROR 			1
#define JOB_DONE_FOUND 		2
#define JOB_DONE_NOTFOUND	3

typedef long long LONG_T;

typedef struct job_struct* job_ptr;
typedef struct job_struct
{
	int 	status;
	int 	label_size;
	LONG_T*	labels;
	int		job_depth;
	int		cur_depth;
	int 	buffer_size;
	char* 	buffer;
	int 	max_depth;

} job;


/** Constants **/
#define REQUEST_JOB 	1
#define NEW_JOB 		2
#define UPDATE_JOB 		3
#define COMPLETE_JOB	4
#define PARTIAL_JOB		5

#define JOB_OPEN		1

#define RESULT_NOT_FOUND 100
#define RESULT_FOUND	 200

/* all of these options can be ANDed together */
#define OPT_CHECKONLYLAST	1
#define OPT_USEPRUNE		2
#define	OPT_USETOCHILD		4

typedef struct job_option_struct* job_opt_ptr;
typedef struct job_option_struct
{
	/* initialize data for the given max depth */
	void 	(*init)(int);

	/* iterate to the next child of the given depth */
	LONG_T	(*nextChild)(int);

	/* move to the specified child of the given depth (depth,index) */
	LONG_T 	(*toChild)(int,LONG_T);

	/* check for pruning the current node at the specified depth */
	/* @return 0 if should not prune, 1 if should prune */
	int		(*prune)(int);

	/* run the final algorithm on the search node at the given depth */
	int		(*check)(int);

	/* write the data of the final solution to a buffer,
	 * 	return the buffer length  */
	int		(*writeSolution)(int, char**);

	/* write statistics that were gathered in this job */
	/* return the buffer length */
	int 	(*writeStatistics)(int, char**);

	/* write the data of the current job to a buffer,
	 * 	return the buffer length  */
	int		(*writeJob)(int, FILE*);

	/* clean up all data remnants */
	void	(*cleanup)(void);

	/* the number of stages in the search */
	int		stage_num;

	/* the stages to use */
	int*	stage_vals;

	/* maximum depth to search */
	int 	max_depth;

	/* distribute at this depth */
	int 	dist_depth;

	/* store the options OPT_ as bit flags */
	int 	mode;

	/* Time interval for sending updates (in seconds) */
	int		update_interval;

	/* Time interval since last update for assuming a job has died */
	int		kill_interval;
} job_opt;

/**
 * Set all options to defaults.
 */
int defaultOptions(job_opt_ptr options);

/**
 * Assuming currently at search node of given depth,
 * navigate through all subnodes.
 */
int searchSubtree(job_opt_ptr options, int depth, int cur_depth);

/**
 * Write files for job list.
 */
int generateJobs(job_opt_ptr options, job_ptr job_p);

/**
 * search through the job given by navigating to the
 * 	givne node.
 */
int searchJob(job_opt_ptr options, job_ptr job_p);

/**
 * Load a job from an input file.
 */
job_ptr loadJob(FILE* infile, job_opt_ptr options);

/**
 * Clean up a job object
 */
job_ptr freeJob(job_ptr job_p);

/**
 * Clean up a job options object
 */
job_ptr freeJobOptions(job_opt_ptr options);

#endif /* TREESEARCH_H_ */
