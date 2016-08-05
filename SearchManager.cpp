/***********************************************************

 Copyright Derrick Stolee 2011.

 This file is part of SearchLib.

 SearchLib is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 SearchLib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with SearchLib.  If not, see <http://www.gnu.org/licenses/>.

 *************************************************************/

/*
 * SearchManager.cpp
 *
 *  Created on: Feb 26, 2010
 *      Author: derrickstolee
 */

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <list>
#include "SearchManager.hpp"

/** SIGNAL HANDLER **/
SearchManager* global_manager = 0;

void term_signal(int sig)
{
	if ( global_manager != 0 )
	{
		/* global_manager->stop(); */
		global_manager->writePartialJob(stdout);
		char* buffer = global_manager->writeStatistics();

		if ( buffer != 0 )
		{
			fprintf(stdout, "%s\n", buffer);
			free(buffer);
		}

		fflush(stdout);
	}

	exit(0);
}

SearchNode::SearchNode(LONG_T label)
{
	this->label = label;
	this->curChild = -1;
}

SearchNode::SearchNode(SearchNode& sn)
{
	this->label = sn.label;
	this->curChild = sn.curChild;
}

SearchNode::~SearchNode()
{
}

/**
 * Copy request.  This allows a deep copy of SearchManager.
 */
SearchNode* SearchNode::copy()
{
	return new SearchNode(*this);
}

/**
 * Default constructor.
 *
 * Part of the constructor for an extension must
 *  insert the root node in to the stack.
 */
SearchManager::SearchManager()
{
	this->searchDepth = -1;
	this->root = new SearchNode(0);
	this->maxSolutions = -1;
	this->foundSolutions = 0;
	this->numStages = 0;
	this->stages = 0;
	this->jobLabels = 0;
	this->num_nodes = 0;
	this->end_time = 0;
	this->partialDepth = 0;
	this->labelSize = 0;
	this->deepeningMode = 0;
	this->time_in_search = 0;
	this->killtime = 0;
	this->num_prunes = 0;
	this->jobDepth = 0;
	this->numJobsFound = 0;
	this->maxSolutions = 0;
	this->maxJobsFound = 100;
	this->maxDepth = -1;
	this->haltAtSolutions = false;
}

/**
 * Copy constructor
 */
SearchManager::SearchManager(SearchManager& sm)
{
	int i;
	this->root = new SearchNode(0);
	this->jobDepth = sm.jobDepth;
	this->partialDepth = sm.partialDepth;
	this->maxSolutions = sm.maxSolutions;
	this->foundSolutions = sm.foundSolutions;
	this->numJobsFound = sm.numJobsFound;
	this->haltAtSolutions = false;
	this->jobLabels = 0;

	if ( sm.partialDepth > 0 )
	{
		this->jobLabels = (LONG_T*) malloc(sm.partialDepth * sizeof(LONG_T));

		for ( i = 0; i < sm.partialDepth; i++ )
		{
			this->jobLabels[i] = sm.jobLabels[i];
		}
	}

	this->killtime = sm.killtime;
	this->end_time = sm.end_time;
	this->deepeningMode = sm.deepeningMode;
	this->maxDepth = sm.maxDepth;

	this->numStages = sm.numStages;

	if ( sm.numStages > 0 )
	{
		this->stages = (int*) malloc(sm.numStages * sizeof(int));

		for ( i = 0; i < sm.numStages; i++ )
		{
			this->stages[i] = sm.stages[i];
		}
	}

}

/**
 * Destructor
 */
SearchManager::~SearchManager()
{
	if ( this->jobLabels != 0 )
	{
		free(this->jobLabels);
	}
	if ( this->stages != 0 )
	{
		free(this->stages);
	}

	delete this->root;

	this->stack.clear();
}

