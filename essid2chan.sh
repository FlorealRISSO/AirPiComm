#!/bin/sh

[ "$1" ] || { printf 'usage: %s <essid>\n' "$(basename "$0")" >&2; exit 1; }
essid=$1

iwlist scan 2>/dev/null |
while read fst _; do echo "$fst"; done |
while IFS=':' read key val; do
	case $key in
	Channel)
		printf '%s' "$val"
		;;
	ESSID)
		printf '%s\n' "$val"
		;;
	esac
done |
sort |
uniq |
grep -F "\"$essid\"" |
cut -d '"' -f 1 |
sort -n |
head -n 1
