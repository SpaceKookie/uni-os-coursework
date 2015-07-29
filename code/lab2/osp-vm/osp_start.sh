#!/bin/sh

CPUS=2
MEM=2024

echo "Check overlays"

if [ ! -f ubuntu14.04_4GB.ovl ]; then
  qemu-img create -b ubuntu14.04_4GB.img -f qcow2 ubuntu14.04_4GB.ovl
fi

if [ ! -f kernel_build_4G.ovl ]; then
  qemu-img create -b kernel_build_4G.img -f qcow2 kernel_build_4G.ovl
fi

echo "Start qemu"

qemu-system-x86_64 -curses -boot c -smp $CPUS -m $MEM -hda 'ubuntu14.04_4GB.ovl' -hdb 'kernel_build_4G.ovl'