/**
 * importArguments -- take the command line arguments
 * 	and convert them into options.
 *
 * This includes the following options:
 * 	mode: generate or run
 *  stages: -s [count] [s0] [s1] ... [sk]
 *  killtime: -k [killtime]
 *  maxdepth: -m [maxdepth]
 *  maxsolutiosn: --maxsols [maxsolutions]
 *
 *  It is set as so that one can strip
 *   custom arguments, if necessary.
 */
void SearchManager::importArguments(int argc, char** argv)
{
	if ( argc < 2 )
	{
		fprintf(stderr, "Usage: [program] mode [max_depth]\n");
		fprintf(stderr, "modes: generate run\n");
		fprintf(stderr, "\tGenerate requires a max_depth.\n");
		return;
	}

	int mode = 0;

	int i = 1;
	mode = -1;
	this->maxDepth = 0;

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

			this->killtime = atoi(argv[i + 1]);
			i += 2;
		}
		else if ( strcmp("-m", argv[i]) == 0 )
		{
			if ( i == argc - 1 )
			{
				fprintf(stderr, "Argument -m needs an integer following.\n");
				exit(1);
			}

			this->maxDepth = atoi(argv[i + 1]);
			i += 2;
		}
		else if ( strcmp("generate", argv[i]) == 0 )
		{
			if ( mode < 0 )
			{
				mode = 1; /* GENERATE */
				this->deepeningMode = DEEPEN_GENERATE;
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
				this->deepeningMode = DEEPEN_FULL;
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
				this->numStages = atoi(argv[i + 1]);
				if ( i + this->numStages + 1 < argc )
				{
					this->stages = (int*) malloc(this->numStages * sizeof(int));

					int j = 0;
					for ( j = 0; j < this->numStages; j++ )
					{
						this->stages[j] = atoi(argv[i + 2 + j]);
					}

					i += this->numStages + 2;
				}
				else
				{
					i++;
					this->numStages = 0;
					this->stages = 0;
				}
			}
		}
		else if ( strcmp("--maxsols", argv[i]) == 0 )
		{
			if ( i < argc - 1 )
			{
				this->maxSolutions = atoi(argv[i + 1]);

				i += 2;
			}
			else
			{
				i++;
			}
		}
		else if ( strcmp("--maxjobs", argv[i]) == 0 )
		{
			if ( i < argc - 1 )
			{
				this->maxJobsFound = atoi(argv[i + 1]);

				i += 2;
			}
			else
			{
				i++;
			}
		}
		else
		{
			/* unrecognized... */
			// printf("Unrecognized argument: %s\n", argv[i]);
			i++;
		}
	}

	if ( (this->deepeningMode & DEEPEN_GENERATE) != 0 && this->maxDepth == 0 )
	{
		printf("[importArguments] Need a maxDepth with generate mode!\n");
		exit(1);
	}

	if ( this->maxDepth == 0 )
	{
		this->maxDepth = 5000;
	}

	this->jobLabels = (LONG_T*) malloc(this->maxDepth * sizeof(LONG_T));
}

/**
 * readJob -- read a job description
 *  from a given input stream.
 *  This will also navigate to the proper search node.
 *
 * After reading the job, call doSearch() to perform the
 * 	search from that position.
 *
 * @param file The input stream.
 * @return 1 for success, -1 for no success.
 */
int SearchManager::readJob(FILE* file)
{
	char type[100];
	int i;

	if ( fscanf(file, "%s", type) == EOF )
	{
		//		printf("--[doSearch] end of file...\n");
		return -1;
	}

	if ( type[0] != 'J' && type[0] != 'P' )
	{
		//		printf("--[doSearch] not a job %s...\n", type);
		return -1;
	}

	/* TODO: make this input error-sensitive */
	fscanf(file, "%d", &(this->jobDepth));
	fscanf(file, "%d", &(this->partialDepth));

	this->labelSize = this->maxDepth;

	if ( this->labelSize < this->partialDepth )
	{
		this->labelSize = this->partialDepth;
	}

	if ( this->jobLabels != 0 )
	{
		free(this->jobLabels);
		this->jobLabels = 0;
	}

	this->jobLabels = (LONG_T*) malloc(this->labelSize * sizeof(LONG_T));
	bzero(this->jobLabels, this->labelSize * sizeof(LONG_T));

	for ( i = 0; i < this->partialDepth; i++ )
	{
		fscanf(file, "%llX", &(this->jobLabels[i]));
	}

	if ( this->jobDepth > 0 )
	{
		this->deepeningMode = this->deepeningMode | DEEPEN_JOB;
	}

	if ( this->partialDepth > 0 )
	{
		this->deepeningMode = this->deepeningMode | DEEPEN_PARTIAL;
	}

	this->searchDepth = -3;
	this->foundSolutions = 0;
	this->numJobsFound = 0;

	return 1;
}

