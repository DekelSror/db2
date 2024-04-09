# Db2 - simple, local database

### [link to build](#build)
### [link to run](#run)
### [link to commands](#commands)
### [link to Things to Tackle](#things-to-tackle)

## build
- server - make server
- cli - make cli

## run
### server
```bash
./bin/db2.out
```

### client
```
./bin/cli.out [command] {arg1 .. argk}
```

## commands
### key/value
- get [key]
- set [key] [value]
- delete [key]
- filter []
### timeseries
- create [name]
- add [series_descriptor] [value]
- get_range [start] [end]
- start [series_descriptor]
- end [series_descriptor]


## Things to Tackle
* memory - currently mempool is a wrapper around malloc and free. 
    - consider kv memory, timeseries, and the future (tables, etc.)
    - the idea is to not have to use syscalls for memory
* query on kv - 
    - how to store and handle filter results?
    - how to provide easy use, extendability and not too much work?
    - pipe queries ? meaning support this - filter key startswith 'user' <strong>&&</strong> value.email !has '@'
    - so that the second part only looks at the results of the first, not the whole kv
    - when value is an object, how to access its fields for querying?
* cache
    - what it takes to turn kv into a cache
* config
    - only what must be decided before build 
    - total memory
    - max kv entries (optional, if you don't intend to use kv)
    - limit kv key / value size 
    - max number of time-series
    - max entries per time-series
    - if and how to compress time-series
    - cache defualt ttl
* management cli - 
    - add / remove services like - ./db2_mgmt start kv
    - check status for services, memory etc.
    - flag / block connections
    - add / remove nodes (far future)
* time-series compression
    - ask Mr. GPT - "what are some popular strategies to compress a time-series?"
    - implement 1 or 2, make clear interface to add more
    - how to store and access these compressed series?
* ingest in bulk
* support http/s connections