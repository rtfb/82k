#!/bin/bash

git checkout $1
make clean
make t
time ./82k
gprof 82k |head
git checkout master
