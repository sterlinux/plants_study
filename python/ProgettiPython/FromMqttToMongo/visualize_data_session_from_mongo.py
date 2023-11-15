import matplotlib.pyplot as plt
from pymongo import MongoClient

# Richiedi all'utente di inserire il valore di "session" da terminale
session_value = input("Inserisci il valore di session: ")

# Connessione a MongoDB
client = MongoClient("mongodb://localhost:27017")
db = client.test
collezione = db.coll1


start_index = input("Inserisci il limite di inizio (-1 per bypassare): ")  # Indice del documento di partenza (0-based index)
end_index = input("Inserisci il limite di fine: ")   # Indice del documento di fine

if(start_index == ""):
    start_index = "-1"

# Recupero dei dati dalla collezione
#documenti = collezione.find({"session": "85"}).sort("millisec", 1).limit(1000)
documenti = collezione.find({"session": session_value}).sort("millisec", 1)

if(start_index != "-1"):
    # Salta i documenti fino all'indice di partenza
    documenti.skip(int(start_index))

    # Limita il numero di documenti da recuperare (in questo caso, 501 documenti)
    documenti.limit(int(end_index) - int(start_index) + 1)


# Estrazione delle variabili di interesse e creazione di list comprehensions
plant_data = [doc["plant"] for doc in documenti]
documenti.rewind()

moisture_data = [docs['moisture'] for docs in documenti]
documenti.rewind()

luminosity_data = [doct["luminosity"] for doct in documenti]
documenti.rewind()

plant_data = [float(valore) for valore in plant_data]
moisture_data = [float(valore) for valore in moisture_data]
luminosity_data = [float(valore) for valore in luminosity_data]



# Creazione del grafico
plt.figure(figsize=(10, 6))
plt.plot(plant_data, label="Plant")
plt.plot(moisture_data, label="Moisture")
plt.plot(luminosity_data, label="Luminosity")
plt.xlabel("Timestamp")
plt.ylabel("Valore")
plt.title("Grafico dei dati")
plt.legend()
plt.grid(True)

# Mostra il grafico
plt.show()
