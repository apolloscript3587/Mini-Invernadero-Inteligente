/*
 * Mini-Invernadero Inteligente
 * Plataforma de referencia: ESP32 DevKit V1 (ESP32-WROOM-32)
 * Sensores: DHT22, BH1750 y sensor capacitivo de humedad del suelo.
 *
 * Riego opcional mediante rele en GPIO 26.
 * DESACTIVADO por defecto: calibrar sensor y verificar circuito de potencia
 * antes de habilitarlo.
 *
 * Librerias: Adafruit DHT sensor library, Adafruit Unified Sensor, BH1750.
 */
#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>

// Pines propuestos para ESP32 DevKit V1 / ESP32-WROOM-32
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t SOIL_PIN = 34;   // ADC1, entrada solamente
constexpr uint8_t I2C_SDA_PIN = 21;
constexpr uint8_t I2C_SCL_PIN = 22;
constexpr uint8_t RELAY_PIN = 26;  // salida opcional para modulo de rele

#define DHT_TYPE DHT22

// Mantener false hasta verificar rele, bomba, fuente y calibracion.
constexpr bool ENABLE_IRRIGATION = false;
// Muchos modulos de rele son activos en LOW. Confirmar el modelo real.
constexpr bool RELAY_ACTIVE_LOW = true;

// CALIBRACION OBLIGATORIA: sustituir -1 por lecturas reales del ADC.
// SOIL_DRY_RAW: lectura en sustrato seco.
// SOIL_WET_RAW: lectura en sustrato humedo.
constexpr int SOIL_DRY_RAW = -1;
constexpr int SOIL_WET_RAW = -1;

// Umbrales iniciales propuestos; ajustar al cultivo y sustrato.
constexpr float IRRIGATION_START_BELOW_PERCENT = 35.0f;
constexpr float IRRIGATION_STOP_ABOVE_PERCENT = 45.0f;

// Protecciones basicas, ajustar tras validar la bomba real.
constexpr unsigned long MAX_PUMP_RUNTIME_MS = 8000UL;
constexpr unsigned long MIN_PUMP_OFF_TIME_MS = 60000UL;
constexpr unsigned long SENSOR_SAMPLE_INTERVAL_MS = 2500UL;
constexpr uint8_t SOIL_SAMPLES = 12;

DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightMeter;

bool lightSensorReady = false;
bool pumpIsOn = false;
bool pumpLockout = false;
unsigned long pumpStartedAt = 0;
unsigned long lastPumpStoppedAt = 0;
unsigned long lastSampleAt = 0;

bool soilCalibrationIsValid() {
  return SOIL_DRY_RAW >= 0 &&
         SOIL_WET_RAW >= 0 &&
         SOIL_DRY_RAW <= 4095 &&
         SOIL_WET_RAW <= 4095 &&
         SOIL_DRY_RAW != SOIL_WET_RAW;
}

int readSoilRawAverage() {
  uint32_t total = 0;
  analogRead(SOIL_PIN); // descartar primera lectura
  delay(5);
  for (uint8_t i = 0; i < SOIL_SAMPLES; i++) {
    total += analogRead(SOIL_PIN);
    delay(5);
  }
  return static_cast<int>(total / SOIL_SAMPLES);
}

float soilRawToPercent(int rawValue) {
  if (!soilCalibrationIsValid()) return NAN;
  // Admite sensores cuya lectura suba o baje al aumentar la humedad.
  const float percent =
      (static_cast<float>(rawValue - SOIL_DRY_RAW) * 100.0f) /
      static_cast<float>(SOIL_WET_RAW - SOIL_DRY_RAW);
  return constrain(percent, 0.0f, 100.0f);
}

void writeRelay(bool turnOn) {
  const uint8_t activeLevel = RELAY_ACTIVE_LOW ? LOW : HIGH;
  const uint8_t inactiveLevel = RELAY_ACTIVE_LOW ? HIGH : LOW;
  digitalWrite(RELAY_PIN, turnOn ? activeLevel : inactiveLevel);
}

void setPump(bool turnOn) {
  if (pumpIsOn == turnOn) return;
  pumpIsOn = turnOn;
  writeRelay(turnOn);
  if (turnOn) {
    pumpStartedAt = millis();
    Serial.println(F("# RIEGO: bomba ACTIVADA"));
  } else {
    lastPumpStoppedAt = millis();
    Serial.println(F("# RIEGO: bomba DESACTIVADA"));
  }
}

