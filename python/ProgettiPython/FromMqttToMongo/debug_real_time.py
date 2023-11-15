import paho.mqtt.client as mqtt

# Funzione di callback richiamata quando il client si connette al broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connesso al broker MQTT")
        client.subscribe("/plantione")  # Sostituisci con il tuo topic
    else:
        print(f"Connessione fallita con codice di ritorno {rc}")

# Funzione di callback richiamata quando il client riceve un messaggio sul topic sottoscritto
def on_message(client, userdata, msg):
    print(f"Ricevuto messaggio su topic '{msg.topic}': {msg.payload.decode()}")

# Configura il client MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Specifica l'indirizzo del broker MQTT
mqtt_broker_host = "localhost"

# Connettiti al broker MQTT
client.connect(mqtt_broker_host, 1883, 60)

# Loop di ascolto per ricevere i messaggi
client.loop_forever()
