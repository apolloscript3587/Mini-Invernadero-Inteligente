# Hardware y cableado

## Plataforma

ESP32 DevKit V1 con módulo ESP32-WROOM-32 clásico.

## Sensores (solo dos)

| Elemento | GPIO | Nota |
|---|---:|---|
| Sensor capacitivo de humedad AO | 34 | ADC1, entrada solamente |
| Caudalímetro: salida de pulsos | 27 | Interrupción; adaptar nivel a 3,3 V |

## Actuador

| Elemento | GPIO | Nota |
|---|---:|---|
| Entrada de driver/relé de electroválvula | 26 | No conectar la bobina al GPIO |

La electroválvula debe ser normalmente cerrada y alimentarse con fuente externa adecuada. Para válvula DC, dimensionar MOSFET logic-level, cableado, fuente y protección flyback según tensión/corriente de la bobina. Para válvula AC se requiere una etapa aislada específica; no usar el esquema DC.

## Sensor capacitivo

- Confirmar tensión de alimentación del fabricante.
- AOUT nunca debe superar 3,3 V en GPIO34; adaptar nivel si corresponde.
- Calibrar ADC en sustrato seco y húmedo real; cargar los valores en el firmware.
- Evitar sumergir la electrónica y proteger conexiones de salpicaduras.

## Sensor de flujo

- Instalar después de la electroválvula, respetando la flecha de flujo.
- Confirmar que el modelo mida el caudal bajo disponible por gravedad.
- Calibrar pulsos por litro con un volumen recogido en el montaje definitivo.
- Adaptar toda salida de 5 V a 3,3 V antes de GPIO27.

## Electroválvula y tanque

- La válvula debe ser apta para la presión estática del tanque. Las válvulas piloto pueden requerir una presión mínima que un tanque bajo no proporciona.
- No alimentar la válvula desde el ESP32. Usar driver/relé y fuente externa.
- Verificar que la válvula quede cerrada al arrancar y al perder alimentación.
- Asegurar el tanque y evitar fugas sobre la electrónica.

## Aplicación de escritorio

La carpeta `godot/` conserva la interfaz de la rama feature. Su protocolo TCP/AP corresponde al firmware anterior y no está integrado aún con el firmware de riego por gravedad; no asumir que se conecta hasta completar esa integración.
