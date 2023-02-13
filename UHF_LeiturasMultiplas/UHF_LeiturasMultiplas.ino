
#include <M5Stack.h>
#include <M5GFX.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "Unit_UHF_RFID.h"

M5GFX display;
M5Canvas canvas(&display);
Unit_UHF_RFID uhf;
String info = "";

const char* ssid = "nome-da-sua-rede";
const char* senha = "senha-da-sua-rede";
unsigned int localPort = 3333;
WiFiUDP conexao;
IPAddress ip;
int porta = 0;

void setup() {
    M5.begin();
    uhf.begin(&Serial2, 115200, 16, 17, false);
    while (1) {
        info = uhf.getVersion();
        if (info != "ERROR") {
            Serial.println(info);
            break;
        }
    }

    // max: 26dB
    uhf.setTxPower(2600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, senha);
  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, senha);
    delay(10);
  }
  Serial.println(WiFi.localIP());
  conexao.begin(localPort);
  M5.Lcd.println(WiFi.localIP());
  delay(500);

  display.begin();
  canvas.setColorDepth(1);  // mono color
  canvas.setFont(&fonts::efontCN_14);
  canvas.createSprite(display.width(), display.height());
  canvas.setTextSize(2);
  canvas.setPaletteColor(1, GREEN);
  canvas.setTextScroll(true);
  canvas.println(info);
  canvas.println("Leitura de Tags simultaneas");
  canvas.pushSprite(0, 0);
}

uint8_t write_buffer[]  = {0xab, 0xcd, 0xef, 0xdd};
uint8_t reade_buffer[4] = {0};

String hex2str(String numHex) {
  const String hexDigits = "0123456789ABCDEF";
  numHex.toUpperCase();
  int result = 0;
  for (int i = 0; i < numHex.length(); i++) {
    result <<= 4;
    result |= hexDigits.indexOf(numHex[i]);
  }
  return String(result);
}

void log(String info) {
  canvas.println(info);
  canvas.pushSprite(0, 0);
}

void loop() {
  if (M5.BtnB.wasPressed()) {
    M5.Lcd.println(WiFi.localIP());  
  }
  
  int tamanhoPacket = conexao.parsePacket();
  if (tamanhoPacket) {    
    ip = conexao.remoteIP();
    porta = conexao.remotePort();    
  }else{
    porta = 0;
  }    
  
  //uint8_t result = uhf.pollingOnce();
  // polling 10 times
  uint8_t result = uhf.pollingMultiple(20);
  Serial.printf("%d Tags Lidas \r\n", result);  
  if (result > 0) {
    for (uint8_t i = 0; i < result; i++) {
      log("rssi: " + hex2str(uhf.cards[i].rssi_str));
      log("epc: " + uhf.cards[i].epc_str.substring(16));
      log("-----------------");      
        
      if (porta > 0){
        conexao.beginPacket(ip, porta);
        conexao.printf("%s", uhf.cards[i].epc_str.substring(16));
        conexao.endPacket();
      }
      delay(10);
    }
  }
  conexao.flush();
  M5.update();
}
