

HOME=$(shell echo ~)

server:
	rm -rf bin/**
	mkdir -p bin
	rm -rf ~/.db2/**
	rmdir ~/.db2
	mkdir ~/.db2
	gcc -g -Wextra -Wall -std=c2x \
		utilities.c db2_mempool.c \
		db2_kv.c db2_timeseries.c \
		db2_server.c \
		-o bin/db2.out \
		-Ddb2_socket_path='"$(HOME)/.db2/db2_comm"'

