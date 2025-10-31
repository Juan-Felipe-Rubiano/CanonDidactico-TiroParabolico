package edu.javeriana.juanfe.backfisica.controllers;

import edu.javeriana.juanfe.backfisica.mqtt.MqttService;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/control")
@CrossOrigin(origins = "*")
public class ControlController {
    private final MqttService mqttService;

    public ControlController(MqttService mqttService) {
        this.mqttService = mqttService;
    }

    @PostMapping("/servos")
    public String enviarServos(@RequestParam int angulo, @RequestParam double tension){
        String mensaje = String.format("{\"angulo\": %d, \"tension\": %.2f}", angulo, tension);
        mqttService.publishServoData(mensaje);
        return "Comando enviado: " + mensaje;
    }
}
