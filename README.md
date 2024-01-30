# Db2 - simple, local database

### Features

1. Key-Value API
    - insert (key, key_len, val, val_len) - (updates on existing key)
    - find (key, key_len) -> pointer to a copy of the value
    - remove (key, key_len)
    - detailed signatures in db2_client.h
2. Time Series API
    - timeseries entry struct - _time is a 64-bit time field, and _value is a 64-bit floating point (double)
    - create (name, name_len) -> series descriptor
    - add (descriptor, value)
    - get range (descriptor, start, end) -> array of timesries entries
    - agg functions (sum, avg, median, min, max) - for series or a range
    - custom queries


### Server
```
make server
./bin/db2.out
```

Server parameters
- DB2_MAX_CLIENTS (default 10)


### Client

```
cd c_client/
make
./../bin/driver2.c [msg]
```

* the msg arg is mandatory

DB parameters (default value) - currently fixed values
- KV_MAX_ENTRIES (256)
- TS_MAX_TIMESERIES (20)


### System resources - 
1. Memory
    - DB2_MAX_MEM (1GB)
    
2. Socket
    - DB2_SOCKET_PATH (~/.db2/db2_comm)



     
### dev notes

structure - 
* db_server.c has the server entry point
    - primary header for server is db2_server.h - implementations for the declarations there can be found in db2_timeseries.c and db2_kv.c
* driver2.c has the client's (driver) entry point
    - primary header for the client is db2_client.h. All its declarations are implemented in db2_client.c
