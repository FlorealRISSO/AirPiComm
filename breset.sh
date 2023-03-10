#!/bin/sh
addr=${BADDR:-B8:27:EB:24:E8:A5}
bluetoothctl remove "$addr"
bluetoothctl discoverable off
bluetoothctl pairable off
