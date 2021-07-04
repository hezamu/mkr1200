#include <SigFox.h>
#include <ArduinoLowPower.h>

#define DEBUG           false  // Enable SigFox debug features and send messages to Serial
#define SLEEPTIME       15 * 60 * 1000   // Set the delay to 15 minutes (15 min x 60 seconds x 1000 milliseconds)
#define UINT16_t_MAX    65536
#define INT16_t_MAX     UINT16_t_MAX / 2

/*
   Max payload lenght: 12 bytes uplink, 8 bytes downlink
   Max messages per day: 140 uplink, 4 downlink
   Example message payload: 7717 0000 e4c1 0000 0000 0000
*/
typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t lastMessageStatus;
  uint8_t unused1;
  int16_t moduleTemperature;
  uint16_t unused2;
  uint16_t unused3;
  uint16_t unused4;
  uint16_t unused5;
} SigfoxMessage;

SigfoxMessage msg; // Stub for message which will be sent

void setup() {
  if (DEBUG) {
    pinMode(6, OUTPUT);
    Serial.begin(9600);
    while (!Serial) {}
  }

  if (!SigFox.begin()) {
    if (DEBUG) Serial.println("Shield error or not present!");
    // Something is really wrong, try rebooting
    // Reboot is useful if we are powering the board using an unreliable power source
    // (eg. solar panels or other energy harvesting methods)
    reboot();
  }

  SigFox.end(); // Send module to standby until we need to send a message

  // Enable DEBUG prints and LED indication disable automatic deep sleep if we are testing
  if (DEBUG) {
    SigFox.debug();
  } else {
    SigFox.noDebug();
  }

  if (DEBUG) printModuleInfo();

  msg.unused1 = 0xaa;
  msg.unused2 = 0xbbbb;
  msg.unused3 = 0xcccc;
  msg.unused4 = 0xdddd;
  msg.unused5 = 0xeeee;
}

void loop() {
  if (DEBUG) digitalWrite(6, HIGH);

  if (!DEBUG) sendSigFoxMessage();

  if (DEBUG) {
    delay(100); // Make sure the debug led is on at least for 200ms
    digitalWrite(6, LOW);
  }

  if (!DEBUG) LowPower.deepSleep(SLEEPTIME);
}

void sendSigFoxMessage() {
  if (DEBUG) Serial.println(">>> sendSigFoxMessage()");

  SigFox.begin();

  delay(100); // Wait at least 30ms after first configuration (100ms before)

  // We can only read the module temperature before SigFox.end()
  float t = SigFox.internalTemperature();
  msg.moduleTemperature = convertoFloatToInt16(t, 60, -60);

  // Clears all pending interrupts
  SigFox.status();
  delay(1);

  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 12);

  msg.lastMessageStatus = SigFox.endPacket();
  if (DEBUG) Serial.println("Message sent, temp: " + String(t) + ", status: " + String(msg.lastMessageStatus));

  SigFox.end();

  if (DEBUG) Serial.println("<<< sendSigFoxMessage()");
}

void reboot() {
  NVIC_SystemReset();
  while (1) ;
}

int16_t convertoFloatToInt16(float value, long max, long min) {
  float conversionFactor = (float) (INT16_t_MAX) / (float)(max - min);
  return (int16_t)(value * conversionFactor);
}

uint16_t convertoFloatToUInt16(float value, long max) {
  float conversionFactor = (float) (UINT16_t_MAX) / (float)(max);
  return (uint16_t)(value * conversionFactor);
}

void printModuleInfo() {
  if (DEBUG) {
    String version = SigFox.SigVersion();
    String ID = SigFox.ID();
    String PAC = SigFox.PAC();  // Display module informations
    Serial.println("MKRFox1200 Sigfox first configuration");
    Serial.println("SigFox FW version " + version + ", ID  = " + ID + ". PAC = " + PAC);
    Serial.print("Module temperature: " + String(SigFox.internalTemperature()));
    //  Serial.println("Register your board on https://backend.sigfox.com/activate with provided ID and PAC");
  }
}
