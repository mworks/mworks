#define MESSAGE_LENGTH 4

#define MODE_MASK 0b111
#define MODE_OFF  0b000
#define MODE_CW   0b001
#define MODE_SW   0b010
#define MODE_ISW  0b011
#define MODE_SQW  0b100

#define CHANNEL_MASK 0b11000000


static byte message[MESSAGE_LENGTH];
static size_t bytesRead = 0;


static bool validateMessage() {
  // Check mode
  switch (message[0] & MODE_MASK) {
    case MODE_OFF:
    case MODE_CW:
    case MODE_SW:
    case MODE_ISW:
    case MODE_SQW:
      break;
    default:
      return false;
  }

  // All four possible values of the two bits for channel are allowed, so
  // we don't need to check them

  // Confirm that the usused bits of byte 1 are zero
  if (message[0] & ~(MODE_MASK | CHANNEL_MASK)) {
    return false;
  }

  // All values of bytes 2-4 are allowed, so we don't need to check them

  return true;
}


void setup() {
  Serial.begin(115200);
  while (!Serial) ;  // Wait for serial port to become ready
}


void loop() {
  if (Serial.available()) {
    message[bytesRead++] = Serial.read();
    if (bytesRead == MESSAGE_LENGTH) {
      if (validateMessage()) {
        // Message is valid.  Echo it back to the sender.
        Serial.write(message, MESSAGE_LENGTH);
      }
      bytesRead = 0;
    }
  }
}
