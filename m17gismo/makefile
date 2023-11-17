
# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g -I /usr/local/include/codec2

# Source files
SRCS = $(wildcard *.cpp)

# Object files
OBJS = $(SRCS:.cpp=.o)

# Linker flags
LIBS = -lcodec2 -lLimeSuite -liio -lncurses -lpthread -lpulse-simple -lm -lrt -lstdc++

# Executable
TARGET = m17gismo

# Default target
all: $(TARGET)

# Compile source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

# Clean the project
clean:
	rm -f m17gismo *.o 
