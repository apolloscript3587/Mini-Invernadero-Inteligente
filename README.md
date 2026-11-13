# Mini-Invernadero Inteligente

Maceta inteligente autónoma basada en **ESP32-WROOM-32**, sensor capacitivo de humedad del suelo y electroválvula. La aplicación de escritorio está desarrollada en **Godot 4**.

## Objetivos de esta primera versión

- Medir la humedad del sustrato de forma periódica.
- Ejecutar el riego localmente, aunque la PC esté apagada.
- Despertar por temporizador o por botón físico.
- Crear un punto de acceso Wi-Fi temporal para conectar Godot.
- Intercambiar telemetría y configuración mediante TCP + JSON Lines.
- Guardar configuración en NVS mediante Preferences.
- Volver a Deep Sleep al terminar la sesión.

## Estructura

```text
.
├── firmware/
│   ├── platformio.ini
│   └── src/main.cpp
├── godot/
│   ├── project.godot
│   ├── scenes/main.tscn
│   └── scripts/main.gd
├── docs/
│   └── hardware.md
└── README.md
```

## Hardware previsto

| Elemento | ESP32 |
|---|---|
| Sensor capacitivo AOUT | GPIO34 (ADC1) |
| Alimentación conmutada del sensor (opcional) | GPIO32 |
| Botón de despertar | GPIO33 a GND, con pull-up externo |
| Gate del MOSFET de válvula | GPIO25 |
| LED de estado externo (opcional) | GPIO26 |

**Importante:** la válvula queda deshabilitada por defecto en el firmware. No habilitarla hasta verificar tensión, corriente, tipo DC/AC, driver, diodo de protección y alimentación. El ESP32 no debe alimentar directamente una electroválvula.

## Firmware (PlatformIO)

1. Instalar VS Code y la extensión PlatformIO.
2. Abrir la carpeta `firmware`.
3. Revisar y calibrar `SOIL_DRY_RAW` y `SOIL_WET_RAW` en `src/main.cpp`.
4. Compilar y cargar en ESP32 Dev Module.
5. Mantener `VALVE_DRIVER_CONFIRMED` en `0` mientras se prueba el sensor y la comunicación.

## Godot

1. Abrir Godot 4.
2. Importar la carpeta `godot`.
3. Ejecutar el proyecto.
4. Presionar el botón físico de la maceta para activar su AP.
5. Conectar manualmente la PC a la red `MACETA_ESP32` (la conexión automática del adaptador Wi-Fi del sistema operativo queda para una siguiente iteración).
6. Pulsar **Conectar** en la aplicación.

Datos iniciales de red (cámbialos antes de desplegar):
- SSID: `MACETA_ESP32`
- Contraseña: `CambiarEstaClave`
- IP: `192.168.4.1`
- TCP: `5000`

## Protocolo

Se utiliza JSON Lines: cada objeto JSON termina en un salto de línea.

- ESP32 → Godot: `telemetry`
- Godot → ESP32: `config`
- Godot → ESP32: `manual_watering`
- Godot → ESP32: `shutdown`

## Seguridad y limitaciones de esta base

- Los porcentajes de humedad requieren calibración en el sustrato real.
- El riego por tiempo solo estima volumen; hay que medir el caudal real con un recipiente graduado.
- El sensor de flujo no está implementado en esta versión.
- No se conoce aún la electroválvula ni la batería; el diseño de potencia no está cerrado.
- El tiempo se mantiene como tiempo transcurrido, no como fecha/hora de calendario. Un RTC externo será necesario si se requiere hora real después de perder alimentación.
- El riego manual y automático están sujetos a límites, pero deben validarse con la válvula desconectada antes de cualquier prueba hidráulica.
- Deep Sleep no garantiza bajo consumo en una placa DevKit completa; regulador, USB-UART y LED de alimentación también consumen.

Consulta [docs/hardware.md](docs/hardware.md) antes de conectar actuadores.
