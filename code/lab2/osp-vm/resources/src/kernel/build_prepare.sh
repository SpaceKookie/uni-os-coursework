#!/bin/sh

(cd /usr/src/kernel; tar xvfj ../linux-source-3.13.0.tar.bz2)

cp /boot/config-3.13.0-29-generic /usr/src/kernel/linux-source-3.13.0/.config

cp -r linux/* /usr/src/kernel/linux-source-3.13.0

(cd /usr/src/kernel/linux-source-3.13.0/; yes "" | make oldconfig; make localmodconfig; make-kpkg clean)
