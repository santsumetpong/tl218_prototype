const int pwmPin = 9;
const int sampleCount = 100;
float sineTable[sampleCount];

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < sampleCount; i++) {
    sineTable[i] = (sin(2 * PI * i / sampleCount) + 1) * 127.5;
  }
}

void loop() {
  for (int i = 0; i < sampleCount; i++) {
    int val = (int)sineTable[i];
    analogWrite(pwmPin, val);

    Serial.println(val);

    delayMicroseconds(500);
  }
}