# Hardware y cableado

## Placa objetivo

ESP32 DevKit V1 con módulo ESP32-WROOM-32 clásico.

## Mapa de pines

| Señal | GPIO | Nota |
|---|---:|---|
| Sensor capacitivo AOUT | 34 | ADC1, entrada solamente |
| Sensor VCC conmutado (si el módulo admite 3.3 V) | 32 | Confirmar consumo y tensión del sensor |
| Botón wake | 33 | Botón entre GPIO33 y GND; pull-up externo de 10 kΩ a 3.3 V |
| MOSFET gate | 25 | Solo después de validar driver y válvula |
| LED externo | 26 | Añadir resistencia limitadora |

## Sensor capacitivo

- Verificar la tensión de alimentación indicada por el fabricante.
- Verificar con multímetro que AOUT nunca supere 3.3 V.
- Si el sensor se alimenta a 5 V y su salida puede superar 3.3 V, usar divisor o adaptación de nivel.
- Calibrar RAW en el sustrato real, tanto seco como en el nivel de humedad deseado.
- La alimentación conmutada por GPIO32 solo es válida si el consumo del módulo está dentro de la capacidad del pin; si no, usar un transistor/MOSFET de alimentación.

## Botón de despertar

- GPIO33 ↔ botón ↔ GND.
- Resistencia externa de 10 kΩ entre GPIO33 y 3.3 V.
- GPIO33 admite despertar EXT0 en ESP32 clásico.
- No entrar en Deep Sleep con el botón presionado; puede provocar despertares inmediatos repetidos.

## Electroválvula

**No conectar la válvula directamente al ESP32.** Antes de activar el driver se debe conocer:

1. AC o DC.
2. Tensión nominal.
3. Corriente nominal y de arranque.
4. Tipo normalmente cerrada (preferido) o normalmente abierta.
5. Si la válvula admite el caudal y presión disponibles.

Para una válvula DC normalmente cerrada, el esquema típico es un MOSFET N-channel logic-level en low-side:
- Positivo de la fuente de válvula → positivo de la bobina.
- Negativo de la bobina → Drain.
- Source → GND común.
- GPIO25 → Gate mediante resistencia serie.
- Gate → GND mediante resistencia pull-down.
- Diodo flyback en paralelo con la bobina: cátodo al positivo, ánodo al Drain.
- Fuente, MOSFET, pistas y conectores dimensionados para la corriente de la bobina.

Si la válvula es AC, este esquema no aplica: usar un módulo aislado apropiado y diseño específico para tensión de red. No manipular tensión de red sin experiencia y protecciones adecuadas.

El firmware mantiene `VALVE_DRIVER_CONFIRMED=0` por seguridad. No cambiarlo a 1 hasta verificar el circuito y probarlo con la válvula desconectada y una carga de prueba apropiada.

## Batería

No seleccionar cargador, regulador ni batería hasta conocer la tensión/corriente de la válvula. Considerar:
- Corriente pico de la válvula.
- Corriente del ESP32 durante transmisión Wi-Fi.
- Regulador con margen de corriente y baja corriente quiescente.
- Protección de batería (sobrecarga, sobredescarga y cortocircuito).
- Medición de batería mediante divisor ADC calculado para el voltaje máximo.
- El DevKit V1 puede consumir bastante en Deep Sleep por su regulador, LED y USB-UART.

## Medición del volumen

La versión inicial estima volumen por tiempo:
`duración_ms = volumen_ml / caudal_ml_s * 1000`.

Calibrar con recipiente graduado y la misma altura, manguera y presión del montaje final. Un caudalímetro solo se añadirá tras verificar que funciona con caudales bajos. Instalar además un límite de tiempo máximo independiente del cálculo de volumen.
