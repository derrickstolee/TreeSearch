/*
 * treesearch.c
 *
 *  Created on: Oct 8, 2009
 *      Author: derrickstolee
 */

#include "treesearch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

const int MAX_OUT_JOBS_IN_RUN_MODE = 1000;

job_opt_ptr GLOBAL_OPTIONS = NULL;

/**
 * Set all options to defaults.
 */
int defaultOptions( job_opt_ptr options )
{
	options->init = NULL;
	options->nextChild = NULL;
	options->toChild = NULL;
	options->prune = NULL;
	options->check = NULL;
	options->writeSolution = NULL;
	options->writeStatistics = NULL;
	options->writeJob = NULL;
	options->cleanup = NULL;

	options->max_depth = -1;
	options->dist_depth = 1;
	options->mode = 0;
	options->update_interval = 60;
	options->kill_interval = 500;

	options->stage_num = 0;
	options->stage_vals = 0;

	return 0;
}

/* search initialization infos! */
LONG_T* child_vals = NULL;
int num_child_vals = 0;

/***********************************************/
/***** SUPPORT METHODS FOR GENERATEJOBS ********/
/***********************************************/

int numJobs = 0;

int writeJobToFile( int depth, FILE* outfile )
{
	/* format: job_depth, cur_depth */
	fprintf(outfile, "J %d %d ", depth, depth);

	/* then, child vals through the list */
	int i;
	for ( i = 0; i < depth; i++ )
	{
		fprintf(outfile, "%llX ", child_vals[i]);
	}
	fprintf(outfile, "\n");

	if ( GLOBAL_OPTIONS->writeJob != NULL )
	{
		GLOBAL_OPTIONS->writeJob(depth, outfile);
	}

	numJobs++;

	return 1;
}

int writeJob( int depth )
{
	/*
	 char filename[200];
	 sprintf(filename, "in.%d", numJobs);

	 FILE* outfile = fopen(filename, "w");
	 */

	int res = writeJobToFile(depth, stdout);

	/*
	 fclose(outfile);
	 */
	return res;
}

/**
 * Write files for job list.
 */
int generateJobs( job_opt_ptr options, job_ptr job_p )
{
	if ( options->max_depth < 0 )
	{
		fprintf(stderr, "Error: To generate jobs, a MAX_DEPTH must be set.\n");
		return -1;
	}

	GLOBAL_OPTIONS = options;

	/* overwrite the check with writeJob */
	options->check = &writeJob;
	options->writeSolution = NULL; /* */

	/* only write when at the last value */
	options->mode |= OPT_CHECKONLYLAST;
	options->kill_interval = 0;

	if ( options->init != NULL )
	{
		if ( job_p != NULL && job_p->max_depth > 0 )
		{
			options->init(job_p->max_depth);
			num_child_vals = job_p->max_depth;
			options->max_depth = job_p->max_depth;
		}
		else if ( options->max_depth > 0 )
		{
			options->init(options->max_depth);
			num_child_vals = options->max_depth;
		}
		else if ( job_p != NULL && job_p->cur_depth > job_p->job_depth )
		{
			options->init(job_p->cur_depth);
			num_child_vals = job_p->cur_depth;
		}
		else if ( job_p != NULL )
		{
			options->init(job_p->job_depth * 2);
			num_child_vals = 2 * job_p->job_depth;
		}
		else
		{
			fprintf(stderr, "Error: Must specify a maximum depth during a generate command.\n");
			exit(1);
		}
	}

	int res = 0;

	/* initialize the list of child values */
	child_vals = (LONG_T*) malloc(num_child_vals * sizeof(LONG_T));

	bzero(child_vals, num_child_vals * sizeof(LONG_T));

	if ( job_p != NULL )
	{
		/* we've got a partial job already! */
		int i, j;

		/* get search node to the one specified by the job */
		if ( options->toChild != NULL )
		{
			for ( i = 0; i < job_p->cur_depth; i++ )
			{
				/* at depth i, move to child given by label */
				child_vals[i] = options->toChild(i, job_p->labels[i]);

				if ( child_vals[i] < 0 )
				{
					/* BIG PROBLEMS */
					fprintf(stderr, "Error: Could not advance to the proper child at depth %d (%lld)!\n", i,
					        job_p->labels[i]);
					break;
				}
			}
		}
		else
		{
			for ( i = 0; i < job_p->cur_depth; i++ )
			{
				/* at depth i, move to child given by label, using sequential items */
				int child = -1;
				for ( j = 0; j <= job_p->labels[i] && child < job_p->labels[i]; j++ )
				{
					child = options->nextChild(i);

					if ( child == -1 )
					{
						/* BIG PROBLEMS */
						printf("Error: Could not advance to the proper child at depth %d (%lld)!\n", i,
						       job_p->labels[i]);
						return -1;
					}
				}

				child_vals[i] = child;
			}
		}

		/* use regular search to complete the method */
		res = searchSubtree(options, i, job_p->cur_depth);
		free(child_vals);
	}
	else
	{
		res = searchSubtree(options, 0, 0);
		free(child_vals);
	}

	if ( options->cleanup != NULL )
	{
		options->cleanup();
	}

	return res;
}

