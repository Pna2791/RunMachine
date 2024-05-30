int latchPin = 19;
int clockPin = 22;
int dataPin = 23;

// int latchPin = 5;
// int clockPin = 6;
// int dataPin = 4;


void setup() {
    Serial.begin(115200);
    pinMode(latchPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
}
void loop() {
    // count from 0 to 255 and display the number
    // on the LEDs
    for (byte numberToDisplay = 0; numberToDisplay < 256; numberToDisplay++) {
        digitalWrite(latchPin, 0);
        shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);
        digitalWrite(latchPin, 1);
        Serial.println(numberToDisplay);
        delay(100);
    }
}