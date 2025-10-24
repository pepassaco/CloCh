/*
 * Parallel Binary Reader with Clock Generation and Overflow Detection
 * 
 * This sketch:
 * 1. Resets an external device using pin 2 (HIGH for 2s, then LOW)
 * 2. Generates a square wave clock on pin 13 for specified duration
 * 3. Monitors a 14-bit parallel counter for overflows during counting
 * 4. Reads final value and calculates total count including overflows
 * 
 * Pin Mapping:
 * - Pin 2:  Reset output
 * - Pin 13: Clock output
 * - D12: Q3  (bit 3)
 * - D11: Q4  (bit 4)
 * - D10: Q5  (bit 5)
 * - D9:  Q6  (bit 6)
 * - D8:  Q7  (bit 7)
 * - D7:  Q8  (bit 8)
 * - D6:  Q9  (bit 9)
 * - D5:  Q11 (bit 11)
 * - D4:  Q12 (bit 12)
 * - D3:  Q13 (bit 13) - MSB for overflow detection
 * 
 * Bits 0, 1, 2, and 10 are assumed to be 0
 */

// ===== CONFIGURATION =====
#define CLOCK_PIN 13          // Pin for clock output
#define RESET_PIN 2           // Pin for reset signal

// Pin definitions for parallel input (bit position → pin number)
#define PIN_Q3  12  // Bit 3
#define PIN_Q4  11  // Bit 4
#define PIN_Q5  10  // Bit 5
#define PIN_Q6  9   // Bit 6
#define PIN_Q7  8   // Bit 7
#define PIN_Q8  7   // Bit 8
#define PIN_Q9  6   // Bit 9
#define PIN_Q11 5   // Bit 11
#define PIN_Q12 4   // Bit 12
#define PIN_Q13 3   // Bit 13 (MSB)

// Timing constants
#define CLOCK_FREQUENCY (1UL << 15)  // Clock frequency in Hz (adjust as needed)
#define RESET_DURATION 4000      // Reset pulse duration in ms
#define STABILIZE_DELAY 1000     // Stabilization delay in ms
#define CLOCK_DURATION 23000     // Clock generation duration in ms

// ===== GLOBAL VARIABLES =====
unsigned long halfPeriod;        // Half period of clock in microseconds
unsigned long clockPulseCount;   // Counter for clock pulses
unsigned long overflowCount;     // Counter for 14-bit overflows
bool previousMSB;                // Previous state of MSB for overflow detection
bool previousB9;
bool bit10;

// Function to read the parallel counter value
unsigned int readParallelValue() {
  int bit3 = digitalRead(PIN_Q3);
  int bit4 = digitalRead(PIN_Q4);
  int bit5 = digitalRead(PIN_Q5);
  int bit6 = digitalRead(PIN_Q6);
  int bit7 = digitalRead(PIN_Q7);
  int bit8 = digitalRead(PIN_Q8);
  int bit9 = digitalRead(PIN_Q9);
  int _bit10 = 0;
  if(bit10){
    _bit10 = 1;
  }
  int bit11 = digitalRead(PIN_Q11);
  int bit12 = digitalRead(PIN_Q12);
  int bit13 = digitalRead(PIN_Q13);
  
  unsigned int value = 0;
  value |= (bit3 << 3);
  value |= (bit4 << 4);
  value |= (bit5 << 5);
  value |= (bit6 << 6);
  value |= (bit7 << 7);
  value |= (bit8 << 8);
  value |= (bit9 << 9);
  value |= (_bit10 << 10);
  value |= (bit11 << 11);
  value |= (bit12 << 12);
  value |= (bit13 << 13);
  
  return value;
}

bool readMSB() {
  return digitalRead(PIN_Q13);
}

