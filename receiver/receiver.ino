#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

// The transmitter details
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define TRANSMITTER_DEVICE_NAME "ESP32_Transmitter1"

// Global variables
BLEClient* pClient = NULL;
BLERemoteCharacteristic* pTemperatureCharacteristic = NULL;
BLEAdvertisedDevice* transmitterDevice = NULL;
bool deviceConnected = false;

// Callback for notifications (when new temperature data arrives)
void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* data, size_t length, bool isNotify) {
  char tempStr[length + 1];
  memcpy(tempStr, data, length);
  tempStr[length] = '\0';
  
  Serial.print("Temperature: ");
  Serial.println(tempStr);
}

// Callback for finding BLE devices
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Only care about our specific transmitter
    if (advertisedDevice.getName() == TRANSMITTER_DEVICE_NAME) {
      Serial.print("Found transmitter: ");
      Serial.println(advertisedDevice.toString().c_str());
      
      // Stop scanning - we found what we're looking for
      BLEDevice::getScan()->stop();
      
      // Save a copy of the device
      transmitterDevice = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

// Connect to the transmitter
bool connectToTransmitter() {
  if (transmitterDevice == NULL) {
    Serial.println("No transmitter found to connect to");
    return false;
  }

  Serial.println("Connecting to the transmitter...");
  
  // Create a client
  pClient = BLEDevice::createClient();
  
  // Connect to the remote device
  if (!pClient->connect(transmitterDevice)) {
    Serial.println("Connection failed");
    return false;
  }
  
  Serial.println("Connected successfully!");
  
  // Get the service we want
  Serial.println("Looking for temperature service...");
  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find the temperature service");
    pClient->disconnect();
    return false;
  }
  
  // Get the characteristic we want
  Serial.println("Looking for temperature characteristic...");
  pTemperatureCharacteristic = pRemoteService->getCharacteristic(BLEUUID(TEMPERATURE_CHARACTERISTIC_UUID));
  if (pTemperatureCharacteristic == nullptr) {
    Serial.println("Failed to find the temperature characteristic");
    pClient->disconnect();
    return false;
  }
  
  // Read the current temperature value
  if (pTemperatureCharacteristic->canRead()) {
    String value = pTemperatureCharacteristic->readValue();
    Serial.print("Initial temperature: ");
    Serial.println(value.c_str());
  }
  
  // Set up notification callback
  if (pTemperatureCharacteristic->canNotify()) {
    pTemperatureCharacteristic->registerForNotify(notifyCallback);
    Serial.println("Notifications enabled");
  }
  
  deviceConnected = true;
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== ESP32 BLE Temperature Receiver ===");
  
  // Initialize BLE
  BLEDevice::init("ESP32_Receiver");
  
  // Set up scanning
  Serial.println("Scanning for the transmitter...");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(10); // Scan for 10 seconds
}

void loop() {
  // If we found the device but haven't connected yet
  if (transmitterDevice != NULL && !deviceConnected) {
    if (connectToTransmitter()) {
      Serial.println("Successfully connected and subscribed to temperature data!");
    } else {
      Serial.println("Failed to connect, will retry shortly...");
      delay(2000);
    }
  }
  
  // If we're connected, check connection status
  if (deviceConnected) {
    // Check if we're still connected
    if (!pClient->isConnected()) {
      Serial.println("Connection lost!");
      deviceConnected = false;
      
      // Restart scanning
      Serial.println("Scanning for the transmitter again...");
      BLEScan* pBLEScan = BLEDevice::getScan();
      pBLEScan->start(10);
    }
  }
  
  delay(1000);
}

