#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ==========================================
// 1. CONFIGURAÇÕES DE REDE E MQTT (A contornar a Firewall)
// ==========================================
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";        

// Substitua o EMQX pelo servidor da fundação Eclipse
const char* mqtt_server = "mqtt.eclipseprojects.io";
const int mqtt_port = 1883; // A porta do ESP32 mantém-se (os servidores do Wokwi não têm bloqueio corporativo)
const char* topico_alerta = "gasmonitor/alerta";

WiFiClient espClient;
PubSubClient client(espClient);
Servo servo;

// ==========================================
// 2. MAPEAMENTO DE HARDWARE (Pinos do ESP32 Padrão)
// ==========================================
const int pinSensor = 34; // Entrada Analógica (Sinal MQ-2)
const int pinServo = 2;   // Saída PWM (Atuador Mecânico)
const int pinLED = 4;     // Saída Digital (Alerta Visual Vermelho)

// ==========================================
// MÓDULO DE CONEXÃO WI-FI
// ==========================================
void setup_wifi() {
  delay(10);
  Serial.println("\n[INFO] Inicializando modulo Wi-Fi...");
  Serial.print("[INFO] Tentando conexao com SSID: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[STATUS] Conexao Wi-Fi estabelecida com sucesso.");
}

// ==========================================
// MÓDULO DE CONEXÃO MQTT
// ==========================================
void reconnect() {
  while (!client.connected()) {
    Serial.print("[INFO] Negociando conexao com Broker MQTT (EMQX)... ");
    
    // Gera ID único para não cair a conexão
    String clientId = "Mackenzie-IoT-Node-";
    clientId += String(random(0, 1000));
    
    if (client.connect(clientId.c_str())) {
      Serial.println("[OK] Conectado ao Broker MQTT.");
    } else {
      Serial.print("[ERRO] Falha na conexao. Codigo: ");
      Serial.print(client.state());
      Serial.println(" -> Retentativa em 5 segundos.");
      delay(5000); 
    }
  }
}

// ==========================================
// INICIALIZAÇÃO DO SISTEMA
// ==========================================
void setup() {
  Serial.begin(115200); 
  Serial.println("\n[SYS] Inicializando Sistema Inteligente de Deteccao de Gas...");
  
  pinMode(pinLED, OUTPUT);
  servo.attach(pinServo);
  servo.write(0); 

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("[SYS] Sistema operando em modo de monitoramento continuo.\n");
}

// ==========================================
// LOOP DE CONTROLE PRINCIPAL
// ==========================================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  // Leitura e Conversão
  int gasValueBruto = analogRead(pinSensor);
  long ppm = map(gasValueBruto, 0, 4095, 0, 10000);
  
  // Limite configurado para o seu ambiente
  const int limitePPM = 3000; 

  if (ppm > limitePPM) {
    servo.write(90);             
    digitalWrite(pinLED, HIGH);  
    
    String payload = "{\"ppm\": " + String(ppm) + ", \"status\": \"poluido\"}";
    client.publish(topico_alerta, payload.c_str());
    
    Serial.print("[CRITICO] Risco! Nivel de gas: ");
    Serial.print(ppm);
    Serial.println(" PPM. Valvula aberta. Payload enviado.");
    
    delay(2000); 
  } 
  else {
    servo.write(0);              
    digitalWrite(pinLED, LOW);   
  }
  
  delay(500); 
}