/***********************************************/
/******* METHOD FOR ALLOCATED SUB-JOB **********/
/***********************************************/

int num_jobs_output = 0;

/**
 * Assuming currently at search node of given cur_depth
 * as a partial solution to the subtree at depth,
 * navigate through all subnodes.
 */
int searchSubtree( job_opt_ptr options, const int min_depth, int cur_depth )
{
	int MAX_DEPTH = -1;
	num_jobs_output = 0;

	if ( options->max_depth > 0 )
	{
		MAX_DEPTH = options->max_depth;
	}

	time_t start_time = time(NULL);

	time_t kill_time = -1;
	int do_kill = 0;

	if ( options->kill_interval > 0 )
	{
		kill_time = start_time + options->kill_interval;
		do_kill = 1;
	}

	int i;
	int status = NEW_JOB;

	/* determine stage */
	int stage = -1;
	int stage_up = -1;

	if ( options->stage_num > 0 )
	{
		for ( i = 0; i + 1 < options->stage_num; i++ )
		{
			if ( min_depth >= options->stage_vals[i] && min_depth < options->stage_vals[i + 1] )
			{

				printf("--Stage %d because %d <= %d < %d.\n", i + 1, options->stage_vals[i], min_depth,
				       options->stage_vals[i + 1]);
				stage = i + 1;
				stage_up = options->stage_vals[i + 1];
			}
		}
	}

	/* now, do the search. */
	int last_down_direction = -1;
	int moving_up = 0;

	while ( cur_depth >= min_depth && (do_kill == 0 || time(NULL) < kill_time) )
	{
		/* get next child */
		LONG_T child = options->nextChild(cur_depth);

		if ( child < 0 )
		{
			/* no more children here! */
			/* pop the stack and start over */
			if ( moving_up == 0 )
			{
				moving_up = 1;
				last_down_direction = cur_depth;
			}

			cur_depth--;

			continue;
		}

		/* going down to this depth is just fine! */
		moving_up = 0;
		last_down_direction = cur_depth;

		/* put this child number in the list! */
		if ( cur_depth >= num_child_vals )
		{
			num_child_vals += 5;
			child_vals = (LONG_T*) realloc(child_vals, num_child_vals * sizeof(LONG_T));
		}
		child_vals[cur_depth] = child;

		/* attempt pruning */
		if ( options->prune != NULL && options->prune(cur_depth + 1) != 0 )
		{
			/* need to prune this child! */
			/* move to the next one... */
			continue;
		}

		if ( MAX_DEPTH < 0 || cur_depth + 1 < MAX_DEPTH )
		{
			/* We can deepen the search */
			cur_depth++;

			if ( (options->mode & OPT_CHECKONLYLAST) == 0 )
			{
				/* We don't need to wait for MAX_DEPTH to run the check */
				int res = options->check(cur_depth);

				if ( res != 0 )
				{
					/* check returned successfully! */
					char* buffer = NULL;
					int buffer_size = 0;

					if ( options->writeSolution != NULL )
					{
						/* solution found! */
						buffer_size = options->writeSolution(cur_depth, &buffer);
					}
					status = RESULT_FOUND;

					/* now, print a solution! */
					printf("S %d %d ", min_depth, cur_depth);

					/* print the subtree path */
					for ( i = 0; i <= cur_depth; i++ )
					{
						printf("%llX ", child_vals[i]);
					}
					printf("\n");

					if ( buffer_size > 0 && buffer != NULL )
					{
						printf("%s\n", buffer);
						free(buffer);
					}

					/* don't stop won't stop can't stop */

					/* move up the tree so the child can be moved */
					cur_depth--;
					continue;
				}
			}

			/* ok, this node is fine, so go to the next child! */
		}
		else if ( cur_depth + 1 >= MAX_DEPTH )
		{
			/* we are at the stopping point */
			int res = options->check(cur_depth + 1);

			if ( res != 0 )
			{
				char* buffer = NULL;
				int buffer_size = 0;

				if ( options->writeSolution != NULL )
				{
					/* solution found! */
					buffer_size = options->writeSolution(cur_depth, &buffer);
				}
				status = RESULT_FOUND;

				/* now, print a solution! */
				printf("S %d %d ", min_depth, cur_depth);

				/* print the subtree path */
				for ( i = 0; i <= cur_depth; i++ )
				{
					printf("%llX ", child_vals[i]);
				}
				printf("\n");

				if ( buffer_size > 0 && buffer != NULL )
				{
					printf("%s\n", buffer);
					free(buffer);
				}
			}

			/* this child at depth cur_depth+1 is done, so don't decrease cur_depth */
			continue;
		}

		if ( stage_up > 0 && cur_depth >= stage_up )
		{
			if ( options->check != &writeJob && num_jobs_output > MAX_OUT_JOBS_IN_RUN_MODE )
			{
				/* killed by too many jobs! */
				/* print out the partial value */
				printf("P %d %d ", min_depth, last_down_direction);

				for ( i = 0; i <= last_down_direction; i++ )
				{
					printf("%llX ", child_vals[i]);
				}

				//				printf("%llX ", 0);
				printf("\n");

				if ( options->writeStatistics != NULL )
				{
					char* buffer = NULL;
					int b_size = options->writeStatistics(cur_depth, &buffer);

					if ( b_size > 0 )
					{
						printf("%s\n", buffer);

						free(buffer);
					}
				}

				return status;
			}

			/* we have exceeded the depth of this stage! */
			writeJobToFile(cur_depth, stdout);
			num_jobs_output++;

			cur_depth--;
		}
	}

	if ( cur_depth > min_depth )
	{
		/* killed by the time! */
		/* print out the partial value */
		printf("P %d %d ", min_depth, last_down_direction);

		for ( i = 0; i <= last_down_direction; i++ )
		{
			printf("%llX ", child_vals[i]);
		}

		//		printf("%llX ", 0);
		printf("\n");
	}
	else
	{
		printf("C %d %d ", min_depth, min_depth);

		for ( i = 0; i < min_depth; i++ )
		{
			printf("%llX ", child_vals[i]);
		}
		printf("\n");
	}

	if ( options->writeStatistics != NULL )
	{
		char* buffer = NULL;
		int b_size = options->writeStatistics(cur_depth, &buffer);

		if ( b_size > 0 )
		{
			printf("%s\n", buffer);

			free(buffer);
		}
	}

	return status;
}

