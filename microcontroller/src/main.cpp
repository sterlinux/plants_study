/*
  Primo prototipo completo di PlantaIot (nome provvisorio).
  Il software sarà multi-thread.
  Avrà la capacità di:
  - leggere i valori di sensori analogici con una frequenza determinata
  - memorizzare i dati letti in un file csv ben strutturato
  - inviare i dati letti ad un broker MQTT su un topic predeterminato.


  File di configurazione
  ------------------------------
  Sarà previsto un file di configurazione che sarà letto all'avvio del dispositivo.
  In esso saranno memorizzati i parametri di configurazione dello stesso.
  - credenziali di accesso alla WiFi
  - credenziali e indirizzo del broker MQTT


  Setup
  ------------------------------
  La funzione si avvia all'avvio del dispositivo.
  Dovrà eseguire alcune operazioni:
  - abilitare il dispositivo SDCard
  - leggere il file di configurazione
  - impostare i parametri per la connessione WiFi
  - impostare le credenziali per l'MQTT
  - abilitare la queue di scambio dati fra i due Thread


  Primo Thread
  ------------------------------
  E' quello più importante: la priorità impostata è la più alta.
  Riguarda la lettura dei dati dai sensori e la loro
  memorizzazione nella coda.
  - leggere i dati dai sensori
  - inserire i dati letti nella coda
  - timing frequenza di campionamento


  Secondo Thread
  ------------------------------
  E' ugualmente importante ma la priorità è più bassa.
  Il suo compito è quello di smaltire la coda leggendone i dati ed inserendoli
  nel file csv che è stato abilitato per questo.
  Oltre a questo dovranno essere inviati i dati ad un broker MQTT per poi essere processati in tempo reale.
  - estrarre i dati dalla coda
  - memorizzare i dati nel file csv
  - inviare i dati al broker MQTT


  Passaggio dei dati fra Thread
  ------------------------------
  Viene utilizzata una struttura dati, una Queue, messa a disposizione da FreeRTOS.
  Il primo thread accederà in scrittura alla coda, inserendo i dati dei sensori letti
  Il secondo thread accederà in lettua, consumando i dati.

*/

#include <Arduino.h>

// Inclusione delle librerie per l'utilizzo di FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Inclusione delle librerie per l'interazione con la SDCard
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Inclusione delle librerie per il WiFi e MQTT
#include <WiFi.h>
#include <PubSubClient.h>

// Definizione della GPIO
#define pinButtonRec 34
#define pinButtonStop 35

//  MOISTURE SENSOR
//#define VCCMOISTPIN 16       // Digital pin per alimentare il moisture sensor
#define INMOISTPIN 36     // Analog pin su cui sarà letto il valore del moisture sensor
#define LUMENPIN 39     // Analog pin su cui sarà letto il valore della luminosità

//  AD8232 sensor
#define ECG8232 32

// Imposta le credenziali WiFi
const char *ssid = "";
const char *password = "";

// Imposta le informazioni del broker MQTT
const char *mqtt_server = "";
int mqtt_port = 1883;           // Porta MQTT predefinita
const char *mqtt_username = ""; // Lascia vuoto se non è richiesta l'autenticazione
const char *mqtt_password = ""; // Lascia vuoto se non è richiesta l'autenticazione
const char *mqtt_topic = "";    // Il topic MQTT su cui vuoi pubblicare il messaggio

int canRegister = 0;

int registeringSession = 0;

bool enableMqtt = true;
bool enableWiFi = true;

// Oggetto per gestire la connessione WiFi
WiFiClient espClient;

// Oggetto per la gestione del client MQTT
PubSubClient client(espClient);

// Definizione della coda
xQueueHandle SensorDataQueue;

char filename[50];
File file;

// Struttura dati per la memorizzazione dei dati rilevati
typedef struct
{
  int plantSignal;
  int moistureSignal;
  int luminositySignal;
  int millisecond;
  int session;
} sensor_data;

int indx1 = 0;

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)){
    Serial.println("File written");
  }
  else{
    Serial.println("Write failed");
  }
  file.close();
}

/*
  Bisogna rivedere questa funzione.
  L'apertura e la chiusura del file ad ogni ciclo può essere una operazione
  troppo costosa.
*/
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  // Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file){
    Serial.println("Failed to open file for appending");
    return;
  }

  if (file.print(message)){
    Serial.println("Message appended");
  }
  else{
    Serial.println("Append failed");
  }

  file.close();
}

