#include <Arduino.h>
#include <IRac.h>
#include <IRrecv.h>
#include <IRtext.h>
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

const uint16_t kCaptureBufferSize = 1024;

// kTimeout is the Nr. of milli-Seconds of no-more-data before we consider a
// message ended.
// This parameter is an interesting trade-off. The longer the timeout, the more
// complex a message it can capture. e.g. Some device protocols will send
// multiple message packets in quick succession, like Air Conditioner remotes.
// Air Coniditioner protocols often have a considerable gap (20-40+ms) between
// packets.
// The downside of a large timeout value is a lot of less complex protocols
// send multiple messages when the remote's button is held down. The gap between
// them is often also around 20+ms. This can result in the raw data be 2-3+
// times larger than needed as it has captured 2-3+ messages in a single
// capture. Setting a low timeout value can resolve this.
// So, choosing the best kTimeout value for your use particular case is
// quite nuanced. Good luck and happy hunting.
// NOTE: Don't exceed kMaxTimeoutMs. Typically 130ms.
#if DECODE_AC
// Some A/C units have gaps in their protocols of ~40ms. e.g. Kelvinator
// A value this large may swallow repeats of some protocols
const uint8_t kTimeout = 50;
#else     // DECODE_AC
// Suits most messages, while not swallowing many repeats.
const uint8_t kTimeout = 15;
#endif    // DECODE_AC
// Alternatives:
// const uint8_t kTimeout = 90;
// Suits messages with big gaps like XMP-1 & some aircon units, but can
// accidentally swallow repeated messages in the rawData[] output.
//
// const uint8_t kTimeout = kMaxTimeoutMs;
// This will set it to our currently allowed maximum.
// Values this high are problematic because it is roughly the typical boundary
// where most messages repeat.
// e.g. It will stop decoding a message and start sending it to serial at
//      precisely the time when the next message is likely to be transmitted,
//      and may miss it.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the kTimeout value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
const uint16_t kMinUnknownSize = 1024;

// How much percentage lee way do we give to incoming signals in order to match
// it?
// e.g. +/- 25% (default) to an expected value of 500 would mean matching a
//      value between 375 & 625 inclusive.
// Note: Default is 25(%). Going to a value >= 50(%) will cause some protocols
//       to no longer match correctly. In normal situations you probably do not
//       need to adjust this value. Typically that's when the library detects
//       your remote's message some of the time, but not all of the time.
const uint8_t kTolerancePercentage = kTolerance;    // kTolerance is normally 25%

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv dataReceiver(dataInPin, kCaptureBufferSize, kTimeout, true);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(dataOutPin, OUTPUT);
    digitalWrite(dataOutPin, HIGH);

    Serial.begin(115200);
    while (!Serial) {
        delay(50);
    }
    // Perform a low level sanity checks that the compiler performs bit field
    // packing as we expect and Endianness is as we expect.
    assert(irutils::lowLevelSanityCheck() == 0);

    irReceiver.enableIRIn();

#if DECODE_HASH
    // Ignore messages with less than minimum on or off pulses.
    dataReceiver.setUnknownThreshold(kMinUnknownSize);
#endif                                                  // DECODE_HASH
    dataReceiver.setTolerance(kTolerancePercentage);    // Override the default tolerance.
    dataReceiver.enableIRIn();                          // Start the receiver

    Serial.println();
    Serial.printf("IR on pin %d, data in on pin %d\n", irPin, dataInPin);
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

void loop() {
    decode_results results;
    if (irReceiver.decode(&results)) {
        digitalWrite(LED_BUILTIN, LOW);
        if (handleIrCode(results)) {
            delay(1000);
        }
        irReceiver.resume();
    }

    // Check if the code has been received.
    if (dataReceiver.decode(&results)) {
        // Display a crude timestamp.
        uint32_t now = millis();
        Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);

        // Check if we got an IR message that was to big for our capture buffer.
        if (results.overflow) {
            Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
        }

        // Display the library version the message was captured with.
        Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_ "\n");
        // Display the tolerance percentage if it has been change from the default.
        if (kTolerancePercentage != kTolerance)
            Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
        // Display the basic output of what we found.
        Serial.print(resultToHumanReadableBasic(&results));

        // Display any extra A/C info if we have it.
        String description = IRAcUtils::resultAcToString(&results);
        if (description.length()) {
            Serial.println(D_STR_MESGDESC ": " + description);
        }
        yield();    // Feed the WDT as the text output can take a while to print.

        // Output the results as source code
        Serial.println(resultToSourceCode(&results));
        Serial.println();    // Blank line between entries
        yield();             // Feed the WDT (again)
    }

    // delay(100);
    // digitalWrite(LED_BUILTIN, HIGH);
}
