#include <cassert>
#include <thread>
#include <algorithm>

#include "bdd_internal.h"

namespace bdd {
	namespace internal {
		namespace manager {
            // TODO: we need to set the sizes of these somewhere
			std::unordered_map<Query, Node*> cache;
			NodeSet nodes;
		}
	}
}
