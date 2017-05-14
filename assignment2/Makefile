# Author: Christer Karlsson
# Written Spring 2016 for CSC317 COA.

# Usage:  make target1 target2 ...

#-----------------------------------------------------------------------

# GNU C/C++ compiler and linker:
CC = mpic++
# LIBS = -lpthread
FLAGS = -g -lm -std=c++11 #-Wall

# the build target executable:
TARGET = life ping_pong

all: $(TARGET)

# specific targets
life:	life.cpp
		$(CC) $(FLAGS) -o $@ $? $(LIBS)

ping_pong: ping_pong.cpp
		$(CC) $(FLAGS) -o $@ $? $(LIBS)


# utility targets
clean:
	$(RM) $(TARGET) -f *.o *~
