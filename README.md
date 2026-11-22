# Mini-Invernadero Inteligente — riego por gravedad

Proyecto de riego automático con **exactamente dos sensores**: sensor capacitivo de humedad del suelo y sensor de flujo de agua. El agua procede de un mini tanque elevado; una electroválvula normalmente cerrada controla el paso y el caudalímetro, instalado después de ella, mide el agua suministrada.

## Objetivo y funcionamiento

El ESP32 convierte la lectura del sensor de suelo a porcentaje mediante calibración. Cuando la humedad cae por debajo del umbral mínimo, abre la electroválvula durante un pulso de tiempo limitado. El caudalímetro registra pulsos, caudal y volumen. Después se cierra la válvula para permitir infiltración; transcurrida la pausa se vuelve a medir el suelo. Se repiten pulsos hasta alcanzar la humedad objetivo o un límite de seguridad.

El sensor de flujo mide el agua suministrada; **la decisión de riego depende de la humedad del suelo**.

## Componentes

| Elemento | Cantidad | Función |
|---|---:|---|
| Sensor capacitivo de humedad del suelo | 1 | Variable de decisión del riego |
| Sensor de flujo por pulsos | 1 | Caudal y volumen suministrado |
| ESP32 DevKit V1 / ESP32-WROOM-32 | 1 | Control y procesamiento; confirmar placa real |
| Electroválvula normalmente cerrada | 1 | Control del agua |
| Mini tanque elevado | 1 | Depósito; presión por gravedad |
| Driver/relé y fuente externa | 1 | Alimentación/conmutación de la válvula |

No se utilizan DHT22, BH1750, temperatura, humedad ambiental, iluminación ni otros sensores.

## Umbrales de ejemplo

- Iniciar cuando humedad ≤ 35 %.
- Objetivo de parada: humedad ≥ 45 %.
- Pulso de apertura: 5 s; pausa de infiltración: 30 s.
- Límites iniciales: 30 s acumulados de válvula abierta, 0,5 L y 5 pulsos de riego.

Son valores de demostración: ajustar al cultivo, sustrato, maceta y caudal real.

## Conexiones propuestas

| Módulo | ESP32 |
|---|---|
| Humedad del suelo AO | GPIO34 (ADC1) |
| Caudalímetro señal | GPIO27 (interrupción) |
| Driver de electroválvula IN | GPIO26 |

Hidráulica: tanque → electroválvula NC → sensor de flujo → manguera/goteros → planta. Ver diagramas/conexiones.md.

## Firmware y calibración

Firmware en codigo/mini_invernadero_esp32.ino. Monitor serie a 115200 baudios; telemetría JSON con humedad, pulsos, caudal, volumen y estado de válvula.

Antes de operar, calibrar SOIL_DRY_RAW y SOIL_WET_RAW con el sustrato real, y PULSES_PER_LITER recogiendo un volumen conocido. Los valores típicos del fabricante no sustituyen la calibración del caudalímetro en el montaje por gravedad.

## Seguridad

Usar electroválvula normalmente cerrada apta para la presión estática disponible; algunas válvulas piloto necesitan presión mínima y no funcionan con un tanque bajo. Verificar el caudal mínimo del sensor de flujo. No introducir 5 V en GPIO del ESP32: adaptar la señal si el caudalímetro entrega 5 V. La válvula necesita driver y fuente externa, nunca un GPIO. Proteger electrónica de salpicaduras y usar protección de transitorios para carga inductiva.

Las pruebas físicas deben registrarse con evidencias; no declarar pruebas exitosas sin haberlas realizado.