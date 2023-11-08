

all:
	mkdir -p bin
	rm -rf bin/**
	rm -f db2_comm
	gcc -g -Wextra -Wall -std=c2x db2_mempool.c db2_db.c db2_server.c -o bin/db2.out
	gcc -g -Wextra -Wall -std=c2x db2_client.c driver2.c -o bin/driver2.out