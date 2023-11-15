from pymongo import MongoClient

# Connetti al database MongoDB
client = MongoClient("mongodb://localhost:27017/")
db = client["test"]
collection = db["coll1"]

# Crea un documento da inserire
documento = {
    "timestamp": "2023-09-13T12:00:00Z",
    "paciente_id": "ID_del_paziente",
    "campioni": [
        {"sample_number": 1, "value": 0.12},
        {"sample_number": 2, "value": 0.15},
        # Aggiungi altri campioni...
    ]
}

# Inserisci il documento in MongoDB
collection.insert_one(documento)