void SearchManager::loadEmptyJob()
{
	this->jobDepth = 0;
	this->partialDepth = 0;
	this->searchDepth = -3;
	this->foundSolutions = 0;
	this->numJobsFound = 0;

	this->deepeningMode = 0;
	this->labelSize = this->maxDepth;
	this->jobLabels = (LONG_T*) malloc(this->labelSize * sizeof(LONG_T));
	bzero(this->jobLabels, this->labelSize * sizeof(LONG_T));
}

/**
 * writeJob -- write a job description from the
 *  current stack of SearchNodes.
 */
void SearchManager::writeJob(FILE* file)
{
	fprintf(file, "J %d %d ", this->searchDepth, this->searchDepth);

	int j = 0;

	while ( j < this->searchDepth )
	{
		fprintf(file, "%llX ", this->jobLabels[j]);
		j++;
	}

	fprintf(file, "\n");
	fflush(file);
}

/**
 * writePartialJob -- write a partial job description from the
 *  current stack of SearchNodes, using
 *  the current jobDepth as the transfer from
 *  job to partial nodes.
 */
void SearchManager::writePartialJob(FILE* file)
{
	int jd = this->jobDepth;

	if ( this->searchDepth < jd )
	{
		jd = this->searchDepth;
	}

	fprintf(file, "P %d %d ", jd, this->searchDepth);

	int j = 0;

	while ( j < this->searchDepth )
	{
		fprintf(file, "%llX ", this->jobLabels[j]);
		j++;
	}

	fprintf(file, "\n");
	fflush(file);
}

/**
 * writeCompleteJob -- write a job description from the
 *  current stack of SearchNodes, using
 *  the current jobDepth as the transfer from
 *  job to partial nodes.
 */
void SearchManager::writeCompleteJob(FILE* file)
{
	fprintf(file, "C %d %d ", (int) this->jobDepth, (int) this->jobDepth);

	for ( int i = 0; i < this->jobDepth; i++ )
	{
		fprintf(file, "%llX ", this->jobLabels[i]);
	}

	fprintf(file, "\n");
	fflush(file);
}

/**
 * writeSolutionJob -- write a job description from the
 *  current stack of SearchNodes, using
 *  the current jobDepth as the transfer from
 *  job to partial nodes.
 */
void SearchManager::writeSolutionJob(FILE* file)
{
	fprintf(file, "S %d %d ", (int) this->jobDepth, (int) this->searchDepth);

	int j = 0;

	while ( j < this->searchDepth )
	{
		fprintf(file, "%llX ", this->jobLabels[j]);
		j++;
	}

	fprintf(file, "\n");
	fflush(file);
}

/**
 * doSearch -- recursively perform the search from the current node.
 *
 * This method will pop() before any returning state.
 *
 * @return 1 if a solution is found, 0 if not, -1 if timeout.
 */
