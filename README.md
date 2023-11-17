

# Db2 - simple local database


## Server
```
make server
./bin/db2.out
```

1. Key-Value 
    - insert (key, key_len, val, val_len) - (updates on existing key)
    - find (key, key_lan)
    - remove (key, key_len)
2. Time Series 
    - create (name) -> series descriptor
    - add (descriptor, value)
    - get range (descriptor, start, end)

- set max clients DB2_MAX_CLIENTS (default 10)

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

DB parameters (default value)
- KV_MAX_ENTRIES (256)
- KV_MAX_ENTRY_SIZE (4k)
- TS_MAX_TIMESERIES (20)
- TS_MAX_VALUE_SIZE (?)
- TS_MAX_NAME_LEN (128)

System resources - 
1. Memory
    - set max usage DB2_MAX_MEM = amount in MB
    
2. Socket
    - DB2_SOCKET_PATH ([cwd]/db2_comm)
    - a config file [cwd]/db2_config 
     

