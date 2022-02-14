
#include <M5Stack.h>
#include "RFID_command.h"
#include <WiFi.h>
#include <WiFiUdp.h>

UHF_RFID RFID;
ManyInfo cards;

const char* ssid = "STARK 2.4G";
const char* senha = "01550123";
unsigned int localPort = 3333;
WiFiUDP conexao;
IPAddress ip;
int porta = 0;

void setup() {
  M5.begin();
  pinMode(25, OUTPUT);
  pinMode(37, INPUT);
  digitalWrite(25, LOW);
  M5.Lcd.fillRect(0, 0, 340, 280, RED);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  
  // UHF_RFID set UHF_RFID设置
  RFID.Set_transmission_Power(2600);
  RFID.Set_the_Select_mode();
  RFID.Delay(100);
  RFID.Readcallback();
  RFID.clean_data();
  
  String soft_version;
  soft_version = RFID.Query_software_version();
  Serial.println("versão");
  Serial.println(soft_version);
  while(soft_version.indexOf("V2.3.5") == -1) {
    RFID.clean_data();
    M5.Lcd.fillCircle(310, 10, 6, RED);
    RFID.Delay(150);
    M5.Lcd.fillCircle(310, 10, 6, BLACK);
    RFID.Delay(150);
    soft_version = RFID.Query_software_version();
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, senha);
  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, senha);
    delay(10);
  }
  Serial.println(WiFi.localIP());
  conexao.begin(localPort);

  M5.Lcd.println("Aproxime da tag RFID...");
}

void loop() {
  M5.Lcd.fillCircle(310, 10, 6, GREEN);
  RFID.Delay(50);
  M5.Lcd.fillCircle(310, 10, 6, BLACK);
  RFID.Delay(50);
  
  int tamanhoPacket = conexao.parsePacket();
  if (tamanhoPacket) {    
    ip = conexao.remoteIP();
    porta = conexao.remotePort();    
  }  
        
  int inicio = 40;  
  cards = RFID.Multiple_polling_instructions(15);
  for (size_t i = 0; i < cards.len; i++) {    
    if(cards.card[i]._EPC.length() == 24) {      
      Serial.println("RSSI :" + cards.card[i]._RSSI);
      Serial.println("PC :" + cards.card[i]._PC);
      Serial.println("EPC :" + cards.card[i]._EPC);
      Serial.println("CRC :" + cards.card[i]._CRC);

      M5.Lcd.fillRect(0, 0, 340, 280, BLACK);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(8, inicio);      
      M5.Lcd.print(cards.card[i]._EPC);  
      M5.Lcd.setTextSize(3);
      M5.Lcd.setCursor(8, (inicio+20));      
      M5.Lcd.print("CRC:");
      M5.Lcd.print(cards.card[i]._CRC);
      inicio += 50;      
    }

    if (porta > 0){
      conexao.beginPacket(ip, porta);
      conexao.printf("%s", cards.card[i]._CRC);
      conexao.endPacket();
    }
  }
  RFID.clean_data(); 
  conexao.flush();
  
  if (digitalRead(37) ==0) {
    M5.Lcd.clear();  
  }  
}  
   
