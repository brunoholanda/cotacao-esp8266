#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Configuração do LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuração do Wi-Fi
const char* ssid = "Holanda Home";
const char* password = "q6td99fmq3frvf4vpf7y38jv3";

// URL da API
const char* apiUrl = "https://v6.exchangerate-api.com/v6/your_key/latest/USD";

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");

  WiFi.begin(ssid, password);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 30) {
    delay(1000);
    lcd.setCursor(attempt % 16, 1);
    lcd.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.print("WiFi Conectado!");
    delay(2000);
  } else {
    lcd.clear();
    lcd.print("Erro WiFi");
    while (true);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    client.setInsecure(); // Ignora a verificação de certificado (para APIs públicas)

    HTTPClient http;

    http.begin(client, apiUrl); // Usa WiFiClientSecure
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Payload recebido:");
      Serial.println(payload);

      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        if (doc.containsKey("conversion_rates") && doc["conversion_rates"].containsKey("BRL")) {
          float price = doc["conversion_rates"]["BRL"];
          
          // Converte o valor para string e adiciona o prefixo "R$"
          String formattedPrice = "R$ ";
          formattedPrice += String(price, 2); // Adiciona o valor com 2 casas decimais

          // Exibe no LCD
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("DOLAR REAL TIME");
          lcd.setCursor(0, 1);
          lcd.print(formattedPrice);
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Chave ausente");
          Serial.println("Erro: Chave conversion_rates ou BRL ausente");
        }
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Erro JSON");
        Serial.print("Erro na desserializacao: ");
        Serial.println(error.c_str());
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Erro HTTP");
      Serial.print("Erro HTTP: ");
      Serial.println(http.errorToString(httpCode));
    }
    http.end();
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Off");
    Serial.println("WiFi desconectado");
  }

  delay(60000);
}