void updateIrrigation(float soilPercent) {
  // Ante una lectura no valida, detener y no iniciar el riego.
  if (!isfinite(soilPercent)) {
    setPump(false);
    return;
  }

  const unsigned long now = millis();
  if (pumpIsOn) {
    if (soilPercent >= IRRIGATION_STOP_ABOVE_PERCENT) {
      setPump(false);
      return;
    }
    if (now - pumpStartedAt >= MAX_PUMP_RUNTIME_MS) {
      setPump(false);
      pumpLockout = true;
      Serial.println(F("# SEGURIDAD: tiempo maximo de riego alcanzado"));
      return;
    }
  } else {
    // Rearmar solamente cuando la humedad vuelve al umbral alto.
    if (soilPercent >= IRRIGATION_STOP_ABOVE_PERCENT) {
      pumpLockout = false;
    }
    const bool cooldownFinished =
        (now - lastPumpStoppedAt) >= MIN_PUMP_OFF_TIME_MS;
    if (!pumpLockout &&
        cooldownFinished &&
        soilPercent <= IRRIGATION_START_BELOW_PERCENT) {
      setPump(true);
    }
  }
}

void printJsonFloat(float value, uint8_t decimals) {
  if (isfinite(value)) Serial.print(value, decimals);
  else Serial.print(F("null"));
}

void printTelemetry(float temperatureC,
                    float relativeHumidity,
                    float illuminanceLux,
                    int soilRaw,
                    float soilPercent) {
  Serial.print(F("{\"uptime_ms\":"));
  Serial.print(millis());
  Serial.print(F(",\"temperature_c\":"));
  printJsonFloat(temperatureC, 1);
  Serial.print(F(",\"air_humidity_pct\":"));
  printJsonFloat(relativeHumidity, 1);
  Serial.print(F(",\"light_lux\":"));
  printJsonFloat(illuminanceLux, 1);
  Serial.print(F(",\"soil_raw\":"));
  Serial.print(soilRaw);
  Serial.print(F(",\"soil_moisture_pct\":"));
  printJsonFloat(soilPercent, 1);
  Serial.print(F(",\"soil_calibrated\":"));
  Serial.print(soilCalibrationIsValid() ? F("true") : F("false"));
  Serial.print(F(",\"pump_on\":"));
  Serial.print(pumpIsOn ? F("true") : F("false"));
  Serial.println(F("}"));
}

void setup() {
  // Dejar el rele en reposo al inicializar el pin.
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(RELAY_PIN, OUTPUT);
  writeRelay(false);

  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("# Mini-Invernadero Inteligente - ESP32"));
  Serial.println(F("# Salida serie: JSON a 115200 baudios"));

  analogReadResolution(12); // ESP32 clasico: 0..4095
  analogSetPinAttenuation(SOIL_PIN, ADC_11db);

  dht.begin();
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lightSensorReady = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  if (lightSensorReady) {
    Serial.println(F("# BH1750 detectado en I2C"));
  } else {
    Serial.println(F("# ADVERTENCIA: BH1750 no detectado; revisar SDA/SCL y alimentacion"));
  }

  if (!soilCalibrationIsValid()) {
    Serial.println(F("# ADVERTENCIA: suelo sin calibrar; porcentaje=null y riego bloqueado"));
  }
  if (!ENABLE_IRRIGATION) {
    Serial.println(F("# RIEGO DESHABILITADO en la configuracion"));
  } else {
    Serial.println(F("# RIEGO HABILITADO: verificar rele, bomba y fuente"));
  }
  lastPumpStoppedAt = millis();
}

void loop() {
  const unsigned long now = millis();

  // Controlar limite de tiempo aun entre muestras.
  if (pumpIsOn && now - pumpStartedAt >= MAX_PUMP_RUNTIME_MS) {
    setPump(false);
    pumpLockout = true;
    Serial.println(F("# SEGURIDAD: tiempo maximo de riego alcanzado"));
  }

  if (now - lastSampleAt < SENSOR_SAMPLE_INTERVAL_MS) {
    delay(10);
    return;
  }
  lastSampleAt = now;

  const float temperatureC = dht.readTemperature();
  const float relativeHumidity = dht.readHumidity();

  float illuminanceLux = NAN;
  if (lightSensorReady) {
    illuminanceLux = lightMeter.readLightLevel();
    if (illuminanceLux < 0.0f) illuminanceLux = NAN;
  }

  const int soilRaw = readSoilRawAverage();
  const float soilPercent = soilRawToPercent(soilRaw);

  if (ENABLE_IRRIGATION) updateIrrigation(soilPercent);
  else setPump(false);

  // Cada linea es un objeto JSON, util para guardar registros en PC.
  printTelemetry(temperatureC, relativeHumidity, illuminanceLux,
                 soilRaw, soilPercent);
}
