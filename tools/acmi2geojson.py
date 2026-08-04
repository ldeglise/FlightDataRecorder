import json
import sys
from datetime import datetime

def parse_acmi_to_geojson(acmi_file_path, geojson_file_path):
    # Lire le fichier ACMI
    with open(acmi_file_path, 'r') as f:
        lines = f.readlines()

    # Initialiser la structure GeoJSON
    geojson = {
        "type": "FeatureCollection",
        "features": [],
        "metadata": {}  # Pour stocker les métadonnées globales (ex: informations sur les avions)
    }

    # Dictionnaires pour stocker les trajectoires, les points et les métadonnées
    trajectories = {}  # Pour les LineString (trajectoires complètes)
    points = []         # Pour les Point (chaque point individuel)
    aircraft_metadata = {}  # Pour stocker les métadonnées des avions (ICAO, nom, moteurs, etc.)

    # Parser chaque ligne
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            # Vérifier si la ligne est une métadonnée (ex: #Aircraft1=ICAO,Name,Engines,...)
            if line.startswith("#") and "=" in line:
                # Extraire les métadonnées de l'avion
                parts = line.split("=")
                if len(parts) == 2:
                    metadata_key = parts[0].strip("#").strip()
                    metadata_values = parts[1].strip().split(",")
                    if len(metadata_values) >= 6:
                        icao = metadata_values[0].strip()
                        name = metadata_values[1].strip()
                        num_engines = int(metadata_values[2].strip())
                        engine_type = metadata_values[3].strip()
                        total_power = float(metadata_values[4].strip())
                        power_per_engine = float(metadata_values[5].strip())

                        # Stocker les métadonnées
                        aircraft_metadata[metadata_key] = {
                            "Aircraft ICAO": icao,
                            "Aircraft name": name,
                            "Number of engines": num_engines,
                            "Type of engines": engine_type,
                            "Total power": total_power,
                            "Power per engine": power_per_engine
                        }
            continue  # Ignorer les autres commentaires

        # Séparer les valeurs
        parts = line.split(',')
        if len(parts) < 20:
            continue  # Ligne invalide (ajustez selon votre format ACMI)

        # Extraire les données
        obj_id = parts[1].strip()
        time_str = parts[0].strip()
        lat = float(parts[6].strip())
        lon = float(parts[7].strip())
        baro_alt = float(parts[8].strip())  # Barometric altitude
        height_agl = float(parts[9].strip()) if len(parts) > 9 else None  # Height above ground
        qnh = float(parts[10].strip()) if len(parts) > 10 else None  # QNH
        ias = float(parts[11].strip()) if len(parts) > 11 else None  # Indicated air speed
        tas = float(parts[12].strip()) if len(parts) > 12 else None  # True air speed
        gs = float(parts[13].strip()) if len(parts) > 13 else None  # Ground speed
        vs = float(parts[14].strip()) if len(parts) > 14 else None  # Vertical speed
        cap = float(parts[15].strip()) if len(parts) > 15 else None  # Cap (DEG mag)
        heading = float(parts[16].strip()) if len(parts) > 16 else None  # Heading (DEG true)
        route_mag = float(parts[17].strip()) if len(parts) > 17 else None  # Route mag (DEG mag)
        route_true = float(parts[18].strip()) if len(parts) > 18 else None  # Route true (DEG true)
        pitch = float(parts[19].strip()) if len(parts) > 19 else None  # Pitch (DEG)
        bank = float(parts[20].strip()) if len(parts) > 20 else None  # Bank (DEG)

        # Convertir le temps en timestamp (si nécessaire)
        try:
            time = datetime.strptime(time_str, "%Y-%m-%dT%H:%M:%S.%fZ").timestamp()
        except ValueError:
            time = 0  # Valeur par défaut si le format est invalide

        # Ajouter le point individuel (Feature de type Point)
        point_properties = {
            "id": obj_id,
            "Timestamp UTC": time_str,
            "Longitude": lon,
            "Latitude": lat,
            "Barometric altitude": baro_alt,
            "Height above ground": height_agl,
            "QNH": qnh,
            "Indicated air speed": ias,
            "True air speed": tas,
            "Ground speed": gs,
            "Vertical speed": vs,
            "Cap (DEG mag)": cap,
            "Heading (DEG true)": heading,
            "Route mag (DEG mag)": route_mag,
            "Route true (DEG true)": route_true,
            "Pitch (DEG)": pitch,
            "Bank (DEG)": bank
        }

        # Ajouter les métadonnées de l'avion si disponibles
        if obj_id in aircraft_metadata:
            point_properties.update(aircraft_metadata[obj_id])

        point_feature = {
            "type": "Feature",
            "geometry": {
                "type": "Point",
                "coordinates": [lon, lat, baro_alt]
            },
            "properties": point_properties
        }
        points.append(point_feature)

        # Initialiser la trajectoire si elle n'existe pas
        if obj_id not in trajectories:
            trajectories[obj_id] = {
                "type": "Feature",
                "geometry": {
                    "type": "LineString",
                    "coordinates": []
                },
                "properties": {
                    "id": obj_id,
                    "start_time": time_str,
                    "end_time": time_str,
                    "min_altitude": baro_alt,
                    "max_altitude": baro_alt
                }
            }
            # Ajouter les métadonnées de l'avion à la trajectoire
            if obj_id in aircraft_metadata:
                trajectories[obj_id]["properties"].update(aircraft_metadata[obj_id])

        # Mettre à jour la trajectoire
        trajectories[obj_id]["geometry"]["coordinates"].append([lon, lat, baro_alt])
        trajectories[obj_id]["properties"]["end_time"] = time_str
        trajectories[obj_id]["properties"]["min_altitude"] = min(
            trajectories[obj_id]["properties"]["min_altitude"], baro_alt
        )
        trajectories[obj_id]["properties"]["max_altitude"] = max(
            trajectories[obj_id]["properties"]["max_altitude"], baro_alt
        )

    # Ajouter les trajectoires et les points au GeoJSON
    for trajectory in trajectories.values():
        geojson["features"].append(trajectory)
    geojson["features"].extend(points)

    # Ajouter les métadonnées globales (optionnel, si vous voulez les regrouper)
    geojson["metadata"] = {
        "aircrafts": aircraft_metadata
    }

    # Écrire le fichier GeoJSON
    with open(geojson_file_path, 'w') as f:
        json.dump(geojson, f, indent=2)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python acmi_to_geojson.py <input.acmi> <output.geojson>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    parse_acmi_to_geojson(input_file, output_file)
    print(f"Conversion terminée : {output_file}")