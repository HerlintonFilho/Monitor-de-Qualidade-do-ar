#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED ------------------------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// PMS5003 ---------------------------------------------------------------------
HardwareSerial pmsSerial(2);   // Usando UART2 do ESP32

#define PMS_RX 35  // PMS -> ESP32 (recepção no ESP)
#define PMS_TX 34  // ESP32 -> PMS (transmissão do ESP)

uint8_t buf[32]; // buffer para frame de dados

// LED onboard
#define LED_PIN 2

void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializa OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED NAO DETECTADO!");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // Inicializa comunicação com o PMS5003 (baud rate padrão 9600)
  pmsSerial.begin(9600, SERIAL_8N1, PMS_RX, PMS_TX);

  Serial.println("✅ Sistema iniciado, aguardando dados do PMS5003...");
}

void loop() {

  if (pmsSerial.available() >= 32) {

    if (pmsSerial.read() == 0x42) {   // Primeiro byte do frame
      buf[0] = 0x42;

      if (pmsSerial.read() == 0x4d) { // Segundo byte do frame
        buf[1] = 0x4d;

        // Lê o restante dos 30 bytes do frame
        for (int i = 2; i < 32; i++) {
          buf[i] = pmsSerial.read();
        }

        // Decodifica PM2.5 e PM10 (bytes específicos do frame PMS5003)
        int pm25 = (buf[12] << 8) | buf[13];
        int pm10 = (buf[14] << 8) | buf[15];

        // Indica atualização pelo LED
        digitalWrite(LED_PIN, HIGH);

        // Exibe no OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MONITOR QUALIDADE AR");
        display.println("--------------------");

        display.setTextSize(2);
        display.setCursor(0, 22);
        display.print("PM2.5 ");
        display.print(pm25);
        display.println(" ug/m3");

        display.setCursor(0, 46);
        display.print("PM10  ");
        display.print(pm10);
        display.println(" ug/m3");

        display.display();

        Serial.printf("PM2.5: %d ug/m3 | PM10: %d ug/m3\n", pm25, pm10);
        delay(500);

        digitalWrite(LED_PIN, LOW);
      }
    }
  }
}