int SearchManager::doSearch()
{
	/* check for timing! */
	time_t cur_time = time(NULL);

	if ( this->searchDepth < -1 )
	{
		/* signal handling! */
		global_manager = this;
		signal(SIGTERM, term_signal);

		/* this must be the start */
		this->end_time = cur_time + this->killtime;

		/* check the partial depth and find the stage, if necessary */
		if ( this->numStages > 0 )
		{
			int i;
			for ( i = 0; i < this->numStages; i++ )
			{
				if ( this->stages[i] > this->partialDepth )
				{
					this->maxDepth = this->stages[i];

					/* we are done in this loop */
					break;
				}
			}

			if ( i >= this->numStages && this->maxDepth == 0 )
			{
				this->maxDepth = 2500;
			}
		}

		this->searchDepth = -1;
		this->root->curChild = -1;

		this->initBaseStats();

		/* perform the full search */
		clock_t search_start_clock = clock();
		time_t search_start_time = time(NULL);
		int result = this->doSearch();
		clock_t search_end_clock = clock();
		time_t search_end_time = time(NULL);

		if ( search_end_time - search_start_time > 100 )
		{
			this->time_in_search = search_end_time - search_start_time;
		}
		else
		{
			this->time_in_search = (search_end_clock - search_start_clock) / (double) CLOCKS_PER_SEC;
		}

		char* buffer = this->writeStatistics();

		if ( buffer != 0 )
		{
			fprintf(stdout, "%s\n", buffer);
			free(buffer);
			fflush(stdout);
		}

		global_manager = 0;
		return result;
	} /* end initialization step */

	/* before anything, are we out of time? */
	if ( cur_time > this->end_time )
	{
		(this->searchDepth)++;
		this->writePartialJob(stdout);
		(this->searchDepth)--;
		return -1;
	}

	/* push the depth down */
	this->searchDepth = this->searchDepth + 1;

	/* before pushing, are we at the end of a generation? */
	if ( (this->deepeningMode & DEEPEN_GENERATE) != 0 )
	{
		/* test the final node */
		if ( this->searchDepth >= this->maxDepth )
		{
			this->numJobsFound = this->numJobsFound + 1;
			this->deepeningMode = (this->deepeningMode) & ~((int) (DEEPEN_JOB | DEEPEN_PARTIAL));

			if ( this->prune() == 0 )
			{
				if ( this->numJobsFound >= this->maxJobsFound )
				{
					/* found max number! */
					int temp = this->partialDepth;
					this->partialDepth = this->searchDepth;
					this->writePartialJob(stdout);
					this->partialDepth = temp;

					this->pop();
					this->searchDepth = this->searchDepth - 1;

					return -1;
				}
				else if ( this->numJobsFound == 1 && this->maxDepth < this->partialDepth )
				{
					/* we had a partial job, and this is the FIRST job */
					/* we should output a partial job with a modified jobDepth */
					int tjd = this->jobDepth;
					int tsd = this->searchDepth;
					this->jobDepth = this->maxDepth;
					this->searchDepth = this->partialDepth;

					this->writePartialJob(stdout);

					this->jobDepth = tjd;
					this->searchDepth = tsd;
				}
				else
				{
					this->writeJob(stdout);
				}
			}

			this->pop();
			this->searchDepth = this->searchDepth - 1;
			return 0;
		}
	}

	if ( this->searchDepth >= this->maxDepth )
	{
		/* printf("--AT MAX DEPTH!?!?!?!\n"); */
		this->pop();
		this->searchDepth = this->searchDepth - 1;
		return 0;
	}

	/* DO WE NEED TO CHANGE MODES? */
	if ( this->searchDepth >= this->partialDepth )
	{
		this->deepeningMode = this->deepeningMode & ~((int) DEEPEN_PARTIAL);
	}
	if ( this->searchDepth >= this->jobDepth )
	{
		this->deepeningMode = this->deepeningMode & ~((int) DEEPEN_JOB);
	}

	/* NEED MORE LABELS? */
	if ( this->searchDepth >= this->labelSize )
	{
		this->labelSize = this->labelSize + 100;
		this->jobLabels = (LONG_T*) realloc(this->jobLabels, this->labelSize * sizeof(LONG_T));
	}

	/* from here on out, we may be recursing */
	int level_mode = this->deepeningMode;

	if ( (level_mode & (DEEPEN_JOB | DEEPEN_PARTIAL)) != 0 )
	{
		/* start by navigating to jobLabels[searchDepth] */
		LONG_T goal = this->jobLabels[this->searchDepth];
		LONG_T res = this->pushTo(goal);

		if ( res != goal )
		{
			printf("--[doSearch] Failed to find child %llX at depth %d, got %llX instead.\n", goal, this->searchDepth, res);
			exit(1);
		}

		(this->num_nodes)++;

		int prune_res = this->prune();

		if ( prune_res == 1 )
		{
			printf("--[doSearch] Pruning at child %llX at depth %d.\n", goal, this->searchDepth);
			exit(1);
		}
		else if ( this->isSolution() == 1 )
		{
			this->searchDepth = this->searchDepth + 1;
			this->writeSolutionJob(stdout);
			this->searchDepth = this->searchDepth - 1;

			char* buffer = this->writeSolution();

			if ( buffer != 0 )
			{
				fprintf(stdout, "%s\n", buffer);
				fflush(stdout);
				free(buffer);
			}

			this->foundSolutions = this->foundSolutions + 1;

			if ( this->foundSolutions >= this->maxSolutions )
			{
				this->writePartialJob(stdout);

				/* popping the child */
				this->pop();
				this->searchDepth = this->searchDepth - 1;

				/* popping the current node */
				this->pop();
				this->searchDepth = this->searchDepth - 1;
				return -1;
			}

			if ( this->haltAtSolutions )
			{
				/* TODO: There is something wrong with halt at solutions */
				/* popping the child */
				this->pop();
				(this->searchDepth)--;
			}
			else
			{
				/* deepen! */

				int sResult = this->doSearch();
				/* child popped in search process */

				if ( sResult < 0 )
				{
					this->pop();
					this->searchDepth = this->searchDepth - 1;
					return sResult;
				}

				/* otherwise, popped at exit of the mode */
			}
		}
		else
		{
			int sResult = this->doSearch();

			if ( sResult < 0 )
			{
				this->pop();
				this->searchDepth = this->searchDepth - 1;
				return sResult;
			}
		}

		/* We don't return here, in case of a partial job */
		/* For partial, we keep pushing next after this */
	}

	if ( (level_mode & DEEPEN_JOB) != 0 )
	{
		/* we do not continue this silliness */
		/* moreover, we must be done successfully! */

		if ( this->searchDepth == 0 )
		{
			this->writeCompleteJob(stdout);
		}

		/* pop() here, as we have completed all children */
		this->pop();
		this->searchDepth = this->searchDepth - 1;
		return 0;
	}

	/* The important children loop! */
	while ( (this->jobLabels[this->searchDepth] = this->pushNext()) >= 0 )
	{
		(this->num_nodes)++;

		int prune_res = this->prune();

		if ( prune_res == 1 )
		{
			(this->num_prunes)++;
			this->pop();
			/* just popping the child */
			/* no decrease */
		}
		else if ( this->isSolution() == 1 )
		{
			this->searchDepth = this->searchDepth + 1;
			this->writeSolutionJob(stdout);
			this->searchDepth = this->searchDepth - 1;
			char* buffer = this->writeSolution();

			if ( buffer != 0 )
			{
				fprintf(stdout, "%s\n", buffer);
				fflush(stdout);
				free(buffer);
			}

			this->foundSolutions = this->foundSolutions + 1;

			if ( this->foundSolutions >= this->maxSolutions )
			{
				this->writePartialJob(stdout);

				/* popping the child */
				this->pop();
				(this->searchDepth)--;
				/* popping the current node */
				this->pop();
				this->searchDepth = this->searchDepth - 1;
				return -1;
			}

			if ( this->haltAtSolutions )
			{
				/* TODO: There is something wrong with halt at solutions */
				/* popping the child */
				this->pop();
				(this->searchDepth)--;
			}
			else
			{
				/* deepen! */

				int sResult = this->doSearch();
				/* child popped in search process */

				if ( sResult < 0 )
				{
					this->pop();
					this->searchDepth = this->searchDepth - 1;
					return sResult;
				}

				/* otherwise, popped at exit of the mode */
			}
		}
		else
		{
			int sResult = this->doSearch();
			/* child popped in search process */

			if ( sResult < 0 )
			{
				this->pop();
				this->searchDepth = this->searchDepth - 1;
				return sResult;
			}

			/* otherwise, popped on exit of the mode */
		}
	}

	/* pop() here, as we have completed all children */
	this->pop();
	this->searchDepth = this->searchDepth - 1;

	return 0;
}

