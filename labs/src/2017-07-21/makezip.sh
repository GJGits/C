#!/bin/bash
cp socket.zip .socket.zip.old >& /dev/null
zip -r socket.zip.new source/client[12]/*.[ch] source/server[12]/*.[ch] source/*.[ch]
if [[ $? == 0 ]] ; then
    rm socket.zip >& /dev/null
    mv socket.zip.new socket.zip
fi
