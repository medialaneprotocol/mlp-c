CC ?= gcc
CFLAGS ?= -Iinclude -Wall -Wextra -O2

.PHONY: test clean

test: tests/test_golden tests/test_sender tests/test_cell tests/test_scheduler tests/test_bwe tests/test_input tests/test_fec tests/test_ticket tests/test_migration tests/test_qos tests/test_loss tests/test_assembly tests/test_metrics tests/test_interop tests/test_probe tests/test_pacer tests/test_sm
	./tests/test_golden
	./tests/test_sender
	./tests/test_cell
	./tests/test_scheduler
	./tests/test_bwe
	./tests/test_input
	./tests/test_fec
	./tests/test_ticket
	./tests/test_migration
	./tests/test_qos
	./tests/test_loss
	./tests/test_assembly
	./tests/test_metrics
	./tests/test_interop
	./tests/test_probe
	./tests/test_pacer
	./tests/test_sm

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

tests/test_qos: src/mlp.c tests/test_qos.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_qos.c -o tests/test_qos

tests/test_loss: src/mlp.c tests/test_loss.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_loss.c -o tests/test_loss

tests/test_assembly: src/mlp.c tests/test_assembly.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_assembly.c -o tests/test_assembly

tests/test_metrics: src/mlp.c tests/test_metrics.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_metrics.c -o tests/test_metrics

tests/test_interop: src/mlp.c tests/test_interop.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_interop.c -o tests/test_interop

tests/test_probe: src/mlp.c tests/test_probe.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_probe.c -o tests/test_probe

tests/test_pacer: src/mlp.c tests/test_pacer.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_pacer.c -o tests/test_pacer

tests/test_sm: src/mlp.c tests/test_sm.c
	$(CC) $(CFLAGS) src/mlp.c tests/test_sm.c -o tests/test_sm

clean:
	rm -f tests/test_golden
