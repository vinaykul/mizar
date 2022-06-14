#!/bin/bash

pod_name="qos-best-effort"
dest_ip="192.168.64.12"
dest_port="8888"
bw="10G"
interval="3"
duration="${1:-30}"
framelen="1358"

#TODO: Colorize .. flashing..
echo " "
echo "Pinging receiver from best-effort QoS class pod..."
echo " "

kubectl exec -ti ${pod_name} -- ping -c1 ${dest_ip}

pod_ip=$(docker exec -ti $(docker ps | grep netctr_qos-best | awk '{print $1}') ip -4 a s eth0 | grep inet | awk '{print $2}' | cut -d/ -f1)

echo " "
echo "Ping ${pod_ip} -> ${dest_ip} success. BESTEFFORT QoS pod waiting for READY-SET-GO!"
echo " "

while [ ! -f /tmp/rdysetgo ] ;
do
      sleep 1
done

set -x
docker exec -ti $(docker ps | grep netctr_${pod_name} | awk '{print $1}') iperf3 -p ${dest_port} -b ${bw} -i ${interval} -t ${duration} --length ${framelen} -c ${dest_ip} -u
set +x
