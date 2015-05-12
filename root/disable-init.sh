#!/bin/bash

pushd /etc/init.d
allscript=$(find  -executable)
for i in $allscript
do
chkconfig -d ${i:2}
done
popd
