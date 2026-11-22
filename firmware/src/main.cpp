/* Mini-invernadero: ESP32 + humedad de suelo + caudalimetro.
   Wi-Fi AP local para interfaz Godot. Valvula NC mediante driver externo. */
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

constexpr uint8_t SOIL_PIN=34, FLOW_PIN=27, VALVE_PIN=26;
constexpr bool VALVE_ACTIVE_LOW=true;
constexpr char AP_SSID[]="MACETA_ESP32";
constexpr char AP_PASSWORD[]="riego1234"; // Cambiar antes de instalar.
constexpr uint16_t TCP_PORT=5000;
constexpr int SOIL_DRY_RAW=-1, SOIL_WET_RAW=-1; // calibrar
constexpr float DEFAULT_PULSES_PER_LITER=450.0f; // calibrar en el montaje
constexpr uint8_t SOIL_SAMPLES=12;
constexpr unsigned long SAMPLE_MS=2500, FLOW_TIMEOUT_MS=8000;
volatile uint32_t flowPulses=0;
enum State { IDLE, WATERING, SOAKING, LOCKOUT };
State state=IDLE;
float soilStart=35.0f, soilTarget=45.0f, pulsesPerLiter=DEFAULT_PULSES_PER_LITER;
float burstSeconds=5.0f, soakSeconds=30.0f, maxTotalSeconds=30.0f, maxVolumeL=0.50f;
float soilPct=NAN, flowLMin=0.0f, liters=0.0f;
unsigned long lastSample=0, stateAt=0, lastPulseAt=0, totalOpenMs=0, lastFlowCalc=0, lastTelemetry=0;
uint32_t irrigationStartPulses=0, lastFlowPulses=0, lastSeenPulseCount=0;
uint8_t cycles=0;
String eventText="Iniciando";
WiFiServer server(TCP_PORT);
WiFiClient godotClient;
char rxLine[256];
size_t rxLength=0;

void IRAM_ATTR flowISR(){ flowPulses++; }
uint32_t pulseCount(){ noInterrupts(); uint32_t n=flowPulses; interrupts(); return n; }
bool calibrated(){ return SOIL_DRY_RAW>=0 && SOIL_DRY_RAW<=4095 && SOIL_WET_RAW>=0 && SOIL_WET_RAW<=4095 && SOIL_DRY_RAW!=SOIL_WET_RAW; }
int readSoil(){ uint32_t sum=0; analogRead(SOIL_PIN); for(uint8_t i=0;i<SOIL_SAMPLES;i++){sum+=analogRead(SOIL_PIN);delay(4);} return sum/SOIL_SAMPLES; }
float soilPercent(int raw){ if(!calibrated())return NAN; float p=100.0f*(raw-SOIL_DRY_RAW)/(SOIL_WET_RAW-SOIL_DRY_RAW); return constrain(p,0.0f,100.0f); }
void valve(bool open){ uint8_t on=VALVE_ACTIVE_LOW?LOW:HIGH, off=VALVE_ACTIVE_LOW?HIGH:LOW; digitalWrite(VALVE_PIN,open?on:off); }
const char* stateName(){switch(state){case IDLE:return "idle";case WATERING:return "watering";case SOAKING:return "soaking";case LOCKOUT:return "lockout";}return "unknown";}
void sendJson(JsonDocument &doc){ if(godotClient && godotClient.connected()){serializeJson(doc,godotClient);godotClient.print('\n');} }
void sendAck(const char* message,bool ok=true){JsonDocument d;d["type"]="ack";d["ok"]=ok;d["message"]=message;d["event"]=eventText;sendJson(d);}
void lockout(const char* why){valve(false);state=LOCKOUT;stateAt=millis();eventText=why;Serial.print("# BLOQUEO: ");Serial.println(why);}
void beginBurst(){cycleStartPulses();}
// Keep each watering burst bounded; the flow meter is a safety/measurement input,
// while soil moisture remains the control variable.
void startBurst(){uint32_t n=pulseCount();lastSeenPulseCount=n;lastPulseAt=millis();stateAt=millis();state=WATERING;cycles++;valve(true);eventText="Pulso de riego activo";}
void updateFlow(){unsigned long now=millis();if(now-lastFlowCalc<1000)return;uint32_t n=pulseCount();flowLMin=((n-lastFlowPulses)/pulsesPerLiter)*60.0f/((now-lastFlowCalc)/1000.0f);lastFlowPulses=n;lastFlowCalc=now;}
void updateIrrigation(){unsigned long now=millis();if(!calibrated()||!isfinite(soilPct)){valve(false);if(state!=IDLE){state=IDLE;eventText="Calibrar sensor de humedad";}return;}
 switch(state){
 case IDLE: if(soilPct<=soilStart){cycles=0;totalOpenMs=0;irrigationStartPulses=pulseCount();liters=0;startBurst();} break;
 case WATERING:{uint32_t n=pulseCount();if(n>lastSeenPulseCount){lastSeenPulseCount=n;lastPulseAt=now;}liters=(n-irrigationStartPulses)/pulsesPerLiter;unsigned long elapsed=now-stateAt;
 if(soilPct>=soilTarget){totalOpenMs+=elapsed;valve(false);state=SOAKING;stateAt=now;eventText="Humedad objetivo alcanzada; infiltrando";}
 else if(now-lastPulseAt>=FLOW_TIMEOUT_MS)lockout("Sin pulsos de flujo: revisar tanque, valvula y sensor");
 else if(liters>=maxVolumeL)lockout("Volumen maximo por ciclo alcanzado");
 else if(totalOpenMs+elapsed>=maxTotalSeconds*1000.0f)lockout("Tiempo maximo de valvula alcanzado");
 else if(elapsed>=burstSeconds*1000.0f){totalOpenMs+=elapsed;valve(false);state=SOAKING;stateAt=now;eventText="Pausa para infiltracion";}break;}
 case SOAKING: if(soilPct>=soilTarget){state=IDLE;eventText="Riego finalizado: humedad objetivo";}
 else if(now-stateAt>=soakSeconds*1000.0f){if(cycles>=5||totalOpenMs>=maxTotalSeconds*1000.0f||liters>=maxVolumeL)lockout("Limite de riego alcanzado");else startBurst();}break;
 case LOCKOUT: if(soilPct>=soilTarget){state=IDLE;eventText="Bloqueo rearmado al alcanzar humedad objetivo";}break;
 }}
