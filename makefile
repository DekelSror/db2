

HOME=$(shell echo ~)

server:
	rm -rf bin/db2.out
	mkdir -p bin
	rm -rf ~/.db2
	mkdir ~/.db2
	gcc -g -Wextra -Wall -std=c2x \
		db2_types.c utilities.c db2_mempool.c db2_time/db2_time.c \
		db2_kv.c db2_timeseries.c \
		db2_server.c \
		-o bin/db2.out \
		-Ddb2_socket_path='"$(HOME)/.db2/db2_comm"' \
		-D_POSIX_C_SOURCE=200809L \
		-D__USE_MISC=1 \
		-I. -I./db2_time
