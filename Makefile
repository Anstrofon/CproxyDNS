TARGET = DNSproxy

CC = gcc

CFLAGS = -Wall -Wextra -O2

SRC = src/dns_proxy.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
