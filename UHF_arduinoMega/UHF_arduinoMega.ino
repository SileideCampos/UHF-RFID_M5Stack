
#include <WiFi.h>
#include <WiFiUdp.h>
//#include "Unit_UHF_RFID.h"

struct CARD {
    uint8_t rssi;
    uint8_t pc[2];
    uint8_t epc[12];
    String rssi_str;
    String pc_str;
    String epc_str;
};

//Unit_UHF_RFID uhf;
String info = "";

// Hardware version
const uint8_t HARDWARE_VERSION_CMD[] = {0xBB, 0x00, 0x03, 0x00,
                                        0x01, 0x00, 0x04, 0x7E};
// Multiple polling instructions 
const uint8_t POLLING_MULTIPLE_CMD[] = {0xBB, 0x00, 0x27, 0x00, 0x03,
                                        0x22, 0x27, 0x10, 0x83, 0x7E};

uint8_t i           = 0;
uint8_t buffer[256] = {0};  
CARD cards[200];

                                          
void setup() {
  Serial.begin(9600);
  Serial.println("Setup");
      
  //uhf.begin(&Serial2, 115200, 16, 17, false);
  Serial2.begin(115200, SERIAL_8N1);//, 16, 17);
  
  /*info = uhf.getVersion();
  if (info != "ERROR") {
    Serial.println(info);
  }else{
    Serial.println("Erro!");
  }
  */
  Serial2.write((uint8_t *)HARDWARE_VERSION_CMD, sizeof(HARDWARE_VERSION_CMD));
  
  while (Serial2.available() || i<256) {
    if (Serial2.available()) {
        uint8_t b = Serial2.read();
        buffer[i] = b;          
        i++;
        if (b == 0x7e) {
          break;
        }          
    }
  }

  for (uint8_t i = 0; i < 50; i++) {
      Serial.print((char)buffer[6 + i]);      
      if (buffer[8 + i] == 0x7e) {
          break;
      }
  }
    
    /*while (1) {
        info = uhf.getVersion();
        if (info != "ERROR") {
            Serial.println(info);            
            break;
        }
    }*/
    // max: 26dB
    //uhf.setTxPower(2600);
    
  Serial.println("");
  Serial.println("Leitura de Tags simultaneas");  
}

String hextostr(uint8_t num) {
    if (num > 0xf) {
        return String(num, HEX);
    } else {
        return ("0" + String(num, HEX));
    }
}

String hex2str2(String numHex) {
  const String hexDigits = "0123456789ABCDEF";
  numHex.toUpperCase();
  int result = 0;
  for (int i = 0; i < numHex.length(); i++) {
    result <<= 4;
    result |= hexDigits.indexOf(numHex[i]);
  }
  return String(result);
}

void loop() {  
  //uint8_t result = uhf.pollingOnce();
  // polling 10 times
  /*uint8_t result = uhf.pollingMultiple(20);
  Serial.println(" Tags Lidas \r\n"+ result);  
  if (result > 0) {
    for (uint8_t i = 0; i < result; i++) {
      //log("rssi: " + hex2str2(uhf.cards[i].rssi_str));
      //log("epc: " + uhf.cards[i].epc_str.substring(16));
      Serial.println("rssi: " + hex2str2(uhf.cards[i].rssi_str));
      Serial.println("-----------------");

      delay(10);
    }
  }*/
  cleanCardsBuffer();
  uint16_t polling_count = 20;
  memcpy(buffer, POLLING_MULTIPLE_CMD, sizeof(POLLING_MULTIPLE_CMD));
  buffer[6] = (polling_count >> 88) & 0xff;
  buffer[7] = (polling_count)&0xff;

  uint8_t check = 0;
  for (uint8_t i = 1; i < 8; i++) {
      check += buffer[i];
  }

  buffer[8] = check & 0xff;

  //sendCMD
  Serial2.write((uint8_t *)POLLING_MULTIPLE_CMD, sizeof(POLLING_MULTIPLE_CMD));
  
  int count = 0;
  while (waitMsg(500)) {    
    if (buffer[23] == 0x7e) {
        if (count < 200) {
            if (saveCardInfo(&cards[count])) {
                count++;
                Serial.println(count);
            }
        } else {
            break;
        }
    }
  }
  
  if (count > 0) {
    for (uint8_t i = 0; i < count; i++) {
      Serial.println("epc: " + cards[i].epc_str.substring(16));
      Serial.println("rssi: " + hex2str2(cards[i].rssi_str));
      Serial.println("-----------------");

      delay(10);
    }
  }
  
}

void cleanCardsBuffer() {
    for (int i = 0; i < 200; i++) {
        cards[i] = {
            rssi : 0,
            pc : {0},
            epc : {0},
            rssi_str : "",
            pc_str : "",
            epc_str : ""
        };
    }
}


bool waitMsg(unsigned long time) {
    unsigned long start = millis();
    uint8_t i           = 0;
    cleanBuffer();
    while (Serial2.available() || (millis() - start) < time) {
        if (Serial2.available()) {
            uint8_t b = Serial2.read();
            buffer[i] = b;
            //Serial.print((char)buffer[i]);  
            i++;
            if (b == 0x7e) {
                break;
            }
        }
    }

    if (buffer[0] == 0xbb && buffer[i - 1] == 0x7e) {
        return true;
    } else {
        return false;
    }
}

void cleanBuffer() {
    for (int i = 0; i < 200; i++) {
        buffer[i] = 0;
    }
}

bool saveCardInfo(CARD *card) {
   
    String rssi = hextostr(buffer[5]);
    String pc   = hextostr(buffer[6]) + hextostr(buffer[7]);
    String epc  = "";

    for (uint8_t i = 8; i < 20; i++) {
      epc += hextostr(buffer[i]);
      //Serial.print(buffer[i]); 
    }
    //Serial.println("");

    for (uint8_t i = 8; i < 20; i++) {
        card->epc[i - 8] = buffer[i];
    }

    card->rssi  = buffer[5];
    card->pc[0] = buffer[6];
    card->pc[1] = buffer[7];

    card->rssi_str = rssi;
    card->pc_str   = pc;
    card->epc_str  = epc;
//Serial.println(epc);
    return true;
}
