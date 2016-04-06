#include <cassert>

#include "bdd.h"

// Lockfree set Kappa
namespace bdd_internal {
	template <typename T> class Set {
		public:
			T* lookupOrCreate(const T& value);

		private:
			std::set<T*> _set;
	};

	template <typename T> T* Set<T>::lookupOrCreate(const T& value) {
		auto search = _set.find(value);
		if (search != _set.end()) {
			return *search;
		}
		else {
			// TODO: create node and insert
			assert(false);
		}
	}
}
