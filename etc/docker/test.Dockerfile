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

#FROM python:3.9.18
FROM ubuntu:22.04
RUN apt-get update -y
RUN apt-get install -y iproute2
RUN apt-get install -y iputils-ping
RUN apt-get install -y netcat
RUN apt-get install -y iperf3
RUN apt-get install -y net-tools
RUN apt-get install -y tcpdump
RUN apt-get install -y ethtool
RUN apt-get install -y sudo
RUN apt-get install -y ca-certificates
COPY teste2e/ /var/mizar/test
RUN mkdir -p /etc/ssl/certs
COPY teste2e/scripts/test_key.pem /etc/ssl/certs/
COPY teste2e/scripts/test_server_cert.pem /etc/ssl/certs/
COPY teste2e/scripts/test_client_cert.pem /etc/ssl/certs/
RUN update-ca-certificates
EXPOSE 8000 9001 5001 7000 7443
CMD /var/mizar/test/scripts/run_servers.sh
