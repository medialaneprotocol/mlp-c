CC ?= gcc
CFLAGS ?= -Iinclude -Wall -Wextra -O2

.PHONY: test clean

test: tests/test_golden tests/test_sender tests/test_cell tests/test_scheduler tests/test_bwe tests/test_input tests/test_fec tests/test_ticket tests/test_migration
	./tests/test_golden
	./tests/test_sender
	./tests/test_cell
	./tests/test_scheduler
	./tests/test_bwe
	./tests/test_input
	./tests/test_fec
	./tests/test_ticket
	./tests/test_migration

tests/test_golden: src/mlp.c tests/test_golden.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_golden.c -o tests/test_golden

tests/test_sender: src/mlp.c tests/test_sender.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_sender.c -o tests/test_sender

tests/test_cell: src/mlp.c tests/test_cell.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_cell.c -o tests/test_cell

tests/test_scheduler: src/mlp.c tests/test_scheduler.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_scheduler.c -o tests/test_scheduler

tests/test_bwe: src/mlp.c tests/test_bwe.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_bwe.c -o tests/test_bwe

tests/test_input: src/mlp.c tests/test_input.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_input.c -o tests/test_input

tests/test_fec: src/mlp.c tests/test_fec.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_fec.c -o tests/test_fec

tests/test_ticket: src/mlp.c tests/test_ticket.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_ticket.c -o tests/test_ticket

tests/test_migration: src/mlp.c tests/test_migration.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_migration.c -o tests/test_migration

clean:
	rm -f tests/test_golden
