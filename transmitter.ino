#define LASERPIN 7

void setup() {
  pinMode(LASERPIN, OUTPUT);

  char myText[] = "A quick brown fox jumped over the lazy dog";
  int length = sizeof(myText);
  int ar[50];
  int m;
  int bits[8];

  // Convert each character to its ASCII value
  for (int i = 0; i < length; i++) {
    ar[i] = int(myText[i]);
  }

  // Process each character
  for (int n = 0; n < length; n++) {
    m = ar[n];
    int z;
    int bin[7];
    int newbin[7];

    // Convert ASCII value to binary (LSB first)
    for (z = 0; z < 8; z++) {
      bin[z] = m % 2;
      m = m / 2;
    }

    // Reverse to get MSB first
    for (int j = 7; j >= 0; j--) {
      newbin[7 - j] = bin[j];
    }

    // Map binary bits to HIGH/LOW pulses
    for (int p = 0; p < 8; p++) {
      if (newbin[p] == 1) {
        bits[p] = HIGH;
      }
      if (newbin[p] == 0) {
        bits[p] = LOW;
      }
    }
    bits[0] = HIGH;   // start bit

    // Transmit the 8 bits via LASER flicker
    for (int i = 0; i < 8; i++) {
      digitalWrite(LASERPIN, bits[i]);
      delay(10);
    }

    digitalWrite(LASERPIN, LOW);
    delay(100);   // gap before next character
  }
}

void loop() {
  // Transmission happens once in setup()
}