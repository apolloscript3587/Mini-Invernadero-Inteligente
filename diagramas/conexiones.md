# Diagrama de conexiones — ESP32 DevKit V1

## Alcance y supuesto

El repositorio original identifica DHT22, BH1750 y sensor capacitivo de humedad del suelo, pero no indica microcontrolador ni pines. Este esquema propone **ESP32 DevKit V1 / ESP32-WROOM-32** como plataforma de referencia. Confirmar contra la placa y módulos físicos antes de cablear.

## Tabla de cableado

| Módulo | Pin del módulo | Conectar a ESP32 | Nota |
|---|---|---|---|
| DHT22 | VCC | 3V3 | Según tensión admitida por el módulo |
| DHT22 | GND | GND | Tierra común |
| DHT22 | DATA | GPIO 4 | DHT22 desnudo: pull-up 4,7–10 kΩ a 3V3; breakout puede incluirla |
| BH1750 | VCC | 3V3 | Confirmar tensión admitida por breakout |
| BH1750 | GND | GND | Tierra común |
| BH1750 | SDA | GPIO 21 | I²C SDA |
| BH1750 | SCL | GPIO 22 | I²C SCL |
| Sensor capacitivo | VCC | 3V3 si lo admite | Evitar que AO supere el límite del ESP32 |
| Sensor capacitivo | GND | GND | Tierra común |
| Sensor capacitivo | AO | GPIO 34 | Entrada ADC1; AO nunca debe superar 3,3 V |
| Módulo de relé (opcional) | IN | GPIO 26 | Confirmar lógica de 3,3 V y polaridad activa |
| Módulo de relé (opcional) | VCC/GND | Según módulo | Alimentación según fabricante; GND común en control |

## Bomba de riego opcional

La bomba no está confirmada como componente instalado en la información original. Si se incorpora, usar los contactos COM/NO del relé (o un driver MOSFET adecuado) y una fuente externa del voltaje/corriente nominal de la bomba. No conectar la bomba a un GPIO ni al pin 3V3. Añadir protección de transitorios adecuada para carga inductiva DC. Mantener el agua separada y protegida de la electrónica.

## Diagrama lógico

~~~mermaid
flowchart LR
  DHT[DHT22: temperatura y HR] -->|GPIO 4| ESP[ESP32 DevKit]
  LUX[BH1750: iluminacion lux] -->|I2C SDA 21 / SCL 22| ESP
  SOIL[Sensor capacitivo: humedad del suelo] -->|AO / ADC GPIO 34| ESP
  ESP -->|GPIO 26 opcional| RELAY[Modulo de rele]
  RELAY --> PUMP[Bomba + fuente externa]
  ESP --> SERIAL[Monitor serie: JSON 115200]
~~~

## Alimentación y precauciones

- Todas las tierras de sensores comparten GND con el ESP32.
- No aplicar 5 V a GPIO4, GPIO21, GPIO22, GPIO34 ni otros GPIO.
- Para el sensor capacitivo, medir AO con multímetro; debe permanecer dentro del límite eléctrico de la placa. La atenuación ADC no protege contra sobretensión.
- En el DHT22 desnudo, instalar pull-up DATA–3V3; comprobar si el breakout ya trae resistencia.
- Conectar el relé sólo después de identificar si es activo en LOW o HIGH.
- El firmware mantiene `ENABLE_IRRIGATION = false` hasta validar circuito y calibrar el sensor.

## Referencias técnicas

- [ADC Arduino-ESP32 — Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html)
- [DHT sensor library — Adafruit](https://github.com/adafruit/DHT-sensor-library)
- [BH1750 — referencia de producto aportada en el README original](https://www.mactronica.com.co/sensor-de-luz-bhl1750-con-coraza)
