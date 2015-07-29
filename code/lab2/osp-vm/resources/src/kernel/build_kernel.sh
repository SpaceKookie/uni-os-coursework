#!/bin/sh

if [ "x$COPY" = "x1" ]; then
  cp -r linux/* /usr/src/kernel/linux-source-3.13.0
fi

(cd /usr/src/kernel/linux-source-3.13.0; CONCURRENCY_LEVEL=8 make-kpkg --initrd kernel_image kernel_headers modules_image)
