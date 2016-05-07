#include <cassert>
#include <unistd.h>

#include "bdd_internal.h"

static size_t mem_available() {
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);

    assert(pages != -1);
    assert(page_size != -1);

    return static_cast<size_t>(pages) * static_cast<size_t>(page_size);
}

namespace bdd {
	namespace internal {
		namespace manager {
            // TODO: we need to set the sizes of these somewhere
		//	std::unordered_map<Query, Node*> cache;
			NodeSet nodes;
            Cache cache;

            struct ConstructorHack {
                ConstructorHack() {
                    // Leave 256 MB for other people, taking at most 16 GB
                    size_t max_mem = 0x400000000;
                    //size_t max_mem = 0x40000000; // TODO: small tables for development only XXX
                    size_t extra_mem = 0x10000000;

                    size_t cache_size = 0x20000000;
                    size_t mem = std::min(mem_available() - extra_mem, max_mem) - cache_size;

                    nodes.init(mem);
                    cache.init(cache_size);
                }
            } hack;
		}
	}
}
