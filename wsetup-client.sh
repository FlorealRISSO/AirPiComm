#!/bin/sh
[ $(id -u) -eq 0 ] || {
	printf 'this script must be run as root.\n' >&2
	exit 1
}
[ "$1" ] || {
	printf 'usage: %s <ip>\n' "$(basename "$0")" >&2
	exit 1
}
# 192.168.15.40
ip=$1
essid=Flalexis
chan=
while [ -z "$chan" ]; do
	chan=$(./essid2chan.sh "$essid")
done
printf 'ip: %s, chan: %s\n' "$ip" "$chan"
systemctl stop networking dhcpcd wpa_supplicant
iwconfig wlan0 mode ad-hoc channel "$chan" essid "$essid"
ifconfig wlan0 "$ip" netmask 255.255.255.0
