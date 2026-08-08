# **Cahier des Charges Technique et Fonctionnel**

## **Plugin FlightDataRecorder pour X-Plane 12**

---

## **1. Introduction**

### **1.1 Contexte**

- **Problématique** : Absence de plugin natif dans **X-Plane 12** pour enregistrer des paramètres de vol et générer des fichiers **GeoJSON** (conformes à la [RFC 7946](https://tools.ietf.org/html/rfc7946)).
- **Utilité** :
  - Analyse post-vol (logiciels SIG comme QGIS, Google Earth, OpenStreetMap).
  - Éviter l’utilisation de plateformes de tracking tierces (Volanta, Elevatex, FsHub, etc.).
- **Solution** : Développer un plugin **léger, automatique et fiable** pour combler ce manque.

### **1.2 Objectifs**

- **Principal** : Enregistrer des données de vol dans X-Plane 12 et produire un fichier GeoJSON exploitable.
- **Secondaires** :
  - Intégration transparente (pas d’IHM, activation/désactivation via **Plugin Admin**).
  - Conformité totale avec la **RFC 7946**.
  - Détection automatique des phases de vol (**décollage/atterrissage**).

---

## **2. Périmètre Fonctionnel**

### **2.1 Fonctionnalités Principales**

 | **ID** | **Fonctionnalité** | **Description** |  
 | -------- | --------------------- | ---------------- |
 | **F1** | **Monitoring des *datarefs*** | Capturer les *datarefs* listées ci-dessous **toutes les 1 seconde**. |
 | **F2** | **Stockage en mémoire tampon** | Stocker les données en mémoire tampon **uniquement après détection du décollage**. Aucune donnée n’est stockée avant le décollage. |
 | **F3** | **Écriture périodique sur disque** | Écrire les données sur disque **toutes les 20 secondes** (20 points de mesure). |
 | **F4** | **Génération du fichier GeoJSON** | Produire un fichier GeoJSON contenant : |

- Les **points de mesure** (toutes les secondes) **avec les unités des *datarefs***.
- La **trace du vol** (ligne connectant les points).
- Les **métadonnées** (modèle d’appareil, OACI, nombre de moteurs, puissance totale). |
 | **F5** | **Détection automatique du décollage** | Détecter le décollage lorsque :
- **Altitude AGL** (convertie en pieds) > **50 pieds**.
- **Vitesse indiquée (IAS)** (convertie en nœuds) > **55 nœuds**.
- **Durée minimale** : Les conditions doivent être maintenues **pendant 10 secondes** pour éviter les fausses détections (rebonds).
**Les conversions en pieds et nœuds sont effectuées en interne par le plugin.** |
 | **F6** | **Détection automatique de l’atterrissage** | Détecter l’atterrissage lorsque :
- **Altitude AGL** (convertie en pieds) < **10 pieds**.
- **Vitesse indiquée (IAS)** (convertie en nœuds) < **30 nœuds**.
- **Durée minimale** : Les conditions doivent être maintenues **pendant 10 secondes** pour éviter les fausses détections (rebonds). |
 | **F7** | **Gestion des fichiers** | Créer un dossier **`FlightDataRecorder`** dans le répertoire utilisateur :
- Linux : `/home/<user>/FlightDataRecorder/`
- Windows : `C:\Users\<user>\FlightDataRecorder\`
- macOS : `/Users/<user>/FlightDataRecorder/`
Le nom des fichiers suit le format : `<ALÉA>_AAAA-MM-JJTHH:mm:SS.geojson` (ex: `ABCD_2026-08-08T14:30:00.geojson`). |
 | **F8** | **Fermeture propre des fichiers** | Fermer tous les fichiers ouverts à la désactivation du plugin ou à la fermeture de X-Plane. |
 | **F9** | **Horodatage des points de mesure** | Chaque point de mesure doit inclure un **timestamp conforme à l'ISO 8601** (ex: `2026-08-08T14:30:20Z`), **construit à partir des *datarefs*** :
- `sim/time/zulu_time_sec` (secondes depuis minuit Zulu).
- `sim/time/local_date_days` (jours depuis le 1er janvier local). |

---

## **3. Datarefs à Monitorer (Liste Complète et Exacte)**

**Toutes les *datarefs* que vous avez spécifiées sont incluses, avec leur rôle précis.**

 | **Paramètre** | **Dataref X-Plane** | **Type** | **Unité** | **Fréquence** | **Rôle** |
 | --------------- | ---------------------- | ---------- | ----------- | --------------- | ---------- |
 | Code OACI de l'appareil | `sim/aircraft/view/acf_ICAO` | String | - | 1x (métadonnées) | Métadonnées GeoJSON |
 | Modèle d'appareil | `sim/aircraft/view/acf_ui_name` | String | - | 1x (métadonnées) | Métadonnées GeoJSON |
 | Nombre de moteurs | `sim/aircraft/engine/acf_num_engines` | Int | - | 1x (métadonnées) | Métadonnées GeoJSON |
 | Puissance totale par moteur | `sim/aircraft/engine/acf_pmax` | Float | Watts | 1x (métadonnées) | Métadonnées GeoJSON |
 | Secondes depuis minuit Zulu | `sim/time/zulu_time_sec` | Float | Secondes | Toutes les 1s | Horodatage (Points + Trace) |
 | Jours depuis le 1er janvier (local) | `sim/time/local_date_days` | Int | Jours | Toutes les 1s | Horodatage (Points + Trace) |
 | Longitude | `sim/flightmodel/position/longitude` | Double | Degrés décimaux | Toutes les 1s | Points + Trace |
 | Latitude | `sim/flightmodel/position/latitude` | Double | Degrés décimaux | Toutes les 1s | Points + Trace |
 | **Altitude barométrique (MSL)** | `sim/flightmodel/position/elevation` | Double | **Mètres** | Toutes les 1s | Points + Trace **Points de mesure (GeoJSON)** |
 | **Hauteur (AGL)** | `sim/flightmodel/position/y_agl` | Float | Mètres | Toutes les 1s | **Détection des phases de vol uniquement** |
 | Cap vrai | `sim/flightmodel/position/psi` | Float | Degrés | Toutes les 1s | Points + Trace |
 | Cap magnétique | `sim/flightmodel/position/mag_psi` | Float | Degrés | Toutes les 1s | Points + Trace |
 | Vitesse indiquée | `sim/flightmodel/position/indicated_airspeed` | Float | km/h | Toutes les 1s | Points + Trace |
 | Vitesse sol | `sim/flightmodel/position/groundspeed` | Float | km/h | Toutes les 1s | Points + Trace |

---

## **4. Contraintes Fonctionnelles**

- **Toutes les *datarefs* listées ci-dessus doivent être monitorées**.
- **Enregistrement dans le GeoJSON** :
  - **Altitude barométrique (MSL)** est **la seule altitude enregistrée** dans les points de mesure.
  - **Altitude AGL** est **uniquement utilisée pour la détection des phases de vol** (décollage/atterrissage).
- **Les unités dans le GeoJSON sont strictement celles des *datarefs*** (mètres pour MSL, km/h pour les vitesses, etc.).
- **Les conversions en pieds et nœuds** (pour la détection des phases de vol) sont **effectuées en interne par le plugin** et **ne sont pas stockées** dans le GeoJSON.
- **Pas de stockage avant décollage** : Les données sont **monitorées mais non stockées** tant que l’avion n’a pas décollé.
- **Horodatage ISO 8601** : Construit à partir de `sim/time/zulu_time_sec` et `sim/time/local_date_days`.

---

## **5. Exemple de Structure GeoJSON**

**Les unités sont celles des *datarefs* (mètres pour MSL, km/h pour les vitesses, etc.) :**

```json
{
  "type": "FeatureCollection",
  "metadata": {
    "aircraft_icao": "DHC2",
    "aircraft_model": "De Havilland DHC-2 Beaver",
    "num_engines": 1,
    "total_power": 450000.0,
    "takeoff_time": "2026-08-08T14:30:00Z"
  },
  "features": [
    {
      "type": "Feature",
      "geometry": {
        "type": "Point",
        "coordinates": [2.3522, 48.8566, 120.5]  // Longitude, Latitude, Altitude MSL (mètres)
      },
      "properties": {
        "timestamp": "2026-08-08T14:30:20Z",
        "elevation_msl": 120.5,       // Altitude barométrique (mètres)
        "true_heading": 120.0,        // Cap vrai (degrés)
        "magnetic_heading": 115.0,   // Cap magnétique (degrés)
        "ias": 101.86,                // Vitesse indiquée (km/h)
        "gs": 92.6,                  // Vitesse sol (km/h)
        "zulu_time_sec": 52220.0,    // Secondes depuis minuit Zulu
        "local_date_days": 220        // Jours depuis le 1er janvier (local)
      }
    },
    {
      "type": "Feature",
      "geometry": {
        "type": "LineString",
        "coordinates": [
          [2.3522, 48.8566, 120.5],
          [2.3600, 48.8600, 180.0]
        ]
      }
    }
  ]
}
