/*
 * example2.cpp
 *
 *  Created on: Feb 26, 2010
 *      Author: derrickstolee
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "SearchManager.hpp"

class ExampleManager: public SearchManager
{
public:
	ExampleManager();
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

};

int main(int argc, char** argv)
{
	ExampleManager* manager = new ExampleManager();

	manager->importArguments(argc, argv);

	manager->readJob(stdin);

	manager->doSearch();

	return 0;
}

ExampleManager::ExampleManager() :
	SearchManager()
{

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
	if ( this->stack.size() == 0 )
	{
		/* we need to make a first level! */
		label = this->root->curChild = this->root->curChild + 1;
	}
	else
	{
		label = this->stack.back()->curChild = this->stack.back()->curChild + 1;
	}

	if ( label >= 2 )
	{
		return -1;
	}

	SearchNode* node = new SearchNode(label);
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
	if ( this->stack.size() == 0 )
	{
		/* we need to make a first level! */
		this->root->curChild = child;
	}
	else
	{
		this->stack.back()->curChild = child;
	}
	if ( child >= 2 )
	{
		return -1;
	}

	SearchNode* node = new SearchNode(child);
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

