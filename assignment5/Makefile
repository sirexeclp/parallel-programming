EXEC     = mpi-test
#comment-out this line to use default mpi implementation
MPI_IMPLEMENTATION = .openmpi
COMPILER = mpic++$(MPI_IMPLEMENTATION)
RUNTIME = mpirun$(MPI_IMPLEMENTATION)


CFLAGS   = -O3
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
	$(RUNTIME) $(EXEC) 
