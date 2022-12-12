#!/bin/bash

# No switch
/system reset-configuration
/interface bridge add name=bridge20
/interface bridge add name=bridge21

# Switch eth3 -> Tux3 eth0
/interface bridge port remove [find interface =ether3]
/interface bridge port add bridge=bridge20 interface=ether3

# Switch eth2 -> Tux2 eth0
/interface bridge port remove [find interface =ether2]
/interface bridge port add bridge=bridge21 interface=ether2


# Switch eth4 -> Tux4 eth0
/interface bridge port remove [find interface =ether4]
/interface bridge port add bridge=bridge20 interface=ether4
# Switch eth5 -> Tux4 eth1
/interface bridge port remove [find interface =ether5]
/interface bridge port add bridge=bridge21 interface=ether5

# Switch eth6 -> Router eth2
/interface bridge port remove [find interface =ether6]
/interface bridge port add bridge=bridge21 interface=ether6

# TUX 3
systemctl restart networking
ifconfig eth0 up 
ifconfig eth0 172.16.20.1/24
route add -net 172.16.21.0/24 gw 172.16.20.254

# Tux 4
systemctl restart networking
ifconfig eth0 up 
ifconfig eth0 172.16.20.254/24
ifconfig eth1 up 
ifconfig eth1 172.16.21.253/24
route add default gw 172.16.21.254

# Subrouting
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcast

# Tux 2
systemctl restart networking
ifconfig eth0 up 
ifconfig eth0 172.16.21.1/24
route add -net 172.16.20.0/24 gw 172.16.21.253
route add default gw 172.16.21.254


# Router
/ip address add address=172.16.2.29/24 interface=ether1
/ip address add address=172.16.21.254/24 interface=ether2
/ip route add dst-address=0.0.0.0/0 gateway=172.16.21.254
/ip route add dst-address=172.16.20.0/24 gateway=172.16.21.253



