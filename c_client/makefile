

HOME=$(shell echo ~)

all:
	gcc -g -Wextra -Wall -std=c2x \
	../utilities.c ../db2_mempool.c ../db2_types.c \
	kv_tests.c large_value_test.c timeseries_test.c \
	db2_client.c \
	driver2.c \
	-o ../bin/driver2.out \
	-I../ -I../db2_time \
	-Ddb2_socket_path='"$(HOME)/.db2/db2_comm"' \
	-D_POSIX_C_SOURCE=200809L \
	-D__USE_MISC=1
