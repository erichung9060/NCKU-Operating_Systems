CC := gcc
override CFLAGS += -O3 -Wall
LDFLAGS := -lrt -pthread

SOURCE1 := sender.c
BINARY1 := sender

SOURCE2 := receiver.c
BINARY2 := receiver

all: $(BINARY1) $(BINARY2)

$(BINARY1): $(SOURCE1) $(patsubst %.c, %.h, $(SOURCE1))
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(BINARY2): $(SOURCE2) $(patsubst %.c, %.h, $(SOURCE2))
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(BINARY1) $(BINARY2)
