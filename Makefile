CC = gcc
CFLAGS = -Wall -fPIC -O2
LDFLAGS = -shared
TARGET = libdiscordbypass.so
SRC = discord_bypass.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET) -ldl

clean:
	rm -f $(TARGET)