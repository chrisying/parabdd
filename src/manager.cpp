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
			NodeSet nodes(1024);

            struct ConstructorHack {
                ConstructorHack() {
                    // Leave 256 MB for other people, taking at most 16 GB
                    size_t max_mem = 16 * 1024 * 1024 * 1024;
                    size_t mem = std::min(mem_available() - (256 * 1024 * 1024), max_mem);
                    printf("%lu MB\n", mem / 1024 / 1024);
                }
            } hack;
		}
	}
}
