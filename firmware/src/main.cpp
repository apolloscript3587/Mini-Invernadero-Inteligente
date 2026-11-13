#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

// -----------------------------------------------------------------------------
// Pinout ESP32-WROOM-32 / DevKit V1
// -----------------------------------------------------------------------------
static constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_33; // botón a GND, pull-up externo
static constexpr uint8_t SOIL_PIN = 34;                // ADC1, solo entrada
static constexpr uint8_t SENSOR_POWER_PIN = 32;        // opcional: alimentar sensor
static constexpr uint8_t VALVE_PIN = 25;               // Gate de MOSFET, NO válvula directa
static constexpr uint8_t LED_PIN = 26;                 // LED externo opcional

// -----------------------------------------------------------------------------
// Seguridad: mantener en 0 hasta verificar hardware y driver.
// La válvula se mantiene inactiva durante la puesta en marcha.
// -----------------------------------------------------------------------------
#define VALVE_DRIVER_CONFIRMED 0
static constexpr uint8_t VALVE_ACTIVE_LEVEL = HIGH;
static constexpr uint8_t VALVE_INACTIVE_LEVEL = LOW;

// -----------------------------------------------------------------------------
// Red local temporal
// -----------------------------------------------------------------------------
static const char *AP_SSID = "MACETA_ESP32";
static const char *AP_PASSWORD = "CambiarEstaClave"; // mínimo 8 caracteres
static constexpr uint16_t TCP_PORT = 5000;
static constexpr uint32_t AP_WAIT_MS = 60000;
static constexpr uint32_t CLIENT_IDLE_MS = 120000;
static constexpr uint32_t TELEMETRY_MS = 1000;

// -----------------------------------------------------------------------------
// Muestreo y configuración inicial. Calibrar en el sustrato real.
// En muchos sensores capacitivos el valor RAW baja al aumentar la humedad.
// -----------------------------------------------------------------------------
static constexpr uint32_t SAMPLE_INTERVAL_SEC = 10 * 60;
static constexpr uint16_t SOIL_DRY_RAW = 3000; // REEMPLAZAR tras calibración
static constexpr uint16_t SOIL_WET_RAW = 1400; // REEMPLAZAR tras calibración
static constexpr uint8_t SOIL_SAMPLES = 20;

struct Config {
  float minHumidity = 30.0f;
  float highHumidity = 65.0f;
  float targetWaterMl = 100.0f;
  float estimatedFlowMlPerSec = 10.0f; // calibrar midiendo el caudal
  uint32_t intervalHours = 24;
  uint32_t maxValveOpenMs = 5000;
  uint32_t minIntervalBetweenWateringsSec = 6 * 60 * 60;
};

Preferences prefs;
WiFiServer server(TCP_PORT);
WiFiClient client;
Config cfg;

RTC_DATA_ATTR uint32_t rtcSecondsSinceWatering = 0;
RTC_DATA_ATTR bool rtcStateInitialized = false;

uint32_t sessionStartedMs = 0;
uint32_t lastClientActivityMs = 0;
uint32_t lastTelemetryMs = 0;
uint32_t lastWateringMs = 0;
String rxBuffer;
bool apSession = false;
bool watering = false;
bool valveState = false;
float lastHumidity = -1.0f;
uint16_t lastRaw = 0;
uint32_t lastWaterMl = 0;
String lastEvent = "boot";

