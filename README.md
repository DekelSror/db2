# Db2 - simple, local database

## Server
```
make server
./bin/db2.out
```

Server parameters
- DB2_MAX_CLIENTS (default 10)

### Features

1. Key-Value
    - insert (key, key_len, val, val_len) - (updates on existing key)
    - find (key, key_lan)
    - remove (key, key_len)
2. Time Series
    - create (name) -> series descriptor
    - add (descriptor, value)
    - get range (descriptor, start, end)


## Clients

C client - 
```
cd c_client/
make
./../bin/driver2.c [msg]
``` 
Node client - 
```
npx ts-node node_client/driver2.ts [msg]
```
Python client - 
```
python3 python_client/driver2.py [msg]
```
* the msg arg is mandatory

DB parameters (default value) - currently fixed values
- KV_MAX_ENTRIES (256)
- TS_MAX_TIMESERIES (20)


System resources - 
1. Memory
    - DB2_MAX_MEM (1GB)
    
2. Socket
    - DB2_SOCKET_PATH (~/.db2/db2_comm)



     

