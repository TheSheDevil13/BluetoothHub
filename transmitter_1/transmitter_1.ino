#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// Define service and characteristic UUIDs (same as receiver)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DEVICE_NAME "ESP32_Transmitter1"

BLEServer* pServer = NULL;
BLECharacteristic* pTemperatureCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long previousMillis = 0;
const long interval = 1000;  // Update temperature every 1 second

// Simulated temperature range (in Celsius)
float minTemp = 20.0;
float maxTemp = 30.0;

// Callback class for connection events
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
  Serial.println("BLE Transmitter 1 - Peripheral Device");

  // Initialize BLE
  BLEDevice::init(DEVICE_NAME);
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTemperatureCharacteristic = pService->createCharacteristic(
                      TEMPERATURE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Initial temperature value
  float initialTemp = (minTemp + maxTemp) / 2;
  char tempString[8];
  dtostrf(initialTemp, 5, 2, tempString);
  pTemperatureCharacteristic->setValue(tempString);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
  Serial.print(DEVICE_NAME);
  Serial.println(" is ready to connect");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Generate new temperature reading every interval
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Generate random temperature
    float temperature = random(minTemp * 100, maxTemp * 100) / 100.0;
    char tempString[8];
    dtostrf(temperature, 5, 2, tempString);
    
    Serial.print("Temperature: ");
    Serial.println(tempString);
    
    // Set the characteristic value
    pTemperatureCharacteristic->setValue(tempString);
    
    // Notify connected clients
    if (deviceConnected) {
      pTemperatureCharacteristic->notify();
    }
  }
  
  // Disconnection handling
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the Bluetooth stack time to get ready
    pServer->startAdvertising(); // Restart advertising
    Serial.println("Started advertising again");
    oldDeviceConnected = deviceConnected;
  }
  
  // Connection handling
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
