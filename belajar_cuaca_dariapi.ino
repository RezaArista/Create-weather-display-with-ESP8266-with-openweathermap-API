#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Informasi koneksi Wi-Fi
const char* ssid = "iphone"; //seuaikan dengan ssid wifi
const char* password = "jnck12345"; //sesuaikan password

// API OpenWeatherMap
const String apiUrl = "http://api.openweathermap.org/data/2.5/weather?lat=-7.084697231960107&lon=111.05648267075021&units=metric&appid=4711f416a49ed661dce8c59c821c575c&lang=id";

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ D8, /* data=*/ D7, /* CS=*/ D6, /* reset=*/ D4); // ESP8266

WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);

  // Inisialisasi WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  u8g2.begin(); // Inisialisasi LCD
}

void loop() {
  float temperature = 0;
  String weatherDescription = "Unknown";
  String wilayah = "belum diketahui";

  // Mendapatkan data cuaca dari OpenWeatherMap
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(wifiClient, apiUrl);

    int httpCode = http.GET();

    if (httpCode == 200) {  // HTTP success
      String payload = http.getString();
      Serial.println("API response: " + payload);  // Cetak respons API

      // Parsing JSON
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("Parsing JSON failed: ");
        Serial.println(error.c_str());
      } else {
        // Jika berhasil, ambil nilai
        temperature = doc["main"]["temp"].as<float>();
        weatherDescription = doc["weather"][0]["description"].as<String>();
        wilayah = doc["name"].as<String>();

        Serial.println("Temperature: " + String(temperature));
        Serial.println("Weather: " + weatherDescription);
      }
    } else {
      Serial.println("Failed to connect to API");
    }
    http.end();
  }

  // Menampilkan Data pada LCD
  u8g2.clearBuffer();
  wilayah.toUpperCase();
  u8g2.setFont(u8g2_font_ncenB08_tr); // Font besar untuk judul
  u8g2.drawStr(0, 15, ("CUACA "+ wilayah).c_str());
  u8g2.drawHLine(0, 18, 128); // Garis pembatas di bawah judul

  u8g2.setFont(u8g2_font_profont10_tr); // Font kecil untuk data
  u8g2.drawStr(0, 30, ("SUHU    : " + String(temperature, 1) + " C").c_str());

  weatherDescription.toUpperCase();  // Mengubah weatherDescription menjadi uppercase
  u8g2.drawStr(0, 40, ("LANGIT  : " + weatherDescription).c_str());  // Menampilkan teks

  // Menampilkan ikon cuaca
  if (weatherDescription.indexOf("CERAH") >= 0) {
    // Gambar ikon matahari
    u8g2.drawCircle(110, 52, 8);  // Lingkaran utama
    u8g2.drawLine(110, 40, 110, 45); // Garis atas
    u8g2.drawLine(110, 59, 110, 64); // Garis bawah
    u8g2.drawLine(100, 52, 105, 52); // Garis kiri
    u8g2.drawLine(115, 52, 120, 52); // Garis kanan
  } else if (weatherDescription.indexOf("AWAN MENDUNG") >= 0) {
    // Gambar ikon awan
    u8g2.drawCircle(100, 52, 6);  // Awan kecil kiri
    u8g2.drawCircle(110, 52, 8);  // Awan besar kanan
    u8g2.drawBox(94, 52, 26, 5);  // Bagian bawah awan
  } else if (weatherDescription.indexOf("HUJAN") >= 0) {
    // Gambar ikon hujan
    u8g2.drawCircle(105, 48, 8);   // Awan
    u8g2.drawLine(100, 52, 110, 52);
    u8g2.drawLine(100, 55, 110, 55);
    u8g2.drawLine(102, 57, 104, 60); // Tetesan hujan kiri
    u8g2.drawLine(108, 57, 110, 60); // Tetesan hujan kanan
  }

  u8g2.sendBuffer();
  delay(60000); // Memperbarui data setiap 60 detik
}
