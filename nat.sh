iptables -F

iptables -t nat -vnL POSTROUTING --line-number
iptables -t nat -D POSTROUTING 1



#iptables -L FORWARD
#iptables -D FORWARD 1

iptables -t nat -A POSTROUTING -s 10.0.0.0/24 -j SNAT --to-source 59.66.134.64
iptables -t nat -vnL POSTROUTING --line-number



#iptables -t filter -A FORWARD -d 10.0.0.0/24  -j ACCEPT
#iptables  -A FORWARD -j QUEUE
#iptables -L FORWARD

#iptables –A FORWARD –d 10.0.0.0/24 –j accept
#echo 1 > /proc/sys/net/ipv4/ip_forward

#sudo ifconfig enp3s0:0 10.0.0.3 up
