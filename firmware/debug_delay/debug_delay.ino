void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, INPUT_PULLUP);
  
  delay(1000);
}

void loop() {
  unsigned long measurements[10];
  
  for (int i = 0; i < 10; i++) {
    // Turn pin 2 ON for 2 seconds, then OFF
    digitalWrite(2, HIGH);
    delay(2000);
    digitalWrite(2, LOW);
    
    // Turn pin 13 ON and start timer
    digitalWrite(13, HIGH);
    unsigned long startTime = millis();
    
    // Poll pin 12 until it goes LOW
    while (digitalRead(12) == HIGH) {
      // Wait for pin 12 to go LOW
    }
    
    // Calculate elapsed time
    measurements[i] = millis() - startTime;
    
    // Print the result
    Serial.print("Iteration ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(measurements[i]);
    Serial.println(" ms");
    
    digitalWrite(13, LOW);
    delay(500);
  }
  
  // Calculate mean
  float sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += measurements[i];
  }
  float mean = sum / 10.0;
  
  // Calculate standard deviation
  float variance = 0;
  for (int i = 0; i < 10; i++) {
    float diff = measurements[i] - mean;
    variance += diff * diff;
  }
  variance /= 10.0;
  float stdDev = sqrt(variance);
  
  // Print statistics
  Serial.println("\n=== Results ===");
  Serial.print("Mean: ");
  Serial.print(mean, 2);
  Serial.println(" ms");
  Serial.print("Standard Deviation: ");
  Serial.print(stdDev, 2);
  Serial.println(" ms");
  
  while(1);  // Stop after completion
}