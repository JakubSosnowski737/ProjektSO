CC = gcc
CFLAGS = -Wall -pthread -D_POSIX_C_SOURCE=200809L

SOURCES = main.c dispatcher.c worker.c truck.c
OBJECTS = $(SOURCES:.c=.o)
EXEC = cegielnia

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(EXEC)
