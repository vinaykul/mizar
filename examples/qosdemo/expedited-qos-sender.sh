#!/bin/bash

pod_name="qos-expedited"
dest_ip="192.168.64.11"
dest_port="6666"
bw="10G"
interval="3"
duration="30"
framelen="1358"

#TODO: Colorize .. flashing..
echo " "
echo "Pinging receiver from expedited QoS class pod..."
echo " "

kubectl exec -ti ${pod_name} -- ping -c2 ${dest_ip}

echo " "
echo "Ping successful. Expedited QoS class pod waiting for READY-SET-GO!!"
echo " "

while [ ! -f /tmp/rdysetgo ] ;
do
      sleep 1
done

set -x
docker exec -ti $(docker ps | grep netctr_${pod_name} | awk '{print $1}') iperf3 -p ${dest_port} -b ${bw} -i ${interval} -t ${duration} --length ${framelen} -c ${dest_ip} -u
set +x
