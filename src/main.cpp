#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <driver/i2s.h>
#include "image.h"
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <PubSubClient.h>

WebServer server(80);

// TFT display pin definitions - ST7789S 240x320
// #define TFT_CS   41
// #define TFT_RST  45
// #define TFT_DC   40
// #define TFT_MOSI 47
// #define TFT_SCLK 21
// #define TFT_BL   42
//NEW SCREEN DEFINITION
#define TFT_CS   45
#define TFT_RST  21
#define TFT_DC   47
#define TFT_MOSI 20
#define TFT_SCLK 19
#define TFT_BL   38
// OLD I2S AUDIO DEFINITION
#define I2S_DOUT    39   // SPK_DOUT -> GPIO14 (Audio data output from ESP32 to the amplifier)
#define I2S_LRCK    41   // SPK_LRCK -> GPIO39 (Left/Right channel clock (LRCLK / Word Select))
#define I2S_BCLK    40   // SPK_BCLK -> GPIO38 (Bit clock (BCLK) used to synchronize audio data transmission)
//New I2S audio pin definitions
//#define I2S_DOUT    14   // SPK_DOUT -> GPIO14 (I2S[TRANSLATE])
//#define I2S_LRCK    39   
//#define I2S_BCLK    38  

const char* ssid = "YOUR WIFI"; //REPLACE WITH YOUR WIFI USERNAME
const char* password = "PASSWORD"; //REPLACE WITH PASSWORD

const char* mqtt_server = "192.168.68.106";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Create ST7789 display object (240x320resolution)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// I2S audio configuration
#define I2S_SAMPLE_RATE 44100
#define I2S_NUM         I2S_NUM_0
#define I2S_SAMPLE_BITS 16
const int TONE_FREQUENCY = 1000; // notification tone frequency (Hz)

void playMelody1();
void playMelody2();
void playTwinkle();
void playFurElise();
void playOdeToJoy();
void playHarryPotter();
void playImperialMarch();
void playAvengers();
void playCustomMelody();

void initTFT() {
    Serial.println("=== Initialize ST7789S TFT display ===");
    
    // Initialize backlight pin
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Turn on backlight
    
    // Initialize ST7789S TFT display
    tft.init(240, 320);           // [TRANSLATE]ST7789S，240x320resolution
    tft.setRotation(1);           // Set display rotation (landscape320x240)
    tft.fillScreen(ST77XX_BLACK); // Clear screen to black
    
    // Test whether the display works properly
    Serial.println("Running display test...");
    tft.fillScreen(ST77XX_RED);
    delay(200);
    tft.fillScreen(ST77XX_GREEN);
    delay(200);
    tft.fillScreen(ST77XX_BLUE);
    delay(200);
    tft.fillScreen(ST77XX_BLACK);
    
    Serial.println("ST7789SDisplay initialization complete! (resolution: 320x240landscape[TRANSLATE])");
}

void displayImage() {
    // Display static image
    Serial.println("Display image on screen...");
    
    // Clear screen
    tft.fillScreen(ST77XX_BLACK);
    
    // [TRANSLATE]
    // Note: This is a simple demo. Actual image data should be converted using image_converter.py.
    int img_width = image_1_width;
    int img_height = image_1_height;
    
    // Calculate centered position（[TRANSLATE]Display）
    int start_x = (320 - img_width) / 2;
    int start_y = (240 - img_height) / 2;
    if (start_x < 0) start_x = 0;
    if (start_y < 0) start_y = 0;
    
    Serial.printf("Image size: %dx%d, Display position: (%d,%d)\n", img_width, img_height, start_x, start_y);
    
    // Display actual image data
    // Use converted image data
    tft.drawRGBBitmap(start_x, start_y, image_1, img_width, img_height);
}

// Dynamic update function disabled; only static image is displayed.
// void updateDisplay() {
//     // Dynamic update disabled; only static image is displayed.
// }

