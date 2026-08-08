# Expression de besoins

## Contexte et objectifs

Dans le monde de la simulation de vol, et particulièrement X-Plane 12, il n'existe pas de plugin natif permettant d'enregistrer certains paramètres concernant un vol afin de produire un fichier GeoJson. Ce type de fichier s'avère pourtant fort utile a des fin d'analyse post vol, à des fins d'utilisation dans des système SIG, ou simplement pour etre affiché dans des logiciels comme Google Earth, Open Street Map et tous systèmes pouvant importer du GeoJson.

En effet de nombreux pilotes virtuels peuvent vouloir garder des traces de leurs vols pour le traitement de leur choix ; et pourtant ne souhaitent pas s’inscrire sur des plateforme de tracking comme Volanta, Elevatex, FsHub, ou autres.

Le plugin natif FlightDataRecorder a pour ambition de palier à ce manque en proposant une solution simple, légère, et automatique d'enregistrer certains paramètres concernant un vol, et de produire un fichier GeoJson conforme à la RFC 7946.

### Rappel de l'existant et des raisons du projet

### But principal et gains attendus

## Périmètre et cibles

### Périmètre de monitoring

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

### Utilisateurs finaux ou publics concernés

### Limites de ce qui est inclus ou exclu du projet

## Besoins fonctionnels

### Liste des actions ou des options principales

### Classement par ordre de priorité (essentiel ou secondaire)

## Contraintes et repères

### Dates clés et calendrier prévisionnel

### Enveloppe financière ou budget maximal
