#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <Timezone.h>

// High tension system enable
#define HV_EN 6

// Wifi control
#define WIFI_SS 9
#define WIFI_IRQ 16
#define WIFI_RESET 15
#define WIFI_EN 14

// Shift register control
#define N_SER 19
#define N_SCK 2
#define N_RCK 4
#define N_SCL 3

// How long between NTP re-sync's (millis)
#define NTP_SYNC_INTERVAL 600000

// How long between display updates (millis)
#define DISPLAY_UPDATE_INTERVAL 50

// Wifi details
char ssid[] = "your_ssid_here";
char pass[] = "your_password_here";

// WIFI101 internals
WiFiUDP Udp;
int status = WL_IDLE_STATUS;
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
unsigned int localPort = 2390;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

// Timezone rules
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
Timezone usEastern(usEDT, usEST);

// State
unsigned long millis_offset = 0;
unsigned long lastNtpSync = 0;
unsigned long lastDisplayUpdate = 0;

/**
 * Set pinmodes, initialize WiFi chipset, enable the high voltage system
 */
void setup() {
    //// Set up pins
    // High voltage shutdown
    pinMode(HV_EN, OUTPUT);

    // Wifi
    pinMode(WIFI_SS, OUTPUT);
    pinMode(WIFI_IRQ, OUTPUT);
    pinMode(WIFI_RESET, OUTPUT);
    pinMode(WIFI_EN, OUTPUT);

    // Shift register
    pinMode(N_SER, OUTPUT);
    pinMode(N_SCK, OUTPUT);
    pinMode(N_RCK, OUTPUT);
    pinMode(N_SCL, OUTPUT);
    digitalWrite(N_SCL, HIGH);

    //// Start wifi
    digitalWrite(WIFI_EN, HIGH);
    delay(250);
    WiFi.setPins(WIFI_SS, WIFI_IRQ, WIFI_RESET, -1);
    initWifi();

    // Turn on display
    digitalWrite(HV_EN, HIGH);

    // Fetch initial offset
    sendNTPpacket(timeServer);
}

/**
 * On each loop, there are three things we want to check:
 * - Whether we need to re-sync with the NTP server
 * - Whether there is a pending UDB datagram to handle
 * - Whether it's time to update the display
 */
void loop() {
    // If it's been more than NTP_SYNC_INTERVAL since we last updated our time
    // offset, then issue a new NTP sync request.
    if (millis() - lastNtpSync > NTP_SYNC_INTERVAL) {
        lastNtpSync = millis();
        sendNTPpacket(timeServer);
    }

    // If we have a pending NTP response datagram, handle it
    if (Udp.parsePacket()) {
        parseNtpResponse();
    }

    // If it's been too long since our last refresh of the display, trigger one
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = millis();
        updateDisplay();
    }
}

/**
 * Update our stored offset of the timezone-local epoch time against our
 * internal millisecond counter.
 */
void updateOffset(unsigned long localTime) {
    millis_offset = (localTime - (millis() / 1000));
}

/**
 * Initialize the WiFi chipset,
 * internal millisecond counter.
 */
void initWifi() {
    // Check if the WiFi chipset is actually presenty
    if (WiFi.status() == WL_NO_SHIELD) {
        // If not, lock up
        while (true);
    }

    // Loop until we successfully connect to the network
    while (status != WL_CONNECTED) {
        status = WiFi.begin(ssid, pass);
        delay(10000);
    }

    // Start up our UDP socket
    Udp.begin(localPort);
}

/**
 * Parse the response datagram from the NTP timeserver
 */
void parseNtpResponse() {
    // Read the response packet into the buffer
    Udp.read(packetBuffer, NTP_PACKET_SIZE);

    // The info we care about, the NTP timestamp, begins at byte 40 in the
    // packet and is 4 bytes long. Grab those bytes and convert them to an
    // unsigned long.
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

    // NTP counts seconds since 1900, not seconds since 1970.
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    // So to convert to UNIX epoch, just need to subtract 70 years in seconds
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;

    // Now, convert that epoch to our local timezone
    unsigned long localTime =  usEastern.toLocal(epoch);

    // And update our millisecond offset
    updateOffset(localTime);
}


/**
 * Issue an NTP sync request to our timeserver.
 * This involves building up a packet by hand - reference to RFC5905 is key
 * https://tools.ietf.org/html/rfc5905#section-7.3
 */
