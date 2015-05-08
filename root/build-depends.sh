#!/bin/bash
echo "digraph {  concentrate=true; rankdir=LR; node [fontsize=10]; node [nodesep=0.75]; node [ranksep=0.75]; "
echo " node [shape=box]; edge [weight=1.2]; "
#    node [color=none];
#    graph [bb="0,0,1000,1000"];
#graph[size="100,100"]; 
# graph [ratio=0.5];

cd /etc/init.d
grep Required-Start * | cut -d ':' -f 1,3 | \
while read line
do
node=$(echo $line | cut -d ':' -f 1)
for parent in $(echo $line | cut -d ':' -f 2)
do
 echo " \"$parent\" -> \"$node\""
done
done 

grep ^\\$  /etc/insserv.conf | \
while read line
do
node=$(echo $line | cut -d ' ' -f 1)
for parent in $(echo $line | cut -d ' ' -f 2)
do
 echo " \"$parent\" ->  \"$node\" "
done
done

echo "}"
