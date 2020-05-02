CFLAGS += -g -Wall -std=c11

build/%.o: %.c
	gcc -c -o $@ $(CFLAGS) $<

MAIN_OBJECTS = build/ringbuffer.o build/main.o

build/buf: $(MAIN_OBJECTS)
	gcc -o $@ $(MAIN_OBJECTS) -lpthread

all: build/buf

run: build/buf
	./build/buf

test: build/buf
	./tests/test_consistency

clean:
	rm -rf build/*.o build/buf

.PHONY: clean test run