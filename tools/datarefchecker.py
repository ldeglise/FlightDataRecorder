import tkinter as tk
from tkinter import ttk
import requests
import threading
import time

class XPlaneDataRefChecker:
    def __init__(self, root):
        self.root = root
        self.root.title("X-Plane 12 DataRef Checker (REST API v3)")
        self.root.geometry("1025x350")

        # Variables
        self.ip_var = tk.StringVar(value="127.0.0.1")
        self.port_var = tk.StringVar(value="8086")
        self.dataref_name_var = tk.StringVar()
        self.dataref_id_var = tk.StringVar()
        self.result_var = tk.StringVar(value="En attente...")
        self.running = False
        self.current_dataref_id = None

        # GUI Elements
        ttk.Label(root, text="IP:").grid(row=0, column=0, padx=5, pady=5, sticky="e")
        ttk.Entry(root, textvariable=self.ip_var).grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(root, text="Port:").grid(row=1, column=0, padx=5, pady=5, sticky="e")
        ttk.Entry(root, textvariable=self.port_var).grid(row=1, column=1, padx=5, pady=5)

        ttk.Label(root, text="Nom de la DataRef:").grid(row=2, column=0, padx=5, pady=5, sticky="e")
        ttk.Entry(root, textvariable=self.dataref_name_var, width=50).grid(row=2, column=1, padx=5, pady=5)

        ttk.Label(root, text="ID de la DataRef:").grid(row=3, column=0, padx=5, pady=5, sticky="e")
        ttk.Entry(root, textvariable=self.dataref_id_var, state="readonly", width=10).grid(row=3, column=1, padx=5, pady=5, sticky="w")
        ttk.Button(root, text="Rechercher ID", command=self.fetch_dataref_id).grid(row=3, column=1, padx=5, pady=5, sticky="e")

        ttk.Label(root, text="Valeur:").grid(row=4, column=0, padx=5, pady=5, sticky="e")
        ttk.Label(root, textvariable=self.result_var, relief="sunken", width=40).grid(row=4, column=1, padx=5, pady=5)

        # Buttons
        self.start_button = ttk.Button(root, text="Démarrer", command=self.start_checking)
        self.start_button.grid(row=5, column=0, padx=5, pady=10)

        self.stop_button = ttk.Button(root, text="Arrêter", command=self.stop_checking, state="disabled")
        self.stop_button.grid(row=5, column=1, padx=5, pady=10)

    def fetch_dataref_id(self):
        """Récupère l'ID d'une DataRef à partir de son nom."""
        ip = self.ip_var.get()
        port = self.port_var.get()
        dataref_name = self.dataref_name_var.get()
        if not dataref_name:
            self.result_var.set("Veuillez entrer un nom de DataRef")
            return

        base_url = f"http://{ip}:{port}/api/v3"
        try:
            # Recherche la DataRef par nom
            response = requests.get(
                f"{base_url}/datarefs",
                params={"filter[name]": dataref_name},
                headers={"Accept": "application/json"}
            )
            if response.status_code == 200:
                data = response.json()
                if data["data"]:
                    self.current_dataref_id = data["data"][0]["id"]
                    self.dataref_id_var.set(str(self.current_dataref_id))
                    self.result_var.set(f"DataRef trouvée: ID={self.current_dataref_id}")
                else:
                    self.result_var.set(f"DataRef '{dataref_name}' introuvable")
                    self.current_dataref_id = None
            else:
                self.result_var.set(f"Erreur: {response.status_code} - {response.text}")
        except Exception as e:
            self.result_var.set(f"Erreur: {str(e)}")

    def start_checking(self):
        if not self.current_dataref_id:
            self.result_var.set("Veuillez d'abord rechercher une DataRef")
            return

        self.running = True
        self.start_button.config(state="disabled")
        self.stop_button.config(state="normal")
        threading.Thread(target=self.fetch_data, daemon=True).start()

    def stop_checking(self):
        self.running = False
        self.start_button.config(state="normal")
        self.stop_button.config(state="disabled")
        self.result_var.set("Arrêté")

    def fetch_data(self):
        ip = self.ip_var.get()
        port = self.port_var.get()
        base_url = f"http://{ip}:{port}/api/v3"

        while self.running:
            try:
                response = requests.get(
                    f"{base_url}/datarefs/{self.current_dataref_id}/value",
                    headers={"Accept": "application/json"}
                )
                if response.status_code == 200:
                    data = response.json()
                    self.result_var.set(str(data.get("data", "Valeur invalide")))
                else:
                    self.result_var.set(f"Erreur: {response.status_code} - {response.text}")
            except Exception as e:
                self.result_var.set(f"Erreur: {str(e)}")
            time.sleep(1)

if __name__ == "__main__":
    root = tk.Tk()
    app = XPlaneDataRefChecker(root)
    root.mainloop()