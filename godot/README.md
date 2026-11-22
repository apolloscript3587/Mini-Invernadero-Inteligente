# Aplicación Godot 4 — Mini-Invernadero

Interfaz de escritorio para el firmware ESP32 del mini-invernadero.

## Conexión

1. Enciende el ESP32 con el firmware de `firmware/src/main.cpp`.
2. Conecta el PC a la red Wi-Fi `MACETA_ESP32`.
3. Contraseña inicial: `riego1234` (configurable en el firmware).
4. Abre este proyecto con Godot 4 y ejecuta la escena principal.
5. Pulsa **Conectar**.

El ESP32 actúa como punto de acceso en `192.168.4.1` y escucha TCP en el puerto `5000`.

## Datos visibles

- Humedad del suelo (%) y lectura ADC.
- Caudal instantáneo (L/min).
- Volumen suministrado en el ciclo (L).
- Estado de la electroválvula, estado del algoritmo y evento actual.

## Parámetros configurables

Umbral de humedad para iniciar, humedad objetivo, duración de pulso, pausa de infiltración, tiempo máximo acumulado de válvula y volumen máximo del ciclo. El ESP32 valida los rangos y responde con ACK. Los valores se guardan en RAM y se restablecen al reiniciar.

## Protocolo

Mensajes JSON por línea (UTF-8 + LF). La interfaz envía `ping`, `get_status` y `config`; recibe `telemetry` y `ack`. No existe un comando de riego manual ni apagado remoto.

## Diagnóstico

- Si no conecta, comprueba que el PC esté asociado al AP del ESP32 y que no haya otro cliente TCP conectado (el firmware admite un cliente a la vez).
- Si la humedad aparece como “Sin calibrar”, establece `SOIL_DRY_RAW` y `SOIL_WET_RAW` en `firmware/src/main.cpp`.
- Si hay bloqueo por ausencia de flujo, comprueba la dirección de montaje, el caudal mínimo y la calibración del sensor.
