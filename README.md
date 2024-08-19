# Project Report

> Floréal Risso and Alexis Paronnaud \
> SECIL

## Table of Contents

1. [Program Explanation](#program-explanation)
    - [Sender](#sender)
    - [Scripts](#scripts)
2. [Demonstration](#demonstration)
    - [Wi-Fi Setup](#wi-fi-setup)
    - [Bluetooth Setup and Data Exchange](#bluetooth-setup-and-data-exchange)

## Program Explanation

### Sender

The Sender software consists of two executables: a client and a server.

#### Server

The server can be started with several parameters, depending on the mode—TCP/IP or Bluetooth.

- **Bluetooth Mode:** The server requires a channel for communication, the maximum number of simultaneous connections, and optionally, a flag to enable logging in the console.
- **TCP/IP Mode:** The server requires a port number, the maximum number of simultaneous connections, and optionally, a flag to enable logging. The server will listen on the specified port indefinitely.

**Usage:**
```txt
Usage: server <MODE> <port/channel> <max_client> [-l]
-l :  enable logging
MODE:
  -b :  bluetooth
  -i :  ip
```

**Examples:**
```sh
# Start the server in TCP/IP mode on port 9999, allowing up to 1 client, with logging enabled.
server -i 9999 1 -l

# Start the server in TCP/IP mode on port 19191, allowing up to 1000 clients.
server -i 19191 1000

# Start the server in Bluetooth mode on channel 10, allowing up to 1 client, with logging enabled.
server -b 10 1 -l

# Start the server in Bluetooth mode on channel 1, allowing up to 8 clients.
server -b 1 8
```

#### Client

The client can be started with several parameters, depending on the mode—TCP/IP or Bluetooth.

- **Bluetooth Mode:** The client requires a Bluetooth MAC address, a channel, a list of actions, and optionally, a flag to enable logging.
- **TCP/IP Mode:** The client requires an IP address, a port number, a list of actions, and optionally, a flag to enable logging.

Actions can include sending a list of files or executing a command on the target machine.

**Usage:**
```txt
Usage: client MODE <addr> <port/channel> [-f <file>] [-c <command>] [-l]
-l :  enable logging
MODE:
  -b : bluetooth
  -i : ip
```

**Examples:**
```txt
# Send files f1.txt, f2.txt, and f3.txt over Bluetooth to the address AA:AA:AA:AA on channel 1 with logging enabled.
client -b AA:AA:AA:AA 1 -f f1.txt f2.txt f3.txt -l

# Send file f1.txt over Bluetooth to the address AA:AA:AA:AA on channel 1.
client -b AA:AA:AA:AA 1 -f f1.txt

# Execute the command `ls` on the machine with the Bluetooth address AA:AA:AA:AA via channel 1.
client -b AA:AA:AA:AA 1 -c ls

# Send files f1.txt, f2.txt, and f3.txt over TCP/IP to the address 127.0.0.1 on port 10000 with logging enabled.
client -i 127.0.0.1 10000 -f f1.txt f2.txt f3.txt -l

# Send file f1.txt over TCP/IP to the address 127.0.0.1 on port 10000.
client -i 127.0.0.1 10000 -f f1.txt

# Execute the command `ls` on the machine with IP address 127.0.0.1 via port 10000.
client -i 127.0.0.1 10000 -c ls
```

### Scripts

To automate several tasks, particularly setting up the networks, we have developed the following scripts:

- **`bestchan.sh`:** Identifies the least-used Wi-Fi channel among nearby stations.
    ```sh
    $ ./bestchan.sh
    1
    ```

- **`essid2chan.sh`:** Identifies the channel used by a specific station.
    ```sh
    $ ./essid2chan.sh <essid>
    $ ./essid2chan.sh Flalexis
    2
    ```

- **`wsetup-server.sh`:** Sets up an ad-hoc Wi-Fi network (server-side).
    ```sh
    $ sudo ./wsetup-server.sh 192.168.15.40
    ```

- **`wsetup-client.sh`:** Sets up an ad-hoc Wi-Fi network (client-side).
    ```sh
    $ sudo ./wsetup-client.sh 192.168.15.41
    ```

- **`bsetup-client.sh`:** Pairs devices over Bluetooth. The Bluetooth MAC address can be set via the `BADDR` environment variable.
  
- **`bsetup-server.sh`:** Pairs devices over Bluetooth.
  
- **`breset.sh`:** Resets Bluetooth settings and removes a paired device. The Bluetooth MAC address can be set via the `BADDR` environment variable.

We also developed a script, `ishell.sh`, which sets up a reverse shell using `netcat`. This script provides a simple solution for the second part of the lab exercise:

On the server side:
```sh
$ ./ishell.sh
```

On the client side, connect to the server, which gives access to an interactive shell:
```sh
$ nc 192.168.15.41 6000
pwd
/home/pi/tp_rsf
whoami
pi
...
```

## Demonstration

This demonstration scenario involves Raspberry Pi A (rA), an embedded system (e.g., a surveillance camera), and Raspberry Pi B (rB), a control device.

We assume that the script configuring the ad-hoc network automatically runs on rA at startup, along with the Sender server in TCP/IP mode.

### Wi-Fi Setup

#### Server

The `wsetup-server.sh` script automatically runs on the server at startup, setting up an ad-hoc network on the least congested channel, determined by the `bestchan.sh` script.

```sh
$ sudo ./wsetup-server.sh 192.168.15.41
ip: 192.168.15.41, chan: 2
```

#### Client

Meanwhile, on the client side, the user runs the `wsetup-client.sh` script, which detects the channel corresponding to the ESSID and configures the network interface accordingly.

```sh
$ sudo ./wsetup-client.sh 192.168.15.40
ip: 192.168.15.40, chan: 2
```

### Bluetooth Setup and Data Exchange

#### Data Exchange via Wi-Fi

After the Wi-Fi setup, the server automatically starts the `sender/server` in TCP/IP mode, allowing remote interaction without physical access to the machine.

```sh
$ ./sender/server -i 9999 10 -l
[1906] Waiting for a new client
```

The client then starts the `sender/client` program in TCP/IP mode and uses it to send the Bluetooth setup script (`bsetup-server.sh`) to the server.

```sh
$ ./sender/client -i 192.168.15.41 9999 -f bsetup-server.sh
```

The server logs will reflect the file transfer:

```sh
[1907] Waiting for a new job
[1907] Downloading a file...
[1907] Create : tmp/tmp_1644611798.out
[1907] Receiving file...
[1907] File received
[1907] Waiting for a new job
[1907] The connection has been closed
```

#### Bluetooth Setup

To establish the Bluetooth connection, the client retrieves the server's Bluetooth MAC address and executes the setup script on the server.

```sh
# Retrieve the server's Bluetooth MAC address.
bmac=$(
  sender/client -i 192.168.15.41 9999 \
    -c 'bluetoothctl show | head -n 1' |
    tail -n 1 |
    cut -d ' ' -f 2
)

# Execute the Bluetooth setup script on the server.
sender/client -i 192.168.15.41 9999 -c 'sh tmp/$(ls -t tmp | head -n 1)'

# Execute the Bluetooth setup script on the client.
BADDR=$bmac ./bsetup-client
```

The server logs show the executed commands:

```sh
[1908] Waiting for a new job
[1908] Executing a command
[1908] $ sh bmac=$(sender/client -i 192.168.15.41 9999 \
    -c 'bluetoothctl show | head -n 1' | tail -n 1 | cut -d ' ' -f 2)
[1908] Waiting for a new job
[1908] The connection has been closed
[1909] Waiting for a new job
[1909] Executing a command
[1909] $ sh tmp/$(ls -t tmp | head -n 1)
[1909] Waiting for a new job
[1909] The connection has been closed


```

Now both machines are connected via Bluetooth.

#### Data Exchange via Bluetooth

With the Bluetooth connection established, the `sender/server` is started in Bluetooth mode.

```sh
$ sender/client -i 192.168.15.41 9999 -c 'sender/server -b 1 -l'
```

From this point forward, all communication can be done over Bluetooth. For example, you can run the classic "Hello, World!" command via Bluetooth:

```sh
$ sender/client -b 1 1 -c 'echo "Hello, world!"'
Hello, world!
```
