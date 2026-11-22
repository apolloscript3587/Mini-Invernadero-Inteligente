# Calibración y pruebas

## Humedad del suelo
1. Leer ADC en sustrato seco y registrar SOIL_DRY_RAW.
2. Humedecer el sustrato, esperar estabilización y registrar SOIL_WET_RAW.
3. Repetir y verificar que el porcentaje calculado sea coherente.

## Caudalímetro
1. Instalar después de la electroválvula y respetar la flecha.
2. Usar tanque a la altura real.
3. Recoger un volumen conocido y contar pulsos.
4. Calcular PULSES_PER_LITER = pulsos / litros. Repetir al menos tres veces.
5. Comparar volumen calculado con volumen recogido y verificar caudal mínimo.

## Ajuste
Los umbrales 35 % y 45 % son ejemplos, no valores universales. Ajustar tiempos, límites y humedad a la planta, sustrato y caudal real.

## Pruebas por documentar
| ID | Prueba | Resultado esperado |
|---|---|---|
| P01 | Suelo seco/húmedo | Lectura estable y calibración coherente |
| P02 | Caudalímetro con volumen conocido | Volumen calculado próximo al recogido |
| P03 | Electroválvula | Cerrada al arranque; abre/cierra bajo control |
| P04 | Flujo por gravedad | Pulsos detectados en altura definitiva |
| P05 | Algoritmo | Inicia bajo umbral, pausa y finaliza al objetivo |
| P06 | Fallo de flujo/límite | Cierra válvula y entra en bloqueo |

Registrar fecha, procedimiento, resultado esperado/observado y evidencia. No declarar pruebas exitosas sin realizarlas.
