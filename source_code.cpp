// ========================================================
// Fixed 6-Digit Clock Code (HH:MM:SS Format & Single Increments)
// ========================================================

// 1. Decoder Pin Definitions (BCD)
const int pinA = 2; // Input A (LSB)
const int pinB = 3; // Input B
const int pinC = 4; // Input C
const int pinD = 5; // Input D (MSB)

// 2. Digit Enable Pins (Left to Right: HH MM SS)
const int digitPins[6] = {11, 10, 9, 8, 7, 6};

// 3. Pushbutton Pins (Active HIGH)
const int btnSelect = 12; // Button 1: Mode/Select
const int btnInc    = A2; // Button 2: Increment (+)
const int btnDec    = A0; // Button 3: Decrement (-)
const int btnPause  = A1; // Button 4: Pause / Resume

// 4. Timekeeping Variables
int hours   = 12; // Start at 12
int minutes = 0;
int seconds = 0;

// 5. System States
int editMode = 0;   // 0 = Normal Run, 1 = Edit Hours, 2 = Edit Minutes, 3 = Edit Seconds
bool isPaused = false;

// Button state tracking for edge detection (Prevents +10 jumps)
bool lastSelectState = LOW;
bool lastIncState    = LOW;
bool lastDecState    = LOW;
bool lastPauseState  = LOW;

unsigned long previousMillis = 0;
const long interval = 1000; 

void setup() {
  // Turn off internal LED on Pin 13
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // BCD Decoder Pins
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);

  // Digit Enable Pins
  for (int i = 0; i < 6; i++) {
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], LOW);
  }

  // Button Inputs
  pinMode(btnSelect, INPUT);
  pinMode(btnInc, INPUT);
  pinMode(btnDec, INPUT);
  pinMode(btnPause, INPUT);
}

void loop() {
  checkButtons();

  // 1-Second Timekeeping Counter Logic
  if (!isPaused && editMode == 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      
      seconds++;
      if (seconds >= 60) {
        seconds = 0;
        minutes++;
        if (minutes >= 60) {
          minutes = 0;
          hours++;
          if (hours >= 24) { // Strictly caps hours at 24 (0-23)
            hours = 0;
          }
        }
      }
    }
  }

  refreshDisplay();
}

void writeBCD(int val) {
  digitalWrite(pinA, (val & 0x01) ? HIGH : LOW);
  digitalWrite(pinB, (val & 0x02) ? HIGH : LOW);
  digitalWrite(pinC, (val & 0x04) ? HIGH : LOW);
  digitalWrite(pinD, (val & 0x08) ? HIGH : LOW);
}

// Fixed Button Handling Logic (Triggers ONLY once per press)
void checkButtons() {
  bool currentSelect = digitalRead(btnSelect);
  bool currentInc    = digitalRead(btnInc);
  bool currentDec    = digitalRead(btnDec);
  bool currentPause  = digitalRead(btnPause);

  // Button 1: Select / Mode (Triggers on press transition LOW -> HIGH)
  if (currentSelect == HIGH && lastSelectState == LOW) {
    editMode = (editMode + 1) % 4;
    delay(50); // Small debounce
  }

  // Button 2: Increment (+1)
  if (currentInc == HIGH && lastIncState == LOW) {
    if (editMode == 1) hours = (hours + 1) % 24;       // 0 to 23
    else if (editMode == 2) minutes = (minutes + 1) % 60; // 0 to 59
    else if (editMode == 3) seconds = (seconds + 1) % 60; // 0 to 59
    delay(50);
  }

  // Button 3: Decrement (-1)
  if (currentDec == HIGH && lastDecState == LOW) {
    if (editMode == 1) hours = (hours == 0) ? 23 : hours - 1;
    else if (editMode == 2) minutes = (minutes == 0) ? 59 : minutes - 1;
    else if (editMode == 3) seconds = (seconds == 0) ? 59 : seconds - 1;
    delay(50);
  }

  // Button 4: Pause / Resume
  if (currentPause == HIGH && lastPauseState == LOW) {
    isPaused = !isPaused;
    delay(50);
  }

  // Store current states for next check
  lastSelectState = currentSelect;
  lastIncState    = currentInc;
  lastDecState    = currentDec;
  lastPauseState  = currentPause;
}

// Display Multiplexing HH MM SS
void refreshDisplay() {
  int digits[6];
  
  // Digit Assignment strictly HH MM SS (Left to Right)
  digits[0] = hours / 10;   // Hour Tens
  digits[1] = hours % 10;   // Hour Ones
  digits[2] = minutes / 10; // Min Tens
  digits[3] = minutes % 10; // Min Ones
  digits[4] = seconds / 10; // Sec Tens
  digits[5] = seconds % 10; // Sec Ones

  for (int i = 0; i < 6; i++) {
    // Visual Blinking indicator during Edit Mode
    if (editMode == 1 && (i == 0 || i == 1) && (millis() % 400 < 200)) continue;
    if (editMode == 2 && (i == 2 || i == 3) && (millis() % 400 < 200)) continue;
    if (editMode == 3 && (i == 4 || i == 5) && (millis() % 400 < 200)) continue;

    writeBCD(digits[i]);
    digitalWrite(digitPins[i], HIGH);
    delayMicroseconds(2000); 
    digitalWrite(digitPins[i], LOW);
  }
}
