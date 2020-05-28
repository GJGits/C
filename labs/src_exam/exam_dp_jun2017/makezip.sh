#!/bin/bash
mv socket.zip .socket.zip.old >& /dev/null 
zip -r socket.zip source/client[12]/*.[ch] source/server[12]/*.[ch] source/*.[ch]
