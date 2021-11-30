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

void loop() {
    if (irrecv.decode(&results)) {
        digitalWrite(LED_BUILTIN, LOW);
        // print() & println() can't handle printing long longs. (uint64_t)
        serialPrintUint64(results.value, HEX);
        Serial.println("");
        irrecv.resume();    // Receive the next value
    }
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
}
