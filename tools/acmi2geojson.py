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
        "features": []
    }

    # Dictionnaires pour stocker les trajectoires et les points
    trajectories = {}  # Pour les LineString (trajectoires complètes)
    points = []         # Pour les Point (chaque point individuel)

    # Parser chaque ligne
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            continue  # Ignorer les commentaires et lignes vides

        # Séparer les valeurs
        parts = line.split(',')
        if len(parts) < 10:
            continue  # Ligne invalide

        # Extraire les données
        obj_id = parts[1].strip()
        time_str = parts[0].strip()
        lat = float(parts[6].strip())
        lon = float(parts[7].strip())
        alt = float(parts[8].strip())

        # Convertir le temps en timestamp (si nécessaire)
        try:
            time = datetime.strptime(time_str, "%Y-%m-%dT%H:%M:%S.%fZ").timestamp()
        except ValueError:
            time = 0  # Valeur par défaut si le format est invalide

        # Ajouter le point individuel (Feature de type Point)
        point_feature = {
            "type": "Feature",
            "geometry": {
                "type": "Point",
                "coordinates": [lon, lat, alt]
            },
            "properties": {
                "id": obj_id,
                "time": time_str,
                "altitude": alt
            }
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
                    "min_altitude": alt,
                    "max_altitude": alt
                }
            }

        # Mettre à jour la trajectoire
        trajectories[obj_id]["geometry"]["coordinates"].append([lon, lat, alt])
        trajectories[obj_id]["properties"]["end_time"] = time_str
        trajectories[obj_id]["properties"]["min_altitude"] = min(
            trajectories[obj_id]["properties"]["min_altitude"], alt
        )
        trajectories[obj_id]["properties"]["max_altitude"] = max(
            trajectories[obj_id]["properties"]["max_altitude"], alt
        )

    # Ajouter les trajectoires et les points au GeoJSON
    for trajectory in trajectories.values():
        geojson["features"].append(trajectory)
    geojson["features"].extend(points)

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