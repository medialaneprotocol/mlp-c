CC ?= gcc
CFLAGS ?= -Iinclude -Wall -Wextra -O2

.PHONY: test clean

test: tests/test_golden
	./tests/test_golden

tests/test_golden: src/mlp.c tests/test_golden.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_golden.c -o tests/test_golden

clean:
	rm -f tests/test_golden
