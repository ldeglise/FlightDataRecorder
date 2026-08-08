# Expression de besoins

## Contexte et objectifs

Dans le monde de la simulation de vol, et particulièrement X-Plane 12, il n'existe pas de plugin natif permettant d'enregistrer certains paramètres concernant un vol afin de produire un fichier GeoJson. Ce type de fichier s'avère pourtant fort utile a des fin d'analyse post vol, à des fins d'utilisation dans des système SIG, ou simplement pour etre affiché dans des logiciels comme Google Earth, Open Street Map et tous systèmes pouvant importer du GeoJson.

En effet de nombreux pilotes virtuels peuvent vouloir garder des traces de leurs vols pour le traitement de leur choix ; et pourtant ne souhaitent pas s’inscrire sur des plateforme de tracking comme Volanta, Elevatex, FsHub, ou autres.

Le plugin natif FlightDataRecorder a pour ambition de palier à ce manque en proposant une solution simple, légère, et automatique d'enregistrer certains paramètres concernant un vol, et de produire un fichier GeoJson conforme à la RFC 7946.

### But principal et gains attendus

Fournir une solution, fiable et propre, pour enregistrer des vols dans X-Plane pour pouvoir réutiliser ces données dans des systèmes SIG.

## Périmètre et cibles

### Périmètre de monitoring

Les dataref suivante devront être monitorées

- Code OACI de l'appareil               dataref : "sim/aircraft/view/acf_ICAO"                      (string)
- Modèle d'appareil                     dataref : "sim/aircraft/view/acf_ui_name"                   (string)
- Nombre de moteurs                     dataref : "sim/aircraft/engine/acf_num_engines"             (int)
- Puissance totale par moteur           dataref : "sim/aircraft/engine/acf_pmax"                    (float)(watts)
- Secondes depuis minuit Zulu           dataref : "sim/time/zulu_time_sec"                          (float)(secondes)
- Jours depuis le 1er janvier (local)   dataref : "sim/time/local_date_days"                        (int)(jours)
- Longitude                             dataref : "sim/flightmodel/position/longitude"              (double)(degrés décimaux)
- Latitude                              dataref : "sim/flightmodel/position/latitude"               (double)(degrés décimaux)
- Altitude barométrique (MSL)           dataref : "sim/flightmodel/position/elevation"              (double)(mètres)
- Hauteur (AGL)                         dataref : "sim/flightmodel/position/y_agl"                  (float)(mètres)
- Cap vrai                              dataref : "sim/flightmodel/position/psi"                    (float)(degrés)
- Cap magnétique                        dataref : "sim/flightmodel/position/mag_psi"                (float)(degrés)
- Vitesse indiquée                      dataref : "sim/flightmodel/position/indicated_airspeed"     (float)(km/h)
- Vitesse sol                           dataref : "sim/flightmodel/position/groundspeed"            (float)(km/h)

### Cibles

Plugin natif pour X-Plane 12, et X-Plane 11 sur les OS Windows, Linux, et Mac OS.

## Besoins fonctionnels

Au lancement de chaque nouveau vol, le plugin doit monitorer les datarefs prévus ci dessus a une fréquence de 1hz.

A la detection du décollage, le plugin doit créer un fichier GeoJson et ecrire dedans les dataref monitorées avec une fréquence de 1hz.

Les données sont écris dans le GeoJSON avec les unitées au format SI de manière à être compatible avec les sytèmes SIG.
Longitude et latitude devront être en degrés décimaux pour être compatible avec les sytèmes SIG.
Les unités utilisées devront être indiquées dans le GeoJSON une et une seul fois, sous forme de meta data.
Si des convertion ou des calcule sont a faire par des sytèmes tiers, ce sera de leur responsabilité des les faire.

Attention, le code OACI de l'appareil, le modèle de l'appareil, le nombre de moteur, et la puissance total de l'appareil (somme de tous les moteurs), ne seront écrit qu'une et une seul fois, sous forme de meta data dans le fichier GeoJSON

Toutefois, pour eviter des acces disque trop fréquent, le plugin se servira d'un tampon mémoire pour stoker les infomations, et n'écrira physiquement sur le disque que toutes les 20 secondes, donc 20 points de mesure.

Le fichier GeoJSON devra fournir a la fois les points de mesure (toutes les secondes), mais aussi la trace du vol sous forme d'une ligne.
E effet l'exploitation a posteriori de ce GeoJSON pour porter soit sur l'analyse des points de mesure, soit sur le tracé du vol, soit les deux.

L'écriture s'arrete et le fichier est fermé à la detection de l'atterrissage.

La détéction du décollage et de l'atterrissage doit etre automatique, fiable, et pertinente afin de se premunir d'eventuelle rebonds de l'appareil sur la piste.

Le fichier GeoJSON devra être enregistré dans le dossier FligtDataRecorder situer dans le dossier de l'utilisateur en fonction de l'OS. Ce dossier sera a créer par le plugin lors de la premiere utilisation du plugin, ou recréer i celui ci a était effacé.

Le fichier aura comme nom la date et l'heure (zulu) du décollage (au format ISO 8601 ) issue du simulateur, avec l'extention .geojson
Il faudra prevoir en préfix du nom de fichier un aléa permettant de se premunir de la création d'un fichier pouvant écraser un autre fichier ayant commmencé à la même date et à la même heure.

Exemple : XXXX_AAAA-MM-JJTHH:mm:SS.geojson

Le fichier GeoJSON devra être 100% conforme à la RFC 7946

Si le plugin est desactivé, ou à la fermeture de x-plane (plugin stop) tous les fichiers ouverts doivent etre fermés proprement

Le plugin ne possède pas d'ihm, ni de menu dans X-Plane. Pour le desactiver, la seul solution est de passer par le plugin admin de X-Plane. Par conséquence, sans action volontaire de l'utilisateur ce plugin est toujours actif lors d'un vol.
