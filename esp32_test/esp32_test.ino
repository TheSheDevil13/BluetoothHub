/*
 * ESP32 C3 Mini Basic Test
 * This code tests if serial communication is working
 */

void setup() {
  // Delay for ESP32 to fully boot
  delay(3000);
  
  // Initialize serial
  Serial.begin(115200);
  delay(500);
  
  // Print repeated startup messages
  for (int i = 0; i < 10; i++) {
    Serial.println("ESP32 STARTUP TEST");
    delay(200);
  }
  
  Serial.println("Setup complete");
}

void loop() {
  // Simple loop counter
  static int counter = 0;
  
  // Print a message
  Serial.print("Loop count: ");
  Serial.println(counter++);
  
  // Use a simple delay
  delay(1000);
} 