#!/bin/bash -x

kubeadm init --config ~/YML/kubeadm-config.yaml > /tmp/kainit.log

echo -n $(cat /tmp/kainit.log | grep "kubeadm join " | cut -d'\' -f1) > /tmp/kajoincmd && echo -n " $(cat /tmp/kainit.log | grep discovery-token-ca-cert-hash)" >> /tmp/kajoincmd

ssh root@192.168.64.10 -- $(cat /tmp/kajoincmd)
ssh root@192.168.64.11 -- $(cat /tmp/kajoincmd)
ssh root@192.168.64.12 -- $(cat /tmp/kajoincmd)

#sleep 10
#kubectl create -f ~/YML/qos-deploy-mizar.yaml
