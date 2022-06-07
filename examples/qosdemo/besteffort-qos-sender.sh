#!/bin/bash

pod_name="qos-best-effort"
dest_ip="192.168.64.12"
dest_port="8888"
bw="10G"
interval="3"
duration="30"
framelen="1358"

#TODO: Colorize .. flashing..
echo " "
echo "Pinging receiver from best-effort QoS class pod..."
echo " "

kubectl exec -ti ${pod_name} -- ping -c2 ${dest_ip}

echo " "
echo "Ping successful. Best-effort QoS class pod waiting for READY-SET-GO!!"
echo " "

while [ ! -f /tmp/rdysetgo ] ;
do
      sleep 1
done

set -x
docker exec -ti $(docker ps | grep netctr_${pod_name} | awk '{print $1}') iperf3 -p ${dest_port} -b ${bw} -i ${interval} -t ${duration} --length ${framelen} -c ${dest_ip} -u
set +x
