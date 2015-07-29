#!/bin/sh

make clean
make
cp ./sarlkm.ko /dev/shm
sudo rmmod sarlkm
sudo insmod /dev/shm/sarlkm.ko

