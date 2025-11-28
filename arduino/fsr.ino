#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Casa_42";
const char* password = "elgato2024";

const char* mqtt_server = "192.168.1.11";
const int mqtt_port = 1883;

const char* topic_pub_fsr = "esp/fsr";
const char* MESSAGE_IMPACTO = "Impacto detectado";

WiFiClient espClient;
PubSubClient client(espClient);

#define FORCE_SENSOR_PIN A0
#define UMBRAL_FUERZA 500

long lastMsg = 0;
bool impactoEnviado = false;

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
Serial.print("IP asignada al ESP: ");
Serial.println(WiFi.localIP());
}

void reconnect() {
while (!client.connected()) {
Serial.print("Intentando conexión MQTT a ");
Serial.print(mqtt_server);
Serial.print("...");
if (client.connect("ESP8266_FSR_Client")) {
Serial.println("conectado");
} else {
Serial.print("Error (rc=");
Serial.print(client.state());
Serial.println("). Asegúrese de que la IP del servidor es correcta y el broker está activo.");
delay(5000);
}
}
}

void setup() {
 Serial.begin(115200);

 setup_wifi();
 client.setServer(mqtt_server, mqtt_port);

 Serial.println("ESP-FSR listo para detectar impacto");
}

void loop() {
if (!client.connected()) {
 reconnect();
}
client.loop();

long now = millis();
if (now - lastMsg > 200) {
lastMsg = now;

int analogReading = analogRead(FORCE_SENSOR_PIN);

if (analogReading > UMBRAL_FUERZA && !impactoEnviado) {
Serial.println(">>> Impacto detectado. Envando a MQTT...");

client.publish(topic_pub_fsr, MESSAGE_IMPACTO);

impactoEnviado = true;

} else if (analogReading < UMBRAL_FUERZA) {
impactoEnviado = false;
}
}
}
