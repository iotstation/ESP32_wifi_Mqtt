#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "your wifi";
const char* password = "your password";

// MQTT Broker settings
const char* mqtt_server = "4.tcp.ngrok.io"; // e.g., "192.168.1.100"
const int mqtt_port = 16619;  // 1883
const char* mqtt_user = "iot"; // Leave blank if none
const char* mqtt_password = "1234567890"; // Leave blank if none

// DHT11 settings
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQTT client
WiFiClient espClient;        //creates a basic TCP connection.
PubSubClient client(espClient);    //uses that connection to talk to the MQTT broker

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); //while waiting, then confirms once connected.
  }
  Serial.println("\nConnected to WiFi");

  // Connect to MQTT
  client.setServer(mqtt_server, mqtt_port);  // Sets the MQTT server address and port.
  reconnect();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}
// Keeps the MQTT connection alive. If disconnected, tries to reconnect.
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read DHT11 data
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {                                //If either temp or hum is not a number, then
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Publish data to MQTT topics:    NOTE: MQTT publish() expects text or binary, not float
  char tempStr[8];   // Creates two character arrays 7 characters + 1 null character= total 8 characters
  char humStr[8];
  dtostrf(temp, 6, 2, tempStr); // convert a floating-point number into a text string. 6 total size and 2 decimal
  dtostrf(hum, 6, 2, humStr);

  client.publish("sensor/temperature", tempStr);
  client.publish("sensor/humidity", humStr);

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" °C, Hum: ");
  Serial.print(hum);
  Serial.println(" %");

  delay(6000); // Publish every 60 seconds
}