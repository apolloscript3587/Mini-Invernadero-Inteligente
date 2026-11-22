# Firmware ESP32 — riego por gravedad

## Sensores y pines

- Humedad capacitiva del suelo: AO a GPIO34 (ADC1).
- Caudalímetro de pulsos: señal a GPIO27.
- Driver/relé de electroválvula: control desde GPIO26.

Plataforma de referencia ESP32 DevKit V1 / ESP32-WROOM-32; confirmar placa real. No se requieren librerías externas.

## Calibración obligatoria

En el sketch configurar SOIL_DRY_RAW y SOIL_WET_RAW con lecturas ADC reales en sustrato seco y húmedo. Configurar PULSES_PER_LITER recogiendo un volumen conocido y dividiendo pulsos contados entre litros recogidos. Repetir en la altura y tubería definitivas: el caudal por gravedad puede ser bajo.

El algoritmo inicia por debajo del umbral inferior, abre la válvula por pulsos de tiempo limitado, mide caudal/volumen, cierra para infiltración y vuelve a leer humedad. Ajustar umbrales y límites a la especie y sustrato. Monitor serie a 115200 baudios.

## Seguridad

Electroválvula normalmente cerrada, apta para baja presión y accionada por driver/fuente externa. No alimentar la válvula desde el ESP32. Adaptar cualquier señal de caudalímetro de 5 V a lógica de 3,3 V. Proteger la electrónica del agua y la carga inductiva.