void initAudio() {
    Serial.println("=== Initialize I2S audio system ===");
    
    // I2S configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,    // GPIO38
        .ws_io_num = I2S_LRCK,     // GPIO39
        .data_out_num = I2S_DOUT,  // GPIO14
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    // Install and start I2S driver
    esp_err_t result = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        Serial.printf("I2S driver installation failed: %d\n", result);
        return;
    }
    
    result = i2s_set_pin(I2S_NUM, &pin_config);
    if (result != ESP_OK) {
        Serial.printf("I2S pin configuration failed: %d\n", result);
        return;
    }
    
    Serial.println("I2S audio system initialized!");
    Serial.printf("  DOUT: GPIO%d, LRCK: GPIO%d, BCLK: GPIO%d\n", I2S_DOUT, I2S_LRCK, I2S_BCLK);
    Serial.println("  Connect to audio amplifier or codec -> speaker");
}


void playTone(int frequency, int duration) {
    Serial.printf("Playing tone: %dHz, [TRANSLATE]%dms\n", frequency, duration);
    
    // Calculate number of samples
    const int samples_per_ms = I2S_SAMPLE_RATE / 1000;
    const int total_samples = samples_per_ms * duration;
    
    // Create audio buffer (16[TRANSLATE])
    int16_t *audio_buffer = (int16_t*)malloc(total_samples * 2 * sizeof(int16_t));
    if (audio_buffer == NULL) {
        Serial.println("Failed to allocate audio buffer!");
        return;
    }
    
    // Generate sine wave tone
    for (int i = 0; i < total_samples; i++) {
        double t = (double)i / I2S_SAMPLE_RATE;
        int16_t sample = (int16_t)(sin(2.0 * M_PI * frequency * t) * 16384); // 50%[TRANSLATE]
        audio_buffer[i * 2] = sample;     // [TRANSLATE]
        audio_buffer[i * 2 + 1] = sample; // [TRANSLATE]
    }
    
    // Output audio through I2S
    size_t bytes_written;
    esp_err_t result = i2s_write(I2S_NUM, audio_buffer, total_samples * 2 * sizeof(int16_t), &bytes_written, pdMS_TO_TICKS(duration + 100));
    
    if (result != ESP_OK) {
        Serial.printf("I2S write failed: %d\n", result);
    } else {
        Serial.printf("I2S audio output: %d[TRANSLATE]\n", bytes_written);
    }
    
    // Release buffer
    free(audio_buffer);
    
    // // [TRANSLATE]TFT[TRANSLATE]Audio[TRANSLATE] - [TRANSLATE]320x240landscape
    // tft.fillRect(160, 210, 150, 15, ST77XX_BLACK);
    // tft.setTextColor(ST77XX_MAGENTA);
    // tft.setTextSize(1);
    // tft.setCursor(160, 210);
    // tft.printf("♪ I2S Audio: %dHz", frequency);
}

void playWelcomeTone() {
    // Play welcome tone sequence
    Serial.println("[TRANSLATE]...");
    playTone(523, 200);  // C5
    delay(50);
    playTone(659, 200);  // E5
    delay(50);
    playTone(784, 300);  // G5
    
    // [TRANSLATE]Audio[TRANSLATE]
    // tft.fillRect(160, 210, 150, 15, ST77XX_BLACK);
}

void showWiFiScreen(const char* message) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(30, 60);
    tft.println("WiFi Setup");

    tft.setTextSize(1);
    tft.setCursor(30, 110);
    tft.println(message);
}



void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String message;

    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.println("MQTT Received: " + message);

    if (message == "melody1")
    {
        playMelody1();
    }
    else if (message == "melody2")
    {
        playMelody2();
    }
    else if (message == "twinkle")
    {
        playTwinkle();
    }
    else if (message == "furelise")
    {
        playFurElise();
    }
    else if (message == "harry")
    {
        playHarryPotter();
    }
    else if (message == "avengers")
    {
        playAvengers();
    }
}

