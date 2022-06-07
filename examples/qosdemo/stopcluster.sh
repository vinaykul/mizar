#!/bin/bash -x

echo 'y' | kubeadm reset && rm /etc/cni/net.d/10-mizarcni.conf && rm -rf /var/mizar && rm /opt/cni/bin/mizarcni
