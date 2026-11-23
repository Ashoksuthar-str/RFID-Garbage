#include <WiFi.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_HX711.h>


// ============================================================
// SERVER
// ============================================================

// Change this to your computer's local IP address.
//
// Example:
// http://192.168.1.100:5000/
//
// IMPORTANT:
// Do NOT use localhost here.
// The ESP32 needs the computer's IP address.
const char* SERVER_URL = "http://172.20.10.2:5000";


// ============================================================
// PN532 RFID / NFC
// ============================================================

#define PN532_IRQ    4
#define PN532_RESET  5

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);


// ============================================================
// HX711 WEIGHT SENSOR
// ============================================================



Adafruit_HX711 wetMachine(26,13);
Adafruit_HX711 metalMachine(27,14);
Adafruit_HX711 paperMachine(18,19);
Adafruit_HX711 plasticMachine(32,33);


// ============================================================
// WEIGHT CALIBRATION
// ============================================================

// Based on your measurements:
//
// 0 g   -> -28543
// 217 g -> 39117
// 473 g -> 117406

// Weight calibration
const float plasticCF = 308;
const float plasticZero = -22700;
const float paperCF = 300;
const float paperZero = -679200;
const float metalCF = 302;
const float metalZero = -562800;
const float wetCF = 273;
const float wetZero = -1185600;


void setup() {
  Serial.begin(115200);

  delay(1000);


  // ----------------------------------------------------------
  // START SCREEN
  // ----------------------------------------------------------

  showHeader();


  // ----------------------------------------------------------
  // I2C
  // ----------------------------------------------------------

  Wire.begin(21, 22);


  // ----------------------------------------------------------
  // START HX711
  // ----------------------------------------------------------

  wetMachine.begin();
  metalMachine.begin();
  paperMachine.begin();
  plasticMachine.begin();


  // ----------------------------------------------------------
  // START PN532
  // ----------------------------------------------------------

  Serial.println("Starting RFID reader...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();


  if (!versiondata) {

    showError("RFID READER NOT FOUND");

    while (true) {
      delay(1000);
    }
  }


  nfc.SAMConfig();

  Serial.println("RFID reader ready.");
  Serial.println();


  // ----------------------------------------------------------
  // CONNECT TO WIFI
  // ----------------------------------------------------------

  connectToWiFi();


  // ----------------------------------------------------------
  // CHECK SERVER
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Checking server...");


  if (checkServer()) {

    Serial.println("Server connected.");

  }

  else {

    Serial.println("Server unavailable.");
    Serial.println("Please check the server.");
  }


  // ----------------------------------------------------------
  // READY
  // ----------------------------------------------------------

  showReadyScreen();
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop() {

  uint8_t uid[7];
  uint8_t uidLength;


  // ----------------------------------------------------------
  // WAIT FOR CARD
  // ----------------------------------------------------------

  bool success = nfc.readPassiveTargetID(
    PN532_MIFARE_ISO14443A,
    uid,
    &uidLength
  );


  if (!success) {
    return;
  }


  // ----------------------------------------------------------
  // CONVERT UID TO STRING
  // ----------------------------------------------------------

  String uidString = "";


  for (uint8_t i = 0; i < uidLength; i++) {

    if (uid[i] < 0x10) {
      uidString += "0";
    }

    uidString += String(uid[i], HEX);


    if (i < uidLength - 1) {
      uidString += ":";
    }
  }


  uidString.toUpperCase();


  // ----------------------------------------------------------
  // CARD DETECTED
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("----------------------------------------");
  Serial.println("CARD DETECTED");
  Serial.println("----------------------------------------");

  Serial.print("Card ID : ");
  Serial.println(uidString);

  Serial.println();


  // ----------------------------------------------------------
  // READ WEIGHT
  // ----------------------------------------------------------

  Serial.println("Reading weight...");


  int32_t rawValue1 = wetMachine.readChannelBlocking(
  CHAN_A_GAIN_128
  );
  int32_t rawValue2 = metalMachine.readChannelBlocking(
  CHAN_A_GAIN_128
  );
  int32_t rawValue3 = paperMachine.readChannelBlocking(
  CHAN_A_GAIN_128
  );
  int32_t rawValue4 = plasticMachine.readChannelBlocking(
  CHAN_A_GAIN_128
  );

  Serial.print("RAW1 = ");
  Serial.print(rawValue1);
  Serial.print(" | ");
  Serial.print(rawValue2);
  Serial.print(" | ");
  Serial.print(rawValue3);
  Serial.print(" | ");
  Serial.println(rawValue4);

  // Convert raw value to grams

  float wetGrams =
    (rawValue1 - wetZero) / wetCF;
  float metalGrams =
    (rawValue2 - metalZero) / metalCF;
  float paperGrams =
    (rawValue3 - paperZero) / paperCF;
  float plasticGrams =
    (rawValue4 - plasticZero) / plasticCF;


  // Prevent negative weight

  if (wetGrams < 0) {
    wetGrams = 0;
  }
  if (metalGrams < 0) {
    metalGrams = 0;
  }
  if (paperGrams < 0) {
    paperGrams = 0;
  }
  if (plasticGrams < 0) {
    plasticGrams = 0;
  }


  // Convert grams to kilograms

  float weightWetKg = wetGrams / 1000.0;
  float weightMetalKg = metalGrams / 1000.0;
  float weightPaperKg = paperGrams / 1000.0;
  float weightPlasticKg = plasticGrams / 1000.0;


  Serial.print("Weight Wet : ");
  Serial.print(weightWetKg, 3);
  Serial.print(" kg | Metal : ");
  Serial.print(weightMetalKg, 3);
  Serial.print(" kg | Paper : ");
  Serial.print(weightPaperKg, 3);
  Serial.print(" kg | Plastic");
  Serial.print(weightPlasticKg, 3);
  Serial.println(" kg");


  Serial.println();


  // ----------------------------------------------------------
  // SEND DATA TO SERVER
  // ----------------------------------------------------------

  Serial.println("Saving transaction...");


  bool uploaded = sendData(
    uidString,
    weightWetKg,
    weightMetalKg,
    weightPaperKg,
    weightPlasticKg
  );


  if (uploaded) {

    Serial.println();
    Serial.println("========================================");
    Serial.println("        TRANSACTION SAVED");
    Serial.println("========================================");

  }

  else {

    Serial.println();
    Serial.println("========================================");
    Serial.println("        TRANSACTION FAILED");
    Serial.println("========================================");

    Serial.println("Please try again.");
  }


  Serial.println();


  // ----------------------------------------------------------
  // WAIT BEFORE ACCEPTING ANOTHER CARD
  // ----------------------------------------------------------

  delay(3000);


  showReadyScreen();
}


// ============================================================
// WIFI CONNECTION
// ============================================================

void connectToWiFi() {

  String wifiName;
  String wifiPassword;


  Serial.println();
  Serial.println("========================================");
  Serial.println("             WIFI SETUP");
  Serial.println("========================================");
  Serial.println();


  // ----------------------------------------------------------
  // ASK FOR WIFI NAME
  // ----------------------------------------------------------

  Serial.println("Enter Wi-Fi / Hotspot name:");
  Serial.print("> ");


  while (Serial.available() == 0) {
    delay(100);
  }


  wifiName = Serial.readStringUntil('\n');

  wifiName.trim();


  // Clear remaining Serial data

  while (Serial.available()) {
    Serial.read();
  }


  // ----------------------------------------------------------
  // ASK FOR PASSWORD
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Enter Wi-Fi password:");
  Serial.print("> ");


  while (Serial.available() == 0) {
    delay(100);
  }


  wifiPassword = Serial.readStringUntil('\n');

  wifiPassword.trim();


  // Clear remaining Serial data

  while (Serial.available()) {
    Serial.read();
  }


  // ----------------------------------------------------------
  // CONNECT
  // ----------------------------------------------------------

  Serial.println();
  Serial.println("Connecting to Wi-Fi...");


  WiFi.begin(
    wifiName.c_str(),
    wifiPassword.c_str()
  );


  int attempts = 0;


  while (
    WiFi.status() != WL_CONNECTED &&
    attempts < 30
  ) {

    delay(500);

    Serial.print(".");

    attempts++;
  }


  Serial.println();


  // ----------------------------------------------------------
  // CONNECTION SUCCESS
  // ----------------------------------------------------------

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("Wi-Fi connected.");

  }

  else {

    Serial.println();
    Serial.println("Could not connect to Wi-Fi.");
    Serial.println();
    Serial.println("Restart the ESP32 and try again.");


    while (true) {
      delay(1000);
    }
  }
}


// ============================================================
// CHECK FLASK SERVER
// ============================================================

bool checkServer() {

  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }


  HTTPClient http;


  http.begin(SERVER_URL);


  int responseCode = http.GET();


  http.end();


  if (
    responseCode >= 200 &&
    responseCode < 300
  ) {

    return true;
  }


  return false;
}