void reconnectMQTT()
{
    if (!mqttClient.connected())
    {
        Serial.print("Connecting MQTT...");

        if (mqttClient.connect("ESP32_S3_Melody_Player"))
        {
            Serial.println("connected!");
            mqttClient.subscribe("music/control");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.println(mqttClient.state());
        }
    }
}

bool connectWiFi() {
    showWiFiScreen("Connecting to WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        showWiFiScreen("WiFi Connected!");
        tft.setCursor(30, 140);
        tft.print("IP: ");
        tft.println(WiFi.localIP());

        delay(5000);
        return true;
    } else {
        Serial.println("\nWiFi failed.");
        showWiFiScreen("WiFi Failed");
        delay(3000);
        return false;
    }
}

void playMelody1() {
    playTone(523, 200);
    delay(50);
    playTone(659, 200);
    delay(50);
    playTone(784, 300);
}

void playMelody2() {
    playTone(784, 150);
    delay(50);
    playTone(659, 150);
    delay(50);
    playTone(523, 300);
}

void playTwinkle() {
    playTone(262, 300); delay(80);
    playTone(262, 300); delay(80);
    playTone(392, 300); delay(80);
    playTone(392, 300); delay(80);
    playTone(440, 300); delay(80);
    playTone(440, 300); delay(80);
    playTone(392, 500); delay(150);
}

void playFurElise() {
    playTone(659, 200); delay(50);
    playTone(622, 200); delay(50);
    playTone(659, 200); delay(50);
    playTone(622, 200); delay(50);
    playTone(659, 200); delay(50);
    playTone(494, 200); delay(50);
    playTone(587, 200); delay(50);
    playTone(523, 200); delay(50);
    playTone(440, 400); delay(150);
}

void playOdeToJoy() {
    playTone(330, 300); delay(50);
    playTone(330, 300); delay(50);
    playTone(349, 300); delay(50);
    playTone(392, 300); delay(50);
    playTone(392, 300); delay(50);
    playTone(349, 300); delay(50);
    playTone(330, 300); delay(50);
    playTone(294, 300); delay(50);
    playTone(262, 300); delay(50);
}

void playHarryPotter() {
    playTone(494, 400); delay(100);
    playTone(659, 600); delay(100);
    playTone(784, 200); delay(100);
    playTone(740, 400); delay(100);
    playTone(698, 600); delay(100);
}

void playImperialMarch() {
    playTone(440, 400); delay(100);
    playTone(440, 400); delay(100);
    playTone(440, 400); delay(100);
    playTone(349, 300); delay(50);
    playTone(523, 150); delay(50);
    playTone(440, 600); delay(150);
}

void playAvengers() {
    playTone(262, 300); delay(50);
    playTone(330, 300); delay(50);
    playTone(392, 500); delay(100);
    playTone(523, 300); delay(50);
    playTone(494, 300); delay(50);
    playTone(392, 500); delay(100);
}

String loadCustomMelody() {
    if (!SPIFFS.exists("/custom.txt")) {
        return "C4,300\nC4,300\nG4,300\nG4,300\nA4,300\nA4,300\nG4,500";
    }

    File file = SPIFFS.open("/custom.txt", "r");
    String data = file.readString();
    file.close();
    return data;
}

void saveCustomMelody(String data) {
    File file = SPIFFS.open("/custom.txt", "w");
    file.print(data);
    file.close();
}

float noteToFreq(String note) {

    if (note == "C4") return 262;
    if (note == "D4") return 294;
    if (note == "E4") return 330;
    if (note == "F4") return 349;
    if (note == "G4") return 392;
    if (note == "A4") return 440;
    if (note == "B4") return 494;

    if (note == "C5") return 523;
    if (note == "D5") return 587;
    if (note == "E5") return 659;
    if (note == "F5") return 698;
    if (note == "G5") return 784;
    if (note == "A5") return 880;
    if (note == "B5") return 988;
    
    if (note == "C#4") return 277;
    if (note == "D#4") return 311;
    if (note == "F#4") return 370;
    if (note == "G#4") return 415;
    if (note == "A#4") return 466;
    
    if (note == "C#5") return 554;
    if (note == "D#5") return 622;
    if (note == "F#5") return 740;
    if (note == "G#5") return 831;
    if (note == "A#5") return 932;

    return 0;
}

