#docker exec -ti $(docker ps | grep netctr_netpod | awk '{print $1}') iperf3 -s -p 4444
iperf3 -s -p 4444
