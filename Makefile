CC=mpiCC
CFLAGS=-fopenmp -O3 -std=c++11 -DDEBUG
EXE=tp

SRC=combine.cpp threading.cpp line.cpp
OBJ = $(SRC:.cpp=.o)

# Main executable
main: $(OBJ) main.cpp
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o

format:
	@clang-format -style=file -i *.cpp *.hpp

archive:
	git archive master --format=tar | gzip -9 - > comp90024-a1.tar.gz
