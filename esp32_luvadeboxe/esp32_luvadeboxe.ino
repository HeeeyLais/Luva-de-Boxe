// Inclui a biblioteca de comunicação Serial via Bluetooth
#include "BluetoothSerial.h"

// Verifica se o Bluetooth está ativo no chip do ESP32
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error O Bluetooth nao esta ativado! Ative-o nas configuracoes da placa.
#endif

// Cria o objeto para controle do Bluetooth
BluetoothSerial SerialBT;

// Definições de Pinos e Ajustes
const int PIN_PIEZO = 32;       // Pino analógico correspondente ao GPIO32

// Limiar para detectar o soco. Valores abaixo disso são ignorados (ruído).
// Como seu backend usa o LIMIAR_LEVE em Newtons, aqui filtramos por valor bruto do ADC.
const int THRESHOLD = 150;      

// Tempo de espera (Debounce) em milissegundos para evitar ler o mesmo soco duas vezes
const unsigned long DEBOUNCE_TIME = 300; 

void setup() {
  // Inicializa a Serial nativa (via cabo USB) apenas para monitoramento/testes
  Serial.begin(115200);
  
  // Inicializa o Bluetooth clássico. 
  // O seu computador/notebook vai procurar por este nome para parear:
  SerialBT.begin("ESP32_SmashSense"); 
  
  // Configura o pino do piezoelétrico como entrada
  pinMode(PIN_PIEZO, INPUT);
  
  Serial.println("O dispositivo Bluetooth 'ESP32_SmashSense' esta pronto para parear!");
}

void loop() {
  // Realiza a leitura analógica bruta (retorna um valor entre 0 e 4095)
  int valorLeitura = analogRead(PIN_PIEZO);

  // Se a leitura for maior que o Threshold, um impacto foi detectado
  if (valorLeitura > THRESHOLD) {
    int picoDoGolpe = 0;
    unsigned long tempoInicio = millis();

    // Janela de captura (50ms): Como o impacto gera uma onda senoidal rápida,
    // rodamos esse laço para capturar o exato ponto máximo (pico de força) do soco
    while (millis() - tempoInicio < 50) {
      int leituraAtual = analogRead(PIN_PIEZO);
      if (leituraAtual > picoDoGolpe) {
        picoDoGolpe = leituraAtual;
      }
    }

    // Garante que o valor limite superior respeite o teto do ADC
    if (picoDoGolpe > 4095) {
      picoDoGolpe = 4095;
    }

    // --- ENVIO VIA BLUETOOTH ---
    // Envia apenas se houver um dispositivo (o seu script Python) conectado
    if (SerialBT.connected()) {
      // Envia APENAS o número bruto (ex: "2450") seguido de nova linha (\n)
      // Isso valida a condição 'if linha.isdigit():' no seu backend.py
      SerialBT.println(picoDoGolpe);
      
      // Envia uma cópia para o cabo USB (opcional, para debugar no monitor serial)
      Serial.print("Soco enviado via BT: ");
      Serial.println(picoDoGolpe);
    } else {
      Serial.print("Golpe detectado (");
      Serial.print(picoDoGolpe);
      Serial.println("), mas o Python nao esta conectado ao BT.");
    }

    // Tempo de descanso para a vibração do impacto cessar na luva
    delay(DEBOUNCE_TIME);
  }
}