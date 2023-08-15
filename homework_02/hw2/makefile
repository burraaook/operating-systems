HDR = include/memoryManagement.h include/util.h
SRC = src/memoryManagement.cpp operateArrays.cpp part3.cpp src/util.cpp
TRGT1 = operateArrays
TRGT2 = part3
FLAGS = -Wall -Wextra -std=c++11 -pthread
CC = g++

OBJS1 = obj/memoryManagement.o \
	   obj/operateArrays.o \
	   obj/util.o

OBJS2 = obj/memoryManagement.o \
	   obj/part3.o \
	   obj/util.o

all: $(TRGT1) $(TRGT2)

# Path: obj/memoryManagement.o
obj/%.o: src/%.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

# Path: obj/util.o
obj/%.o: src/%.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

# Path: obj/operateArrays.o
obj/%.o: %.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

# Path: obj/part3.o
obj/%.o: %.cpp $(HDR)
	mkdir -p obj
	$(CC) $(FLAGS) -c -o $@ $<

$(TRGT1): $(OBJS1)
	$(CC) $(FLAGS) -o $@ $(OBJS1)

$(TRGT2): $(OBJS2)
	$(CC) $(FLAGS) -o $@ $(OBJS2)

clean:
	rm -f $(OBJS1) $(OBJS2) $(TRGT1) $(TRGT2)
