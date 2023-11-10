#!/bin/bash
messages=("client1" "client2" "client3")

for msg in ${messages[@]}; do
    (
        ./bin/driver2.out "$msg" 
    ) &
done





