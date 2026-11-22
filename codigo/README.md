# Firmware

El firmware principal integrado con la interfaz Godot está en `firmware/src/main.cpp` y se compila con PlatformIO desde la carpeta `firmware/`. Usa ArduinoJson 7 y crea un AP Wi-Fi `MACETA_ESP32` (contraseña inicial `riego1234`) con servidor TCP en `192.168.4.1:5000`.

El archivo `codigo/mini_invernadero_esp32.ino` es una variante serial de referencia; no implementa el protocolo de red de Godot.

Calibrar humedad y pulsos por litro antes de operar. No activar la válvula hasta validar la salida del driver y el caudal por gravedad.
