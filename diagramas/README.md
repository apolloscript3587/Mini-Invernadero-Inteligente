# Diagramas del sistema

La implementación de referencia utiliza ESP32 DevKit V1 / ESP32-WROOM-32 porque el material inicial identificaba los sensores, pero no la placa ni los pines. Confirmar el modelo del microcontrolador real antes de cablear.

- [conexiones.md](conexiones.md): tabla de pines, alimentación, tierra común y precauciones.
- [conexiones_esp32.svg](conexiones_esp32.svg): diagrama visual de cableado para la placa propuesta.
- [arquitectura.mmd](arquitectura.mmd): diagrama Mermaid editable de arquitectura e interconexión.

Los sensores contemplados son DHT22, BH1750 y sensor capacitivo de humedad del suelo. El relé y la bomba se representan como elementos opcionales porque no constan como componentes confirmados del prototipo original.

**Importante:** verificar tensión de alimentación y nivel de salida AO antes de conectar. Ninguna entrada GPIO del ESP32 debe recibir 5 V. La bomba requiere etapa de potencia y fuente externa; nunca debe conectarse directamente a un GPIO.
