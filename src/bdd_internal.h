#ifndef _BDD_INTERNAL_H_
#define _BDD_INTERNAL_H_

#include <unordered_map>

#include "node.h"
#include "nodeset.h"

namespace bdd {
	namespace internal {
        typedef struct Query {
            enum {
                ITE,
                EVAL_AT
            } type;
            union {
                struct {
                    Node* a;
                    Node* b;
                    Node* c;
                } ite_data;
                struct {
                    Node* node;
                    Variable var;
                    bool value;
                } evalaute_data;
            } data;
        } Query;

		// Cache line width (bytes) on x86
		constexpr size_t cache_width = 64;

		namespace manager {
            // TODO: no concept of ordering/reordering
			// extern std::unordered_map<Query, Node*> cache;
			extern NodeSet nodes;
		}
	}
}

#endif
