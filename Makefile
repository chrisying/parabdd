CC=g++ -std=c++11

all: $(addsuffix .o,bdd manager node parabdd)
	$(CC) $(addprefix bin/,$(addsuffix .o,bdd manager node parabdd)) -o parabdd

%.o: src/%.cpp
	$(CC) -c $< -o bin/$@

clean:
	rm -f bin/*.o

veryclean: clean
	rm parabdd
