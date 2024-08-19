#!/bin/sh

iwlist scan 2>/dev/null |
	{
		awk -F ':' '/Channel:/ { print $2; }'
		seq 1 11
	} |
	sort -n |
	uniq -c |
	sort -n -k 1 |
	head -n 1 |
	{ read count chan; echo "$chan"; }
