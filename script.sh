#!/bin/bash

SERVER=192.18.1.1
PORT=5002

DIR=$(dirname $0)
NOW=$(date +'%Y%m%d%H%M%S')

cd $DIR

if [ ! -d "bin" ];then
	mkdir bin
fi

if [ ! -d "log" ];then
	mkdir log
fi

make clean
make

#for BYTES in `echo "64 128 256 512 1024 1280 1518"`
for BYTES in `echo "1518 1280 1024 512 256 128 64"`
do

	#echo "bin/sender $SERVER $PORT $BYTES |tee -a log/$BYTES-$NOW.log"
	#bin/sender $SERVER $PORT $BYTES |tee -a log/$BYTES-$NOW.log

	echo "bin/sender $SERVER $PORT $BYTES"
	bin/sender $SERVER $PORT $BYTES

done
