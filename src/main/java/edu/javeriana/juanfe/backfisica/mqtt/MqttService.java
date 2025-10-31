package edu.javeriana.juanfe.backfisica.mqtt;

import edu.javeriana.juanfe.backfisica.config.WebSocketHandler;
import jakarta.annotation.PostConstruct;
import org.eclipse.paho.client.mqttv3.*;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

@Service
public class MqttService {
    private final MqttClient client;
    private final WebSocketHandler webSocketHandler;

    @Value("${mqtt.topic.servos}")
    private String servoTopic;

    @Value("${mqtt.topic.fsr}")
    private String fsrTopic;

    public MqttService(MqttClient client, WebSocketHandler webSocketHandler) {
        this.client = client;
        this.webSocketHandler = webSocketHandler;
    }

    @PostConstruct
    private void init() {
        subscribeToFsrTopic();
    }

    public void publishServoData(String message) {
        try {
            client.publish(servoTopic, new MqttMessage(message.getBytes()));
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void subscribeToFsrTopic() {
        try {
            System.out.println("Suscribiéndose al topic FSR: " + fsrTopic);
            client.subscribe(fsrTopic, (topic, msg) -> {
                String payload = new String(msg.getPayload());
                System.out.println("FSR recibió: " + payload);
                webSocketHandler.sendToAll("Impacto detectado");
            });
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }
}
