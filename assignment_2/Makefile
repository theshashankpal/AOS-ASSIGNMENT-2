CC = gcc
OBJ = manager.o scheduler.o queue.o implementation.o
CFLAGS = -c
LFLAGS = -lrt

vpath %.c src
vpath %.h src

output: $(OBJ) 
	$(CC) scheduler.o queue.o implementation.o -o $@ $(LFLAGS)
	$(CC) manager.o -o manager $(LFLAGS)


scheduler.o: scheduler.c project.h queue.h
	$(CC) $(CFLAGS) $^

manager.o: manager.c project.h
	$(CC) $(CFLAGS) $^

implementation.o: implementation.c project.h
	$(CC) $(CFLAGS) $^

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) $^

clean: 
	rm -rf *.o scheduler manager output