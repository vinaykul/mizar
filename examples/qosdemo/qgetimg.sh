#!/bin/bash -x
  
mkdir -p /tmp/mzimg
rm -rf /tmp/mzimg/*
scp root@192.168.64.9:/root/tmp/mzimg/* /tmp/mzimg/
gunzip /tmp/mzimg/*
docker rmi -f skiibum/mizar-$(uname -p):qos
docker rmi -f skiibum/dropletd-$(uname -p):qos
docker rmi -f skiibum/endpointopr-$(uname -p):qos
docker load -i /tmp/mzimg/dr.tar
docker load -i /tmp/mzimg/ep.tar
docker load -i /tmp/mzimg/mz.tar