// -----------------------------------------------------------------------------
// Persistencia
// -----------------------------------------------------------------------------
void loadConfig() {
  prefs.begin("planter", true);
  cfg.minHumidity = prefs.getFloat("minHum", cfg.minHumidity);
  cfg.highHumidity = prefs.getFloat("highHum", cfg.highHumidity);
  cfg.targetWaterMl = prefs.getFloat("waterMl", cfg.targetWaterMl);
  cfg.estimatedFlowMlPerSec = prefs.getFloat("flow", cfg.estimatedFlowMlPerSec);
  cfg.intervalHours = prefs.getUInt("intervalH", cfg.intervalHours);
  cfg.maxValveOpenMs = prefs.getUInt("maxValveMs", cfg.maxValveOpenMs);
  prefs.end();

  cfg.minHumidity = constrain(cfg.minHumidity, 1.0f, 95.0f);
  cfg.highHumidity = constrain(cfg.highHumidity, cfg.minHumidity + 1.0f, 99.0f);
  cfg.targetWaterMl = constrain(cfg.targetWaterMl, 1.0f, 1000.0f);
  cfg.estimatedFlowMlPerSec = constrain(cfg.estimatedFlowMlPerSec, 0.1f, 100.0f);
  cfg.intervalHours = constrain(cfg.intervalHours, 1U, 720U);
  cfg.maxValveOpenMs = constrain(cfg.maxValveOpenMs, 500U, 30000U);
}

void saveConfig() {
  prefs.begin("planter", false);
  prefs.putFloat("minHum", cfg.minHumidity);
  prefs.putFloat("highHum", cfg.highHumidity);
  prefs.putFloat("waterMl", cfg.targetWaterMl);
  prefs.putFloat("flow", cfg.estimatedFlowMlPerSec);
  prefs.putUInt("intervalH", cfg.intervalHours);
  prefs.putUInt("maxValveMs", cfg.maxValveOpenMs);
  prefs.end();
}

// -----------------------------------------------------------------------------
// Sensor
// -----------------------------------------------------------------------------
uint16_t readSoilRaw() {
  digitalWrite(SENSOR_POWER_PIN, HIGH);
  delay(150); // estabilización del sensor
  uint32_t sum = 0;
  for (uint8_t i = 0; i < SOIL_SAMPLES; ++i) {
    sum += analogRead(SOIL_PIN);
    delay(10);
  }
  digitalWrite(SENSOR_POWER_PIN, LOW);
  return static_cast<uint16_t>(sum / SOIL_SAMPLES);
}

float rawToHumidity(uint16_t raw) {
  if (SOIL_DRY_RAW == SOIL_WET_RAW) return -1.0f;
  const float pct = 100.0f *
    (static_cast<float>(SOIL_DRY_RAW) - raw) /
    (static_cast<float>(SOIL_DRY_RAW) - SOIL_WET_RAW);
  return constrain(pct, 0.0f, 100.0f);
}

float updateSoilReading() {
  lastRaw = readSoilRaw();
  lastHumidity = rawToHumidity(lastRaw);
  return lastHumidity;
}

// -----------------------------------------------------------------------------
// Válvula: salida segura por defecto. La implementación queda bloqueada hasta
// confirmar válvula DC, corriente, MOSFET, diodo y fuente.
// -----------------------------------------------------------------------------
void setValve(bool on) {
#if VALVE_DRIVER_CONFIRMED
  digitalWrite(VALVE_PIN, on ? VALVE_ACTIVE_LEVEL : VALVE_INACTIVE_LEVEL);
  valveState = on;
#else
  (void)on;
  digitalWrite(VALVE_PIN, VALVE_INACTIVE_LEVEL);
  valveState = false;
#endif
}

bool canWater(float humidity) {
  if (humidity < 0.0f) return false; // lectura inválida
  if (humidity >= cfg.highHumidity) return false;
  if (rtcSecondsSinceWatering < cfg.minIntervalBetweenWateringsSec) return false;
  return true;
}