/*
  Bisogna rivedere questa funzione.
  L'apertura e la chiusura del file ad ogni ciclo può essere una operazione
  troppo costosa.
*/
void appendFile2(fs::FS &fs, const char *path, const char *message){
  // Serial.printf("Appending to file: %s\n", path);

  //File file = fs.open(path, FILE_APPEND);
  if (!file){
    Serial.println("Failed to open file for appending");
    return;
  }

  file.print(message);
  
  /*
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  */

  //file.close();
}



/*
  Primo thread. E' quello designato alla lettura dei dati dai sensori
  e il loro inserimento nella coda.
*/
void task1(void *parameter)
{
  int volte = 0;
  while (true)
  {
    if (canRegister == 100)  // la registrazione avviene solo se è stata abilitata
    { 
      //Serial.println("Task 1 is running");
      sensor_data ptrtostruct;
      //digitalWrite(VCCMOISTPIN,HIGH);
      int moistureValue = analogRead(INMOISTPIN);
      int luminosityValue = analogRead(LUMENPIN);
      int plantValue = analogRead(ECG8232);

      ptrtostruct.plantSignal = plantValue;                 // valore dell'Action Potential della pianta
      ptrtostruct.luminositySignal = luminosityValue;       // valore della luminosità ambientale
      ptrtostruct.moistureSignal = moistureValue;           // valore dell'umidità del terreno
      ptrtostruct.millisecond = millis();                   // valore del tempo
      ptrtostruct.session = registeringSession;             // sessione di registrazione

      xQueueSend(SensorDataQueue, &ptrtostruct, portMAX_DELAY);

      /*
      if (xQueueSend(SensorDataQueue, &ptrtostruct, portMAX_DELAY) == pdPASS)
      {
        Serial.println("ins ok");
      }

      */

      indx1 = indx1 + 1;
      volte++;
      
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for 1 second
  }
}


/*
  Secondo thread. E' quello designato alla elaborazione dei dati una volta
  estratti dalla coda. L'elaborazione iniziale in realtà consiste soltanto nella loro:
  - formattazione in csv (di fatto operazione costosa)
  - invio del dato acquisito al broker MQTT (operazione veloce)
  - memorizzazione del dato acquisito nella SD Card (operazione parecchio costosa). 
*/
void task2(void *parameter)
{

  while (true)
  {
    if (canRegister == 100)  // la registrazione avviene solo se è stata abilitata
    { 
      //Serial.println("Task 2 is running");
      sensor_data receivedData;
      if (xQueueReceive(SensorDataQueue, &receivedData, portMAX_DELAY))
      {
        //Serial.println("Task 2 received data:");
        // Serial.println(receivedData.plantSignal);

        char csvRow[256]; // Buffer per la stringa risultante

        char part1[20];
        sprintf(part1, "%d", receivedData.plantSignal);
        strcat(csvRow, part1);
        strcat(csvRow, ",");

        char part2[20];
        sprintf(part2, "%d", receivedData.moistureSignal);
        strcat(csvRow, part2);
        strcat(csvRow, ",");

        char part3[20];
        sprintf(part3, "%d", receivedData.luminositySignal);
        strcat(csvRow, part3);
        strcat(csvRow, ",");

        char part4[20];
        sprintf(part4, "%d", receivedData.millisecond);
        strcat(csvRow, part4);
        strcat(csvRow, ",");
        
        char part5[20];
        sprintf(part5, "%d", receivedData.session);
        strcat(csvRow, part5);
        
        
        
        //strcat(csvRow, "\n");

        String message = String(csvRow) + "\n";
        String mqtt_message = String(csvRow);
        Serial.println(message);
        csvRow[0] = '\0';
        part1[0] = '\0';
        part2[0] = '\0';
        part3[0] = '\0';
        part4[0] = '\0';
        part5[0] = '\0';

        //appendFile(SD, filename, message.c_str());
        appendFile2(SD, filename, message.c_str());
        if(enableMqtt){
          client.publish(mqtt_topic, mqtt_message.c_str());
        }
      }
    }

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

/*
  Recupero i parametri di configurazione per accedere ai vari servizi
  IoT: WiFi e MQTT.

  I parametri sono memorizzati in un file config.cfg che si trova nella
  root della SDCard inserita nel lettore.

*/
void getConfiguration(fs::FS &fs, const char *path)
{
  Serial.printf("Reading configuration file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    // Serial.write(file.read());
    // Serial.write("-");
    String line = file.readStringUntil('\n');
    // Dividi la riga in chiave e valore
    int separatorIndex = line.indexOf(':');
    if (separatorIndex != -1)
    {
      String key = line.substring(0, separatorIndex);
      String value = line.substring(separatorIndex + 1);
      if (key == "wifi_ssid")
      { // WiFi
        Serial.print("wifi_ssid: ");
        Serial.println(value);
        ssid = strdup(value.c_str());
      }
      else if (key == "wifi_password")
      {
        Serial.print("wifi_password: ");
        Serial.println(value);
        password = strdup(value.c_str());
      }
      else if (key == "mqtt_server")
      {
        Serial.print("mqtt_server: ");
        Serial.println(value);
        mqtt_server = strdup(value.c_str());
      }
      else if (key == "mqtt_port")
      {
        Serial.print("mqtt_port: ");
        Serial.println(value);
        mqtt_port = value.toInt();
      }
      else if (key == "mqtt_username")
      {
        Serial.print("mqtt_username: ");
        Serial.println(value);
        mqtt_username = strdup(value.c_str());
      }
      else if (key == "mqtt_password")
      {
        Serial.print("mqtt_password: ");
        Serial.println(value);
        mqtt_password = strdup(value.c_str());
      }
      else if (key == "mqtt_topic")
      {
        Serial.print("mqtt_topic: ");
        Serial.println(value);
        mqtt_topic = strdup(value.c_str());
      }
    }
  }
  file.close();

  /*
    Inizializzo il numero della sessione
  */
  File fileSession = fs.open("/session.cfg");
  if (!fileSession){
    Serial.println("Failed to open file for reading");
    return;
  }

  while (fileSession.available()){
    String line = fileSession.readStringUntil('\n');
    registeringSession = line.toInt();
    registeringSession++;
  }

  fileSession.close();




  Serial.println("Success! Complete setup.");
  // Qui va accesso il led verde.
}

void setup(){
  Serial.begin(115200);
  // Inizializzazione dei pin per i pulsanti
  pinMode(pinButtonRec, INPUT_PULLUP);
  pinMode(pinButtonStop, INPUT_PULLUP);

  // Inizializzazione dei pin per il Moisture Sensor
  pinMode(LUMENPIN,INPUT);
  pinMode(INMOISTPIN,INPUT);
  pinMode(ECG8232,INPUT);

  // Setup SDCard
  if (!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  // Recupero dei parametri di configurazione
  getConfiguration(SD, "/config.cfg");


  if(enableWiFi){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED){
      delay(1000);
      Serial.println("Connessione WiFi in corso...");
    }
    Serial.println("Connessione WiFi stabilita.");
  }
  

  if(enableMqtt){
    // Imposta il server MQTT e le credenziali (se necessario)
    client.setServer(mqtt_server, mqtt_port);
    // client.setCredentials(mqtt_username, mqtt_password);

    // Attendi la connessione al server MQTT
    while (!client.connected()){
      Serial.println("Connessione al server MQTT...");
      if (client.connect("ESP32Client")){
        Serial.println("Connesso al server MQTT!");
        client.loop();
      }
      else
      {
        Serial.print("Errore di connessione MQTT, rc=");
        Serial.print(client.state());
        Serial.println(" Riprova tra 5 secondi.");
        delay(5000);
      }
    }
  }
  

  // Creazione della coda per lo scambio dei dati fra i Thread
  SensorDataQueue = xQueueCreate(100, sizeof(sensor_data));

  xTaskCreatePinnedToCore(task1, "Task 1", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(task2, "Task 2", 10000, NULL, 3, NULL, 1);
}



void reconnect(){
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop()
{
  /*
      GESTIONE DEL PULSANTE DI START REGISTRAZIONE
  */

  int buttonRecState = digitalRead(pinButtonRec);

  

  if (buttonRecState == LOW && canRegister == 0)
  {
    Serial.println("Inizia la registrazione.");
    
    if(enableMqtt){
      if( !client.connected() ){ 
        reconnect(); 
      }
      client.loop();
    }

    canRegister = 100;
    
    
    // sprintf(filename, "/test_%d.csv", esp_random());
    sprintf(filename, "/test_%d.csv", registeringSession);
    Serial.println(filename);
    writeFile(SD, filename, "plant,moisture,luminosiy,millisec,session\n");

    // Apertura del file per evitare di aprirlo nel loop
    file = SD.open(filename, FILE_APPEND);
    


    delay(1000);
  }


  /*
      GESTIONE DEL PULSANTE DI STOP REGISTRAZIONE
  */
  int buttonStop = digitalRead(pinButtonStop);

  if (buttonStop == LOW && canRegister == 100){
    Serial.println("Termina la registrazione.");
    canRegister = 0;
    file.close();
    filename[0] = '\0';
    registeringSession++;
    writeFile(SD, "/session.cfg", String(registeringSession).c_str());
    
    delay(1000);
  }
}
