CC=mpiCC
CFLAGS=-std=c++11 -O3 -lmpi -fopenmp
EXE=tp

SRC=combine.cpp threading.cpp line.cpp
OBJ=$(SRC:.cpp=.o)

# Main executable
tp: $(OBJ) main.cpp
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o

format:
	@clang-format -style=file -i *.cpp *.hpp

archive:
	git archive master --format=zip -9 > Steven-Tang-832031.zip
