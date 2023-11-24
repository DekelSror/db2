

all:
	rm -f db2_types.h utilities.h db2_mempool.h
	ln -s ../db2_types.h
	ln -s ../utilities.h
	ln -s ../db2_mempool.h
	gcc -g -Wextra -Wall -std=c2x ../utilities.c ../db2_mempool.c db2_client.c driver2.c -o ../bin/driver2.out	
