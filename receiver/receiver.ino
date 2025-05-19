#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "GREENz";
const char* password = "1234@ilic";
const char* serverURL = "http://172.16.0.124:5000/data"; 

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define HUMIDITY_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define TRANSMITTER1_NAME "ESP32_Transmitter1"
#define TRANSMITTER2_NAME "ESP32_Transmitter2"

BLEClient* pClient1 = NULL;
BLEClient* pClient2 = NULL;
BLERemoteCharacteristic* pTempChar1 = NULL;
BLERemoteCharacteristic* pHumidityChar1 = NULL;
BLERemoteCharacteristic* pTempChar2 = NULL;
BLERemoteCharacteristic* pHumidityChar2 = NULL;
BLEAdvertisedDevice* transmitter1Device = NULL;
BLEAdvertisedDevice* transmitter2Device = NULL;
bool device1Connected = false;
bool device2Connected = false;
unsigned long lastScanTime = 0;
const long scanInterval = 5000; 

void notifyCallback1(BLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
  char valueStr[length + 1];
  memcpy(valueStr, data, length);
  valueStr[length] = '\0';

  String sensorType = (pChar->getUUID().toString() == TEMPERATURE_CHARACTERISTIC_UUID) ? "temperature" : "humidity";
  
  Serial.print("Transmitter 1 - ");
  Serial.print(sensorType);
  Serial.print(": ");
  Serial.println(valueStr);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"device\": \"transmitter1\", \"type\": \"" + sensorType + "\", \"value\": \"" + String(valueStr) + "\"}";
    http.POST(payload);
    http.end();
  }
}

void notifyCallback2(BLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
  char valueStr[length + 1];
  memcpy(valueStr, data, length);
  valueStr[length] = '\0';

  String sensorType = (pChar->getUUID().toString() == TEMPERATURE_CHARACTERISTIC_UUID) ? "temperature" : "humidity";
  
  Serial.print("Transmitter 2 - ");
  Serial.print(sensorType);
  Serial.print(": ");
  Serial.println(valueStr);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"device\": \"transmitter2\", \"type\": \"" + sensorType + "\", \"value\": \"" + String(valueStr) + "\"}";
    http.POST(payload);
    http.end();
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found device: ");
    Serial.println(advertisedDevice.getName().c_str());
    
    if (advertisedDevice.getName() == TRANSMITTER1_NAME) {
      Serial.println("Found Transmitter 1!");
      BLEDevice::getScan()->stop();
      transmitter1Device = new BLEAdvertisedDevice(advertisedDevice);
    }
    else if (advertisedDevice.getName() == TRANSMITTER2_NAME) {
      Serial.println("Found Transmitter 2!");
      BLEDevice::getScan()->stop();
      transmitter2Device = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

bool connectToTransmitter1() {
  if (transmitter1Device == NULL) {
    Serial.println("Transmitter 1 device not found!");
    return false;
  }

  Serial.println("Connecting to Transmitter 1...");
  pClient1 = BLEDevice::createClient();
  if (!pClient1->connect(transmitter1Device)) {
    Serial.println("Failed to connect to Transmitter 1");
    return false;
  }
  Serial.println("Connected to Transmitter 1");

  BLERemoteService* pRemoteService = pClient1->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service on Transmitter 1");
    pClient1->disconnect();
    return false;
  }

  pTempChar1 = pRemoteService->getCharacteristic(BLEUUID(TEMPERATURE_CHARACTERISTIC_UUID));
  pHumidityChar1 = pRemoteService->getCharacteristic(BLEUUID(HUMIDITY_CHARACTERISTIC_UUID));
  
  if (pTempChar1 == nullptr || pHumidityChar1 == nullptr) {
    Serial.println("Failed to find characteristics on Transmitter 1");
    pClient1->disconnect();
    return false;
  }

  if (pTempChar1->canNotify()) {
    pTempChar1->registerForNotify(notifyCallback1);
  }
  if (pHumidityChar1->canNotify()) {
    pHumidityChar1->registerForNotify(notifyCallback1);
  }

  device1Connected = true;
  Serial.println("Transmitter 1 setup complete");
  return true;
}

bool connectToTransmitter2() {
  if (transmitter2Device == NULL) {
    Serial.println("Transmitter 2 device not found!");
    return false;
  }

  Serial.println("Connecting to Transmitter 2...");
  pClient2 = BLEDevice::createClient();
  if (!pClient2->connect(transmitter2Device)) {
    Serial.println("Failed to connect to Transmitter 2");
    return false;
  }
  Serial.println("Connected to Transmitter 2");

  BLERemoteService* pRemoteService = pClient2->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service on Transmitter 2");
    pClient2->disconnect();
    return false;
  }

  pTempChar2 = pRemoteService->getCharacteristic(BLEUUID(TEMPERATURE_CHARACTERISTIC_UUID));
  pHumidityChar2 = pRemoteService->getCharacteristic(BLEUUID(HUMIDITY_CHARACTERISTIC_UUID));
  
  if (pTempChar2 == nullptr || pHumidityChar2 == nullptr) {
    Serial.println("Failed to find characteristics on Transmitter 2");
    pClient2->disconnect();
    return false;
  }

  if (pTempChar2->canNotify()) {
    pTempChar2->registerForNotify(notifyCallback2);
  }
  if (pHumidityChar2->canNotify()) {
    pHumidityChar2->registerForNotify(notifyCallback2);
  }

  device2Connected = true;
  Serial.println("Transmitter 2 setup complete");
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 Receiver...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  BLEDevice::init("ESP32_Receiver");
  Serial.println("BLE initialized");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);
  Serial.println("Scanning for devices...");
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle Transmitter 1
  if (transmitter1Device != NULL && !device1Connected) {
    connectToTransmitter1();
  }

  if (device1Connected && !pClient1->isConnected()) {
    Serial.println("Transmitter 1 disconnected");
    device1Connected = false;
  }

  // Handle Transmitter 2
  if (transmitter2Device != NULL && !device2Connected) {
    connectToTransmitter2();
  }

  if (device2Connected && !pClient2->isConnected()) {
    Serial.println("Transmitter 2 disconnected");
    device2Connected = false;
  }

  // If either device is not found, scan periodically
  if ((transmitter1Device == NULL || transmitter2Device == NULL) && 
      (currentMillis - lastScanTime >= scanInterval)) {
    Serial.println("Scanning for missing devices...");
    BLEDevice::getScan()->start(10);
    lastScanTime = currentMillis;
  }

  delay(1000);
}