void playCustomMelody() {
    String data = loadCustomMelody();

    int start = 0;
    while (start < data.length()) {
        int end = data.indexOf('\n', start);
        if (end == -1) end = data.length();

        String line = data.substring(start, end);
        line.trim();

        if (line.length() > 0) {
            int comma = line.indexOf(',');
            if (comma > 0) {
                

                String note = line.substring(0, comma);
                note.trim();
                int freq = (int)noteToFreq(note);
                int dur = line.substring(comma + 1).toInt();

                if (freq > 0 && dur > 0) {
                    playTone(freq, dur);
                    delay(80);
                }
            }
        }

        start = end + 1;
    }
}

void handleRoot() {
    String html =
    "<html><body>"
    "<h1>ESP32 Melody Player</h1>"
    "<p><a href='/melody1'>Happy Melody</a></p>"
    "<p><a href='/melody2'>Soft Melody</a></p>"
    "<p><a href='/twinkle'>Twinkle Little Star</a></p>"
    "<p><a href='/furelise'>Fur Elise</a></p>"
    "<p><a href='/odetojoy'>Ode To Joy</a></p>"
    "<p><a href='/harry'>Harry Potter</a></p>"
    "<p><a href='/imperial'>Imperial March</a></p>"
    "<p><a href='/avengers'>Avengers Theme</a></p>"
    "<a href='/editor'>Edit Custom Melody</a>"
    "</body></html>";

    server.send(200, "text/html", html);
}