/**
 * pushNext -- deepen the search to the next child
 * 	of the current node.
 *
 * @return the label for the new node. -1 if none.
 */
LONG_T SearchManager::pushNext()
{
	LONG_T label = -1;
	if ( this->stack.size() == 0 )
	{
		/* we need to make a first level! */
		label = this->root->curChild = this->root->curChild + 1;
	}
	else
	{
		label = this->stack.back()->curChild = this->stack.back()->curChild + 1;
	}
	SearchNode* node = new SearchNode(label);
	node->curChild = -1;

	if ( node->label >= 2 )
	{
		delete node;
		return -1;
	}

	this->stack.push_back(node);
	return node->label;
}

/**
 * pushTo -- deepen the search to the specified child
 * 	of the current node.
 *
 * @param child the specified label for the new node
 * @return the label for the new node. -1 if none, or failed.
 */
LONG_T SearchManager::pushTo(LONG_T child)
{
	/* not implemented in base class */
	return -1;
}

/**
 * pop -- remove the current node and move up the tree.
 *
 * @return the label of the node after the pop.
 * 		This return value is used for validation purposes
 * 		to check proper implementation of push*() and pop().
 */
LONG_T SearchManager::pop()
{
	if ( this->stack.empty() )
	{
		return -1;
	}

	/* not implemented in base class */
	SearchNode* node = this->stack.back();

	this->stack.pop_back();

	LONG_T label = node->label;

	delete node;

	return label;
}

