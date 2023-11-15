import paho.mqtt.client as mqtt
from pymongo import MongoClient

# Configurazione per il broker MQTT
mqtt_broker_host = "localhost"
mqtt_topic = "/plantione"

# Configurazione per MongoDB
mongo_host = "localhost"
mongo_port = 27017
mongo_db_name = "test"
mongo_collection_name = "coll1"

# Funzione di gestione del messaggio MQTT
def on_message(client, userdata, message):
    message_payload = message.payload.decode('utf-8','ignore')
    message_payload = message_payload.replace('\n', '')  # Rimuove il carattere di nuova riga
    
    # Suddividi la stringa in una lista di valori
    valori = message_payload.split(',')

    # Crea un dizionario con le intestazioni
    documento = {
        "topic": message.topic,
        "plant": valori[0].strip(),
        "moisture": valori[1].strip(),
        "luminosity": valori[2].strip(),
        "millisec": valori[3].strip(),
        "session": valori[4].strip()
    }
    
    print(f"Ricevuto messaggio su topic '{message.topic}': {documento}")
    
    # Connessione a MongoDB
    client_mongo = MongoClient(mongo_host, mongo_port)
    db = client_mongo[mongo_db_name]
    collection = db[mongo_collection_name]
    
    # Inserimento del messaggio in MongoDB
    #collection.insert_one({"topic": message.topic, "message": documento})
    collection.insert_one(documento)
    
    # Chiusura della connessione a MongoDB
    client_mongo.close()

# Configurazione del client MQTT
client = mqtt.Client()
client.on_message = on_message

# Connessione al broker MQTT
client.connect(mqtt_broker_host, 1883, 60)
client.subscribe(mqtt_topic, qos=0)

print("Attivati i servizi...\n")

# Avvio del loop di ascolto dei messaggi MQTT
client.loop_forever()
