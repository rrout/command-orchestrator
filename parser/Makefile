TARGET = xml-test
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

install:
	@echo Creating orch_xml directory in /tmp
	mkdir -p /tmp/orch_xml
	@echo Copying xml files to /tmp/orch_xml
	cp -r *.xml /tmp/orch_xml/.

uninstall:
	@echo Deleting orch_xml directory from /tmp
	rm -rf /tmp/orch_xml
