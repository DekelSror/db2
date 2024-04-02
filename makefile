modules=db2_types/db2_types utilities/utilities db2_mempool/db2_mempool db2_time/db2_time
module_sources=$(addsuffix .c, $(modules))
module_headers=$(addsuffix .h, $(modules))

flags = -g -Wall -Wextra -pedantic -pedantic-errors -std=c2x
macros = -Ddb2_socket_path='"$(PWD)/.db2/db2_comm"' -D_POSIX_C_SOURCE=200809L

server: pre_install headers
	gcc $(flags) \
	$(module_sources) \
	db2_kv.c db2_kv_handlers.c \
	db2_timeseries.c db2_ts_handlers.c \
	db2_server.c \
	-o bin/db2.out \
	$(macros) \
	-I./include

cli: pre_install headers
	gcc $(flags) \
	$(module_sources) \
	c_client/db2_client.c \
	cli.c \
	-o bin/cli.out \
	$(macros) \
	-I./include

headers: pre_install
	ln -sf $(addprefix $(PWD)/, $(module_headers) c_client/db2_client.h db2_server.h db2_kv.h db2_kv_handlers.h db2_timeseries.h db2_ts_handlers.h include)


.PHONY:
pre_install:
	mkdir -p bin include .db2
	# $(PWD).db2/db2_comm

clean:
	rm -rf .db2/** bin/** include/**
	rmdir .db2 bin include


