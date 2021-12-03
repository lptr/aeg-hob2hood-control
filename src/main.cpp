#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint8_t irPin = D5;

// IR commands from AEG hob2hood device
const uint64_t IRCMD_VENT_1 = 0xE3C01BE2;       // Hob2hood On (level 1)
const uint64_t IRCMD_VENT_2 = 0xD051C301;       // Hob2hood level 2
const uint64_t IRCMD_VENT_3 = 0xC22FFFD7;       // Hob2hood level 3
const uint64_t IRCMD_VENT_4 = 0xB9121B29;       // Hob2hood level 4
const uint64_t IRCMD_VENT_OFF = 0x55303A3;      // Hob2hood off
const uint64_t IRCMD_LIGHT_ON = 0xE208293C;     // Light on (Hood on)
const uint64_t IRCMD_LIGHT_OFF = 0x24ACF947;    // Light off (Automatic after 2min)

IRrecv irReceiver(irPin);

const uint8_t dataInPin = D7;
const uint8_t dataOutPin = D8;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(dataInPin, INPUT);
    pinMode(dataOutPin, OUTPUT);

    Serial.begin(115200);
    while (!Serial) {
        delay(50);
    }

    irReceiver.enableIRIn();

    Serial.println();
    Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
    Serial.println(irPin);
}

bool handleIrCode(const decode_results& results) {
    Serial.print("Received: ");
    serialPrintUint64(results.value, HEX);
    Serial.print(" (");
    Serial.print(results.bits, DEC);
    Serial.println(" bits)");
    Serial.println();

    Serial.print("> ");
    switch (results.value) {
        case IRCMD_LIGHT_ON:
            Serial.println("Hood on");
            return true;

        case IRCMD_LIGHT_OFF:
            Serial.println("LIGHT OFF");
            return true;

        case IRCMD_VENT_1:
            Serial.println("Ventilation level 1");
            return true;

        case IRCMD_VENT_2:
            Serial.println("Ventilation level 2");
            return true;

        case IRCMD_VENT_3:
            Serial.println("Ventilation level 3");
            return true;

        case IRCMD_VENT_4:
            Serial.println("Ventilation level 4");
            return true;

        case IRCMD_VENT_OFF:
            Serial.println("Ventilation off, hood off");
            return true;

        default:
            Serial.println("Received unknown code");
            return false;
    }
};

int previousData = -1;

void loop() {
    int data = digitalRead(dataInPin);
    if (data != previousData) {
        previousData = data;
        Serial.print("Data: ");
        serialPrintUint64(data, HEX);
        Serial.println();
    }

    decode_results results;
    if (irReceiver.decode(&results)) {
        digitalWrite(LED_BUILTIN, LOW);
        if (handleIrCode(results)) {
            delay(1000);
        }
        irReceiver.resume();
    }
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
}
