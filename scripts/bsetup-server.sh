#!/bin/sh
bluetoothctl power on
bluetoothctl agent on
bluetoothctl discoverable on
bluetoothctl pairable on
setsid -f bluetoothctl scan on >/dev/null &
