# Hardware y comunicación

## Plataforma y sensores

ESP32 DevKit V1 / ESP32-WROOM-32 clásico. El sistema usa exclusivamente dos sensores:

| Elemento | GPIO | Nota |
|---|---:|---|
| Sensor capacitivo de humedad AO | 34 | ADC1; AOUT máximo 3,3 V |
| Caudalímetro: pulsos | 27 | Interrupción; adaptar nivel a 3,3 V |

## Actuador

GPIO26 controla la entrada de un driver/relé. La electroválvula debe ser normalmente cerrada, apta para la presión estática del tanque y alimentada por fuente externa. No conectar bobina al GPIO. Elegir la etapa de potencia según tensión y corriente de válvula.

## Hidráulica

Tanque elevado → electroválvula NC → caudalímetro → manguera/goteros → planta. Confirmar que la válvula abra con la presión disponible (preferir acción directa si la presión es baja) y que el sensor de flujo mida el caudal mínimo real.

## Wi-Fi y Godot

El firmware crea un punto de acceso:
- SSID: `MACETA_ESP32`
- Contraseña inicial: `riego1234` (cambiar en firmware antes de desplegar)
- TCP: `192.168.4.1:5000`
- Protocolo: JSON delimitado por salto de línea.

La interfaz Godot se conecta a ese AP y recibe telemetría. La configuración enviada desde Godot se aplica en RAM y se pierde al reiniciar. No se implementa control remoto de riego manual ni apagado remoto.

## Calibración y seguridad

- Calibrar ADC seco/húmedo en sustrato real.
- Calibrar pulsos por litro recogiendo volumen conocido a la altura definitiva del tanque.
- Adaptar cualquier salida de 5 V antes de conectarla a GPIO.
- Mantener electrónica protegida del agua.
- Verificar válvula cerrada al arranque y ante pérdida de alimentación.
- Probar primero con la válvula desconectada y después en banco, con supervisión.
