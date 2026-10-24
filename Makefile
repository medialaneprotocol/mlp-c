CC ?= gcc
CFLAGS ?= -Iinclude -Wall -Wextra -O2

.PHONY: test clean

test: tests/test_golden tests/test_sender tests/test_cell
	./tests/test_golden
	./tests/test_sender
	./tests/test_cell

tests/test_golden: src/mlp.c tests/test_golden.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_golden.c -o tests/test_golden

tests/test_sender: src/mlp.c tests/test_sender.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_sender.c -o tests/test_sender

tests/test_cell: src/mlp.c tests/test_cell.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_cell.c -o tests/test_cell

clean:
	rm -f tests/test_golden