void sendTelemetry(int raw){if(!godotClient||!godotClient.connected())return;JsonDocument d;d["type"]="telemetry";d["soil_raw"]=raw;if(isfinite(soilPct))d["soil_moisture_pct"]=soilPct;else d["soil_moisture_pct"]=nullptr;d["flow_pulses"]=pulseCount();d["flow_l_min"]=flowLMin;d["water_liters_cycle"]=liters;d["valve_open"]=(state==WATERING);d["state"]=stateName();d["event"]=eventText;d["soil_start_percent"]=soilStart;d["soil_target_percent"]=soilTarget;d["burst_seconds"]=burstSeconds;d["soak_seconds"]=soakSeconds;d["max_total_seconds"]=maxTotalSeconds;d["max_volume_l"]=maxVolumeL;sendJson(d);}
void handleCommand(char* line){JsonDocument d;DeserializationError err=deserializeJson(d,line);if(err){sendAck("JSON no valido",false);return;}const char* type=d["type"]|"";
 if(strcmp(type,"ping")==0){JsonDocument a;a["type"]="ack";a["ok"]=true;a["message"]="pong";a["event"]=eventText;sendJson(a);return;}
 if(strcmp(type,"config")==0){float start=d["soil_start_percent"]|soilStart;float target=d["soil_target_percent"]|soilTarget;float burst=d["burst_seconds"]|burstSeconds;float soak=d["soak_seconds"]|soakSeconds;float maxTime=d["max_total_seconds"]|maxTotalSeconds;float maxVol=d["max_volume_l"]|maxVolumeL;
 if(start<1||start>90||target<start+1||target>99||burst<1||burst>10||soak<5||soak>180||maxTime<burst||maxTime>120||maxVol<0.05f||maxVol>2.0f){sendAck("Configuracion fuera de rango",false);return;}
 soilStart=start;soilTarget=target;burstSeconds=burst;soakSeconds=soak;maxTotalSeconds=maxTime;maxVolumeL=maxVol;eventText="Configuracion actualizada (RAM)";sendAck("Configuracion aplicada; no persiste al reiniciar");return;}
 if(strcmp(type,"get_status")==0){sendTelemetry(isfinite(soilPct)?(int)roundf(soilPct): -1);return;}
 if(strcmp(type,"manual_watering")==0||strcmp(type,"shutdown")==0){sendAck("Comando no disponible: riego automatico protegido",false);return;}
 sendAck("Comando desconocido",false);
}
void pollClient(){if(!godotClient||!godotClient.connected()){if(godotClient)godotClient.stop();WiFiClient incoming=server.available();if(incoming){godotClient=incoming;godotClient.setNoDelay(true);rxLength=0;eventText="Interfaz Godot conectada";sendAck("ESP32 conectado");}}
 if(!godotClient||!godotClient.connected())return;
 while(godotClient.available()){char c=(char)godotClient.read();if(c=='\n'){rxLine[rxLength]='\0';if(rxLength>0)handleCommand(rxLine);rxLength=0;}else if(c!='\r'){if(rxLength<sizeof(rxLine)-1)rxLine[rxLength++]=c;else rxLength=0;}}
}
void setup(){digitalWrite(VALVE_PIN,VALVE_ACTIVE_LOW?HIGH:LOW);pinMode(VALVE_PIN,OUTPUT);valve(false);Serial.begin(115200);analogReadResolution(12);analogSetPinAttenuation(SOIL_PIN,ADC_11db);pinMode(FLOW_PIN,INPUT_PULLUP);attachInterrupt(digitalPinToInterrupt(FLOW_PIN),flowISR,FALLING);WiFi.mode(WIFI_AP);WiFi.softAP(AP_SSID,AP_PASSWORD);server.begin();server.setNoDelay(true);eventText=calibrated()?"AP listo; esperando Godot":"Calibrar SOIL_DRY_RAW y SOIL_WET_RAW";Serial.printf("AP %s / IP %s / TCP %u\n",AP_SSID,WiFi.softAPIP().toString().c_str(),TCP_PORT);if(!calibrated())Serial.println("# Riego bloqueado hasta calibrar humedad");}
void loop(){pollClient();updateFlow();unsigned long now=millis();if(now-lastSample>=SAMPLE_MS){lastSample=now;int raw=readSoil();soilPct=soilPercent(raw);updateIrrigation();sendTelemetry(raw);Serial.printf("soil=%s flow=%.3f L/min volume=%.3f L state=%s\n",isfinite(soilPct)?String(soilPct,1).c_str():"uncalibrated",flowLMin,liters,stateName());}
 if(state==WATERING&&totalOpenMs+now-stateAt>=maxTotalSeconds*1000.0f)lockout("Tiempo maximo de valvula alcanzado");delay(2);}
