CC=g++
CFLAGS=-O3
EXE=tp

main:
	$(CC) $(CFLAGS) -o $(EXE) main.cpp

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o

format:
	@clang-format -style=file -i *.cpp *.hpp

archive:
	git archive master --format=tar | gzip -9 - > comp90024-a1.tar.gz
