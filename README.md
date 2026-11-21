# Mini-Invernadero Inteligente

Sistema de monitoreo para un mini-invernadero. La propuesta contempla medir tres variables del entorno y del cultivo: temperatura del aire, iluminación ambiental y humedad del suelo. Este repositorio organiza la documentación técnica y las evidencias del desarrollo.

> **Estado de la documentación:** el README original del proyecto identificaba los sensores DHT22, BH1750 (escrito originalmente como BHL1750) y un sensor capacitivo de humedad del suelo. No especificaba microcontrolador, actuador de riego, pines, alimentación, código ni resultados medidos. Esos datos se dejan señalados como pendientes de confirmar para no atribuir al prototipo componentes o pruebas que no estén verificadas.

## 1. Integrantes

- **Pendiente:** agregar los nombres completos de los integrantes y la contribución de cada uno.

## 2. Problema identificado

Las plantas de un invernadero requieren condiciones ambientales y humedad del sustrato adecuadas. Sin mediciones, el seguimiento de estas condiciones depende de la observación manual. El proyecto busca instrumentar un mini-invernadero para obtener información de temperatura, luz y humedad del suelo y facilitar su supervisión.

## 3. Objetivos

### Objetivo general
Desarrollar un mini-invernadero inteligente que permita monitorear variables ambientales y del suelo mediante sensores electrónicos.

### Objetivos específicos
- Medir la temperatura y humedad relativa del aire con un DHT22.
- Medir la iluminación ambiental con un sensor BH1750.
- Medir la humedad del sustrato con un sensor capacitivo.
- Integrar las lecturas en un microcontrolador y documentar su validación.
- Definir e implementar una estrategia de riego automatizado si el prototipo incorpora un actuador de riego.

## 4. Descripción del funcionamiento

La arquitectura prevista obtiene datos de tres sensores:
1. El DHT22 proporciona temperatura del aire y humedad relativa.
2. El BH1750 mide la iluminación ambiental.
3. El sensor capacitivo entrega una señal relacionada con la humedad del suelo.

Un microcontrolador (modelo por confirmar) deberá adquirir y procesar estas lecturas. La visualización, conectividad y automatización del riego no están descritas en la información disponible y deben documentarse cuando se confirme su implementación.

## 5. Componentes utilizados

| Componente | Cantidad prevista | Función |
|---|---:|---|
| DHT22 | 1 | Medición de temperatura y humedad relativa del aire |
| Sensor BH1750 | 1 | Medición de iluminación ambiental |
| Sensor capacitivo de humedad del suelo | 1 | Medición de humedad del sustrato |
| Microcontrolador | Por confirmar | Lectura y procesamiento de sensores |
| Actuador de riego (bomba/válvula y etapa de potencia) | Por confirmar | Riego, únicamente si está instalado |

