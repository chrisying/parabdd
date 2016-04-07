CC=g++ -std=c++1y

FILES=bdd manager node parabdd
OBJECTS=$(addprefix bin/,$(addsuffix .o,$(FILES)))

.PHONY: all clean veryclean

all: $(OBJECTS)
	$(CC) $(OBJECTS) -o parabdd

bin/%.o: src/%.cpp
	$(CC) -c $< -o $@

clean:
	rm -f bin/*.o

veryclean: clean
	rm parabdd