bool waterForMl(float requestedMl, bool manual) {
  if (watering || !VALVE_DRIVER_CONFIRMED) {
    lastEvent = "valve_driver_not_confirmed";
    return false;
  }

  const float humidity = updateSoilReading();
  if (!canWater(humidity)) {
    lastEvent = "watering_blocked_by_soil_or_interval";
    return false;
  }

  if (manual && humidity >= cfg.highHumidity) {
    lastEvent = "manual_watering_blocked_soil_wet";
    return false;
  }

  requestedMl = constrain(requestedMl, 1.0f, cfg.targetWaterMl);
  uint32_t durationMs = static_cast<uint32_t>(
    (requestedMl / cfg.estimatedFlowMlPerSec) * 1000.0f
  );
  durationMs = constrain(durationMs, 100U, cfg.maxValveOpenMs);

  watering = true;
  setValve(true);
  const uint32_t started = millis();

  // Bucle acotado: siempre cierra por tiempo máximo.
  while (millis() - started < durationMs) {
    delay(10);
  }

  setValve(false);
  watering = false;

  rtcSecondsSinceWatering = 0;
  rtcStateInitialized = true;
  lastWateringMs = millis();
  lastWaterMl = static_cast<uint32_t>(
    cfg.estimatedFlowMlPerSec * durationMs / 1000.0f
  );
  lastEvent = manual ? "manual_watering_complete" : "scheduled_watering_complete";
  return true;
}

// -----------------------------------------------------------------------------
// JSON Lines
// -----------------------------------------------------------------------------
void sendJson(JsonDocument &doc) {
  if (!client || !client.connected()) return;
  serializeJson(doc, client);
  client.print('\n');
}

void sendAck(const char *event, bool ok, const char *message) {
  JsonDocument doc;
  doc["type"] = "ack";
  doc["event"] = event;
  doc["ok"] = ok;
  doc["message"] = message;
  sendJson(doc);
}

void sendTelemetry() {
  JsonDocument doc;
  doc["type"] = "telemetry";
  doc["humidity"] = lastHumidity;
  doc["soil_raw"] = lastRaw;
  doc["valve"] = valveState;
  doc["watering"] = watering;
  doc["last_water_ml_estimated"] = lastWaterMl;
  doc["next_watering_minutes"] = max(
    0L,
    static_cast<long>(cfg.intervalHours * 60UL) -
      static_cast<long>(rtcSecondsSinceWatering / 60UL)
  );
  doc["battery_voltage"] = nullptr; // pendiente divisor ADC y calibración
  doc["event"] = lastEvent;
  sendJson(doc);
}

void handleConfig(JsonDocument &doc) {
  if (doc["min_humidity"].is<float>())
    cfg.minHumidity = constrain(doc["min_humidity"].as<float>(), 1.0f, 95.0f);
  if (doc["high_humidity"].is<float>())
    cfg.highHumidity = constrain(doc["high_humidity"].as<float>(), cfg.minHumidity + 1.0f, 99.0f);
  if (doc["water_ml"].is<float>())
    cfg.targetWaterMl = constrain(doc["water_ml"].as<float>(), 1.0f, 1000.0f);
  if (doc["interval_hours"].is<uint32_t>())
    cfg.intervalHours = constrain(doc["interval_hours"].as<uint32_t>(), 1U, 720U);

  saveConfig();
  sendAck("config", true, "Configuracion guardada");
}

void processMessage(const String &line) {
  JsonDocument doc;
  const DeserializationError err = deserializeJson(doc, line);
  if (err) {
    sendAck("parse", false, "JSON invalido");
    return;
  }

  const char *type = doc["type"] | "";
  if (strcmp(type, "config") == 0) {
    handleConfig(doc);
  } else if (strcmp(type, "manual_watering") == 0) {
    const float ml = doc["water_ml"] | cfg.targetWaterMl;
    const bool ok = waterForMl(ml, true);
    sendAck("manual_watering", ok, ok ? "Riego terminado" : "Riego bloqueado o driver no habilitado");
  } else if (strcmp(type, "shutdown") == 0) {
    sendAck("shutdown", true, "Cerrando sesion");
    lastEvent = "remote_shutdown";
    apSession = false;
  } else if (strcmp(type, "ping") == 0) {
    sendAck("ping", true, "pong");
  } else {
    sendAck("unknown", false, "Tipo de mensaje desconocido");
  }
}

