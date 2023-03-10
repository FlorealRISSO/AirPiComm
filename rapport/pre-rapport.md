# Quelques informations

Le contenu nécessaire pour le rapport:

- Faire une démonstration étape par étape de la connexion à l'envoi des donneés.
    - Inventer un petit scénario, dans lequel nous mettons en scène cette démonstration. (contexte + explication)
    - Capture d'écran des étapes.
    - Copie du terminal.
    - Preuve de la bonne réception des donnés.

- Expliqué nos logiciels et script.
    - Fonctionnement du logiciel en C (sender).
    - Fonctionnement des scripts.
    - Explication de l'alchimie entre les logiciels.

- Faire une archive des codes (avec un makefile).


## Explication

### Sender

Ce logiciel est composé de deux exécutables: un client et un serveur.

#### Le serveur

Le serveur se lance avec de nombreux paramètres, le mode utilisé qui peut être tcp/ip, ou bluetooth.

- En mode bluetooth il attend : un canal pour communiquer, le nombre de
connexions simultanées maximal et, de façon optionnel, un flag activant
l'affichage des logs dans la console.
- En mode tcp/ip il attend : un port, le nombre de connexions simultanées
maximal et, de façon optionel, un flag activant l'affichage des logs
dans la console. Il restera à l'écoute du port indéfiniment.

Usage:
```txt
Usage: server <MODE> <port/channel> <max_client> [-l]
-l :  enable logging
MODE:
  -b :  bluetooth
  -i :  ip
```

Exemple:
```sh
# lance le serveur à l'écoute du port 9999 en mode tcp/ip, accepte au plus 1 client et affiche les logs.
server -i 9999 1 -l

# lance le serveur à l'écoute du port 19191 en mode tcp/ip, accepte au plus 1000 client.
server -i 19191 1000


# lance le serveur sur le canal 10 en mode bluetooth, accepte au plus 1 client et affiche les logs.
server -b 10 1 -l

# lance le serveur sur le canal 1 en mode bluetooth, accepte au plus 8 client.
server -b 1 8
```

#### Le client

Le client se lance avec de nombreux paramètres, en premier lieu le mode
utilisé qui peut-être tcp/ip ou bluetooth.
- En mode bluetooth il attend : une adresse MAC bluetooth, un canal, une liste d'action puis le flag de log.
- En mode tcp/ip il attend : une adresse ip, un port, une liste d'action puis le flag de log.

Une action peut être : l'envoi d'une liste de fichier, l'envoi d'une commande à exécuter.

Usage:
```txt
Usage: client MODE <addr> <port/channel> [-f <file>] [-c <command>] [-l]
-l :  enable logging
MODE:
  -b : bluetooth
  -i : ip
```

Exemple:
```txt
# lance le client et envoie les fichiers f1.txt, f2.txt et f3.txt sur le canal 1 à l'addresse AA:AA:AA:AA en affichant les logs.
client -b AA:AA:AA:AA 1 -f f1.txt f2.txt f3.txt -l

# lance le client et envoie le fichier f1.txt sur le canal 1 à l'addresse AA:AA:AA:AA.
client -b AA:AA:AA:AA 1 -f f1.txt

# lance le client et exécute la commande `ls` sur la machine AA:AA:AA:AA via le canal 1.
client -b AA:AA:AA:AA 1 -c ls


# lance le client et envoie les fichiers f1.txt, f2.txt et f3.txt sur le port 10000 à l'addresse AA:AA:AA:AA en affichant les logs.
client -i 127.0.0.1 10000 -f f1.txt f2.txt f3.txt -l

# lance le client et envoie le fichier f1.txt sur le port 10000 à l'addresse 127.0.0.1.
client -i 127.0.0.1 10000 -f f1.txt

# lance le client et exécute la commande `ls` sur la machine 127.0.0.1 via le port 10000.
client -i 127.0.0.1 10000 -c ls
```

### Scripts

Afin d'automatiser un certain nombre de tâches, en particulier de mise en place des réseaux utilisés, nous avons dévelopé les scripts suivant:

- `bestchan.sh` : permet d'obtenir le channel le moins utilisé parmi les stations environnantes.
e.g.:
```sh
$ ./bestchan.sh
1
```

- `essid2chan.sh` : permet d'obtenir le channel utilisé par une station en particulier.
e.g.:
```sh
$ ./essid2chan.sh
usage: essid2chan.sh <essid>
$ ./essid2chan.sh Flalexis
2
```

- `wsetup-server.sh` : mise en place d'un réseau WiFi ad-hoc (côté serveur).
e.g.: en tant que root :
```sh
$ ./wsetup-server.sh 192.168.15.40
```

- `wsetup-client.sh` : mise en place d'un réseau WiFi ad-hoc (côté client).
e.g.: en tant que root :
```sh
$ ./wsetup-client.sh 192.168.15.41
```

- `bsetup-client.sh` : script d'appariement des appareils en bluetooth.
  l'adresse MAC bluetooth utilisée peut-être modifié en définissant la variable
  d'environnement `BADDR`.

- `bsetup-server.sh` : script d'appariement des appareils en bluetooth.

