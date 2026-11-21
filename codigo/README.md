# Firmware ESP32

## Plataforma de referencia

Firmware de referencia para **ESP32 DevKit V1 / ESP32-WROOM-32**. El material inicial no especificaba microcontrolador, por lo que se eligió ESP32 como base de implementación. Confirmar la placa física o adaptar pines antes de montar.

## Pines propuestos

| Elemento | ESP32 |
|---|---|
| DHT22 DATA | GPIO 4 |
| BH1750 SDA | GPIO 21 |
| BH1750 SCL | GPIO 22 |
| Sensor capacitivo AO | GPIO 34 (ADC1) |
| Relé de riego opcional IN | GPIO 26 |

## Librerías Arduino IDE

Instalar desde Library Manager:
1. **DHT sensor library** de Adafruit.
2. **Adafruit Unified Sensor**.
3. **BH1750** (librería de claws).

Referencias: [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library), [API ADC Arduino-ESP32](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html).

## Carga

1. Abrir `mini_invernadero_esp32.ino` en Arduino IDE.
2. Seleccionar ESP32 Dev Module (o modelo exacto de placa) y puerto.
3. Instalar las librerías anteriores.
4. Compilar, cargar y abrir Monitor Serie a **115200 baudios**.
5. Verificar las lecturas antes de conectar cualquier bomba.

El firmware emite una línea JSON cada 2,5 segundos: temperatura, humedad relativa, lux, lectura ADC cruda del suelo, porcentaje calibrado y estado de la bomba. Lecturas no disponibles aparecen como `null`.

## Calibración de humedad del suelo

En el sketch, sustituir `SOIL_DRY_RAW = -1` y `SOIL_WET_RAW = -1` por valores realmente medidos:
- `SOIL_DRY_RAW`: lectura ADC estable con la sonda en el sustrato seco de referencia.
- `SOIL_WET_RAW`: lectura ADC estable con el sustrato humedecido.

Los valores dependen del sensor y del sustrato. No copiar calibraciones ajenas. Si no se calibra, el porcentaje es `null` y el riego no funcionará.

## Riego opcional

El riego está **desactivado** con `ENABLE_IRRIGATION = false`. No cambiarlo hasta confirmar un relé compatible con señal de 3,3 V, bomba, fuente externa, polaridad del relé y calibración del sensor. La bomba jamás debe alimentarse desde un GPIO o desde el pin 3V3 del ESP32.

## Seguridad eléctrica

- GPIO del ESP32 no toleran 5 V. La salida AO del sensor debe ser menor o igual al máximo admitido por la placa; usar margen y medirla antes de conectar.
- Alimentar el sensor capacitivo a 3,3 V sólo si su ficha técnica lo permite.
- El DHT22 desnudo suele necesitar pull-up DATA–3V3 de 4,7–10 kΩ; un módulo puede traerla.
- Confirmar tensión de alimentación del breakout BH1750.
- Usar fuente independiente adecuada para la bomba y protección para carga inductiva.
- Mantener agua y conexiones eléctricas aisladas.

## Estado de validación

El firmware se generó como implementación propuesta y **no se ha probado en el prototipo físico**. La compilación y las lecturas deben verificarse en el hardware del equipo antes de marcar pruebas como exitosas.