void serviceTcp() {
  if (!client || !client.connected()) {
    WiFiClient incoming = server.available();
    if (incoming) {
      client.stop();
      client = incoming;
      client.setNoDelay(true);
      lastClientActivityMs = millis();
      rxBuffer = "";
      lastEvent = "client_connected";
    }
  }

  if (client && client.connected()) {
    while (client.available()) {
      const char c = static_cast<char>(client.read());
      lastClientActivityMs = millis();
      if (c == '\n') {
        processMessage(rxBuffer);
        rxBuffer = "";
      } else if (c != '\r' && rxBuffer.length() < 1024) {
        rxBuffer += c;
      } else if (rxBuffer.length() >= 1024) {
        rxBuffer = "";
        sendAck("message", false, "Mensaje demasiado largo");
      }
    }

    if (millis() - lastTelemetryMs >= TELEMETRY_MS) {
      lastTelemetryMs = millis();
      sendTelemetry();
    }

    if (millis() - lastClientActivityMs > CLIENT_IDLE_MS) {
      lastEvent = "client_idle_timeout";
      client.stop();
      apSession = false;
    }
  }
}

// -----------------------------------------------------------------------------
// Wi-Fi session
// -----------------------------------------------------------------------------
void startApSession() {
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(true);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  server.begin();
  server.setNoDelay(true);
  sessionStartedMs = millis();
  lastTelemetryMs = 0;
  apSession = true;
  lastEvent = "ap_started";
}

void stopApSession() {
  if (client) client.stop();
  server.end();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  apSession = false;
}

// -----------------------------------------------------------------------------
// Control de riego autónomo
// -----------------------------------------------------------------------------
void runAutonomousCheck() {
  const float humidity = updateSoilReading();
  if (humidity < 0.0f) {
    lastEvent = "soil_read_error";
    return;
  }

  const bool intervalReached =
    rtcSecondsSinceWatering >= cfg.intervalHours * 3600UL;
  const bool soilVeryDry = humidity <= cfg.minHumidity;

  if ((intervalReached || soilVeryDry) && canWater(humidity)) {
    waterForMl(cfg.targetWaterMl, false);
  } else if (humidity >= cfg.highHumidity) {
    lastEvent = "soil_wet_skip";
  } else {
    lastEvent = "monitoring";
  }
}

// -----------------------------------------------------------------------------
// Deep Sleep: GPIO33 LOW (botón) o timer.
// El botón requiere pull-up externo y no debe quedarse presionado al dormir.
// -----------------------------------------------------------------------------
void enterDeepSleep() {
  setValve(false);
  stopApSession();
  digitalWrite(SENSOR_POWER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  rtc_gpio_init(BUTTON_PIN);
  rtc_gpio_set_direction(BUTTON_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en(BUTTON_PIN);
  rtc_gpio_pulldown_dis(BUTTON_PIN);

  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0);
  esp_sleep_enable_timer_wakeup(
    static_cast<uint64_t>(SAMPLE_INTERVAL_SEC) * 1000000ULL
  );

  esp_deep_sleep_start();
}

// -----------------------------------------------------------------------------
// Arduino
// -----------------------------------------------------------------------------
void setup() {
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, VALVE_INACTIVE_LEVEL);
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, LOW);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  analogReadResolution(12);
  analogSetPinAttenuation(SOIL_PIN, ADC_11db);

  Serial.begin(115200);
  delay(100);

  loadConfig();

  if (!rtcStateInitialized) {
    rtcSecondsSinceWatering = 0;
    rtcStateInitialized = true;
  }

  const esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0) {
    startApSession();
  } else {
    runAutonomousCheck();
    rtcSecondsSinceWatering += SAMPLE_INTERVAL_SEC;
    enterDeepSleep();
  }
}

void loop() {
  if (!apSession) {
    enterDeepSleep();
  }

  serviceTcp();

  if (millis() - sessionStartedMs > AP_WAIT_MS &&
      (!client || !client.connected())) {
    lastEvent = "connection_timeout";
    enterDeepSleep();
  }

  if (!apSession) {
    enterDeepSleep();
  }

  delay(5);
}
