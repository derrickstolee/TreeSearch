/*
 * runjob.c
 *
 *  Created on: Oct 8, 2009
 *      Author: derrickstolee
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "treesearch.h"

void example_init(int depth);
LONG_T example_nextChild(int depth);
int example_check(int depth);
void example_cleanup();

int main(int argc, char** argv)
{
	if ( argc < 2 )
	{
		fprintf(stderr, "Usage: example mode [max_depth]\n");
		fprintf(stderr, "modes: generate run\n");
		fprintf(stderr, "\tGenerate requires a max_depth.\n");
		return 1;
	}

	int mode = 0;

	if ( strcmp("generate", argv[1]) == 0 )
	{
		mode = 1;
	}

	job_opt options;
	defaultOptions(&options);

	/* HERE: set the methods to use */
	options.init = &example_init;
	options.nextChild = &example_nextChild;
	options.check = &example_check;
	options.cleanup = &example_cleanup;


	options.stage_num = 4;
	options.stage_vals = (int*)malloc(options.stage_num * sizeof(int));

	options.stage_vals[0] = 3;
	options.stage_vals[1] = 6;
	options.stage_vals[2] = 8;
	options.stage_vals[3] = 12;

	options.kill_interval = 60L; /* one minute */

	options.max_depth = 16;

	int i = 1;
	mode = -1;

	while ( i < argc )
	{
		if ( strcmp("-k", argv[i]) == 0 )
		{
			/* -k KILLTIME */
			if ( i == argc - 1 )
			{
				fprintf(stderr, "Argument -k needs an integer following.\n");
				exit(1);
			}

			options.kill_interval = atoi(argv[i + 1]);
			i += 2;
		}
		else if ( strcmp("-m", argv[i]) == 0 )
		{
			if ( i == argc - 1 )
			{
				fprintf(stderr, "Argument -m needs an integer following.\n");
				exit(1);
			}

			options.max_depth = atoi(argv[i + 1]);
			i += 2;
		}
		else if ( strcmp("generate", argv[i]) == 0 )
		{
			if ( mode < 0 )
			{
				mode = 1; /* GENERATE */
			}
			else
			{
				printf("Cannot have two modes!\n");
			}

			i++;
		}
		else if ( strcmp("run", argv[i]) == 0 )
		{
			if ( mode < 0 )
			{
				mode = 0; /* RUN A JOB */
			}
			else
			{
				printf("Cannot have two modes!\n");
			}

			i++;
		}
		else if ( strcmp("-s", argv[i]) == 0 )
		{
			if ( i < argc - 1 )
			{
				options.stage_num = atoi(argv[i + 1]);
				options.stage_vals = (int*) malloc(options.stage_num * sizeof(int));

				if ( i + options.stage_num + 1 < argc )
				{
					int j = 0;
					for ( j = 0; j < options.stage_num; j++ )
					{
						options.stage_vals[j] = atoi(argv[i + 2 + j]);
					}

					i += options.stage_num + 2;
				}
				else
				{
					i++;
					options.stage_num = -1;
					free(options.stage_vals);
				}
			}
		}
		else
		{
			/* unrecognized... */
			printf("Unrecognized argument: %s\n", argv[i]);
			i++;
		}
	}

	return 0;
}

int last_depth = 0;
int max_depth = 0;
LONG_T* cur_children = 0;

void example_init(int depth)
{
	cur_children = (LONG_T*) realloc(cur_children, depth * sizeof(LONG_T));
	bzero(&cur_children[last_depth], (depth - last_depth) * sizeof(LONG_T));
	max_depth = depth;
}

LONG_T example_nextChild(int depth)
{
	if ( depth > max_depth )
	{
		return -1;
	}

	if ( depth > last_depth )
	{
		cur_children[depth] = 0;
	}
	else
	{
		cur_children[depth]++;

		if ( cur_children[depth] >= 3 )
		{
			/* 3 children in total */
			cur_children[depth] = 0;

			/* -1 means there are no more children here! */
			return -1;
		}
	}

	last_depth = depth;
	return cur_children[depth];
}

int example_check(int depth)
{
	if ( depth > 9 && (cur_children[depth - 1] % 2) == 0 )
	{
		/* a very lame example! */
		return 1;
	}

	return 0;
}

void example_cleanup()
{
	if ( cur_children != 0 )
	{
		free(cur_children);
		cur_children = 0;
		last_depth = 0;
	}
}
