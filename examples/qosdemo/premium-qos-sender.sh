#!/bin/bash

pod_name="qos-premium"
dest_ip="192.168.64.10"
dest_port="4444"
bw="10G"
interval="3"
duration="30"
framelen="1358"

#TODO: Colorize .. flashing.. 
echo " "
echo "Pinging receiver from premium QoS class pod..."
echo " "

kubectl exec -ti ${pod_name} -- ping -c1 ${dest_ip}

pod_ip=$(docker exec -ti $(docker ps | grep netctr_qos-best | awk '{print $1}') ip -4 a s eth0 | grep inet | awk '{print $2}' | cut -d/ -f1)

echo " "
echo "Ping ${pod_ip} -> ${dest_ip} success. PREMIUM QoS pod waiting for READY-SET-GO!"
echo " "

while [ ! -f /tmp/rdysetgo ] ;
do
      sleep 1
done

set -x
docker exec -ti $(docker ps | grep netctr_${pod_name} | awk '{print $1}') iperf3 -p ${dest_port} -b ${bw} -i ${interval} -t ${duration} --length ${framelen} -c ${dest_ip} -u
set +x
