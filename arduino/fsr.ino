#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ------------------------------------------------
// 1. CONFIGURACIÓN DE RED Y MQTT
// ------------------------------------------------

// *** ¡IMPORTANTE! Reemplaza con tus credenciales de red de 2.4 GHz ***
const char* ssid = "juanfe";
const char* password = "12345678";

// *** ¡IMPORTANTE! Reemplaza con la IP de tu PC (donde corre Mosquitto/Spring) ***
const char* mqtt_server = "172.30.48.1";
const int mqtt_port = 1883;
// Este topic debe coincidir con 'mqtt.topic.fsr' en application.properties
const char* topic_pub_fsr = "esp/fsr";

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------------------------------------
// 2. CONFIGURACIÓN DEL SENSOR Y LÓGICA
// ------------------------------------------------

#define FORCE_SENSOR_PIN A0
// Umbral de fuerza. Ajústalo si 500 es muy sensible o muy poco.
#define UMBRAL_FUERZA 500

// Variables de control de lectura
long lastMsg = 0;
bool impactoEnviado = false; // Bandera para evitar enviar múltiples mensajes por un golpe

// ------------------------------------------------
// 3. FUNCIONES DE CONEXIÓN
// ------------------------------------------------

void setup_wifi() {
 delay(10);
 Serial.println();
 Serial.print("Conectando a WiFi: ");
 Serial.println(ssid);

 WiFi.begin(ssid, password);

 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }

 Serial.println("");
 Serial.println("Conectado a WiFi");
 Serial.print("IP asignada: ");
 Serial.println(WiFi.localIP());
}

// Función de reconexión a MQTT
void reconnect() {
while (!client.connected()) {
 Serial.print("Intentando conexión MQTT...");
    // Importante: El ClientID debe ser ÚNICO en la red
 if (client.connect("ESP8266_FSR_Client")) {
 Serial.println("conectado");
 } else {
 Serial.print("Error (rc=");
 Serial.print(client.state());
 Serial.println("). Reintentando en 5s...");
 delay(5000);
 }
 }
}

// ------------------------------------------------
// 4. SETUP Y LOOP PRINCIPAL
// ------------------------------------------------

void setup() {
  // Usar 115200 baudios para un ESP8266 (es más estable)
  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  // Este ESP solo publica, no necesita client.setCallback

  Serial.println("ESP-FSR listo para detectar impacto...");
}

void loop() {
if (!client.connected()) {
  reconnect();
 }
 client.loop();

  // Lógica de lectura y publicación (solo cada 200ms para evitar spam y rebotes)
 long now = millis();
 if (now - lastMsg > 200) {
 lastMsg = now;

 int analogReading = analogRead(FORCE_SENSOR_PIN);
 Serial.print("Lectura FSR = ");
 Serial.println(analogReading);

    // Si la fuerza supera el umbral Y no hemos enviado el impacto de este golpe...
if (analogReading > UMBRAL_FUERZA && !impactoEnviado) {
 Serial.println(">>> ¡IMPACTO DETECTADO! Enviando a MQTT...");

      // Publica un mensaje en el topic. El contenido es 'IMPACTO'
 client.publish(topic_pub_fsr, "IMPACTO");

 impactoEnviado = true; // Marca como enviado

 } else if (analogReading < UMBRAL_FUERZA) {
      // Si la fuerza cae por debajo del umbral, reseteamos la bandera
 impactoEnviado = false;
 }
 }
}