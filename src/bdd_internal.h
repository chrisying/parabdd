#ifndef _BDD_INTERNAL_H_
#define _BDD_INTERNAL_H_

#include <unordered_map>

#include "node.h"

namespace bdd {
	namespace internal {
		typedef int Query; // Temporary hack

		// Cache line width (bytes) on x86
		constexpr size_t cache_width = 64;

		namespace manager {
			// TODO: how is ordering managed?

			extern std::unordered_map<Query, Node*> cache;
			extern NodeSet nodes;
		}
	}
}

#endif