- `breset.sh` : réinitialisation des paramètres bluetooth et suppression d'un appareil
  l'adresse MAC bluetooth utilisée peut-être modifié en définissant la variable
  d'environnement `BADDR`.

Nous avons aussi développé un script, `ishell.sh`, permettant de faire
un reverse-shell en utilisant netcat (cela constiturait donc une solution
élémentaire à la deuxième partie tu TP :

côté serveur :
```sh
$ ./ishell.sh
````

côté client, on se connecte au serveur qui donne accès à un shell
intéractif :
```sh
$ nc 192.168.15.41 6000
pwd
/home/pi/tp_rsf
whoami
pi
...
```


## Démonstration

Idée de démonstration, soit rasberry A (rA) un système embarqué (caméra de surveillance par exemple),
soit rasberry B (rB) un ordintateur depuis lequel on veut controller le rA.

Nous supposons que le script qui configure le réseau en "ad hoc" se lance automatiquement au demarrage du rA,
ainsi que sender en mode serveur tcp/ip.

(tous doit ce faire via sender en mode TCP.)
Scénario
1. rB trouve le réseau de rA.
2. rB se connecte à rA.
3. rB envois un fichier à rA (example script d'apairage).
4. rB exécute le script d'apairage sur rA.
5. rB lance sur rA sender en mode serveur bluetooth.
6. rB se connecte en bluetooth à rA.
7. rB exécute le script pour trouver le meilleurs canal.
8. rB lance le logiciel C en mode bluetooth sur rA.
9. rB lance une commande via bluetooth à rA.


### Setup du réseau Wifi

Serveur:

Sur le serveur, `wsetup-server.sh` s'exécute au lancement de la machine, elle
créé un réseau ad-hoc sur le canal le moins encombré en le détectant avec le
script `bestchan.sh`.

```sh
$ sudo ./wsetup-server.sh 192.168.15.41
ip: 192.168.15.41, chan: 2
```

Pendant ce temps, sur le client:

L'utilisateur lance le script `wsetup-client.sh`, qui détecte le CANAL
correspondant au ESSID utilisé, et configure l'interface réseau pour
communiquer sur le couple (ESSID, CANAL).

```sh
$ sudo ./wsetup-server.sh 192.168.15.40
ip: 192.168.15.40, chan: 2
```

### Échange de données

Serveur:

Automatiquement, après le setup, le serveur lance le programme
`sender/server` en mode TCP/IP. Afin de n'avoir jamais a intéragir avec
la machine physiquement.

```sh
$ ./sender/server -i 9999 10 -l
[1906] Waiting for a new client
```

Client:

L'utilisateur lance de son côté le programme `sender/client` en mode TCP/IP.
Et il en profite pour envoyer le script de setup du bluetooth `bsetup-server.sh`.

```sh
$ ./sender/client -i 192.168.15.41 9999 -f bsetup-server.sh'
```

Cette commande envoie le fichier de setup bluetooth côté serveur
`bsetup-server.sh`.

Cela créé les logs suivant sur le serveur:

```sh
[1907] Waiting for a new job
[1907] Downloading a file...
[1907] Create : tmp/tmp_1644611798.out
[1907] Receiving file...
[1907] File received
[1907] Waiting for a new job
[1907] The connection has been closed
```

### Setup bluetooth

Client:

Pour mettre en place la connexion bluetooth, le client doit récupérer
l'adresse MAC bluetooth du serveur puis exécuter le script envoyé au
serveur. Ce script permet d'activer le bluetooth.

```sh
# Permet au client de récupérer l'adresse mac du serveur dans la variable bmac.
$ bmac=$(
    sender/client -i 192.168.15.41 9999 \
        -c 'bluetoothctl show | head -n 1' |
            tail -n 1 |
            cut -d ' ' -f 2
  )

# lance le script de setup bluetooth sur le serveur.
$ sender/client -i 192.168.15.41 9999 \
    -c 'sh tmp/$(ls -t tmp | head -n 1)'
 sh tmp/$(ls -t tmp | head -n 1)

# lance le script de setup bluetooth sur le client.
$ BADDR=$bmac ./bsetup-client
```

Serveur:

Aucune action n'est requise par le serveur.

Voici les logs:
```
[1908] Waiting for a new job
[1908] Execution a cmd
[1908] $ sh bmac=$(sender/client -i 192.168.15.41 9999 \
    -c 'bluetoothctl show | head -n 1' | tail -n 1 | cut -d ' ' -f 2)
[1908] Waiting for a new job
[1908] The connection has been closed
[1909] Waiting for a new job
[1909] Execution a cmd
[1909] $ sh tmp/$(ls -t tmp | head -n 1)
[1909] Waiting for a new job
[1909] The connection has been closed
```

Les deux machines sont connectées en bluetooth.

### Échange de données en bluetooth

Maintenant que les deux machines sont connectées en bluetooth, il faut lancer
sender/server en mode bluetooth.

Client:

```sh
$ sender/client -i 192.168.15.41 9999 -c 'sender/server -b 1 -l'
```

Dorénavant nous pouvons communiquer avec le serveur en bluetooth
seulement. Par exemple, on peut faire le traditionnel "Hello, World!" en
bluetooth :

```sh
$ sender/client -b 1 1 -c 'echo "Hello, world!"'
Hello, world!
```

