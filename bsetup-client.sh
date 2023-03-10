#!/bin/sh
addr=${BADDR:-B8:27:EB:24:E8:A5}
bluetoothctl power on
bluetoothctl agent on
bluetoothctl discoverable on
bluetoothctl pairable on
bluetoothctl scan on &
pid=$!
sleep 30
bluetoothctl trust "$addr"
bluetoothctl pair "$addr"
kill -15 $pid
bluetoothctl scan off
