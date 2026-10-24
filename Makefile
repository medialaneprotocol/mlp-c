CC ?= gcc
CFLAGS ?= -Iinclude -Wall -Wextra -O2

.PHONY: test clean

test: tests/test_golden tests/test_sender
	./tests/test_golden
	./tests/test_sender

tests/test_golden: src/mlp.c tests/test_golden.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_golden.c -o tests/test_golden

tests/test_sender: src/mlp.c tests/test_sender.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_sender.c -o tests/test_sender

clean:
	rm -f tests/test_golden
