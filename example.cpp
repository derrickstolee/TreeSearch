/*
 * example.cpp
 *
 *  Created on: Feb 26, 2010
 *      Author: derrickstolee
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "SearchManager.hpp"

class ExampleNode: public SearchNode
{
public:
	int bits;

	ExampleNode(LONG_T label);
	ExampleNode(LONG_T label, int parent_bits);
};

ExampleNode::ExampleNode(LONG_T label) :
	SearchNode(label)
{
	this->bits = (int) label;
}
ExampleNode::ExampleNode(LONG_T label, int parent_bits) :
	SearchNode(label)
{
	this->bits = parent_bits + (int) label;
}

class ExampleManager: public SearchManager
{
protected:
	/**
	 * k -- the maximum number of 1's in the binary representation.
	 */
	int k;

public:
	ExampleManager(int k);
	~ExampleManager();

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
};

/**
 * The main method creates a manager of the derived type,
 * 	loads the proper arguments,
 *  reads a job,
 *  and executes the search.
 */
int main(int argc, char** argv)
{
	int k = 0;

	for ( int i = 1; i < argc; i++ )
	{
		if ( strcmp(argv[i], "--bits") == 0 )
		{
			if ( i >= argc - 1 )
			{
				printf("Usage: example --bits [k]\n");
				exit(1);
			}

			k = atoi(argv[i + 1]);
		}
	}

	/* Create an instance of the derived SearchManager */
	ExampleManager* manager = new ExampleManager(k);

	/* Load arguments such as -k, -m, --maxjobs, --maxsolutions, generate/run */
	manager->importArguments(argc, argv);

	/* read jobs from standard in */
	while ( manager->readJob(stdin) >= 0 )
	{
		/* execute each as it goes */
		manager->doSearch();
	}

	return 0;
}

ExampleManager::ExampleManager(int k) :
	SearchManager()
{
	this->k = k;
	this->haltAtSolutions = true;
}

ExampleManager::~ExampleManager()
{

}

/**
 * pushNext -- deepen the search to the next child
 * 	of the current node.
 *
 * @return the label for the new node. -1 if none.
 */
LONG_T ExampleManager::pushNext()
{
	LONG_T label = -1;
	ExampleNode* parent = 0;
	if ( this->stack.size() == 0 )
	{
		/* we need to make a first level! */
		if ( this->root != 0 )
		{
			delete this->root;
		}
		this->root = new ExampleNode(0);
		label = this->root->curChild + 1;
		this->root->curChild = label;
		parent = (ExampleNode*) this->root;
	}
	else
	{
		/* advance the current child using the current top of the stack */
		parent = (ExampleNode*) this->stack.back();
		label = parent->curChild = parent->curChild + 1;
	}

	/* Here, we are enumerating all binary strings, so we stop for digits >= 2 */
	if ( label >= 2 )
	{
		return -1;
	}

	/* Create a new search node (or derivation) and place it in the stack */
	ExampleNode* node = new ExampleNode(label, parent->bits);
	this->stack.push_back(node);

	return label;

}

/**
 * pushTo -- deepen the search to the specified child
 * 	of the current node.
 *
 * @param child the specified label for the new node
 * @return the label for the new node. -1 if none, or failed.
 */
LONG_T ExampleManager::pushTo(LONG_T child)
{
	ExampleNode* parent = 0;
	if ( this->stack.size() == 0 )
	{
		/* we need to make a first level! */
		if ( this->root != 0 )
		{
			delete this->root;
		}
		this->root = new ExampleNode(0);
		this->root->curChild = child;
		parent = (ExampleNode*) this->root;
	}
	else
	{
		/* use the specified child */
		parent = (ExampleNode*) this->stack.back();
		parent->curChild = child;
	}

	if ( child >= 2 )
	{
		/* if we have a mal-formed child, then halt with error */
		return -1;
	}

	/* create a new search node for this child */
	ExampleNode* node = new ExampleNode(child, parent->bits);
	this->stack.push_back(node);

	return child;
}

/**
 * pop -- remove the current node and move up the tree.
 *
 * @return the label of the node after the pop.
 * 		This return value is used for validation purposes
 * 		to check proper implementation of push*() and pop().
 */
LONG_T ExampleManager::pop()
{
	if ( this->stack.size() == 0 )
	{
		return 0;
	}

	/* remove the top node from the stack */
	SearchNode* node = this->stack.back();
	LONG_T label = (LONG_T) -1;
	if ( node != 0 )
	{
		label = node->label;

		delete node;
	}

	this->stack.pop_back();

	return label;
}

/**
 * prune -- Perform a check to see if this node should be pruned.
 *
 * If there are MORE than k 1's, prune.
 *
 * @return 0 if no prune should occur, 1 if prune should occur.
 */
int ExampleManager::prune()
{
	ExampleNode* node = (ExampleNode*) this->stack.back();

	if ( node->bits > k )
	{
		/* too many bits! */
		return 1;
	}

	return 0;
}

/**
 * isSolution -- Perform a check to see if a solution exists
 * 		at this point.
 *
 * @return 0 if no solution is found, 1 if a solution is found.
 */
int ExampleManager::isSolution()
{
	if ( this->stack.size() == this->maxDepth )
	{
		ExampleNode* node = (ExampleNode*) this->stack.back();

		if ( node->bits == k )
		{
			/* just the right number of bits */
			return 1;
		}
	}

	return 0;
}
