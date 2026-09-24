CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O1
CPPFLAGS = -Isrc
SAN_FLAGS = -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie
.PHONY: all test sanitize clean clean-rtos demo demo-slow
all: build/producer

build:
	mkdir -p build

build/producer: src/main.c src/controller.c src/alarm.c src/telemetry.c $(wildcard src/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/main.c src/controller.c src/alarm.c src/telemetry.c $(LDFLAGS) -o $@

build/test_protocol: tests/test_protocol.c src/telemetry.c $(wildcard src/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_protocol.c src/telemetry.c $(LDFLAGS) -o $@

build/test_controller: tests/test_controller.c src/controller.c src/alarm.c $(wildcard src/*.h) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_controller.c src/controller.c src/alarm.c $(LDFLAGS) -o $@

build/test_alarm: tests/test_alarm.c src/alarm.c src/alarm.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_alarm.c src/alarm.c $(LDFLAGS) -o $@

test: all build/test_protocol build/test_controller build/test_alarm
	./build/test_protocol
	./build/test_controller
	./build/test_alarm
	python3 tests/test_producer.py
	python3 tests/test_trace_checker.py

demo: all
	./build/producer > build/samples.bin
	od -An -tx1 build/samples.bin

demo-slow: all
	./build/producer --slow-consumer > build/slow-samples.bin
	od -An -tx1 build/slow-samples.bin

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) $(SAN_FLAGS)" LDFLAGS="$(SAN_FLAGS)" test

clean:
	rm -f build/producer build/test_protocol build/test_controller build/test_alarm build/samples.bin build/slow-samples.bin

clean-rtos:
	rm -rf build/rtos
