#include <cassert>
#include <unordered_map>
#include <iostream>

#include "bdd.h"

int main() {

    using namespace bdd;
    Bdd a(1);
    Bdd b(2);

    std::unordered_map<Variable, bool> map = a.one_sat();
    assert(map[1]);

    Bdd c1 = a | b;
    map = c1.one_sat();
    assert(map[1] || map[2]);

    Bdd c2 = !a;
    map = c2.one_sat();
    assert(!map[1]);

    Bdd c3 = a & !b;
    map = c3.one_sat();
    assert(map[1] && !map[2]);

    Bdd d(1);
    Bdd c(2);
    Bdd e(3);
    Bdd f(4);
    Bdd g1 = (d | c) & (e | f);
    map = g1.one_sat();
    assert((map[1] || map[2]) && (map[3] || map[4]));

	return 0;
}
