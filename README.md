# Mini-Invernadero Inteligente

Sistema de maceta inteligente con **riego automático por gravedad**, basado en ESP32-WROOM-32 / DevKit V1 y dos sensores: humedad del suelo y flujo de agua. El agua se almacena en un mini tanque elevado; una electroválvula normalmente cerrada controla el paso y el caudalímetro, instalado después de ella, mide el agua suministrada.

## Objetivo y funcionamiento

El ESP32 convierte la lectura del sensor capacitivo a porcentaje mediante calibración. Cuando la humedad cae por debajo del umbral inferior, abre la electroválvula durante un pulso de tiempo limitado. El caudalímetro registra pulsos, caudal y volumen. Después se cierra la válvula para permitir infiltración; transcurrida la pausa se vuelve a medir el suelo. Se repiten pulsos hasta alcanzar la humedad objetivo o un límite de seguridad.

**La decisión de riego depende de la humedad del suelo.** El sensor de flujo confirma y cuantifica el agua suministrada; el tiempo de apertura y el volumen máximo son límites adicionales.

## Hardware principal

| Elemento | Cantidad | Función |
|---|---:|---|
| Sensor capacitivo de humedad del suelo | 1 | Variable de decisión del riego |
| Sensor de flujo por pulsos | 1 | Caudal y volumen suministrado |
| ESP32 DevKit V1 / ESP32-WROOM-32 | 1 | Control |
| Electroválvula normalmente cerrada | 1 | Control del agua |
| Mini tanque elevado | 1 | Depósito y presión por gravedad |
| Driver/relé y fuente externa | 1 | Conmutación/alimentación de válvula |

No se utilizan DHT22, BH1750, temperatura, humedad ambiental, iluminación ni otros sensores.

## Estructura

- `codigo/mini_invernadero_esp32.ino`: firmware de referencia para Arduino IDE.
- `firmware/src/main.cpp` y `firmware/platformio.ini`: entrada de firmware para PlatformIO.
- `docs/hardware.md`: seguridad eléctrica, selección de válvula y hardware.
- `godot/`: aplicación de escritorio Godot 4, conservada de la rama feature como interfaz de configuración/monitorización en desarrollo.
- `diagramas/`: conexiones y arquitectura del nuevo sistema.
- `documentacion/` y `pruebas/`: calibración y validación.

## Conexiones de referencia

| Módulo | ESP32 |
|---|---|
| Humedad del suelo AO | GPIO34 (ADC1) |
| Caudalímetro señal | GPIO27 (interrupción) |
| Driver de electroválvula IN | GPIO26 |

Hidráulica: tanque elevado → electroválvula NC → sensor de flujo → manguera/goteros → planta. Consultar `diagramas/conexiones.md`.

## Algoritmo y valores iniciales

- Iniciar cuando humedad ≤ 35 %.
- Objetivo de parada: humedad ≥ 45 %.
- Pulso de apertura: 5 s; pausa de infiltración: 30 s.
- Límites iniciales: 30 s acumulados de válvula abierta, 0,5 L y 5 pulsos de riego.

Son valores de demostración, no universales. Ajustar a especie, sustrato, maceta y caudal real.

## Firmware

Arduino IDE: abrir `codigo/mini_invernadero_esp32.ino`. PlatformIO: abrir la carpeta `firmware`. Monitor serie a 115200 baudios; la telemetría JSON informa humedad, pulsos, caudal, volumen y estado de válvula.

Antes de operar, calibrar `SOIL_DRY_RAW` y `SOIL_WET_RAW` con el sustrato real, y `PULSES_PER_LITER` recogiendo un volumen conocido. Los valores de fábrica no sustituyen la calibración en el montaje por gravedad.

## Aplicación Godot (feature conservada)

Se conserva `godot/` por su interfaz de escritorio, controles de configuración y visualización. **La comunicación TCP/AP que espera la aplicación pertenece al firmware anterior y todavía no está integrada en el firmware nuevo de riego por gravedad**. Por tanto, la interfaz es una base útil, pero no debe considerarse conectada/operativa hasta implementar y probar ese protocolo en el ESP32.

## Seguridad y limitaciones

- Usar electroválvula normalmente cerrada apta para la presión estática disponible. Algunas válvulas piloto requieren presión mínima; preferir acción directa si el tanque tiene poca altura.
- Verificar el caudal mínimo del caudalímetro. Un sensor de turbina puede no registrar caudales bajos y generar pérdida de carga.
- No introducir 5 V en GPIO del ESP32: adaptar la señal del caudalímetro a lógica de 3,3 V.
- La válvula requiere driver y fuente externa; nunca alimentarla desde un GPIO.
- Proteger la electrónica del agua e instalar protección de transitorios para la carga inductiva.
- El firmware debe compilarse y probarse con el hardware real antes de activar la válvula. Las pruebas físicas deben documentarse con evidencias.
