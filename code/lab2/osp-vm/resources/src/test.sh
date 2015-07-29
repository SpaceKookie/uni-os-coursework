#!/bin/bash

make clean
make

SUM=0;
P=0

if [ -e ./sarlkm.ko ]; then

cp ./sarlkm.ko /dev/shm

sudo rmmod sarlkm > /dev/null
sudo insmod /dev/shm/sarlkm.ko

############################################### T E S T   1 ###########################################

echo -n "Test 1 (LKM): "
if [ -f /dev/shm/sarlkm.ko ]; then
  let SUM=SUM+1
  echo "OK."
else
  echo "failed."
  echo "No LKM -> 0 points"
  exit 0;
fi

############################################### T E S T   2 ###########################################

echo -n "Test 2 (Procfile): "
if [ -f /proc/sarlkm ]; then
  let SUM=SUM+1
  let P=P+1
  PROCFS=1
  echo "OK. (+1)"
else
  PROCFS=0
  echo "failed."
fi

############################################### T E S T   3 ###########################################

echo -n "Test 3 (sysfsfile): "
if [ -f /proc/sys/kernel/prompt ]; then
  let SUM=SUM+1
  let P=P+2
  SYSFS=1
  echo "OK. (+2)"
else
  SYSFS=0
  echo "failed."
fi

############################################### T E S T   4 ###########################################

echo -n "Test 4 (Write Sysctl, Read Proc): "

if [ "x$SYSFS" = "x1" ] && [ "x$PROCFS" = "x1" ]; then
  sudo sysctl -w kernel.prompt=test4 > /dev/null
  T1=`cat /proc/sarlkm | sed "s#,# #g" | awk '{print $1}'`
fi

if [ "x$T1" = "xtest4" ]; then
  let SUM=SUM+1
  let P=P+2
  echo "OK. (+2)"
else
  echo "failed."
fi

############################################### T E S T   5 ###########################################

echo -n "Test 5 (Write Proc, Read Sysctl): "

if [ "x$SYSFS" = "x1" ] && [ "x$PROCFS" = "x1" ]; then
  echo "test5" > /proc/sarlkm
  T1=`sysctl kernel.prompt | awk '{print $3}'`
fi

if [ "x$T1" = "xtest5" ]; then
  let SUM=SUM+1
  let P=P+2
  echo "OK. (+2)"
else
  echo "failed."
fi

############################################### T E S T   6 ###########################################

echo -n "Test 6 (LKM params): "

if [ "x$SYSFS" = "x1" ] && [ "x$PROCFS" = "x1" ]; then
  sudo rmmod sarlkm
  sudo insmod /dev/shm/sarlkm.ko prompt_param=test6

  T1=`sysctl kernel.prompt | awk '{print $3}'`
fi

if [ "x$T1" = "xtest6" ]; then
  let SUM=SUM+1
  let P=P+1
  echo "OK. (+1)"
else
  echo "failed."
fi

fi

############################################### T E S T   7 ###########################################

make tools

if [ -e ./test_syscall ]; then

./test_syscall test7 > /dev/null

NP=`./test_syscall | awk '{print $1}'`

echo -n "Test 7 (syscall prompt(read)): "
if [ "x$NP" = "xtest7" ]; then
  let SUM=SUM+1
  READHN=1
  echo "OK."
else
  READHN=0
  echo "failed."
fi

############################################### T E S T   8 ###########################################

echo -n "Test 8 (syscall prompt(write)): "

./test_syscall test8 > /dev/null

NP=`./test_syscall | awk '{print $1}'`

if [ "x$NP" = "xtest8" ]; then
  let SUM=SUM+1
  if [ $READHN -eq 1 ]; then
    let P=P+2
  fi
  echo "OK. (+2)"
else
  echo "failed."
fi

fi

#################################### T E S T   R E S U L T S #########################################


echo ""
echo "*******************  RESULT  ***************************"
echo "$SUM of 8 Tests are OK. Points: $P of 10"