bool readBit9() {
  return digitalRead(PIN_Q9);
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB)
  }
  
  Serial.println("========================================");
  Serial.println("Parallel Binary Reader with Overflow Detection");
  Serial.println("========================================");
  Serial.println();
  
  // Calculate clock timing
  halfPeriod = (1000000UL / CLOCK_FREQUENCY) / 2;
  Serial.print("Clock Configuration:");
  Serial.println();
  Serial.print("  Frequency: ");
  Serial.print(CLOCK_FREQUENCY);
  Serial.println(" Hz");
  Serial.print("  Half Period: ");
  Serial.print(halfPeriod);
  Serial.println(" microseconds");
  Serial.println();
  
  // Configure reset pin
  Serial.println("Configuring pins...");
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  
  // Configure clock pin
  pinMode(CLOCK_PIN, OUTPUT);
  digitalWrite(CLOCK_PIN, LOW);
  
  // Configure input pins for parallel reading
  pinMode(PIN_Q3, INPUT);
  pinMode(PIN_Q4, INPUT);
  pinMode(PIN_Q5, INPUT);
  pinMode(PIN_Q6, INPUT);
  pinMode(PIN_Q7, INPUT);
  pinMode(PIN_Q8, INPUT);
  pinMode(PIN_Q9, INPUT);
  pinMode(PIN_Q11, INPUT);
  pinMode(PIN_Q12, INPUT);
  pinMode(PIN_Q13, INPUT);
  
  Serial.println("All pins configured");
  Serial.println();
  
  // Step 1: Reset sequence
  Serial.println("========================================");
  Serial.println("STEP 1: Resetting external device");
  Serial.println("========================================");
  
  digitalWrite(RESET_PIN, HIGH);
  delay(RESET_DURATION);
  digitalWrite(RESET_PIN, LOW);
  
  Serial.println("Reset sequence complete!");
  Serial.println();
  
  // Step 2: Stabilization delay
  Serial.println("========================================");
  Serial.println("STEP 2: Stabilization delay");
  Serial.println("========================================");
  delay(STABILIZE_DELAY);
  Serial.println("Stabilization complete!");
  Serial.println();
  
  // Initialize overflow detection
  overflowCount = 0;
  unsigned int initialValue = readParallelValue();
  previousMSB = (initialValue >> 13) & 1;
  previousB9 = (initialValue >> 9) & 1;
  bit10 = false;


  Serial.println("Initial counter state:");
  Serial.print("  Value: ");
  Serial.println(initialValue);
  Serial.print("  MSB (bit 13): ");
  Serial.println(previousMSB);
  Serial.print("  Bit 9: ");
  Serial.println(previousB9);
  Serial.println();
  
  // Step 3: Clock generation with overflow monitoring
  Serial.println("========================================");
  Serial.println("STEP 3: Generating clock and monitoring");
  Serial.println("========================================");
  Serial.print("Starting clock at ");
  Serial.print(CLOCK_FREQUENCY);
  Serial.println(" Hz");
  Serial.print("Duration: ");
  Serial.print(CLOCK_DURATION / 1000);
  Serial.println(" seconds");
  Serial.println();
  
  clockPulseCount = 0;
  unsigned long clockStart = millis();
  unsigned long lastStatusUpdate = millis();
  unsigned long nextToggle = micros();
  bool clockState = false;
  
  while (millis() - clockStart < CLOCK_DURATION) {
    // Generate clock signal
    if (micros() >= nextToggle) {
      clockState = !clockState;
      digitalWrite(CLOCK_PIN, clockState);
      nextToggle += halfPeriod;
      
      if (clockState == true) {
        clockPulseCount++;
        
        /*
        Serial.print("V: ");
        Serial.print(readParallelValue());
        Serial.print(" at: ");
        Serial.println(millis() - clockStart);
        */
        
        // Check for overflow after rising edge
        bool currentMSB = readMSB();
  
        // Detect overflow: MSB transitions from 1 to 0
        if (previousMSB == 1 && currentMSB == 0) {
          overflowCount++;
          Serial.print("*** OVERFLOW DETECTED at pulse ");
          Serial.print(clockPulseCount);
          Serial.print(" | Total overflows: ");
          Serial.println(overflowCount);
        }
        previousMSB = currentMSB;

        // Check for B9 after rising edge
        bool currentB9 = readBit9();
  
        // Detect B10: B9 transitions from 1 to 0
        if (previousB9 == 1 && currentB9 == 0) {
          bit10 = !bit10;
        }
        previousB9 = currentB9;


        

      }else{
        delayMicroseconds(2); // Small delay to let counter settle
      }
    }
    
    // Status update every 5 seconds
    if (millis() - lastStatusUpdate >= 5000) {
      unsigned int currentValue = readParallelValue();
      Serial.print("  Status: ");
      Serial.print((millis() - clockStart) / 1000);
      Serial.print("s | Pulses: ");
      Serial.print(clockPulseCount);
      Serial.print(" | Overflows: ");
      Serial.print(overflowCount);
      Serial.print(" | Current: ");
      Serial.println(currentValue);
      lastStatusUpdate = millis();
    }
  }
  
  // Stop clock
  digitalWrite(CLOCK_PIN, LOW);
  Serial.println();
  Serial.println("Clock generation complete!");
  Serial.println();
  
  // Step 4: Read final value
  Serial.println("========================================");
  Serial.println("STEP 4: Reading final value");
  Serial.println("========================================");
  
  delay(100); // Allow counter to settle
  unsigned int finalReading = readParallelValue();

  digitalWrite(RESET_PIN, HIGH);
  
  // Calculate total count including overflows
  // Each overflow represents 2^14 = 16384 counts
  unsigned long totalCount = (overflowCount * 16384UL) + finalReading;
  
  Serial.println("Final Reading:");
  Serial.print("  Raw counter value: ");
  Serial.println(finalReading);
  Serial.print("  Binary: 0b");
  for (int i = 13; i >= 0; i--) {
    Serial.print((finalReading >> i) & 1);
    if (i % 4 == 0 && i != 0) Serial.print("_");
  }
  Serial.println();
  Serial.print("  Hexadecimal: 0x");
  Serial.println(finalReading, HEX);
  Serial.println();
  
  Serial.println("========================================");
  Serial.println("FINAL RESULTS");
  Serial.println("========================================");
  Serial.print("Clock pulses generated: ");
  Serial.println(clockPulseCount);
  Serial.print("Overflows detected: ");
  Serial.println(overflowCount);
  Serial.print("Final counter reading: ");
  Serial.println(finalReading);
  Serial.println();
  Serial.print("TOTAL COUNT (with overflows): ");
  Serial.println(totalCount);
  Serial.println();
  
  Serial.println("========================================");
  Serial.println("All operations complete!");
  Serial.println("========================================");
}

void loop() {
  // Nothing to do in loop
}
