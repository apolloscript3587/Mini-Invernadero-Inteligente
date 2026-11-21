# Calibración y protocolo de validación

## 1. Calibración de humedad del suelo

1. Montar el sensor capacitivo según la tensión recomendada por su fabricante.
2. Conectar AO a GPIO34 y abrir el monitor serie a 115200 baudios.
3. Mantener la sonda en aire o sustrato seco de referencia y anotar el valor estable de `soil_raw`.
4. Humedecer uniformemente el sustrato, esperar a que se estabilice y anotar `soil_raw`.
5. Copiar los valores medidos a `SOIL_DRY_RAW` y `SOIL_WET_RAW` en el firmware.
6. Comprobar que `soil_moisture_pct` se aproxima a 0 % en el extremo seco y 100 % en el húmedo.
7. Repetir varias veces y ajustar umbrales al cultivo y al sustrato.

No sumergir la placa electrónica ni asumir que todos los sensores entregan valores en el mismo sentido. Si las lecturas secas/húmedas son iguales, erráticas o saturadas, revisar alimentación, GND, cableado y tensión AO.

## 2. Activación del riego opcional

El programa mantiene el riego deshabilitado inicialmente. Antes de habilitarlo:
- Confirmar físicamente que existe bomba, módulo de potencia y fuente externa adecuados.
- Confirmar la polaridad de activación del relé.
- Probar la salida sin bomba conectada y verificar que el estado de reposo sea apagado.
- Calibrar el sensor y comprobar el porcentaje.
- Ajustar umbral de inicio, umbral de parada, tiempo máximo de bomba y tiempo mínimo entre ciclos.
- Cambiar `ENABLE_IRRIGATION` a `true` sólo después de esas verificaciones y bajo supervisión.

El control tiene histéresis: inicia por debajo del umbral bajo y detiene al alcanzar el umbral alto. Incluye tiempo máximo de activación y bloqueo hasta recuperar humedad para reducir ciclos repetidos.

## 3. Pruebas que debe registrar el equipo

| ID | Prueba | Procedimiento | Evidencia a guardar |
|---|---|---|---|
| P01 | DHT22 | Comparar temperatura y humedad relativa del serial con instrumento de referencia, si está disponible | Captura del monitor serie y condiciones |
| P02 | BH1750 | Exponer el sensor a distintas condiciones de iluminación y observar el cambio en lux | Capturas de lecturas y descripción |
| P03 | Sensor de suelo | Comparar lecturas en sustrato seco y húmedo y verificar calibración | Registro ADC y porcentaje |
| P04 opcional | Salida de riego | Con bomba aislada, verificar cambio de salida/relé; luego prueba controlada con fuente adecuada | Video/foto y registro de estado |

Para cada prueba escribir fecha, objetivo, resultado esperado, resultado observado, estado (exitosa/parcial/fallida) y nombre del archivo de evidencia. Esta tabla es un protocolo, no resultados ya obtenidos.

## 4. Formato de registro sugerido

~~~csv
fecha,hora,prueba,valor,unidad,condicion,resultado,evidencia
~~~
