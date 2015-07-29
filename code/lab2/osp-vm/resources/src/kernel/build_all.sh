#!/bin/sh

(cd /usr/src/kernel; tar xvfj ../linux-source-3.13.0.tar.bz2)

cp /boot/config-3.13.0-29-generic /usr/src/kernel/linux-source-3.13.0/.config

cp -r linux/* /usr/src/kernel/linux-source-3.13.0

(cd /usr/src/kernel/linux-source-3.13.0/; yes "" | make oldconfig; make localmodconfig; make-kpkg clean)

if [ "x$COPY" = "x1" ]; then
  cp -r linux/* /usr/src/kernel/linux-source-3.13.0
fi

(cd /usr/src/kernel/linux-source-3.13.0; CONCURRENCY_LEVEL=8 make-kpkg --initrd kernel_image kernel_headers modules_image)

(cd /usr/src/kernel; dpkg -i linux-image-3.13.11.2-sar_3.13.11.2-sar-10.00.Custom_i386.deb linux-headers-3.13.11.2-sar_3.13.11.2-sar-10.00.Custom_i386.deb)