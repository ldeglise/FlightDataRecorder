# Cahier des charges de FlightDataRecorder

## Objectifs

FlightDataRecorder est un plugin natif pour X-Plane 12 et 11. Il est ecrit en C++ et utilise le SDK X-Plane qui semble être écrit en C.

Ce projet devra être pouvoir être compilé pour X-Plane 12 et 11 sur les cibles suivantes : Windows 10 et 11, Linux, et Mac OS.

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
- QNH (hpa et hdg)
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
- total power (HP)
- power per engine (HP)

Toutes les autres informations seront écrites toutes les secondes (1hz) dans un buffer qui sera écrit dans le fichier toutes les 20 secondes afin de limiter les accès disque.

L'écriture du GeoJson commencera après détéction du décollage, et prendra fin après détéction de l'atterissage.