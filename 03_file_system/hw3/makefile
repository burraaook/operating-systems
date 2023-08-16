HDR = include/fat12.h include/util.h
SRC = src/fat12.cpp src/makeFileSystem.cpp src/util.cpp src/fileSystemOper.cpp
TRGT1 = makeFileSystem
TRGT2 = fileSystemOper
FLAGS = -Wall -Wextra -std=c++11
CC = g++

OBJS1 = obj/fat12.o \
	   obj/makeFileSystem.o \
	   obj/util.o

OBJS2 = obj/fat12.o \
	   obj/fileSystemOper.o \
	   obj/util.o

all: $(TRGT1) $(TRGT2)

obj/%.o: src/%.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

obj/%.o: src/%.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

obj/%.o: %.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

obj/%.o: %.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

$(TRGT1): $(OBJS1)
	$(CC) $(FLAGS) -o $@ $(OBJS1)

$(TRGT2): $(OBJS2)
	$(CC) $(FLAGS) -o $@ $(OBJS2)

clean:
	rm -f $(OBJS1) $(OBJS2) $(TRGT1) $(TRGT2)
