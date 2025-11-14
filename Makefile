CC = gcc
CFLAGS = -Wall -fPIC -O2
LDFLAGS = -shared

TARGET = libdiscordbypass.so

SRCS = discord_bypass.c sockmgr.c
OBJS = $(SRCS:.c=.o)
LIBS = -ldl -lpthread

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS)