CC = gcc
CFLAGS = -g -Wall

TARGET = pipe

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o

$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) -c -o $(TARGET).o $(TARGET).c


clean:
	rm -rf *.o $(TARGET)

