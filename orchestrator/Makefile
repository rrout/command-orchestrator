TARGET = orchistrator
LIBS = -lrt -lpthread  -lxml2
INCS = -I/usr/include/libxml2
CC = gcc
CFLAGS = -g -Wall

.PHONY: default all clean

default: $(TARGET)
#all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	@echo Compiling $@
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	@echo Building $@
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
