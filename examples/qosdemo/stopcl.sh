#!/bin/bash -x

echo 'y' | kubeadm reset && rm /etc/cni/net.d/10-mizarcni.conf && rm -rf /var/mizar && rm /opt/cni/bin/mizarcni
ip link del mzbr0
sysctl -w net.bridge.bridge-nf-call-iptables=1