void handleMelody1() {
    playMelody1();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleMelody2() {
    playMelody2();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleTwinkle() {
    playTwinkle();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleFurElise() {
    playFurElise();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleOdeToJoy() {
    playOdeToJoy();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleHarryPotter() {
    playHarryPotter();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleImperialMarch() {
    playImperialMarch();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleAvengers() {
    playAvengers();
    server.sendHeader("Location", "/");
    server.send(303);
}

void handleEditor() {
    String melody = loadCustomMelody();

    String html =
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>"
    "body{font-family:Arial;background:#111827;color:white;text-align:center;padding:20px;}"
    "textarea{width:90%;height:260px;border-radius:12px;padding:12px;font-size:16px;}"
    "button,a{display:block;margin:15px auto;padding:14px 20px;border-radius:12px;border:0;background:#8b5cf6;color:white;text-decoration:none;font-size:18px;max-width:300px;}"
    "</style></head><body>"

    "<h1>Custom Melody Editor</h1>"
    "<p>Format: note,duration</p>"
    "<p>Example: C4,300</p>"
    "<div style='background:#1f2937;padding:12px;border-radius:12px;margin:15px;'>"
    "<b>Note Cheat Sheet</b><br>"
    "C4 = Do = 262 Hz<br>"
    "D4 = Re = 294 Hz<br>"
    "E4 = Mi = 330 Hz<br>"
    "F4 = Fa = 349 Hz<br>"
    "G4 = Sol = 392 Hz<br>"
    "A4 = La = 440 Hz<br>"
    "B4 = Si = 494 Hz<br>"
    "C5 = High Do = 523 Hz"
    "</div>"

    "<form action='/save' method='POST'>"



    "<textarea name='melody'>" + melody + "</textarea>"
    "<button type='submit'>Save Melody</button>"
    "</form>"
    "<a href='/playcustom'>Play Custom Melody</a>"
    "<a href='/'>Back Home</a>"
    "</body></html>";

    server.send(200, "text/html", html);
}

void handleSaveMelody() {
    if (server.hasArg("melody")) {
        saveCustomMelody(server.arg("melody"));
    }

    server.sendHeader("Location", "/editor");
    server.send(303);
}

void handlePlayCustom() {
    playCustomMelody();
    server.sendHeader("Location", "/editor");
    server.send(303);
}

void showMelodyMenu() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.setCursor(30, 40);
    tft.println("Melody Select");
    tft.setCursor(30, 70);
    tft.println(WiFi.localIP());

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(30, 100);
    tft.println("Type in Serial Monitor:");

    tft.setCursor(30, 130);
    tft.println("melody1 - Happy melody");

    tft.setCursor(30, 150);
    tft.println("melody2 - Soft melody");

    tft.setCursor(30, 180);
    tft.println("AND MORE!!");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("");
    Serial.println("========================================");
    Serial.println("ESP32-S3 + ST7789S + Audio[TRANSLATE]");
    Serial.println("========================================");

    Serial.println("Hardware configuration:");
    Serial.println("  Display: ST7789S 240×320 TFT[TRANSLATE]");  
    Serial.println("  Audio: I2S[TRANSLATE]Audio[TRANSLATE]/[TRANSLATE]");
    Serial.println("        GPIO39(DOUT) + GPIO40(BCLK) + GPIO41(LRCK)");
    Serial.println("");

    initTFT();
    initAudio();
    if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS failed!");
  } else {
    Serial.println("SPIFFS started!");
  }
  
  displayImage();

    delay(800);
    playWelcomeTone();

    if (connectWiFi()) {
        mqttClient.setServer(mqtt_server, 1883);
        mqttClient.setCallback(mqttCallback);
        reconnectMQTT();

        server.on("/", handleRoot);

        server.on("/melody1", handleMelody1);
        server.on("/melody2", handleMelody2);
        server.on("/twinkle", handleTwinkle);
        server.on("/furelise", handleFurElise);
        server.on("/odetojoy", handleOdeToJoy);
        server.on("/harry", handleHarryPotter);
        server.on("/imperial", handleImperialMarch);
        server.on("/avengers", handleAvengers);
        server.on("/editor", handleEditor);
        server.on("/save", HTTP_POST, handleSaveMelody);
        server.on("/playcustom", handlePlayCustom);

        server.begin();
        Serial.println("Web server started!");
      } else {
        Serial.println("Web server NOT started because WiFi failed.");
    }

    showMelodyMenu();

    Serial.println("");
    Serial.println("========================================");
    Serial.println("System startup complete!");
    Serial.println("========================================");
    Serial.println("Available serial commands:");
    Serial.println("  tone     - [TRANSLATE]");
    Serial.println("  beep     - [TRANSLATE]");
    Serial.println("  melody1  - Play happy melody");
    Serial.println("  melody2  - Play soft melody");
    Serial.println("  clear    - [TRANSLATE]");
    Serial.println("  test     - [TRANSLATE]");
    Serial.println("========================================");
}

void loop() {

    server.handleClient();
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
     mqttClient.loop();

    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();

        if (command == "tone") {
            playTone(800, 500);
        } else if (command == "beep") {
            playTone(1200, 100);
            delay(100);
            playTone(1200, 100);
        } else if (command == "melody1") {
            playMelody1();
            showMelodyMenu();
        } else if (command == "melody2") {
            playMelody2();
            showMelodyMenu();
        } else if (command == "clear") {
            displayImage();
        } else if (command == "test") {
            tft.fillScreen(ST77XX_RED);
            delay(300);
            tft.fillScreen(ST77XX_GREEN);
            delay(300);
            tft.fillScreen(ST77XX_BLUE);
            delay(300);
            displayImage();
            playTone(1000, 200);
        } else {
            Serial.println("Unknown command: " + command);
            Serial.println("Enter: tone, beep, melody1, melody2, clear, test");
        }
    }

    delay(100);
}
