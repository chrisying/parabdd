CC=g++ -std=c++11
CFLAGS=-pthread

FILES=bdd manager node parabdd
OBJECTS=$(addprefix bin/,$(addsuffix .o,$(FILES)))

.PHONY: all clean veryclean

all: parabdd

parabdd: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o parabdd

bin/%.o: src/%.cpp src/bdd.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o

veryclean: clean
	rm parabdd
