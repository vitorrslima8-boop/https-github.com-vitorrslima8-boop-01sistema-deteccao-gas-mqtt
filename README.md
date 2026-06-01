Sistema Inteligente de Detecção de Vazamento de Gás com Monitoramento MQTT (IoT)
Autores: Vitor Rodrigues Instituição: Universidade Presbiteriana Mackenzie - Faculdade de Computação e Informática

📖 Sobre o Projeto
Este projeto consiste num sistema de Internet das Coisas (IoT) para monitorização ambiental e segurança. Utiliza um microcontrolador ESP32 com ligação Wi-Fi nativa para detetar fugas de gases inflamáveis (como GLP, metano e fumo) através de um sensor MQ-2.

Quando a concentração de gás (calculada em PPM) ultrapassa o limite de segurança configurado, o sistema atua automaticamente de duas formas simultâneas: 1. Ação Local: Abre uma válvula de ventilação (simulada por um Servo Motor) para dissipar o gás e aciona um alerta visual de emergência (LED vermelho). 2. Ação Remota (IoT): Publica um payload de dados estruturado em formato JSON através do protocolo MQTT num broker na nuvem, permitindo o acompanhamento do estado do ambiente em tempo real através de telemóveis ou painéis web.

🛠️ Tecnologias e Hardware Utilizados
Cérebro IoT: Microcontrolador ESP32 DevKit (Atualizado para garantir conectividade Wi-Fi TCP/IP)
Sensores: Módulo Analógico de Gás MQ-2
Atuadores: Micro Servo Motor (SG90) e LED Vermelho (com resistor 220Ω)
Comunicação Remota: Protocolo MQTT (via PubSubClient)
Plataforma de Simulação: Wokwi
Linguagem: C/C++ (Arduino Framework)
🔌 Esquema de Ligações (Pinout ESP32)
A arquitetura do hardware foi mapeada da seguinte forma na simulação:

Componente	Pino do Componente	Pino no ESP32	Função
Sensor MQ-2	AOUT (Verde)	GPIO 34	Leitura Analógica do nível de gás
Servo Motor	PWM (Amarelo)	GPIO 2	Controlo de abertura da válvula de exaustão
LED Vermelho	Anode (Positivo)	GPIO 4	Sinalização Visual de Risco
(Nota: O pino VCC de todos os componentes deve ser ligado aos pinos 3V3 ou 5V/VIN do ESP32. Os pinos GND devem ser partilhados no GND da placa).

🚀 Como Executar a Simulação (Wokwi)
Para que o sistema funcione corretamente e se ligue à internet simulada:

Aceda ao site Wokwi - Novo Projeto ESP32.
Substitua o conteúdo do ficheiro sketch.ino pelo código-fonte presente neste repositório.
Substitua o conteúdo do ficheiro diagram.json pelo código de estruturação do circuito presente neste repositório.
No menu lateral esquerdo do Wokwi, abra o Library Manager (ícone de livros) e instale as bibliotecas obrigatórias:
PubSubClient (por Nick O'Leary) - Para a comunicação MQTT
ESP32Servo (por Kevin Harrington) - Para o controlo de hardware PWM
Clique no botão de Start Simulation (Play verde).
📡 Como Monitorizar os Dados via MQTT (Em Tempo Real)
O sistema foi programado para enviar alertas assim que o nível de gás calculado ultrapassa o limite de tolerância estabelecido no código (ex: 3000 PPM). Para capturar estas mensagens:

Aceda ao cliente oficial via navegador ou telemóvel: HiveMQ Websocket Client
Preencha as credenciais do servidor de telemetria:
Host: broker.hivemq.com
Port: 8000
Clique no botão Connect.
Na secção Subscriptions, adicione exatamente o mesmo tópico definido no microcontrolador:
Topic: gasmonitor/alerta
Clique em Subscribe.
Ao simular um vazamento no Wokwi (arrastando a barra do MQ-2 para a direita), o cliente MQTT receberá instantaneamente um pacote de dados JSON semelhante a este:

```json { "ppm": 5840, "status": "poluido" }
