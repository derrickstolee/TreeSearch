/*
 * SearchManager.hpp
 *
 * Defines the class SearchManager, which
 * is a base class for use in TreeSearch.
 *
 *  Created on: Feb 26, 2010
 *      Author: derrickstolee
 */

#ifndef SEARCHMANAGER_HPP_
#define SEARCHMANAGER_HPP_

#include <stdio.h>
#include <list>
#include <time.h>
#include "treesearch.h"

/**
 * The SearchNode class is used to store data for a
 * single node.  The point is to have a standard
 * base class for all data, which will be stored in
 * a stack in the SearchManager class.
 */
class SearchNode
{
public:
	/**
	 * label -- the label used in the search for this node.
	 *    It specifies how to reach this node from its parent.
	 */
	LONG_T label;

	/**
	 * curChild -- the current child.
	 */
	LONG_T curChild;

	/**
	 * Constructor
	 */
	SearchNode(LONG_T label);

	/**
	 * Copy constructor
	 */
	SearchNode(SearchNode& sn);

	/**
	 * Destructor
	 */
	virtual ~SearchNode();

	/**
	 * Copy request.  This allows a deep copy of SearchManager.
	 */
	virtual SearchNode* copy();
};

/**
 * SearchManager -- the base class for all searches.
 * It maintains a stack of SearchNodes.
 *
 * Subclasses implement the push*, pop, prune, check,
 * 		writeSolution, and writeStatistics methods as
 * 		needed.
 */
class SearchManager
{
protected:
	/**
	 * jobLabels -- the list of job labels given by the input.
	 *
	 * This contains all steps to the partial depth.
	 */
	LONG_T* jobLabels;

	/**
	 * labelSize -- the size of the jobLabels array
	 */
	int labelSize;

	/**
	 * jobDepth -- the depth for the job node.
	 */
	unsigned int jobDepth;

	/**
	 * partialDepth -- the depth for the partial node.
	 */
	unsigned int partialDepth;

	/**
	 * maxSolutions -- the  maximum number of solutions wanted.
	 */
	int maxSolutions;

	/**
	 * foundSolutions -- the number of solutions found.
	 */
	int foundSolutions;

	/**
	 * haltAtSolutions -- stop deepening when a solution is found?
	 */
	bool haltAtSolutions;

	/**
	 * stack -- the linked list storing SearchNode pointers.
	 *
	 * Pointers are important: minimizes data copying.
	 * This means that when objects are removed, they must
	 * be manually deleted.
	 */
	std::list<SearchNode*> stack;

	/**
	 * root -- the "-1" level element that starts the entire search.
	 */
	SearchNode* root;

	/**
	 * killtime -- the number of seconds to run the search.
	 *
	 * Default: 1h.
	 */
	int killtime;

	/**
	 * end_time -- the time point to kill the search.
	 */
	time_t end_time;

	/**
	 * deepeningMode -- the mode of the search.
	 *
	 * mode: -1 -- generating to the next depth.
	 * mode: 0 -- performing a full search.
	 * mode: 1 -- performing an advancement to the jobDepth.
	 * mode: 2 -- performing an advancement to the partialDepth.
	 */
	int deepeningMode;

#define DEEPEN_FULL 0
#define DEEPEN_JOB 1
#define DEEPEN_PARTIAL 2
#define DEEPEN_GENERATE 4


	int numJobsFound;
	int maxJobsFound;

	/**
	 * maxDepth -- the maximum depth of the search.
	 *
	 * This is used instead of stages if stages are not set.
	 */
	unsigned int maxDepth;

	/**
	 * numStages -- the number of stages in the search.
	 *
	 * Each stage corresponds to a level where the
	 * 	search node is output as another job instead of
	 *  deepening further.
	 */
	unsigned int numStages;

	/**
	 * stages -- the list of stage values.
	 *
	 * For instance, 0 < 4 < 10 would specify a search
	 * that generates jobs at level 4 and then
	 * splits jobs that reach level 10.
	 */
	int* stages;

	/**
	 * checkOnlyLast -- an option flag specifying
	 *  if the
	 */
	unsigned int checkOnlyLast;

	/**
	 * searchDepth -- the current depth of the search.
	 */
	int searchDepth;


	/* STATISTIC TRACKING */

	/**
	 * num_nodes -- total number of nodes reached
	 */
	LONG_T num_nodes;

