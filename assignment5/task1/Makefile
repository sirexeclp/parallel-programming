EXEC     = heatmap
#comment-out this line to use default mpi implementation
#MPI_IMPLEMENTATION = .openmpi
COMPILER = mpic++$(MPI_IMPLEMENTATION)
RUNTIME = mpirun$(MPI_IMPLEMENTATION)


CFLAGS   = -O3 -std=c++11
#CFLAGS  += $(shell pkg-config --cflags glib-2.0 gio-2.0)
#LDFLAGS  = $(shell pkg-config --libs   glib-2.0 gio-2.0)

SRC      = $(wildcard *.cpp)
OBJ      = $(SRC:.cpp=.o)

all: $(EXEC)

${EXEC}: $(OBJ)
	$(COMPILER) -o $@ $^ 
	#$(LDFLAGS)

%.o: %.cpp
	$(COMPILER) -o $@ -c $< $(CFLAGS)

.PHONY: clean all test

clean:
	@rm -rf *.o
	@rm $(EXEC)

test: all
	$(RUNTIME) $(EXEC) 20 7 17 hotspots.csv

test2: all
	$(RUNTIME) --oversubscribe -np 16 $(EXEC) 50 20 32 hotspots.csv
validate: all
	python3 validator.py;exit=$$?; if [ $$exit -eq 0 ]; then echo "\033[32mOK"; else echo "\033[31mValidation failed!"; fi; echo "Validation exited with code $$exit";
