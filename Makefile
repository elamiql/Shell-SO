CC = gcc

CFLAGS = -Wall -g

OBJ = main.o history.o input.o pipes.o

TARGET = myshell

all: $(TARGET)

$(TARGET): $(OBJ) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %c
	$(CC) $(CFLAGS) -o $(TARGET) -c $<

clean:
	rm -f $(OBJ) $(TARGET)