/**
 * Search through an allocated job.
 */
int searchJob( job_opt_ptr options, job_ptr job_p )
{
	/* initialize the search */
	int MAX_DEPTH = -1;
	if ( options->init != NULL )
	{
		if ( job_p->max_depth > 0 )
		{
			options->init(job_p->max_depth);
			MAX_DEPTH = job_p->max_depth;
			num_child_vals = job_p->max_depth;
			options->max_depth = job_p->max_depth;
		}
		else if ( options->max_depth > 0 )
		{
			options->init(options->max_depth);
			MAX_DEPTH = options->max_depth;
			num_child_vals = options->max_depth;
		}
		else if ( job_p->cur_depth > job_p->job_depth )
		{
  			options->init(job_p->cur_depth * 2);
			num_child_vals = 2 * job_p->cur_depth;
		}
		else
		{
			options->init(job_p->job_depth * 2);
			num_child_vals = 2 * job_p->job_depth;
		}
	}

	GLOBAL_OPTIONS = options;

	child_vals = (LONG_T*) malloc(num_child_vals * sizeof(LONG_T));

	bzero(child_vals, num_child_vals * sizeof(LONG_T));

	int i, j;

	/* get search node to the one specified by the job */
	if ( options->toChild != NULL )
	{
		for ( i = 0; i < job_p->cur_depth; i++ )
		{
			/* at depth i, move to child given by label */
			child_vals[i] = options->toChild(i, job_p->labels[i]);

			if ( child_vals[i] < 0 )
			{
				/* BIG PROBLEMS */
				fprintf(stderr, "--Error: Could not advance to the proper child at depth %d (%llX)!\n", i,
				        job_p->labels[i]);
				break;
			}

			if ( options->prune != NULL && options->prune(i) != 0 )
			{
				/* BIG PROBLEMS */
				fprintf(stderr, "--Error: Had to prune at child for depth %d (%llX)!\n", i, job_p->labels[i]);
				break;
			}
		}
	}
	else
	{
		for ( i = 0; i < job_p->cur_depth; i++ )
		{
			/* at depth i, move to child given by label, using sequential items */
			int child = -1;
			for ( j = 0; j <= job_p->labels[i] && child < job_p->labels[i]; j++ )
			{
				child = options->nextChild(i);

				if ( child == -1 )
				{
					/* BIG PROBLEMS */
					fprintf(stderr, "--Error: Could not advance to the proper child at depth %d (%llX)!\n", i,
					        job_p->labels[i]);
					return -1;
				}
			}

			child_vals[i] = child;

			if ( options->prune != NULL && options->prune(i) != 0 )
			{
				/* BIG PROBLEMS */
				fprintf(stderr, "--Error: Had to prune at child for depth %d (%llX)!\n", i, job_p->labels[i]);
				break;
			}
		}
	}

	/* use regular search to complete the method */
	int res = searchSubtree(options, job_p->job_depth, job_p->cur_depth);

	free(child_vals);

	if ( options->cleanup != NULL )
	{
		options->cleanup();
	}

	return res;
}