	/**
	 * nodes_at_level -- the nodes at each depth
	 */
	LONG_T* nodes_at_level;

	/**
	 * num_prunes -- total number of pruning
	 */
	LONG_T num_prunes;

	/**
	 * prunes_at_level -- the number of prunes that occur at each depth
	 */
	LONG_T* prunes_at_level;

	/**
	 * time_in_prune -- the amount of time spent in the prune operation (by level)
	 */
	double* time_in_prune;

	/**
	 * total time
	 */
	double time_in_search;

	/**
	 *	initBaseStats() -- initialize the statistic variables
	 */
	void initBaseStats();

	/**
	 * writeBaseStats() -- write the statistics for this base class.
	 */
	char* writeBaseStats();


public:
	/**
	 * Default constructor.
	 *
	 * Part of the constructor for an extension must
	 *  insert the root node in to the stack.
	 */
	SearchManager();

	/**
	 * Copy constructor
	 */
	SearchManager(SearchManager& sm);

	/**
	 * Destructor
	 */
	virtual ~SearchManager();

	/**
	 * importArguments -- take the command line arguments
	 * 	and convert them into options.
	 *
	 * This includes the following options:
	 * 	mode: generate or run
	 *  stages: -s [count] [s0] [s1] ... [sk]
	 *  killtime: -k [killtime]
	 *  maxdepth: -m [maxdepth]
	 *
	 *  It is set as virtual so that one can strip
	 *   custom arguments, if necessary.
	 */
	virtual void importArguments(int argc, char** argv);

	/**
	 * readJob -- read a job description
	 *  from a given input stream.
	 *  This will also navigate to the proper search node.
	 *
	 * After reading the job, call doSearch() to perform the
	 * 	search from that position.
	 *
	 * @param file The input stream.
	 */
	int readJob(FILE* file);


	/**
	 * writeJob -- write a job description from the
	 *  current stack of SearchNodes.
	 */
	void writeJob(FILE* file);

	/**
	 * writePartialJob -- write a partial job description from the
	 *  current stack of SearchNodes, using
	 *  the current jobDepth as the transfer from
	 *  job to partial nodes.
	 */
	void writePartialJob(FILE* file);

	/**
	 * writeCompleteJob -- write a job description from the
	 *  current stack of SearchNodes, using
	 *  the current jobDepth as the transfer from
	 *  job to partial nodes.
	 */
	void writeCompleteJob(FILE* file);

	/**
	 * writeSolutionJob -- write a job description from the
	 *  current stack of SearchNodes, using
	 *  the current jobDepth as the transfer from
	 *  job to partial nodes.
	 */
	void writeSolutionJob(FILE* file);

	/**
	 * doSearch -- recursively perform the search from the current node.
	 *
	 * @return 1 if a solution is found, 0 if not.
	 */
	int doSearch();

	/**
	 * pushNext -- deepen the search to the next child
	 * 	of the current node.
	 *
	 * @return the label for the new node. -1 if none.
	 */
	virtual LONG_T pushNext();

	/**
	 * pushTo -- deepen the search to the specified child
	 * 	of the current node.
	 *
	 * @param child the specified label for the new node
	 * @return the label for the new node. -1 if none, or failed.
	 */
	virtual LONG_T pushTo(LONG_T child);

	/**
	 * pop -- remove the current node and move up the tree.
	 *
	 * @return the label of the node after the pop.
	 * 		This return value is used for validation purposes
	 * 		to check proper implementation of push*() and pop().
	 */
	virtual LONG_T pop();

	/**
	 * prune -- Perform a check to see if this node should be pruned.
	 *
	 * @return 0 if no prune should occur, 1 if prune should occur.
	 */
	virtual int prune();

	/**
	 * isSolution -- Perform a check to see if a solution exists
	 * 		at this point.
	 *
	 * @return 0 if no solution is found, 1 if a solution is found.
	 */
	virtual int isSolution();

	/**
	 * writeSolution -- create a buffer that contains a
	 * 	description of the solution.
	 *
	 * Solution strings start with S.
	 *
	 * @return a string of data allocated with malloc().
	 * 	It will be deleted using free().
	 */
	virtual char* writeSolution();

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
	virtual char* writeStatistics();
};

#endif /* SEARCHMANAGER_HPP_ */
