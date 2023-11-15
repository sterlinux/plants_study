import paho.mqtt.client as mqtt
import time

# Funzione per la pubblicazione dei messaggi MQTT
def publish_messages(client, topic, num_messages, message_interval):
    for i in range(1, num_messages + 1):
        message = f"Messaggio {i}"
        client.publish(topic, message)
        print(f"Inviato messaggio {i}: {message}")
        time.sleep(message_interval)

# Configurazione del client MQTT
client = mqtt.Client()
mqtt_broker_host = "localhost"

# Connessione al broker MQTT
client.connect(mqtt_broker_host, 1883, 60)

# Specifica il topic MQTT a cui inviare i messaggi
mqtt_topic = "/plantione"

# Numero di messaggi da inviare
num_messages = 1000

# Intervallo tra i messaggi in millisecondi (5 ms = 0.005 secondi)
message_interval = 0.005

# Avvio della pubblicazione dei messaggi
publish_messages(client, mqtt_topic, num_messages, message_interval)

# Chiusura della connessione al broker MQTT
client.disconnect()
