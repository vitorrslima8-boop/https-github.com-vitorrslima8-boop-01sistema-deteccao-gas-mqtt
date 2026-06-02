#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ==========================================
// 1. CONFIGURAÇÕES DE REDE E MQTT 
// ==========================================
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";        

// Usando o servidor público EMQX (Mais estável e rápido)
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883; 
const char* topico_alerta = "gasmonitor/alerta"; 

WiFiClient espClient;
PubSubClient client(espClient);
Servo servo;

// ==========================================
// 2. MAPEAMENTO DE HARDWARE (Pinos do ESP32)
// ==========================================
const int pinSensor = 34; // Entrada Analógica (Sinal MQ-2)
const int pinServo = 2;   // Saída PWM (Atuador Mecânico)
const int pinLED = 4;     // Saída Digital (Alerta Visual)

// ==========================================
// 3. VARIÁVEIS DE CONTROLE DO SISTEMA
// ==========================================
// Flag para o sistema lembrar se o alarme estava tocando antes
bool estadoPoluido = false; 

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
    
    // Gera ID único para evitar queda de conexão por conflito
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
  servo.write(0); // Garante que o sistema liga com a válvula fechada
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("[SYS] Sistema operando em modo de monitoramento continuo.\n");
}

// ==========================================
// LOOP DE CONTROLE PRINCIPAL
// ==========================================
void loop() {
  // Mantém a conexão MQTT viva
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  // Leitura do sensor e conversão matemática para PPM (Partes por Milhão)
  int gasValueBruto = analogRead(pinSensor);
  long ppm = map(gasValueBruto, 0, 4095, 0, 10000);
  
  // Define a partir de quantos PPM o sistema considera como vazamento perigoso
  const int limitePPM = 3000; 

  // Lógica de Atuação e Telemetria
  if (ppm > limitePPM) {
    servo.write(90);             // Abre a válvula
    digitalWrite(pinLED, HIGH);  // Acende o LED de perigo
    
    // Envia o JSON de emergência
    String payload = "{\"ppm\": " + String(ppm) + ", \"status\": \"poluido\"}";
    client.publish(topico_alerta, payload.c_str());
    
    Serial.print("[CRITICO] Risco! Nivel de gas: ");
    Serial.print(ppm);
    Serial.println(" PPM. Valvula aberta.");
    
    // Levanta a bandeira de perigo para o sistema "lembrar" do estado atual
    estadoPoluido = true; 
    
    delay(2000); // Aguarda 2 segundos para não sobrecarregar o servidor (flood)
  } 
  else {
    servo.write(0);              // Fecha a válvula
    digitalWrite(pinLED, LOW);   // Apaga o LED
    
    // Só envia a mensagem de estabilização SE o alarme estava acionado antes
    if (estadoPoluido == true) {
      // Envia o JSON informando que voltou ao normal
      String payloadLimpo = "{\"ppm\": " + String(ppm) + ", \"status\": \"normal\"}";
      client.publish(topico_alerta, payloadLimpo.c_str());
      
      Serial.println("[INFO] O ambiente foi estabilizado. Ar limpo. Valvula fechada.");
      
      // Abaixa a bandeira, pois o perigo já passou
      estadoPoluido = false; 
    }
  }
  
  delay(500); // Ciclo de amostragem (Lê o sensor a cada meio segundo)
}