Referencias de los sensores consignadas en el README anterior:
- [DHT22 – Mactrónica](https://www.mactronica.com.co/sensor-de-temperatura-y-humedad-dht22)
- [BH1750 – Mactrónica](https://www.mactronica.com.co/sensor-de-luz-bhl1750-con-coraza)
- [Sensor capacitivo de humedad del suelo – Mactrónica](https://www.mactronica.com.co/sensor-de-humedad-del-suelo-capacitivo)

## 6. Arquitectura del sistema

**Entradas:** DHT22, BH1750 y sensor capacitivo de humedad del suelo.  
**Procesamiento:** microcontrolador por identificar.  
**Salidas:** sistema de visualización y/o actuador de riego, por confirmar según el montaje real.

## 7. Diagrama de conexiones

No se ha incluido todavía un esquema eléctrico verificable. Agregarlo en `diagramas/` cuando se confirmen el modelo de placa, los pines, la tensión de alimentación y el cableado real.

| Módulo | Interfaz/pines | Alimentación |
|---|---|---|
| DHT22 | Por confirmar según placa y montaje | Consultar versión del módulo |
| BH1750 | Bus I²C: SDA/SCL; pines de placa por confirmar | Consultar módulo utilizado |
| Sensor capacitivo | Salida analógica; pin ADC por confirmar | Consultar módulo utilizado |
| Controlador de riego | Por confirmar; requiere etapa de potencia adecuada si aplica | Por confirmar |

No conectar una bomba directamente a un pin del microcontrolador. La etapa de potencia y la alimentación deben dimensionarse según el actuador real.

## 8. Desarrollo del software

La carpeta `codigo/` está destinada al código fuente original. Aún debe incorporarse el programa utilizado por el equipo. Al agregarlo, documentar:
- Placa y entorno de desarrollo.
- Librerías y versiones.
- Inicialización de sensores.
- Lectura y tratamiento de datos.
- Umbrales de control, si existen.
- Comunicación o visualización, si se implementó.

No se incluye código ficticio como si fuera el firmware probado del prototipo.

## 9. Instalación y puesta en funcionamiento

1. Identificar la placa y registrar su modelo exacto.
2. Instalar el entorno de desarrollo correspondiente.
3. Incorporar las librerías requeridas por el firmware real.
4. Revisar el diagrama y verificar alimentación, tierra y conexiones.
5. Cargar el programa original al microcontrolador.
6. Abrir el monitor o interfaz que utilice el proyecto y comprobar las lecturas.
7. Si existe riego automático, probarlo inicialmente con medidas de seguridad y supervisión, verificando el actuador y su etapa de potencia.

Los pasos específicos de compilación y carga se completarán cuando se agregue el código fuente.

## 10. Pruebas realizadas

**No se han aportado registros de pruebas ni mediciones verificables.** La siguiente tabla es un plan de validación, no una declaración de pruebas exitosas.

| Prueba | Objetivo / resultado esperado | Resultado obtenido | Estado |
|---|---|---|---|
| Lectura DHT22 | Obtener temperatura y humedad relativa del aire | Pendiente de registrar | No ejecutada/documentada |
| Lectura BH1750 | Obtener una medición de iluminación | Pendiente de registrar | No ejecutada/documentada |
| Lectura del sensor capacitivo | Obtener una señal que cambie con la humedad del sustrato | Pendiente de registrar | No ejecutada/documentada |
| Riego automático (si está instalado) | Activar el actuador bajo la condición definida por el equipo | Pendiente de confirmar | No verificable |

Para cada prueba agregar objetivo, procedimiento, resultado esperado, resultado real, fecha y evidencia (fotografía, captura o registro). Si una prueba falla, documentar el fallo y las acciones correctivas.

## 11. Evidencias fotográficas

Agregar al menos cinco fotografías reales en `evidencias/fotos/`:
1. Componentes electrónicos identificados.
2. Montaje y conexiones del circuito.
3. Proceso de construcción o ensamblaje.
4. Prototipo completo.
5. Prototipo durante una prueba real.

Insertar aquí las imágenes con una breve descripción cuando estén disponibles. No se agregan imágenes de referencia como si fueran fotografías del prototipo.

## 12. Video demostrativo

Pendiente de incorporar un video real de 3 a 5 minutos que presente el problema, los componentes, las conexiones, una demostración del prototipo y los resultados/dificultades. Guardarlo en `evidencias/videos/` o añadir un enlace accesible.

## 13. Dificultades y soluciones

Registrar al menos dos dificultades que hayan ocurrido realmente durante el desarrollo, indicando síntoma, causa identificada, solución intentada y resultado. No se han suministrado incidencias concretas; esta sección queda pendiente de completar por el equipo.

## 14. Conclusiones y estado actual

La información inicial define las variables que el proyecto busca medir y propone tres sensores: DHT22, BH1750 y sensor capacitivo de humedad del suelo. Para cerrar la documentación técnica faltan datos esenciales del montaje, el microcontrolador, el firmware, el diagrama eléctrico y las evidencias de validación. El estado funcional del prototipo debe establecerse a partir de pruebas reales del equipo.

## 15. Contribuciones y créditos

Cada integrante debe indicar su aporte. Reconocer las librerías, diseños y recursos de terceros que se utilicen. No publicar contraseñas Wi-Fi, tokens ni credenciales.

## Estructura del repositorio

```text
.
├── README.md
├── codigo/
├── diagramas/
├── evidencias/
│   ├── fotos/
│   └── videos/
├── pruebas/
└── documentacion/
```
