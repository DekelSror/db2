# Db2 - simple, local database

## build
- server - make server
- cli - make cli

## run
### server
```
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
### timeseries
- create [name]
- add [series_descriptor] [value]
- get_range [start] [end]
- start [series_descriptor]
- end [series_descriptor]