#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <ArduinoJson.h>

const char* ssid = "Casa_42";
const char* password = "elgato2024";

const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;
const char* topic_sub = "esp/servos";

#define PIN_SERVO1 D5
#define PIN_SERVO2 D6

WiFiClient espClient;
PubSubClient client(espClient);

Servo servo1;
Servo servo2;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n-----------------------------------");
  Serial.print("MQTT mensaje recibido en tópico: ");
  Serial.println(topic);

  Serial.print("Longitud: ");
  Serial.println(length);

  String mensaje;
  for (unsigned int i = 0; i < length; i++) mensaje += (char)payload[i];

  Serial.print("Mensaje crudo: ");
  Serial.println(mensaje);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, mensaje);

  if (error) {
    Serial.print("Error parseando JSON: ");
    Serial.println(error.f_str());
    return;
  }

  int angulo = doc["angulo"].as<int>();
  angulo = constrain(angulo, 0, 180);

  Serial.print("Ángulo recibido: ");
  Serial.println(angulo);

  // Mover servos
  servo1.write(angulo);
  servo2.write(angulo);

  Serial.print("Servos movidos a: ");
  Serial.println(angulo);
  Serial.println("-----------------------------------\n");
}

void setup_wifi() {
  Serial.println();
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConectado a WiFi");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266_Servos")) {
      Serial.println("Conectado");
      client.subscribe(topic_sub);
      Serial.print("Suscrito al tópico: ");
      Serial.println(topic_sub);
    } else {
      Serial.print("Error rc=");
      Serial.print(client.state());
      Serial.println(" - Reintentando en 3s...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  servo1.attach(PIN_SERVO1);
  servo2.attach(PIN_SERVO2);

  servo1.write(0);
  servo2.write(0);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("\nEsperando comandos MQTT...");
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
