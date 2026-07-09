#define SOLARPIN A0
#define THRESHOLD 210

int ambientReading = 0;

void setup() {
  pinMode(SOLARPIN, INPUT);
  Serial.begin(9600);
}

void loop() {

  int reading = analogRead(SOLARPIN);
  int bits[8];

  // Listening for the start bit
  if (reading > THRESHOLD) {
    for (int i = 0; i < 8; i++) {
      if (analogRead(SOLARPIN) > THRESHOLD) {
        bits[i] = 1;
      }
      else {
        bits[i] = 0;
      }
      delay(10);   // matches transmitter's bit duration
    }

    // Convert the received 8-bit stream back to a decimal (ASCII) value
    int m = 0;
    for (int j = 1; j < 8; j++) {
      if (bits[j] == 1) {
        m = m + (1 << (7 - j));
      }
    }

    // Convert ASCII value back to character and print it
    char n = m;
    Serial.print(n);
  }
}
