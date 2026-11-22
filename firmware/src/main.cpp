/* Mini-invernadero: ESP32, humedad de suelo + caudalimetro, riego por gravedad */
#include <Arduino.h>
constexpr uint8_t SOIL_PIN=34, FLOW_PIN=27, VALVE_PIN=26;
constexpr bool VALVE_ACTIVE_LOW=true;
// Sustituir por calibraciones medidas en el hardware real.
constexpr int SOIL_DRY_RAW=-1, SOIL_WET_RAW=-1;
// Calibrar recogiendo un volumen conocido; 450 es solo referencia inicial.
constexpr float PULSES_PER_LITER=450.0f;
constexpr float SOIL_START_PERCENT=35.0f, SOIL_TARGET_PERCENT=45.0f;
constexpr unsigned long SAMPLE_MS=2500, BURST_MS=5000, SOAK_MS=30000;
constexpr unsigned long FLOW_TIMEOUT_MS=8000, MAX_TOTAL_MS=30000;
constexpr float MAX_VOLUME_L=0.50f;
constexpr uint8_t MAX_CYCLES=5, SOIL_SAMPLES=12;
volatile uint32_t flowPulses=0;
enum State { IDLE, WATERING, SOAKING, LOCKOUT };
State state=IDLE;
unsigned long lastSample=0, stateAt=0, lastPulseAt=0, totalOpenMs=0;
uint32_t cycleStartPulses=0, irrigationStartPulses=0, lastSeenPulseCount=0;
uint8_t cycles=0;
float soilPct=NAN, liters=0, flowLMin=0;
void IRAM_ATTR flowISR(){ flowPulses++; }
uint32_t pulseCount(){ noInterrupts(); uint32_t n=flowPulses; interrupts(); return n; }
bool calibrated(){ return SOIL_DRY_RAW>=0 && SOIL_DRY_RAW<=4095 && SOIL_WET_RAW>=0 && SOIL_WET_RAW<=4095 && SOIL_DRY_RAW!=SOIL_WET_RAW; }
int readSoil(){ uint32_t s=0; analogRead(SOIL_PIN); for(uint8_t i=0;i<SOIL_SAMPLES;i++){s+=analogRead(SOIL_PIN);delay(5);} return s/SOIL_SAMPLES; }
float soilPercent(int raw){ if(!calibrated()) return NAN; float p=100.0f*(raw-SOIL_DRY_RAW)/(SOIL_WET_RAW-SOIL_DRY_RAW); return constrain(p,0.0f,100.0f); }
void valve(bool open){ uint8_t on=VALVE_ACTIVE_LOW?LOW:HIGH, off=VALVE_ACTIVE_LOW?HIGH:LOW; digitalWrite(VALVE_PIN,open?on:off); }
void lockout(const char* why){ valve(false); state=LOCKOUT; stateAt=millis(); Serial.print(F("# BLOQUEO: ")); Serial.println(why); }
void beginBurst(){ cycleStartPulses=pulseCount(); lastSeenPulseCount=cycleStartPulses; lastPulseAt=millis(); stateAt=millis(); state=WATERING; cycles++; valve(true); Serial.println(F("# Valvula abierta")); }
void updateFlow(){ static uint32_t prev=0; static unsigned long at=0; unsigned long now=millis(); if(now-at<1000)return; uint32_t n=pulseCount(); flowLMin=((n-prev)/PULSES_PER_LITER)*60.0f/((now-at)/1000.0f); prev=n; at=now; }
void updateIrrigation(){ unsigned long now=millis(); if(!calibrated()||!isfinite(soilPct)){valve(false);state=IDLE;return;} switch(state){
case IDLE: if(soilPct<=SOIL_START_PERCENT){cycles=0;totalOpenMs=0;irrigationStartPulses=pulseCount();liters=0;beginBurst();} break;
case WATERING: { uint32_t n=pulseCount(); if(n>lastSeenPulseCount){lastSeenPulseCount=n;lastPulseAt=now;} liters=(n-irrigationStartPulses)/PULSES_PER_LITER; unsigned long elapsed=now-stateAt;
if(soilPct>=SOIL_TARGET_PERCENT){totalOpenMs+=elapsed;valve(false);state=SOAKING;stateAt=now;}
else if(now-lastPulseAt>=FLOW_TIMEOUT_MS)lockout("sin flujo: revisar tanque, valvula y tuberia");
else if(liters>=MAX_VOLUME_L)lockout("volumen maximo alcanzado");
else if(totalOpenMs+elapsed>=MAX_TOTAL_MS)lockout("tiempo maximo alcanzado");
else if(elapsed>=BURST_MS){totalOpenMs+=elapsed;valve(false);state=SOAKING;stateAt=now;Serial.println(F("# Pausa de infiltracion"));} break;}
case SOAKING: if(soilPct>=SOIL_TARGET_PERCENT){state=IDLE;Serial.println(F("# Riego finalizado: humedad objetivo"));}
else if(now-stateAt>=SOAK_MS){if(cycles>=MAX_CYCLES||totalOpenMs>=MAX_TOTAL_MS||liters>=MAX_VOLUME_L)lockout("limite de ciclos/tiempo/volumen");else beginBurst();} break;
case LOCKOUT: if(soilPct>=SOIL_TARGET_PERCENT){state=IDLE;Serial.println(F("# Sistema rearmado"));} break; } }
const char* stateName(){switch(state){case IDLE:return "idle";case WATERING:return "watering";case SOAKING:return "soaking";case LOCKOUT:return "lockout";}return "unknown";}
void setup(){ digitalWrite(VALVE_PIN,VALVE_ACTIVE_LOW?HIGH:LOW); pinMode(VALVE_PIN,OUTPUT); valve(false); Serial.begin(115200); analogReadResolution(12); analogSetPinAttenuation(SOIL_PIN,ADC_11db); pinMode(FLOW_PIN,INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(FLOW_PIN),flowISR,FALLING); Serial.println(F("# Riego por gravedad: sensores suelo + flujo")); if(!calibrated())Serial.println(F("# Calibrar SOIL_DRY_RAW y SOIL_WET_RAW; valvula bloqueada")); Serial.println(F("# Calibrar PULSES_PER_LITER y verificar driver")); }
void loop(){ updateFlow(); unsigned long now=millis(); if(now-lastSample>=SAMPLE_MS){lastSample=now;int raw=readSoil();soilPct=soilPercent(raw);updateIrrigation();
Serial.print(F("{\"soil_raw\":"));Serial.print(raw);Serial.print(F(",\"soil_moisture_pct\":"));if(isfinite(soilPct))Serial.print(soilPct,1);else Serial.print(F("null"));
Serial.print(F(",\"flow_pulses\":"));Serial.print(pulseCount());Serial.print(F(",\"flow_l_min\":"));Serial.print(flowLMin,3);Serial.print(F(",\"water_liters_cycle\":"));Serial.print(liters,3);
Serial.print(F(",\"valve_open\":"));Serial.print(state==WATERING?F("true"):F("false"));Serial.print(F(",\"state\":\""));Serial.print(stateName());Serial.println(F("\"}"));}
if(state==WATERING && totalOpenMs+now-stateAt>=MAX_TOTAL_MS)lockout("tiempo maximo alcanzado"); delay(5); }