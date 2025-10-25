/*
 * Parallel Binary Reader with Clock Generation and Overflow Detection
 * Modified for multiple iterations with statistical analysis
 * 
 * This sketch:
 * 1. Runs multiple measurement iterations
 * 2. For each iteration:
 *    - Resets external device using pin 2 (HIGH for 2s, then LOW)
 *    - Generates a square wave clock on pin 13 for specified duration
 *    - Monitors a 14-bit parallel counter for overflows during counting
 *    - Reads final value and calculates total count including overflows
 * 3. Calculates and prints mean and standard deviation
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
#define CLOCK_FREQUENCY (1UL << 13)  // Do not go over ~50KHz
#define RESET_DURATION 1000      // Reset pulse duration in ms
#define STABILIZE_DELAY 1000     // Stabilization delay in ms
#define CLOCK_DURATION 23000     // Clock generation duration in ms. NEVER below 21s

// Statistical measurement parameters
#define NUMBER_ITERATIONS 1000    // Number of measurement iterations

// ===== GLOBAL VARIABLES =====
unsigned long halfPeriod;        // Half period of clock in microseconds
unsigned long clockPulseCount;   // Counter for clock pulses
unsigned long overflowCount;     // Counter for 14-bit overflows
bool previousMSB;                // Previous state of MSB for overflow detection
bool previousB9;
bool bit10;

// Storage for statistical analysis
unsigned long measurements[NUMBER_ITERATIONS];
int currentIteration = 0;

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

// Function to read only the MSB (bit 13) for overflow detection
bool readMSB() {
  return digitalRead(PIN_Q13);
}

bool readBit9() {
  return digitalRead(PIN_Q9);
}

// Function to perform one measurement iteration
unsigned long performMeasurement() {
  // Step 1: Reset sequence
  digitalWrite(RESET_PIN, HIGH);
  delay(RESET_DURATION);
  digitalWrite(RESET_PIN, LOW);
  
  // Step 2: Stabilization delay
  delay(STABILIZE_DELAY);
  
  // Initialize overflow detection
  overflowCount = 0;
  previousMSB = readMSB();
  previousB9 = readBit9();
  bit10 = false;
  
  // Step 3: Clock generation with overflow monitoring
  clockPulseCount = 0;
  unsigned long clockStart = millis();
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
        bool currentMSB = readMSB();
  
        // Detect overflow: MSB transitions from 1 to 0
        if (previousMSB == 1 && currentMSB == 0) {
          overflowCount++;
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
        // Check for overflow after rising edge
        delayMicroseconds(2); // Small delay to let counter settle
      }
    }
  }
  
  // Stop clock
  digitalWrite(CLOCK_PIN, LOW);
  
  // Step 4: Read final value
  delay(100); // Allow counter to settle
  unsigned int finalReading = readParallelValue();
  
  // Calculate total count including overflows
  // Each overflow represents 2^14 = 16384 counts
  unsigned long totalCount = (overflowCount * 16384UL) + finalReading;
  
  return totalCount;
}

// Function to calculate mean
double calculateMean() {
  double sum = 0.0;
  for (int i = 0; i < NUMBER_ITERATIONS; i++) {
    sum += measurements[i];
  }
  return sum / NUMBER_ITERATIONS;
}

// Function to calculate standard deviation
double calculateVariance(double mean) {
  double sumSquaredDiff = 0.0;
  for (int i = 0; i < NUMBER_ITERATIONS; i++) {
    double diff = measurements[i] - mean;
    sumSquaredDiff += diff * diff;
  }
  return sumSquaredDiff / (NUMBER_ITERATIONS - 1);
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB)
  }
  
  Serial.println("Parallel Binary Reader - Statistical Measurement Mode");
  Serial.print("Clock frequency: ");
  Serial.print(CLOCK_FREQUENCY);
  Serial.println(" Hz");
  Serial.print("Running ");
  Serial.print(NUMBER_ITERATIONS);
  Serial.println(" iterations...");
  Serial.println();
  
  // Calculate clock timing
  halfPeriod = (1000000UL / CLOCK_FREQUENCY) / 2;
  
  // Configure reset pin
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

  Serial.println();
  Serial.println("CSV fiel with individual results:");
  Serial.println();
  Serial.print("#f_clk: ");
  Serial.println(CLOCK_FREQUENCY);
  Serial.println("iteration,n_ticks");
  
  // Perform all measurements
  for (int i = 0; i < NUMBER_ITERATIONS; i++) {
    /*
    Serial.print("Iteration ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(NUMBER_ITERATIONS);
    Serial.print("... ");
    
    measurements[i] = performMeasurement();
    
    Serial.print("Count: ");
    Serial.println(measurements[i]);
    */

    measurements[i] = performMeasurement();

    Serial.print(i + 1);
    Serial.print(",");
    Serial.println(measurements[i]);

  }
  
  Serial.println();
  Serial.println();
  Serial.println("========================================");
  Serial.println("STATISTICAL RESULTS");
  Serial.println("========================================");
  
  // Calculate statistics
  double mean = calculateMean();
  double var = calculateVariance(mean);
  
  Serial.print("Number of iterations: ");
  Serial.println(NUMBER_ITERATIONS);
  Serial.print("Mean: ");
  Serial.println(mean, 2);
  Serial.print("Variance: ");
  Serial.println(var, 2);
  Serial.print("Standard deviation: ");
  Serial.println(sqrt(var), 2);
  Serial.print("R approx: ");
  Serial.println(mean/var, 2);
  Serial.println("========================================");
}

void loop() {
  // Nothing to do in loop
}
