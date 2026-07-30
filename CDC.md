# Cahier des charges de FlightDataRecorder

## Objectifs

FlightDataRecorder est un plugin natif pour X-Plane 12 et 11. Il est écrit en C++ (ou en C si plus pertinent) et utilise le SDK X-Plane qui semble être écrit en C.

Ce projet devra être pouvoir être compilé pour X-Plane 12 et 11 sur les cibles suivantes : Windows 10 et 11, Linux, et Mac OS sous forme d'un plugin X-Plane avec l'extention .xpl (dll, so, dynalib)

Il devra recupérer les informations suivantes toute les secondes (1hz) :

- Aircraft ICAO
- Aircraft name
- Number of engines (int)
- total power (HP)
- power per engine (HP)
- timestampe (LOCAL)
- timestampe (ZULU)
- longitude (Degrés décimaux)
- latitude (Degrés décimaux)
- altitude mean sea level (MSL en pieds)
- altitude above ground level (AGL en pieds)
- QNH (hpa et inHG)
- indicated air speed (IAS en noeuds)
- ground speed (GS en noeuds)
- vertical speed (FPM)
- cap (DEG mag)
- heading (DEG true)
- route mag (DEG mag)
- route true (DEG true)
- pitch (DEG)
- bank (DEG)

Ces informtions seront ensuite utilisées pour générer un fichier GeoJson conforme à la RFC 7946.
Le données suivantes ne seront écrites qu'une seule fois dans le GeoJson sous forme de metadata :

- Aircraft ICAO
- Aircraft name
- Number of engines (int)
- Type de moteur (pistons, turboprops, jet)
- total power (HP quelque soit le type de moteur)
- power per engine (HP quelque soit le type de moteur)

Toutes les autres informations seront écrites toutes les secondes (1hz) dans un buffer qui sera écrit dans le fichier toutes les 20 secondes afin de limiter les accès disque.

Le fichier GeoJson sera écrit dans le dossier output de X-Plane avec la convention de nommage suivante : ISO_8601.geojson

L'écriture du GeoJson commencera à la détection du décollage, et prendra fin à la détection de l'atterrissage.

les critères de détection sont les suivants :

- décollage : Altitude AGL > 0 et GS > 40 kts pendant au moins 10 secondes.
- atterrissage : altitude AGL = 0 et GS >= 40 kts pendant au moins 10 secondes.

## Architecture

Le point d'entrée du plugin est le fichier main.cpp . Il est chargé du bon fonctionnement du plugin : start, stop, enable, desable, activate, deactivate, callback, etc.

La recupération des datarefs et le stockage sera fait par une classe DataPolling.hpp / DataPolling.cpp. la recuperation des datarefs est drivée par le callback de 1HZ. les datarefs sont stokées dans les attribus de la classe prevus a cet effet, Les données sont misent a jour a chaque callback.

La classe, une fois instanciée par main.cpp doit tourner dans un thread séparé asynchrone.

Le monitoring concerant le décollage et l'atterissage devra se faire dans la classe Monitoring.hpp / monitoring.cpp. Cette classe est instanciée par main.cpp et est drivée par le callback.

L'ecriture du fichier GeoJson est effectuée par la classe WriteGeojson.hpp / WriteGeojson.cpp. L'écriture commence à la détéction du décollage, et prend fin à la détéction de l'atterrissage.
Cette classe tournera dans un thread séparé asynchrone, et ira lire les données du DataPolling toutes les 1 secondes, les placera dans un buffer, et fera un flushde ce buffer vers le fichier sur disque toutes les 20 secondes.
