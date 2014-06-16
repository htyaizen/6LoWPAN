CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=userver6.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=userver6

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
