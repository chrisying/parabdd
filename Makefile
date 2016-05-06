CC=/afs/cs/academic/class/15210-s14/cilk/gcc-cilk-install/bin/g++-cilk -std=c++11 -g
CWARNS=-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Wno-unused
CFLAGS=$(CWARNS) -static -pthread -fcilkplus -lcilkrts -ldl -L/afs/cs/academic/class/15210-s14/cilk/gcc-cilk-install/lib

FILES=bdd manager node nodeset cache
OBJECTS=$(addprefix bin/,$(addsuffix .o,$(FILES)))

.PHONY: all clean veryclean

all: parabdd nqueens

parabdd: $(OBJECTS) src/parabdd.cpp
	$(CC) $(CFLAGS) $(OBJECTS) src/parabdd.cpp  -o parabdd

nqueens: $(OBJECTS) src/nqueens.cpp
	$(CC) $(CFLAGS) $(OBJECTS) src/nqueens.cpp -o nqueens

bin/%.o: src/%.cpp src/bdd.h src/bdd_internal.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o

veryclean: clean
	rm parabdd nqueens
