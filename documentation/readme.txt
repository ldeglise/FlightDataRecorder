================================================================================
FLIGHT DATA RECORDER - Plugin X-Plane
================================================================================

DESCRIPTION
-----------
Flight Data Recorder est un plugin pour X-Plane qui enregistre automatiquement
les données de vol au format GeoJSON. Il capture la position, l'altitude, les
vitesses et les caps de votre avion pendant toute la durée du vol.

FONCTIONNALITÉS
--------------
- Enregistrement automatique des vols (détection de décollage/atterrissage)
- Format GeoJSON compatible avec la plupart des outils SIG (QGIS, Google Earth, etc.)
- Précision des coordonnées : 6 décimales (~10 cm)
- Filtrage des données pour réduire les zigzags à basse vitesse
- Métadonnées de vol incluses (heure de décollage, configuration avion)

INSTALLATION
------------
1. Copier le dossier du plugin dans :
   - Windows : X-Plane 11/Resources/plugins/
   - macOS : X-Plane 11/Resources/plugins/
   - Linux : X-Plane 11/Resources/plugins/

2. Lancer X-Plane

3. Le plugin se lance automatiquement et commence à enregistrer dès le décollage

UTILISATION
----------
Aucune configuration nécessaire ! Le plugin fonctionne automatiquement :

- Détection automatique du décollage (AGL > 10m pendant 10 secondes)
- Détection automatique de l'atterrissage (AGL < 1m pendant 20 secondes)
- Enregistrement des données toutes les 10 secondes

FICHIERS GÉNÉRÉS
----------------
Les fichiers sont enregistrés dans :
- Windows : %USERPROFILE%\FlightDataRecorder\
- macOS/Linux : ~/FlightDataRecorder/

Format des fichiers :
  AAAA-MM-JJTHH-MM-SSZ.geojson
  (ex: 2026-08-10T14-30-20Z.geojson)

Chaque fichier contient :
- Une FeatureCollection GeoJSON
- Des points (Point features) avec toutes les données de vol
- Une ligne (LineString feature) représentant la trace du vol lissée

DONNÉES ENREGISTRÉES
--------------------
Pour chaque point :
- Coordonnées (longitude, latitude) - 6 décimales
- Altitude MSL (mètres)
- Cap vrai (degrés)
- Cap magnétique (degrés)
- Vitesse indiquée (IAS) en km/h
- Vitesse sol (GS) en km/h
- Timestamp ISO 8601
- Heure Zulu (HH:mm:ss)

Métadonnées du vol :
- Nombre de moteurs (seed)
- Puissance totale (checksum)
- Heure de décollage

VISUALISATION DES DONNÉES
-------------------------
Vous pouvez visualiser les fichiers GeoJSON avec :

1. QGIS (recommandé) :
   - Ouvrir QGIS
   - Glisser-déposer le fichier .geojson
   - La trace apparaîtra avec les points de données

2. Google Earth :
   - Convertir le GeoJSON en KML avec un outil en ligne
   - Ouvrir le fichier KML dans Google Earth

3. Sites web :
   - Utiliser des outils comme geojson.io pour visualiser en ligne

TECHNIQUE
--------
- Précision : 6 décimales (~10 cm) pour les coordonnées
- Lissage : Moyenne mobile sur 3 points pour la LineString
- Format : GeoJSON RFC 7946
- Encodage : UTF-8

DÉPANNAGE
---------
Si le plugin ne fonctionne pas :
1. Vérifiez que le dossier du plugin est bien dans Resources/plugins/
2. Vérifiez les logs X-Plane (Log.txt) pour les erreurs
3. Assurez-vous que le dossier FlightDataRecorder existe dans votre home

CONTACT
------
Pour toute question ou problème, consulter le dépôt GitHub :
https://github.com/ldeglise/FlightDataRecorder

================================================================================
Version : 1.0
Date : 2026
================================================================================
