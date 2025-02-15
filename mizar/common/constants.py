# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.

# Authors: Sherif Abdelwahab <@zasherif>
#          Phu Tran          <@phudtran>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:The above copyright
# notice and this permission notice shall be included in all copies or
# substantial portions of the Software.THE SOFTWARE IS PROVIDED "AS IS",
# WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
# TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
# FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Constants
group = 'mizar.com'
version = 'v1'


class CONSTANTS:
    TRAN_SUBSTRT_EP = 0
    TRAN_SIMPLE_EP = 1
    TRAN_SCALED_EP = 2
    TRAN_GATEWAY_EP = 3
    ON_XDP_TX = "ON_XDP_TX"
    ON_XDP_PASS = "ON_XDP_PASS"
    ON_XDP_REDIRECT = "ON_XDP_REDIRECT"
    ON_XDP_DROP = "ON_XDP_DROP"
    ON_XDP_SCALED_EP = "ON_XDP_SCALED_EP"
    IPPROTO_TCP = "6"
    IPROTO_UDP = "17"
    MAX_CLI_CHAR_LENGTH = 2000
    XDP_GENERIC = "2"
    XDP_DRIVER = "4"
    XDP_OFFLOAD = "8"
    MIZAR_BRIDGE = "mzbr0"
    MIZAR_EGRESS_BW_TAG = "mizar.com/egress-bandwidth"
    MIZAR_INGRESS_BW_TAG = "mizar.com/ingress-bandwidth"
    MIZAR_NETWORK_CLASS_TAG = "mizar.com/network-class"
    MIZAR_NETWORK_PRIORITY_TAG = "mizar.com/network-priority"
    MIZAR_DEFAULT_EGRESS_BW_LIMIT_PCT = 30
    SUBNETS_NEW_PREFIX = 32
    RPC_ERROR_CODE = "error"
    RPC_FATAL_CODE = "fatal"
    RPC_FAILED_CODE = "failed"
    HTTP_CONFLICT_ERROR = 409
    GRPC_DEVICE_BUSY_ERROR = "Device or resource busy"
    GRPC_FILE_EXISTS_ERROR = "File exists"
    GRPC_UNAVAILABLE = "failed to connect to all addresses"
    NETLINK_FILE_EXISTS_ERROR = 17


class OBJ_STATUS:
    obj_init = 'Init'
    obj_provisioned = 'Provisioned'
    ep_status_init = obj_init
    ep_status_allocated = 'Alloc'
    ep_status_bouncer_ready = 'BouncerReady'
    ep_status_provisioned = obj_provisioned

    net_status_init = obj_init
    net_status_allocated = 'Alloc'
    net_status_ready = 'Ready'
    net_status_provisioned = obj_provisioned

    vpc_status_init = obj_init
    vpc_status_allocated = 'Alloc'
    vpc_status_ready = 'Ready'
    vpc_status_provisioned = obj_provisioned

    droplet_status_init = obj_init
    droplet_status_allocated = 'Alloc'
    droplet_status_ready = 'Ready'
    droplet_status_provisioned = obj_provisioned

    bouncer_status_init = obj_init
    bouncer_status_allocated = 'Alloc'
    bouncer_status_ready = 'Ready'
    bouncer_status_provisioned = obj_provisioned
    bouncer_status_placed = 'Placed'
    bouncer_status_endpoint_ready = 'EndpointReady'
    bouncer_status_divider_ready = 'DividerReady'

    divider_status_init = obj_init
    divider_status_allocated = 'Alloc'
    divider_status_provisioned = 'Ready'
    divider_status_provisioned = obj_provisioned
    divider_status_placed = 'Placed'


class OBJ_DEFAULTS:
    default_ep_vpc = 'vpc0'
    default_ep_net = 'net0'
    default_ep_type = 'simple'
    default_vpc_vni = '1'
    default_vpc_ip = '10.20.0.0'
    default_vpc_prefix = '16'
    default_net_ip = '10.20.0.0'
    default_net_gw = '10.20.0.1'
    default_net_prefix = '16'
    default_n_bouncers = 1
    default_n_dividers = 1
    default_host_ep_prefix = "32"
    ep_type_simple = 'simple'
    ep_type_scaled = 'scaled'
    ep_type_host = 'host'
    ep_type_gateway = 'gateway'
    droplet_eps = [ep_type_simple, ep_type_host]
    host_ep_veth_name = "ehost"
    host_ep_peer_name = "vehost"
    host_ep_name = "hostep"

    mizar_daemon_service_port = 50051
    mizar_operator_arktos_service_port = 50052
    mizar_service_annotation_key = "mizar.com/scaled-endpoint"
    mizar_service_annotation_val = "scaled-endpoint"

    mizar_ep_vpc_annotation = "mizar.com/vpc"
    mizar_ep_subnet_annotation = "mizar.com/subnet"

    # 60 minutes of retries
    kopf_max_retries = 240
    kopf_retry_delay = 15


class RESOURCES:
    endpoints = "endpoints"
    nets = "subnets"
    vpcs = "vpcs"
    droplets = "droplets"
    bouncers = "bouncers"
    dividers = "dividers"


class COMPUTE_PROVIDER:
    kubernetes = "kubernetes"
    k8s = False
    arktos = "arktos"


class LAMBDAS:
    ep_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_init
    ep_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_allocated
    ep_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_provisioned
    ep_status_bouncer_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.ep_status_bouncer_ready

    net_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_init
    net_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_allocated
    net_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_ready
    net_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.net_status_provisioned

    vpc_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_init
    vpc_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_allocated
    vpc_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_ready
    vpc_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.vpc_status_provisioned

    droplet_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_init
    droplet_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_allocated
    droplet_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_ready
    droplet_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.droplet_status_provisioned

    bouncer_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_init
    bouncer_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_allocated
    bouncer_status_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_ready
    bouncer_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_provisioned
    bouncer_status_placed = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_placed
    bouncer_status_endpoint_ready = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.bouncer_status_endpoint_ready

    divider_status_init = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_init
    divider_status_allocated = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_allocated
    divider_status_provisioned = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_provisioned
    divider_status_placed = lambda body, **_: body.get('spec', {}).get(
        'status', '') == OBJ_STATUS.divider_status_placed
    k8s_provider_vanilla = lambda **_: COMPUTE_PROVIDER.k8s == True