job_ptr loadJob( FILE* infile, job_opt_ptr options )
{
	job_ptr j = (job_ptr) malloc(sizeof(job));

	char type;
	fscanf(infile, "%c", &type);

	if ( type != 'J' && type != 'P' )
	{
		return 0;
	}

	fscanf(infile, "%d", &j->job_depth);
	fscanf(infile, "%d", &j->cur_depth);

	if ( type == 'P' )
	{
		/* we need one extra step here */
		j->cur_depth++;
	}

	j->label_size = j->cur_depth + 1;
	j->labels = (LONG_T*) malloc(j->label_size * sizeof(LONG_T));

	int i;
	for ( i = 0; i < j->cur_depth; i++ )
	{
		fscanf(infile, "%llX", &(j->labels[i]));
	}

	j->max_depth = options->max_depth;
	j->buffer = 0;
	j->buffer_size = 0;
	j->status = NEW_JOB;

	return j;
}

/**
 * Clean up a job object
 */
job_ptr freeJob( job_ptr job_p )
{
	if ( job_p != NULL )
	{
		if ( job_p->labels != NULL )
		{
			free(job_p->labels);
			job_p->labels = 0;
		}

		if ( job_p->buffer != NULL )
		{
			free(job_p->buffer);
			job_p->buffer = 0;
		}
	}
}

/**
 * Clean up a job options object
 */
job_ptr freeJobOptions( job_opt_ptr options )
{
	if ( options != NULL )
	{
		if ( options->stage_vals != NULL )
		{
			free(options->stage_vals);
			options->stage_vals = 0;
		}
	}
}
