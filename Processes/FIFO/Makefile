CC = gcc
CFLAGS = -g -Wall

READ = FIFOread

WRITE = FIFOwrite

all: $(READ) $(WRITE)

$(READ): $(READ).o
	$(CC) $(CFLAGS) -o $(READ) $(READ).o 

$(READ).o: $(READ).c
	$(CC) $(CFLAGS) -c $(READ).c

$(WRITE): $(WRITE).o
	$(CC) $(CFLAGS) -o $(WRITE) $(WRITE).o 

$(WRITE).o: $(WRITE).c
	$(CC) $(CFLAGS) -c $(WRITE).c

clean:
	rm -rf *.o myfifo* $(WRITE) $(READ)
