#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint8_t kRecvPin = D5;

IRrecv irrecv(kRecvPin);

decode_results results;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial) {
        delay(50);
    }

    irrecv.enableIRIn();

    Serial.println();
    Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
    Serial.println(kRecvPin);
}

// IR commands from AEG hob2hood device
const uint64_t IRCMD_VENT_1 = 0xE3C01BE2;       // Hob2hood On (level 1)
const uint64_t IRCMD_VENT_2 = 0xD051C301;       // Hob2hood level 2
const uint64_t IRCMD_VENT_3 = 0xC22FFFD7;       // Hob2hood level 3
const uint64_t IRCMD_VENT_4 = 0xB9121B29;       // Hob2hood level 4
const uint64_t IRCMD_VENT_OFF = 0x55303A3;      // Hob2hood off
const uint64_t IRCMD_LIGHT_ON = 0xE208293C;     // Light on (Hood on)
const uint64_t IRCMD_LIGHT_OFF = 0x24ACF947;    // Light off (Automatic after 2min)

bool handleCode(const decode_results& results) {
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

void loop() {
    if (irrecv.decode(&results)) {
        digitalWrite(LED_BUILTIN, LOW);
        if (handleCode(results)) {
            delay(1000);
        }
        irrecv.resume();
    }
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
}