/**
 * prune -- Perform a check to see if this node should be pruned.
 *
 * @return 0 if no prune should occur, 1 if prune should occur.
 */
int SearchManager::prune()
{
	/* not implemented in base class */
	return 0;
}

/**
 * isSolution -- Perform a check to see if a solution exists
 * 		at this point.
 *
 * @return 0 if no solution is found, 1 if a solution is found.
 */
int SearchManager::isSolution()
{
	/* not implemented in base class */
	return 0;
}

/**
 * writeSolution -- create a buffer that contains a
 * 	description of the solution.
 *
 * Solution strings start with S.
 *
 * @return a string of data allocated with malloc().
 * 	It will be deleted using free().
 */
char* SearchManager::writeSolution()
{
	/* not implemented in base class */
	return 0;
}

/**
 * writeStatistics -- create a buffer that contains a
 * 	description of the solution.
 *
 * Statistics take the following format in each line:
 *
 * T [TYPE] [ID] [VALUE]
 *
 * @return a string of data allocated with malloc().
 * 	It will be deleted using free().
 */
char* SearchManager::writeStatistics()
{
	return this->writeBaseStats();
}

/**
 *	initStats() -- initialize the statistic variables
 */
void SearchManager::initBaseStats()
{
	this->num_nodes = 0;

	this->num_prunes = 0;
}

/**
 * writeBaseStats() -- write the statistics for this base class.
 */
char* SearchManager::writeBaseStats()
{
	int buf_len = 5000;
	char* buffer = (char*) malloc(buf_len);
	int cur_loc = 0;

	sprintf(buffer, "T SUM NUM_NODES %lld\nT SUM NUM_PRUNES %lld\n", this->num_nodes, this->num_prunes);

	cur_loc = strlen(buffer);

	sprintf(&(buffer[cur_loc]), "T SUM SEARCH_TIME %lf\n", this->time_in_search);
	cur_loc += strlen(&(buffer[cur_loc]));

	return buffer;
}
