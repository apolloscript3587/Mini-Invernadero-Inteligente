# Firmware ESP32 — riego por gravedad

## Sensores y pines
- Humedad capacitiva del suelo: AO a GPIO34 (ADC1).
- Caudalímetro de pulsos: señal a GPIO27.
- Driver/relé de electroválvula: GPIO26.

Plataforma de referencia: ESP32 DevKit V1 / ESP32-WROOM-32. Sin librerías externas.

## Calibración
Configurar SOIL_DRY_RAW y SOIL_WET_RAW con lecturas ADC reales de sustrato seco y húmedo. Configurar PULSES_PER_LITER recogiendo volumen conocido y dividiendo pulsos entre litros. Repetir con la altura y tubería definitivas.

Umbrales 35 % y 45 %, pulso 5 s y pausa 30 s son valores de ejemplo; ajustar a planta, sustrato y caudal.

## Seguridad
Electroválvula normalmente cerrada apta para baja presión, con driver y fuente externa. No alimentar la válvula desde el ESP32. Adaptar señal de caudalímetro de 5 V a 3,3 V. Verificar caudal mínimo del sensor.
