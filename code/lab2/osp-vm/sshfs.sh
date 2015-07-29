#!/bin/bash

if [ "x$1" = "x" ]; then
  fusermount -qu /ospsshfs
else
  if [ "x$2" = "x" ]; then
    sshfs $1@gruenau.informatik.hu-berlin.de:. /ospsshfs
  else
    sshfs $1@$2:. /ospsshfs
  fi
fi

exit 0
