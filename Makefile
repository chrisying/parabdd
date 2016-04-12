CC=/afs/cs/academic/class/15210-s14/cilk/gcc-cilk-install/bin/g++-cilk -std=c++11
CWARNS=-Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused
CFLAGS=$(CWARNS) -pthread -fcilkplus

FILES=bdd manager node parabdd nodeset
OBJECTS=$(addprefix bin/,$(addsuffix .o,$(FILES)))

.PHONY: all clean veryclean

all: parabdd

parabdd: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o parabdd

bin/%.o: src/%.cpp src/bdd.h src/bdd_internal.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o

veryclean: clean
	rm parabdd
