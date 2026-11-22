# Calibración y pruebas

## Humedad del suelo

1. Leer el ADC en sustrato seco y registrar SOIL_DRY_RAW.
2. Humedecer el sustrato, esperar estabilización y registrar SOIL_WET_RAW.
3. Repetir varias veces y comprobar que el porcentaje calculado sea coherente. No copiar valores de otro sensor/sustrato.

## Caudalímetro

1. Montarlo después de la electroválvula, respetando la flecha.
2. Usar el tanque a la altura real del prototipo.
3. Recoger un volumen conocido y contar pulsos.
4. Calcular PULSES_PER_LITER = pulsos / litros. Repetir al menos tres veces.
5. Comparar volumen calculado con volumen recogido y verificar el caudal mínimo.

## Ajuste del algoritmo

Los umbrales 35 % y 45 % son ejemplos, no valores universales. Ajustar SOIL_START_PERCENT, SOIL_TARGET_PERCENT, WATER_BURST_MS, SOAK_TIME_MS, MAX_TOTAL_WATERING_MS y MAX_WATER_VOLUME_L al cultivo y al caudal real.

## Pruebas por documentar

| ID | Prueba | Resultado esperado |
|---|---|---|
| P01 | Suelo seco/húmedo | Lectura estable y calibración coherente |
| P02 | Caudalímetro con volumen conocido | Volumen medido próximo al recogido |
| P03 | Electroválvula | Cerrada al arranque; abre/cierra bajo control |
| P04 | Flujo por gravedad | Pulsos detectados en la altura definitiva |
| P05 | Algoritmo | Inicia bajo umbral, pausa infiltración y finaliza al objetivo |
| P06 | Fallo de flujo/límite | Cierra válvula y entra en bloqueo |

Registrar fecha, procedimiento, resultado esperado/observado y evidencia. No declarar pruebas exitosas sin realizarlas.
