#ifndef _NODESET_H_
#define _NODESET_H_

#include <list>

#include "node.h"

// Lockfree set Kappa
class NodeSet {
	public:
        bdd::internal::Node* lookupOrCreate(const bdd::internal::Node& value);

	private:
        // Single-threaded implementation
        std::list<bdd::internal::Node*> _set;
};

#endif