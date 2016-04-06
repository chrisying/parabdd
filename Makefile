CC=g++ -std=c++11

all: bdd manager node parabdd
	$(CC) $(addprefix bin/,$(addsuffix .o,bdd manager node parabdd)) -o parabdd

bdd: src/bdd.cpp
	$(CC) -c src/bdd.cpp -o bin/bdd.o

manager: src/manager.cpp
	$(CC) -c src/manager.cpp -o bin/manager.o

node: src/node.cpp
	$(CC) -c src/node.cpp -o bin/node.o

parabdd: src/parabdd.cpp
	$(CC) -c src/parabdd.cpp -o bin/parabdd.o

clean:
	rm -f bin/*.o

veryclean: clean
	rm parabdd
