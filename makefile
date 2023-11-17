

DB2_SOCKET_PATH ?= $(PWD)/db2_comm

server:
	rm -rf bin/**
	mkdir -p bin
	rm -f $(DB2_SOCKET_PATH)
	gcc -g -Wextra -Wall -std=c2x db2_mempool.c db2_kv.c db2_timeseries.c db2_server.c -o bin/db2.out


setup:
	echo $(DB2_SOCKET_PATH) > db2_config


