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
#define TRANSMITTER_DEVICE_NAME "ESP32_Transmitter1"

BLEClient* pClient = NULL;
BLERemoteCharacteristic* pTemperatureCharacteristic = NULL;
BLEAdvertisedDevice* transmitterDevice = NULL;
bool deviceConnected = false;

void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
  char tempStr[length + 1];
  memcpy(tempStr, data, length);
  tempStr[length] = '\0';

  Serial.print("Temperature: ");
  Serial.println(tempStr);

    if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"temperature\": \"" + String(tempStr) + "\"}";
    http.POST(payload);
    http.end();
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == TRANSMITTER_DEVICE_NAME) {
      BLEDevice::getScan()->stop();
      transmitterDevice = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

bool connectToTransmitter() {
  if (transmitterDevice == NULL) return false;

  pClient = BLEDevice::createClient();
  if (!pClient->connect(transmitterDevice)) return false;

  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }

  pTemperatureCharacteristic = pRemoteService->getCharacteristic(BLEUUID(TEMPERATURE_CHARACTERISTIC_UUID));
  if (pTemperatureCharacteristic == nullptr) {
    pClient->disconnect();
    return false;
  }

  if (pTemperatureCharacteristic->canNotify()) {
    pTemperatureCharacteristic->registerForNotify(notifyCallback);
  }

  deviceConnected = true;
  return true;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  BLEDevice::init("ESP32_Receiver");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10);
}

void loop() {
  if (transmitterDevice != NULL && !deviceConnected) {
    connectToTransmitter();
  }

  if (deviceConnected && !pClient->isConnected()) {
    deviceConnected = false;
    BLEDevice::getScan()->start(10);
  }

  delay(1000);
}

