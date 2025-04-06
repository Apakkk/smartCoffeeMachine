#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5  
#define RST_PIN 2  

const char* ssid = "username";
const char* password = "password";
const char* serverUrl = "http://server_ıp/get_coffee_data"; 

MFRC522 rfid(SS_PIN, RST_PIN);


#define LAMP1_PIN 12
#define LAMP2_PIN 13
#define LAMP3_PIN 14


LiquidCrystal_I2C lcd(0x27, 16, 2); 

void connectToNetwork() {
  if(WiFi.status() == WL_CONNECTED) {  
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("WiFi'ye bağlanılıyor");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP Adresses: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed!");
  }
}

void sendCardID(String cardID) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = String(serverUrl) + "?card_id=" + cardID;
    http.begin(url);
    
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response from Server:");
      Serial.println(response);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Coffee: ");
      lcd.setCursor(0, 1);
      lcd.print(response);  

      handleLampControl(response);
    } else {
      Serial.print("Connection Error: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("No WiFi connection!");
  }
}

void handleLampControl(String coffeeType) {
  digitalWrite(LAMP1_PIN, HIGH);  
  digitalWrite(LAMP2_PIN, HIGH);  
  digitalWrite(LAMP3_PIN, HIGH);  

  if (coffeeType.indexOf("Tea") >= 0) {
    digitalWrite(LAMP2_PIN, LOW);
    delay(3000);
  }
  else if (coffeeType.indexOf("Coffee") >= 0) {
    digitalWrite(LAMP1_PIN, LOW);
    delay(3000);
  }
  else if (coffeeType.indexOf("3U1") >= 0) {
    digitalWrite(LAMP3_PIN, LOW);
    delay(3000);
  }

  digitalWrite(LAMP1_PIN, HIGH);  
  digitalWrite(LAMP2_PIN, HIGH);  
  digitalWrite(LAMP3_PIN, HIGH);  
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  connectToNetwork();

  SPI.begin();
  rfid.PCD_Init();

  pinMode(LAMP1_PIN, OUTPUT);
  pinMode(LAMP2_PIN, OUTPUT);
  pinMode(LAMP3_PIN, OUTPUT);
  digitalWrite(LAMP1_PIN, HIGH);
  digitalWrite(LAMP2_PIN, HIGH);
  digitalWrite(LAMP3_PIN, HIGH);

  // LCD başlatma
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Smart Coffee System");
  lcd.setCursor(0, 1);
  lcd.print("Please read the card...");
}

unsigned long previousWifiCheck = 0;
const unsigned long wifiCheckInterval = 10000;

void loop() {
  if (millis() - previousWifiCheck >= wifiCheckInterval) {
    if (WiFi.status() != WL_CONNECTED) {
      connectToNetwork();
    }
    previousWifiCheck = millis();
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardID += String(rfid.uid.uidByte[i], HEX);
  }
  cardID.toUpperCase();

  sendCardID(cardID);

  rfid.PICC_HaltA();
  delay(3000);
}