unsigned long sendNTPpacket(IPAddress& address) {
    // Zero out our packet buffer
    memset(packetBuffer, 0, NTP_PACKET_SIZE);

    // Our first byte contains the Leap Indicator, Version Number and Mode.
    packetBuffer[0] = (
        // Leap indicator is set to 3 (clock not sync'd)
        0x03 << 6 |
        // Version number is 4, the current version of NTP
        0x04 << 3 |
        // Our mode is 3, client
        0x03 << 0
    );

    // Next is stratum, in this case unspecified
    packetBuffer[1] = 0x00;

    // Polling interval - the maximum interbal between successive messages.
    // This is a funky variable; it's in log2 seconds.
    // 6 is the recommended lower bound, so we'll use that
    packetBuffer[2] = 0x06;

    // The precision of our own clock.
    // This is also in log2 seconds (signed). So to specify microseconds, that's
    // 1 / 1,000,000 seconds. log_2(1000000) is 20 (well, 19,9), so for one
    // onemillionth of a second we want -20.
    packetBuffer[3] = 0xEC;

    // 8 bytes intentionally left blank for the root delay and root dispersion.
    // These count time lags for the reference clock, and we don't really care.

    // Next 32 bits are the Reference ID. I can't quite determine what exactly
    // the spec wants here, but 'INIT' is a valid kiss code for not having
    // synchronized yet so we'll go with that
    packetBuffer[12]  = 'I';
    packetBuffer[13]  = 'N';
    packetBuffer[14]  = 'I';
    packetBuffer[15]  = 'T';

    // There are more fields in an NTP packet, but we don't care about them here
    // Now that the packet is all set, we can send it off to the timeserver
    // NTP runs on port 123
    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

/**
 * Updates the Nixie display.
 * Calculate the current local time from our sytem time and millisecond offset,
 * then break it up into hours, minutes and seconds and hand it off to the shift
 * register logic
 */
void updateDisplay() {
    // Take the current system time, convert to seconds and add our offset
    unsigned long time = (millis() / 1000) + millis_offset;

    // Now, modulo the time down to just one day
    time = time % (60L * 60L * 24L);

    // Break out each of the hours, minutes and seconds
    uint8_t hours = (time / 60 / 60) % 24;
    uint8_t minutes = (time / 60) % 60;
    uint8_t seconds = time % 60;

    // Hand the values off to the bit fiddling logic
    write595Time(hours, minutes, seconds);
}

/**
 * Takes the hours, minutes and seconds and maps them into bits on the shift
 * registers. Elegant? Perhaps not. Functional? Yes.
 */
void write595Time(uint8_t hours, uint8_t minutes, uint8_t seconds) {

    // Create an output buffer
    uint8_t out[] = { 0, 0, 0, 0, 0, 0 };

    // Low 3 bites on the first shift register control the hours tens
    switch (hours / 10) {
        case 0: out[0] |= (1 << 0); break;
        case 1: out[0] |= (1 << 1); break;
        case 2: out[0] |= (1 << 2); break;
    }

    // The upper bits of the first, and lower of the second, registers control
    // hours ones
    switch (hours % 10) {
        case 0: out[0] |= (1 << 4); break;
        case 1: out[0] |= (1 << 5); break;
        case 2: out[0] |= (1 << 6); break;
        case 3: out[0] |= (1 << 7); break;

        case 4: out[1] |= (1 << 0); break;
        case 9: out[1] |= (1 << 1); break;
        case 8: out[1] |= (1 << 2); break;
        case 7: out[1] |= (1 << 3); break;
        case 6: out[1] |= (1 << 4); break;
        case 5: out[1] |= (1 << 5); break;
    }

    // Tens places on the minutes
    switch (minutes / 10) {
        case 0: out[2] |= (1 << 0); break;
        case 1: out[2] |= (1 << 1); break;
        case 2: out[2] |= (1 << 2); break;
        case 3: out[2] |= (1 << 3); break;
        case 4: out[2] |= (1 << 4); break;
        case 5: out[2] |= (1 << 5); break;
    }

    // Ones places on the minutes
    switch (minutes % 10) {
        case 0: out[2] |= (1 << 6); break;
        case 1: out[2] |= (1 << 7); break;

        case 2: out[3] |= (1 << 0); break;
        case 3: out[3] |= (1 << 1); break;
        case 4: out[3] |= (1 << 2); break;
        case 9: out[3] |= (1 << 3); break;
        case 8: out[3] |= (1 << 4); break;
        case 7: out[3] |= (1 << 5); break;
        case 6: out[3] |= (1 << 6); break;
        case 5: out[3] |= (1 << 7); break;
    }

    // Tens places on the seconds
    switch (seconds / 10) {
        case 0: out[4] |= (1 << 0); break;
        case 1: out[4] |= (1 << 1); break;
        case 2: out[4] |= (1 << 2); break;
        case 3: out[4] |= (1 << 3); break;
        case 4: out[4] |= (1 << 4); break;
        case 5: out[4] |= (1 << 5); break;
    }

    // Ones places on the seconds
    switch (seconds % 10) {
        case 0: out[4] |= (1 << 6); break;
        case 1: out[4] |= (1 << 7); break;

        case 2: out[5] |= (1 << 0); break;
        case 3: out[5] |= (1 << 1); break;
        case 4: out[5] |= (1 << 2); break;
        case 9: out[5] |= (1 << 3); break;
        case 8: out[5] |= (1 << 4); break;
        case 7: out[5] |= (1 << 5); break;
        case 6: out[5] |= (1 << 6); break;
        case 5: out[5] |= (1 << 7); break;
    }

    // Now shift out the data to the registers
    digitalWrite(N_RCK, LOW);
    for (int i = 5; i >= 0; i--) {
        shiftOut(N_SER, N_SCK, MSBFIRST, out[i]);
    }
    digitalWrite(N_RCK, HIGH);
}
