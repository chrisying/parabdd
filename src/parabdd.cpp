#include "bdd.h"

int main() {

    using namespace bdd;
    Bdd a(1);
    Bdd b(2);
    Bdd c = a | b;

	return 0;
}
