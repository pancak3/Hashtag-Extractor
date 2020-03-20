CC=mpiCC
CFLAGS=-O3
OBJ=process_section.o
EXE=tp

# Main executable
main: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) main.cpp $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o

format:
	@clang-format -style=file -i *.cpp *.hpp

archive:
	git archive master --format=tar | gzip -9 - > comp90024-a1.tar.gz
