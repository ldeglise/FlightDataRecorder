# Flight Data Recorder - GeoJSON Output Format Specification

> **Documentation Technique pour Développeurs**  
> Version: 1.0  
> Dernière mise à jour: 2024  
> Plugin: FlightDataRecorder pour X-Plane 12

---

## 📋 Sommaire

1. [Overview](#-overview)
2. [Structure Générale du Fichier](#-structure-générale-du-fichier)
3. [Métadonnées de la FeatureCollection](#-métadonnées-de-la-featurecollection)
4. [Features de Type Point](#-features-de-type-point)
5. [Feature de Type LineString](#-feature-de-type-linestring)
6. [Types de Données et Unités](#-types-de-données-et-unités)
7. [Précision et Traitement des Données](#-précision-et-traitement-des-données)
8. [Exemple Complet](#-exemple-complet)
9. [Fréquence d'Écriture](#-fréquence-décriture)
10. [Nommage des Fichiers](#-nommage-des-fichiers)
11. [Compatibilité](#-compatibilité)
12. [Cas d'Usage pour Développeurs](#-cas-dusage-pour-développeurs)
13. [Annexes](#-annexes)

---

## 🎯 Overview

Le plugin **Flight Data Recorder** génère des fichiers au format **GeoJSON RFC 7946** contenant les données de vol captées depuis X-Plane. Chaque fichier représente un vol complet, du décollage à l'atterrissage.

### Caractéristiques principales
- **Format**: GeoJSON RFC 7946
- **Encodage**: UTF-8
- **Précision des coordonnées**: 6 décimales (~10 cm)
- **Système de coordonnées**: WGS84 (EPSG:4326) - [longitude, latitude]

---

## 📦 Structure Générale du Fichier

```json
{
  "type": "FeatureCollection",
  "metadata": { ... },
  "features": [
    { "type": "Feature", "geometry": { "type": "Point", ... }, "properties": { ... } },
    ...,
    { "type": "Feature", "geometry": { "type": "LineString", ... }, "properties": {} }
  ]
}
```

### Hiérarchie
```
FeatureCollection
├── metadata (objet)
└── features (array)
    ├── Feature (Point) - Données de vol à un instant t
    ├── Feature (Point) - Données de vol à t+10s
    ├── ...
    └── Feature (LineString) - Trace complète du vol (lissée)
```

---

## 📊 Métadonnées de la FeatureCollection

L'objet `metadata` est situé à la racine de la FeatureCollection et contient des informations sur le vol et l'avion.

### Propriétés

| Propriété | Type | Description | Exemple |
|-----------|------|-------------|---------|
| `seed` | integer | **Nombre de moteurs** de l'avion (valeur obscure) | `2` |
| `checksum` | number | **Puissance totale en watts** (calculée : `pmax * nombre_de_moteurs`) | `1500000.0` |
| `takeoff_time` | string | **Heure de décollage** au format ISO 8601 (UTC) | `"2026-08-10T14:30:20Z"` |

### Exemple
```json
"metadata": {
  "seed": 2,
  "checksum": 1500000.0,
  "takeoff_time": "2026-08-10T14:30:20Z"
}
```

---

## 📍 Features de Type Point

Chaque point représente les données de vol à un instant donné. Les points sont **bufferisés et écrits toutes les 10 secondes** pendant le vol.

### Structure

```json
{
  "type": "Feature",
  "geometry": {
    "type": "Point",
    "coordinates": [longitude, latitude]
  },
  "properties": {
    "timestamp": "2026-08-10T14:30:25Z",
    "elevation_msl": 1500.5,
    "true_heading": 270.5,
    "magnetic_heading": 265.3,
    "ias_kmh": 250.4,
    "gs_kmh": 245.2,
    "zulu_time_sec": 52225.0,
    "zulu_time": "14:30:25"
  }
}
```

### Propriétés des Points

| Propriété | Type | Description | Unité | Plage de valeurs | Précision |
|-----------|------|-------------|-------|------------------|-----------|
| `timestamp` | string | Horodatage UTC du point au format ISO 8601 | - | - | Secondes |
| `elevation_msl` | number | **Altitude barométrique** au-dessus du niveau moyen de la mer | Mètres | -∞ à +∞ | 1 décimale |
| `true_heading` | number | **Cap vrai** (direction vers laquelle pointe l'avion par rapport au Nord vrai) | Degrés | 0.0 à 359.9 | 1 décimale |
| `magnetic_heading` | number | **Cap magnétique** (direction par rapport au Nord magnétique) | Degrés | 0.0 à 359.9 | 1 décimale |
| `ias_kmh` | number | **Vitesse indiquée (IAS)** - Vitesse affichée sur l'anémomètre | km/h | 0 à ~1000 | 1 décimale |
| `gs_kmh` | number | **Vitesse sol (GS)** - Vitesse réelle par rapport au sol | km/h | 0 à ~1000 | 1 décimale |
| `zulu_time_sec` | number | Temps écoulé depuis minuit UTC | Secondes | 0 à 86400 | 1 décimale |
| `zulu_time` | string | Heure UTC au format HH:mm:ss | - | 00:00:00 à 23:59:59 | Secondes |

### Notes sur les vitesses
- **IAS** (Indicated Airspeed): Valeur brute convertie de **knots** vers **km/h** (× 1.852)
- **GS** (Ground Speed): Valeur brute convertie de **m/s** vers **km/h** (× 3.6)

---

## 📈 Feature de Type LineString

La LineString représente la **trace complète du vol** avec un **lissage appliqué** (filtrage par moyenne mobile sur 3 points).

### Structure

```json
{
  "type": "Feature",
  "geometry": {
    "type": "LineString",
    "coordinates": [
      [longitude_1, latitude_1],
      [longitude_2, latitude_2],
      ...,
      [longitude_n, latitude_n]
    ]
  }
}
```

### Caractéristiques
- **Nombre de points**: Variable, dépend de la durée du vol
- **Lissage**: Applique un filtre de **moyenne mobile** avec une fenêtre de 3 points
- **Précision**: 6 décimales pour chaque coordonnée
- **Ordre**: Chronologique (du décollage à l'atterrissage)

### Algorithme de Lissage
Le plugin utilise une **moyenne mobile centrée** avec les paramètres suivants:
- **Taille de la fenêtre**: 3 points (1 point avant, le point courant, 1 point après)
- **Points aux extrémités**: Conservés sans modification
- **Calcul**: Pour chaque point `i`, la coordonnée lissée = moyenne des coordonnées des points `[i-1, i, i+1]`

---

## 📏 Types de Données et Unités

| Catégorie | Propriété | Type JSON | Type Natif | Unité | Conversion |
|----------|-----------|-----------|------------|-------|------------|
| **Position** | longitude | number | double | Degrés décimaux | WGS84 |
| | latitude | number | double | Degrés décimaux | WGS84 |
| **Altitude** | elevation_msl | number | double | Mètres | - |
| **Direction** | true_heading | number | float | Degrés | 0-359.9° |
| | magnetic_heading | number | float | Degrés | 0-359.9° |
| **Vitesse** | ias_kmh | number | float | km/h | knots × 1.852 |
| | gs_kmh | number | float | km/h | m/s × 3.6 |
| **Temps** | timestamp | string | - | ISO 8601 | UTC |
| | zulu_time_sec | number | float | Secondes | Depuis 00:00 UTC |
| | zulu_time | string | - | HH:mm:ss | UTC |

---

## 🎯 Précision et Traitement des Données

### Coordonnées Géographiques
- **Précision**: **6 décimales** (environ 10 cm à l'équateur)
- **Arrondi**: Utilisation de `round(value * 1000000.0) / 1000000.0`
- **Format**: Toujours au format `[longitude, latitude]` (GeoJSON standard)

### Exemple de précision
```
Latitude: 48.856614 → 48.856614 (6 décimales)
Longitude: 2.352222 → 2.352222 (6 décimales)
```

### Valeurs Aberrantes
- Aucune validation explicite dans le code source
- Les valeurs sont directement lues depuis les DataRefs X-Plane
- **Recommandation**: Les consommateurs de ces données devraient implémenter leurs propres validations

---

## 📄 Exemple Complet

Voici un exemple réaliste d'un fichier GeoJSON généré par le plugin :

```json
{
  "type": "FeatureCollection",
  "metadata": {
    "seed": 2,
    "checksum": 1500000.0,
    "takeoff_time": "2026-08-10T14:30:20Z"
  },
  "features": [
    {
      "type": "Feature",
      "geometry": {
        "type": "Point",
        "coordinates": [2.352222, 48.856614]
      },
      "properties": {
        "timestamp": "2026-08-10T14:30:25Z",
        "elevation_msl": 150.5,
        "true_heading": 270.0,
        "magnetic_heading": 265.0,
        "ias_kmh": 200.0,
        "gs_kmh": 195.0,
        "zulu_time_sec": 52225.0,
        "zulu_time": "14:30:25"
      }
    },
    {
      "type": "Feature",
      "geometry": {
        "type": "Point",
        "coordinates": [2.351111, 48.857778]
      },
      "properties": {
        "timestamp": "2026-08-10T14:30:35Z",
        "elevation_msl": 300.2,
        "true_heading": 275.5,
        "magnetic_heading": 270.3,
        "ias_kmh": 220.0,
        "gs_kmh": 215.0,
        "zulu_time_sec": 52235.0,
        "zulu_time": "14:30:35"
      }
    },
    {
      "type": "Feature",
      "geometry": {
        "type": "LineString",
        "coordinates": [
          [2.352222, 48.856614],
          [2.352100, 48.856800],
          [2.352000, 48.857000],
          [2.351500, 48.857500],
          [2.351111, 48.857778]
        ]
      }
    }
  ]
}
```

---

## ⏱️ Fréquence d'Écriture

### Bufferisation
- Les points sont **stockés en mémoire** dans un buffer
- **10 secondes de données** sont accumulées avant écriture sur disque
- Cela réduit les opérations I/O et améliore les performances

### Processus d'écriture
1. Collecte des données toutes les **1 seconde** (appel à `collectData()`)
2. Stockage dans `writeBuffer` et `lineStringBuffer`
3. Toutes les **10 secondes** (compteur `writeCounter >= 10`):
   - `flushWriteBuffer()` écrit les points dans le fichier
   - Le buffer est vidé
4. À l'atterrissage:
   - Écriture des points restants
   - Génération et écriture de la LineString lissée
   - Fermeture du fichier

### Impact sur les données
- **Résolution temporelle**: ~10 secondes entre chaque Point Feature
- **LineString**: Contient TOUS les points collectés (pas seulement ceux écritsures en Points)
- **Dernier point**: Toujours écrit à l'atterrissage

---

## 📁 Nommage des Fichiers

### Format
```
AAAA-MM-JJTHH-MM-SSZ.geojson
```

### Exemple
```
2026-08-10T14-30-20Z.geojson
```

### Règles
- **Timestamp**: Heure système au moment du **décollage** (détection AGL > 10m pendant 10s)
- **Format**: ISO 8601 avec `T` comme séparateur et `Z` pour UTC
- **Caractères spéciaux**: Les `:` sont remplacés par `-` dans le nom de fichier
- **Emplacement**:
  - Windows: `%USERPROFILE%\FlightDataRecorder\`
  - Linux/Mac: `~/FlightDataRecorder/`

---

## ✅ Compatibilité

### Normes
- **GeoJSON**: Conforme à la [RFC 7946](https://tools.ietf.org/html/rfc7946)
- **Système de coordonnées**: WGS84 (EPSG:4326) - Standard GeoJSON
- **Encodage**: UTF-8

### Logiciels Compatibles
| Logiciel | Support GeoJSON | Remarques |
|----------|-----------------|-----------|
| QGIS | ✅ Natif | Recommandé pour l'analyse SIG |
| Google Earth | ⚠️ Via conversion | Nécessite conversion en KML |
| Google Earth Web | ✅ Natif | Import direct possible |
| [geojson.io](https://geojson.io) | ✅ Natif | Visualisation en ligne |
| Leaflet | ✅ Natif | Bibliothèque JavaScript |
| OpenLayers | ✅ Natif | Bibliothèque JavaScript |
| Mapbox GL JS | ✅ Natif | Bibliothèque JavaScript |
| PostGIS | ✅ Natif | Extension PostgreSQL |
| GDAL/OGR | ✅ Natif | Outils en ligne de commande |

---

## 💻 Cas d'Usage pour Développeurs

### 1. Parsing du GeoJSON

#### Python (avec `geojson` ou `json`)
```python
import json

with open('2026-08-10T14-30-20Z.geojson', 'r', encoding='utf-8') as f:
    data = json.load(f)

# Accéder aux métadonnées
seed = data['metadata']['seed']
checksum = data['metadata']['checksum']
takeoff_time = data['metadata']['takeoff_time']

# Extraire tous les points
points = [f for f in data['features'] if f['geometry']['type'] == 'Point']

# Extraire la LineString (trace)
linestring = next(f for f in data['features'] if f['geometry']['type'] == 'LineString')
```

#### JavaScript (Node.js ou navigateur)
```javascript
import fs from 'fs';

const data = JSON.parse(fs.readFileSync('2026-08-10T14-30-20Z.geojson', 'utf-8'));

// Métadonnées
const { seed, checksum, takeoff_time } = data.metadata;

// Points avec propriétés
const points = data.features.filter(f => f.geometry.type === 'Point');

// Trace
const trace = data.features.find(f => f.geometry.type === 'LineString');
```

### 2. Calcul de la Distance du Vol

```python
from geopy.distance import geodesic

# Extraire les coordonnées de la LineString
coordinates = linestring['geometry']['coordinates']

# Calculer la distance totale
total_distance_km = 0
for i in range(len(coordinates) - 1):
    point_a = (coordinates[i][1], coordinates[i][0])  # (lat, lon)
    point_b = (coordinates[i+1][1], coordinates[i+1][0])
    total_distance_km += geodesic(point_a, point_b).kilometers

print(f"Distance totale: {total_distance_km:.2f} km")
```

### 3. Analyse des Performances de Vol

```python
# Calculer la vitesse moyenne
points = [f for f in data['features'] if f['geometry']['type'] == 'Point']

if len(points) >= 2:
    first_point = points[0]['properties']
    last_point = points[-1]['properties']
    
    # Temps écoulé (en heures)
    time_diff = (last_point['zulu_time_sec'] - first_point['zulu_time_sec']) / 3600
    
    # Distance (à calculer comme ci-dessus)
    avg_speed_kmh = total_distance_km / time_diff
```

### 4. Visualisation avec Leaflet

```html
<!DOCTYPE html>
<html>
<head>
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
</head>
<body>
    <div id="map" style="height: 600px;"></div>
    <script>
        const map = L.map('map').setView([48.8566, 2.3522], 12);
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);
        
        fetch('2026-08-10T14-30-20Z.geojson')
            .then(r => r.json())
            .then(data => {
                // Afficher la trace
                const trace = data.features.find(f => f.geometry.type === 'LineString');
                L.geoJSON(trace).addTo(map);
                
                // Afficher les points
                const points = data.features.filter(f => f.geometry.type === 'Point');
                L.geoJSON(points, {
                    pointToLayer: (feature, latlng) => {
                        return L.circleMarker(latlng, {
                            radius: 5,
                            fillColor: '#ff7800',
                            color: '#000',
                            weight: 1,
                            opacity: 1,
                            fillOpacity: 0.8
                        });
                    }
                }).addTo(map);
            });
    </script>
</body>
</html>
```

### 5. Conversion vers d'autres formats

#### GeoJSON → KML (avec `gdal`)
```bash
ogr2ogr -f KML output.kml input.geojson
```

#### GeoJSON → CSV (avec `ogr2ogr`)
```bash
ogr2ogr -f CSV output.csv input.geojson -lco GEOMETRY=AS_WKT
```

#### GeoJSON → Shapefile
```bash
ogr2ogr -f ESRI Shapefile output.shp input.geojson
```

---

## 📚 Annexes

### A. Détection du Décollage et de l'Atterrissage

| Événement | Critère | Compteur | Action |
|-----------|---------|----------|--------|
| **Décollage** | AGL > 10 mètres | 10 secondes consécutives | Début de l'enregistrement |
| **Atterrissage** | AGL < 1 mètre | 20 secondes consécutives | Fin de l'enregistrement |

### B. DataRefs X-Plane Utilisées

| DataRef | Description | Type | Utilisation |
|---------|-------------|------|-------------|
| `sim/flightmodel/position/phi` | Roll | float | Non utilisée |
| `sim/flightmodel/position/theta` | Pitch | float | Non utilisée |
| `sim/flightmodel/position/psi` | **True Heading** | float | ✅ `true_heading` |
| `sim/flightmodel/position/mag_psi` | **Magnetic Heading** | float | ✅ `magnetic_heading` |
| `sim/flightmodel/position/local_x` | Position locale X | float | Non utilisée |
| `sim/flightmodel/position/local_y` | Position locale Y | float | Non utilisée |
| `sim/flightmodel/position/local_z` | Position locale Z | float | Non utilisée |
| `sim/flightmodel/position/latitude` | **Latitude** | double | ✅ `latitude` |
| `sim/flightmodel/position/longitude` | **Longitude** | double | ✅ `longitude` |
| `sim/flightmodel/position/elevation` | **Altitude MSL** | double | ✅ `elevation_msl` |
| `sim/flightmodel/position/y_agl` | Altitude AGL | float | Détection décollage/atterrissage |
| `sim/flightmodel/position/indicated_airspeed` | **IAS (knots)** | float | ✅ `ias` |
| `sim/flightmodel/position/groundspeed` | **GS (m/s)** | float | ✅ `gs` |
| `sim/time/zulu_time_sec` | **Temps Zulu (secondes)** | float | ✅ `zulu_time_sec` |
| `sim/time/local_date_days` | Date locale (jours) | int | Non utilisée |
| `sim/aircraft/engine/acf_num_engines` | **Nombre de moteurs** | int | ✅ `metadata.seed` |
| `sim/aircraft/engine/acf_pmax` | Puissance max par moteur | float | ✅ `metadata.checksum` |

### C. Schéma JSON (JSON Schema)

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "FlightDataRecorder GeoJSON",
  "type": "object",
  "properties": {
    "type": {
      "type": "string",
      "enum": ["FeatureCollection"]
    },
    "metadata": {
      "type": "object",
      "properties": {
        "seed": {
          "type": "integer",
          "minimum": 0,
          "description": "Nombre de moteurs"
        },
        "checksum": {
          "type": "number",
          "minimum": 0,
          "description": "Puissance totale en watts"
        },
        "takeoff_time": {
          "type": "string",
          "format": "date-time",
          "pattern": "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$",
          "description": "Heure de décollage (ISO 8601 UTC)"
        }
      },
      "required": ["seed", "checksum", "takeoff_time"]
    },
    "features": {
      "type": "array",
      "items": {
        "oneOf": [
          {
            "$ref": "#/definitions/pointFeature"
          },
          {
            "$ref": "#/definitions/lineStringFeature"
          }
        ]
      }
    }
  },
  "definitions": {
    "pointFeature": {
      "type": "object",
      "properties": {
        "type": {
          "type": "string",
          "enum": ["Feature"]
        },
        "geometry": {
          "type": "object",
          "properties": {
            "type": {
              "type": "string",
              "enum": ["Point"]
            },
            "coordinates": {
              "type": "array",
              "items": [
                {"type": "number"},
                {"type": "number"}
              ],
              "minItems": 2,
              "maxItems": 2
            }
          },
          "required": ["type", "coordinates"]
        },
        "properties": {
          "type": "object",
          "properties": {
            "timestamp": {
              "type": "string",
              "format": "date-time",
              "pattern": "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$"
            },
            "elevation_msl": {
              "type": "number"
            },
            "true_heading": {
              "type": "number",
              "minimum": 0,
              "maximum": 360
            },
            "magnetic_heading": {
              "type": "number",
              "minimum": 0,
              "maximum": 360
            },
            "ias_kmh": {
              "type": "number",
              "minimum": 0
            },
            "gs_kmh": {
              "type": "number",
              "minimum": 0
            },
            "zulu_time_sec": {
              "type": "number",
              "minimum": 0,
              "maximum": 86400
            },
            "zulu_time": {
              "type": "string",
              "pattern": "^[0-9]{2}:[0-9]{2}:[0-9]{2}$"
            }
          },
          "required": [
            "timestamp",
            "elevation_msl",
            "true_heading",
            "magnetic_heading",
            "ias_kmh",
            "gs_kmh",
            "zulu_time_sec",
            "zulu_time"
          ]
        }
      },
      "required": ["type", "geometry", "properties"]
    },
    "lineStringFeature": {
      "type": "object",
      "properties": {
        "type": {
          "type": "string",
          "enum": ["Feature"]
        },
        "geometry": {
          "type": "object",
          "properties": {
            "type": {
              "type": "string",
              "enum": ["LineString"]
            },
            "coordinates": {
              "type": "array",
              "items": {
                "type": "array",
                "items": [
                  {"type": "number"},
                  {"type": "number"}
                ],
                "minItems": 2,
                "maxItems": 2
              }
            }
          },
          "required": ["type", "coordinates"]
        }
      },
      "required": ["type", "geometry"]
    }
  },
  "required": ["type", "metadata", "features"]
}
```

### D. Glossaire

| Terme | Définition |
|-------|------------|
| **AGL** | Above Ground Level - Altitude au-dessus du sol |
| **MSL** | Mean Sea Level - Niveau moyen de la mer |
| **IAS** | Indicated Airspeed - Vitesse indiquée |
| **GS** | Ground Speed - Vitesse sol |
| **True Heading** | Cap vrai par rapport au Nord géographique |
| **Magnetic Heading** | Cap par rapport au Nord magnétique |
| **WGS84** | World Geodetic System 1984 - Standard de coordonnées GPS |
| **RFC 7946** | Norme officielle du format GeoJSON |
| **FeatureCollection** | Structure GeoJSON contenant plusieurs Features |
| **Feature** | Élément géométrique avec propriétés |
| **Point** | Géométrie représentant un point |
| **LineString** | Géométrie représentant une ligne |

---

## 🔗 Ressources Externes

- [RFC 7946 - GeoJSON Format Specification](https://tools.ietf.org/html/rfc7946)
- [GeoJSON.org](https://geojson.org/)
- [WGS84 (EPSG:4326)](https://epsg.io/4326)
- [X-Plane DataRefs Documentation](https://developer.x-plane.com/datarefs/)
- [QGIS](https://qgis.org/)
- [Leaflet](https://leafletjs.com/)
- [geojson.io](https://geojson.io/)

---

## 📝 Historique des Versions

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2024 | Version initiale de la documentation |

---

> **Contact**: Pour toute question ou suggestion concernant cette documentation, consulter le dépôt GitHub: [https://github.com/ldeglise/FlightDataRecorder](https://github.com/ldeglise/FlightDataRecorder)
