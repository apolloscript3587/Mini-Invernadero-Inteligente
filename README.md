# Mini-Invernadero Inteligente

Sistema de riego automático por gravedad con ESP32 y **dos sensores**: humedad del suelo y flujo de agua. Un tanque elevado alimenta la línea; una electroválvula normalmente cerrada regula el paso y el caudalímetro, instalado después de ella, mide el agua suministrada.

## Arquitectura de control

La humedad del suelo decide cuándo iniciar y finalizar el riego. El caudalímetro mide caudal/volumen y permite detectar ausencia de pulsos durante un pulso activo. El ESP32 riega en pulsos, pausa para infiltración y vuelve a evaluar la humedad. Se aplican límites de tiempo y volumen.

## Interfaz Godot integrada

La aplicación de escritorio Godot 4 se comunica con el ESP32 mediante TCP sobre el punto de acceso Wi-Fi creado por el propio ESP32.

1. Cargar el firmware desde `firmware/` con PlatformIO (entorno `esp32dev`) o usar Arduino IDE con ArduinoJson 7 instalado.
2. Conectar el computador a la red Wi-Fi `MACETA_ESP32`; contraseña inicial `riego1234`.
3. Ejecutar el proyecto `godot/` con Godot 4.
4. Pulsar **Conectar**. El ESP32 escucha TCP en `192.168.4.1:5000`.
5. La pantalla muestra humedad, ADC, caudal instantáneo, litros del ciclo, válvula, estado y evento.
6. Los parámetros de riego se pueden enviar desde la interfaz: umbral de inicio, objetivo, duración de pulso, pausa de infiltración, tiempo máximo y volumen máximo.

La configuración enviada se mantiene en RAM y vuelve a los valores por defecto al reiniciar. La interfaz no permite forzar riego manual ni apagar remotamente el controlador. Cambiar la contraseña del AP en el firmware antes de desplegarlo.

## Archivos principales

- `firmware/src/main.cpp`: firmware ESP32 con algoritmo, punto de acceso Wi-Fi y servidor TCP/JSON.
- `firmware/platformio.ini`: configuración PlatformIO y dependencia ArduinoJson.
- `godot/`: interfaz de monitorización y configuración.
- `codigo/mini_invernadero_esp32.ino`: sketch autónomo de referencia por puerto serie (no incluye el protocolo Godot).
- `docs/hardware.md`, `diagramas/`: conexiones y seguridad.
- `documentacion/`, `pruebas/`: calibración y plan de validación.

## Pines

| Módulo | GPIO |
|---|---:|
| Sensor capacitivo de humedad AO | 34 |
| Caudalímetro (pulsos) | 27 |
| Driver de electroválvula | 26 |

Hidráulica: tanque elevado → electroválvula NC → caudalímetro → goteros/planta.

## Calibración obligatoria

Antes de habilitar el riego, establecer `SOIL_DRY_RAW` y `SOIL_WET_RAW` en `firmware/src/main.cpp` con lecturas ADC reales. Calibrar `pulsesPerLiter` con un volumen conocido. El factor de caudal debe verificarse con la altura y tubería reales.

Los valores iniciales (35 % inicio, 45 % objetivo, pulso 5 s, pausa 30 s, 30 s máximos y 0,5 L) son ejemplos y deben adaptarse al cultivo y sustrato.

## Protocolo de comunicación

TCP en puerto 5000, mensajes JSON terminados en salto de línea. Tipos admitidos: `ping`, `get_status` y `config`. La telemetría usa `type=telemetry` e incluye `soil_moisture_pct`, `flow_l_min`, `water_liters_cycle`, `valve_open`, `state` y `event`. Las respuestas a comandos usan `type=ack`.

## Seguridad y validación

La válvula debe ser NC, apta para baja presión y accionada por driver/fuente externa. No alimentar la bobina desde un GPIO. Adaptar las señales de 5 V a 3,3 V. Proteger electrónica de salpicaduras. Verificar que el caudalímetro detecte el caudal por gravedad y que la válvula opere con la presión disponible.

**No se ha ejecutado una compilación ni una prueba física en esta modificación.** Compilar y validar con la válvula desconectada primero; comprobar niveles lógicos, calibración, comunicación y cierre seguro antes de operar con agua.

## Miembros

### Julian Jimenez
### Juan Chancy
### Santiago Polo
