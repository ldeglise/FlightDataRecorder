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

INSTALLATION
------------
1. Copier le dossier du plugin dans : X-Plane/Resources/plugins/

2. Lancer X-Plane

3. Le plugin se lance automatiquement et commence à enregistrer dès le décollage

UTILISATION
----------
Aucune configuration nécessaire ! Le plugin fonctionne automatiquement :

- Détection automatique du décollage (AGL > 10m pendant 10 secondes)
- Détection automatique de l'atterrissage (AGL < 1m pendant 20 secondes)

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

3. Google Earth web :
   - Importer le fichier .geojson

4. Sites web :
   - Utiliser des outils comme geojson.io pour visualiser en ligne

5. N'importe quel outils pouvant traiter du GeoJSON

TECHNIQUE
--------
- Précision : 6 décimales (~10 cm) pour les coordonnées
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
Version : 1.0b1
Date : 2026
================================================================================


================================================================================
FLIGHT DATA RECORDER - X-Plane Plugin (English Translation)
================================================================================

DESCRIPTION
-----------
Flight Data Recorder is a plugin for X-Plane that automatically records flight data
in GeoJSON format. It captures the position, altitude, speeds, and headings of your
aircraft throughout the entire flight.

FEATURES
--------
- Automatic flight recording (takeoff/landing detection)
- GeoJSON format compatible with most GIS tools (QGIS, Google Earth, etc.)
- Coordinate precision: 6 decimal places (~10 cm)

INSTALLATION
------------
1. Copy the plugin folder to: X-Plane/Resources/plugins/

2. Launch X-Plane

3. The plugin starts automatically and begins recording upon takeoff

USAGE
-----
No configuration required! The plugin works automatically:

- Automatic takeoff detection (AGL > 10m for 10 seconds)
- Automatic landing detection (AGL < 1m for 20 seconds)

GENERATED FILES
----------------
Files are saved in:
- Windows: %USERPROFILE%\FlightDataRecorder\
- macOS/Linux: ~/FlightDataRecorder/

File format:
  YYYY-MM-DDTHH-MM-SSZ.geojson
  (e.g., 2026-08-10T14-30-20Z.geojson)

Each file contains:
- A GeoJSON FeatureCollection
- Points (Point features) with all flight data
- A line (LineString feature) representing the smoothed flight path

RECORDED DATA
--------------
For each point:
- Coordinates (longitude, latitude) - 6 decimal places
- Altitude MSL (meters)
- True heading (degrees)
- Magnetic heading (degrees)
- Indicated Airspeed (IAS) in km/h
- Ground Speed (GS) in km/h
- ISO 8601 timestamp
- Zulu time (HH:mm:ss)

DATA VISUALIZATION
-------------------
You can visualize GeoJSON files with:

1. QGIS (recommended):
   - Open QGIS
   - Drag and drop the .geojson file
   - The flight path will appear with data points

2. Google Earth:
   - Convert GeoJSON to KML using an online tool
   - Open the KML file in Google Earth

3. Google Earth Web:
   - Import the .geojson file

4. Websites:
   - Use tools like geojson.io to visualize online

5. Any tool that supports GeoJSON

TECHNICAL DETAILS
-----------------
- Precision: 6 decimal places (~10 cm) for coordinates
- Format: GeoJSON RFC 7946
- Encoding: UTF-8

TROUBLESHOOTING
----------------
If the plugin does not work:
1. Verify that the plugin folder is correctly placed in Resources/plugins/
2. Check X-Plane logs (Log.txt) for errors
3. Ensure the FlightDataRecorder folder exists in your home directory

CONTACT
-------
For any questions or issues, visit the GitHub repository:
https://github.com/ldeglise/FlightDataRecorder

================================================================================
Version: 1.0b1
Date: 2026
================================================================================
