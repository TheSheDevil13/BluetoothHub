#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define HUMIDITY_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define DEVICE_NAME "ESP32_Transmitter2"

BLEServer* pServer = NULL;
BLECharacteristic* pTemperatureCharacteristic = NULL;
BLECharacteristic* pHumidityCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 3000;  

float minTemp = 20.0;
float maxTemp = 30.0;
float minHumidity = 40.0;
float maxHumidity = 60.0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("BLE Transmitter 2 - Peripheral Device");

  BLEDevice::init(DEVICE_NAME);
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTemperatureCharacteristic = pService->createCharacteristic(
                      TEMPERATURE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pHumidityCharacteristic = pService->createCharacteristic(
                      HUMIDITY_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  float initialTemp = (minTemp + maxTemp) / 2;
  float initialHumidity = (minHumidity + maxHumidity) / 2;
  char tempString[8];
  char humidityString[8];
  dtostrf(initialTemp, 5, 2, tempString);
  dtostrf(initialHumidity, 5, 2, humidityString);
  pTemperatureCharacteristic->setValue(tempString);
  pHumidityCharacteristic->setValue(humidityString);

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.print(DEVICE_NAME);
  Serial.println(" is ready to connect");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    float temperature = random(minTemp * 100, maxTemp * 100) / 100.0;
    float humidity = random(minHumidity * 100, maxHumidity * 100) / 100.0;
    char tempString[8];
    char humidityString[8];
    dtostrf(temperature, 5, 2, tempString);
    dtostrf(humidity, 5, 2, humidityString);
    
    Serial.print("Temperature: ");
    Serial.println(tempString);
    Serial.print("Humidity: ");
    Serial.println(humidityString);
    
    pTemperatureCharacteristic->setValue(tempString);
    pHumidityCharacteristic->setValue(humidityString);
    
    if (deviceConnected) {
      pTemperatureCharacteristic->notify();
      pHumidityCharacteristic->notify();
    }
  }
  
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); 
    pServer->startAdvertising(); 
    Serial.println("Started advertising again");
    oldDeviceConnected = deviceConnected;
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
