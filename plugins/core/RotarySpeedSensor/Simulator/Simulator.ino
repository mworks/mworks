#define START_BYTE 255

#define RPM_MIN 0
#define RPM_MAX (START_BYTE - 1)

#define MESSAGE_INTERVAL_MS 50  // 20 messages per second


static unsigned long nextMessageTimeMS = 0;
static int currentDirection = 1;
static int currentRPM = 1;


void setup() {
    Serial.begin(9600);
    while (!Serial) ;  // Wait for serial port to become ready
}


void loop() {
    unsigned long currentTimeMS = millis();
    if (currentTimeMS >= nextMessageTimeMS) {
        Serial.write(START_BYTE);
        Serial.write((currentDirection + 1) / 2);
        Serial.write(currentRPM);

        if (currentRPM == RPM_MIN || currentRPM == RPM_MAX) {
            currentDirection *= -1;
        }
        currentRPM += currentDirection;

        nextMessageTimeMS = currentTimeMS + MESSAGE_INTERVAL_MS;
    }
}
