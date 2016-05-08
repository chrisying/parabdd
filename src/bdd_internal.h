#ifndef _BDD_INTERNAL_H_
#define _BDD_INTERNAL_H_

#include <unordered_map>

#include "node.h"
#include "nodeset.h"
#include "cache.h"

namespace bdd {
	namespace internal {

		// Cache line width (bytes) on x86
		constexpr size_t cache_width = 64;

		namespace manager {
            // TODO: no concept of ordering/reordering
			extern NodeSet nodes;
            extern Cache cache;
		}
	}
}

#endif
