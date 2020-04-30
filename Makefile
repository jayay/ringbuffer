build/%.o: %.c
	gcc -c -o $@ $(CFLAGS) $<

MAIN_OBJECTS = build/ringbuffer.o build/main.o

build/buf: $(MAIN_OBJECTS)
	gcc -o $@ $(MAIN_OBJECTS) -lpthread

all: build/buf

clean:
	rm -rf build/*.o build/buf

.PHONY: clean