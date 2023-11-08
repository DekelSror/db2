#!/bin/bash


messages=("smallak" "wallak" "tralalalak")

for msg in ${messages[@]}; do
    gnome-terminal -- bash -c "./bin/driver2.out $msg" &
done





