iptables -F

iptables -t nat -vnL POSTROUTING --line-number
iptables -t nat -D POSTROUTING 1



#use follow cmd to do nat and dnat
iptables -t nat -A POSTROUTING -s 10.0.0.0/24 -j SNAT --to-source 59.66.134.69
iptables -t nat -vnL POSTROUTING --line-number

#use follow cmd to avoid rst disconnect tcp
iptables  -t filter -A OUTPUT -p tcp --tcp-flags ALL RST -j DROP
iptables -t filter -vnL OUTPUT --line-number

#echo 1 > /proc/sys/net/ipv4/ip_forward

#USE FOLLOW CMD TO ESTABLISH AN VIRTUAL INTERFACE
#sudo ifconfig enp3s0:0 10.0.0.3 up
