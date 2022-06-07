#!/bin/bash -x
  
docker rmi -f skiibum/mizar-$(uname -p):qos
docker rmi -f skiibum/dropletd-$(uname -p):qos
docker rmi -f skiibum/endpointopr-$(uname -p):qos
docker image build --build-arg "arch=$(uname -p)" -t skiibum/mizar-$(uname -p):qos -f etc/docker/mizar.Dockerfile .
docker image build --build-arg "arch=$(uname -p)" -t skiibum/dropletd-$(uname -p):qos -f etc/docker/daemon.Dockerfile .
docker image build --build-arg "arch=$(uname -p)" -t skiibum/endpointopr-$(uname -p):qos -f etc/docker/operator.Dockerfile .

mkdir -p /root/tmp/mzimg
rm -f /root/tmp/mzimg/*
docker save -o /root/tmp/mzimg/mz.tar skiibum/mizar-$(uname -p):qos
docker save -o /root/tmp/mzimg/dr.tar skiibum/dropletd-$(uname -p):qos
docker save -o /root/tmp/mzimg/ep.tar skiibum/endpointopr-$(uname -p):qos
gzip /root/tmp/mzimg/*
