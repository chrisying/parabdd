#include <cassert>
#include <unordered_map>
#include <iostream>
#include <set>

#include "bdd.h"

int main() {
    using namespace bdd;

    Bdd a(1);
    Bdd b(2);
    Bdd c(3);
    Bdd d(4);
    Bdd e(5);
    Bdd f(6);

    Bdd c1 = !a | !b | e;
    Bdd c2 = a | !b | f;
    Bdd c3 = f | !a | !e;
    Bdd c4 = !c | !a | b;

    Bdd sat = c1 & c2 & c3 & c4;
    sat.print("3-Sat Instance");

	return 0;
}
