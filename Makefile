CC=mpiCC
CFLAGS=-fopenmp -DRESDEBUG -g -DDEBUG # -O3
EXE=tp

SRC=retriever.cpp main.cpp process_section.cpp
OBJ = $(SRC:.cpp=.o)

# Main executable
main: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o

format:
	@clang-format -style=file -i *.cpp *.hpp

archive:
	git archive master --format=tar | gzip -9 - > comp90024-a1.tar.gz
