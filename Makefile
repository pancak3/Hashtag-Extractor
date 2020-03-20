CC=g++
CFLAGS=-O3
EXE=tp


SRCS=retriever.cpp main.cpp

OBJS = $(SRCS:.c=.o)

main:$(OBJS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXE) *.o

format:
	@clang-format -style=file -i *.cpp *.hpp

archive:
	git archive master --format=tar | gzip -9 - > comp90024-a1.tar.gz
