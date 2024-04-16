modules=db2_types/db2_types utilities/utilities db2_mempool/db2_mempool db2_time/db2_time
module_sources=$(addsuffix .c, $(modules))
module_headers=$(addsuffix .h, $(modules))

flags = -g -Wall -Wextra -pedantic -pedantic-errors -std=c2x
macros = -Ddb2_socket_path='"$(PWD)/.db2/db2_comm"' -D_POSIX_C_SOURCE=200809L

config = -Ddb2_num_entries=16 \
	-Dtotal_mem=64000 \
	-Dfeatures=kv \
	-Dmax_clients=10 \
	-Dindex_of_hash_fn=0

server: pre_install headers
	gcc $(flags) \
	$(module_sources) \
	kv/db2_kv.c server/db2_kv_handlers.c \
	timeseries/db2_timeseries.c server/db2_ts_handlers.c \
	server/db2_server.c \
	-o bin/db2.out \
	$(macros) \
	-I./include

cli: pre_install headers
	gcc $(flags) \
	$(module_sources) \
	c_client/db2_client.c \
	cli/cli.c \
	-o bin/cli.out \
	$(macros) \
	-I./include

headers: pre_install
	ln -sf $(addprefix $(PWD)/, $(module_headers) \
	server/db2_server.h \
	server/db2_kv_handlers.h \
	server/db2_ts_handlers.h \
	c_client/db2_client.h \
	kv/db2_kv.h \
	timeseries/db2_timeseries.h \
	include)


.PHONY: pre_install clean
pre_install:
	mkdir -p bin include .db2
	# $(PWD).db2/db2_comm

clean:
	rm -rf .db2/** bin/** include/**
	rmdir .db2 bin include


