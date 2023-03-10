#!/bin/sh
[ $(id -u) -eq 0 ] || {
	printf 'this script must be run as root.\n' >&2
	exit 1
}
[ "$1" ] || {
	printf 'usage: %s <ip>\n' "$(basename "$0")" >&2
	exit 1
}
ip=$1
chan=$(./bestchan.sh)
printf 'ip: %s, chan: %s\n' "$ip" "$chan"
# 192.168.15.41
systemctl stop networking dhcpcd wpa_supplicant
iwconfig wlan0 mode ad-hoc channel "$chan" essid Flalexis
ifconfig wlan0 "$ip" netmask 255.255.255.0
