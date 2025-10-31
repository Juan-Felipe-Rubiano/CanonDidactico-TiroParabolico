#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <ArduinoJson.h>

const char* ssid = "juanfe";
const char* password = "12345678";


const char* mqtt_server = "172.30.48.1";
const int mqtt_port = 1883;
const char* topic_sub = "esp/servos";      // Tema que escucha el ESP

WiFiClient espClient;
PubSubClient client(espClient);

Servo servoTension; // Servo que tensa el resorte (MG995)
Servo servoInclinacion; // Servo que inclina el cañón (CS60)

// Variables de control
int angulo = 90;
int tension = 0;

// --- Función para conectar al WiFi ---
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(")");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());
}

// --- Callback cuando llega un mensaje MQTT ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en [");
  Serial.print(topic);
  Serial.print("]: ");

  String mensaje;
  for (unsigned int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  Serial.println(mensaje);

  // Parsear JSON del mensaje
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, mensaje);

  if (error) {
    Serial.print("Error parseando JSON: ");
    Serial.println(error.f_str());
    return;
  }

  // Leer los valores del JSON
  angulo = doc["angulo"];
  tension = doc["tension"];

  Serial.print("Ángulo: ");
  Serial.println(angulo);
  Serial.print("Tensión: ");
  Serial.println(tension);

  // Mover los servos
  moverServos();
}

// --- Función para mover los servos ---
void moverServos() {
  // Limitar los valores a un rango válido
  angulo = constrain(angulo, 0, 180);
  tension = constrain(tension * 10, 0, 180); // Escalamos la tensión (segundos -> posición aprox.)

  servoInclinacion.write(angulo);
  servoTension.write(tension);

  Serial.print("↩Servos movidos -> Inclinación: ");
  Serial.print(angulo);
  Serial.print("°, Tensión: ");
  Serial.print(tension);
  Serial.println("°");
}

// --- Reconexión a MQTT si se pierde conexión ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266_Servos")) {
      Serial.println("conectado");
      client.subscribe(topic_sub);
    } else {
      Serial.print("Error (rc=");
      Serial.print(client.state());
      Serial.println("). Reintentando en 3s...");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  servoTension.attach(D4);     // Servo de tensión (MG995)
  servoInclinacion.attach(D5); // Servo de inclinación (CS60)

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Esperando comandos MQTT...");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
