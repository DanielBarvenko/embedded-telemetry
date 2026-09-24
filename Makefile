CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O1
CPPFLAGS = -Isrc
SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie
.PHONY: all test sanitize clean
all: build/producer

build:
	mkdir -p build

build/producer: src/main.c src/controller.c src/telemetry.c $(wildcard src/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/controller.c src/telemetry.c $(LDFLAGS) -o $@

build/test_protocol: tests/test_protocol.c src/telemetry.c $(wildcard src/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_protocol.c src/telemetry.c $(LDFLAGS) -o $@

build/test_controller: tests/test_controller.c src/controller.c $(wildcard src/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_controller.c src/controller.c $(LDFLAGS) -o $@

test: all build/test_protocol build/test_controller
	./build/test_protocol
	./build/test_controller

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) $(SAN_FLAGS)" LDFLAGS="$(SAN_FLAGS)" test

clean:
	rm -rf build
