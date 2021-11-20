SRC = main.cpp sched.cpp combat.cpp entropy.cpp \
	  data/creatures.cpp data/items.cpp

all: game

game: $(SRC)
	$(CXX) -iquote . -o $@ $^