// ============================================================
// SEND DATA TO FLASK SERVER
// ============================================================

bool sendData(
  String uid,
  float weight1,
  float weight2,
  float weight3,
  float weight4
) {

  // Check Wi-Fi

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi connection lost.");

    return false;
  }


  HTTPClient http;


  http.begin(SERVER_URL);


  http.addHeader(
    "Content-Type",
    "application/json"
  );


  // ----------------------------------------------------------
  // CREATE JSON
  // ----------------------------------------------------------

  String jsonData =
    "{\"uid\":\"" +
    uid +
    "\",\"weight1\":" +
    String(weight1, 3) +
    ",\"weight2\":" +
    String(weight2, 3) +
    ",\"weight3\":" +
    String(weight3, 3) +
    ",\"weight4\":" +
    String(weight4, 3) +
    "}";


  // ----------------------------------------------------------
  // SEND POST REQUEST
  // ----------------------------------------------------------

  int responseCode = http.POST(jsonData);


  // ----------------------------------------------------------
  // CHECK RESPONSE
  // ----------------------------------------------------------

  bool success = false;


  if (
    responseCode >= 200 &&
    responseCode < 300
  ) {

    success = true;
  }


  http.end();


  return success;
}


// ============================================================
// READY SCREEN
// ============================================================

void showReadyScreen() {

  Serial.println();
  Serial.println("========================================");
  Serial.println("          MACHINE READY");
  Serial.println("========================================");
  Serial.println();
  Serial.println("       PLEASE SCAN YOUR CARD");
  Serial.println();
}


// ============================================================
// HEADER
// ============================================================

void showHeader() {

  Serial.println();
  Serial.println();
  Serial.println("========================================");
  Serial.println("          SMART WASTE SYSTEM");
  Serial.println("========================================");
  Serial.println();
}


// ============================================================
// ERROR SCREEN
// ============================================================

void showError(String message) {

  Serial.println();
  Serial.println("========================================");
  Serial.println("                ERROR");
  Serial.println("========================================");
  Serial.println();
  Serial.println(message);
  Serial.println();
}

