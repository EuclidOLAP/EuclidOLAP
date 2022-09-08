#!/bin/bash

ps -ef | grep euclid | grep -v grep | awk '{print $2}' | xargs kill -9

make clean

make

./euclid &

./euclid --p:mode=worker --p:port=10000 --p:join=127.0.0.1:8760 &
./euclid --p:mode=worker --p:port=20000 --p:join=127.0.0.1:8760 &

./euclid --p:mode=worker --p:port=10100 --p:join=127.0.0.1:10000 &
./euclid --p:mode=worker --p:port=10200 --p:join=127.0.0.1:10000 &

./euclid --p:mode=worker --p:port=20100 --p:join=127.0.0.1:20000 &
./euclid --p:mode=worker --p:port=20200 --p:join=127.0.0.1:20000 &
