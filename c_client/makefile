

HOME=$(shell echo ~)

all:
	gcc -g -Wextra -Wall -std=c2x \
	../utilities.c ../db2_mempool.c db2_client.c driver2.c \
	-o ../bin/driver2.out -I../ \
	-Ddb2_socket_path='"$(HOME)/.db2/db2_comm"'
