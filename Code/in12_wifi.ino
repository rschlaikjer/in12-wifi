#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <Timezone.h>

#define DEBUG 1

#if DEBUG
  #define LOG(X) Serial.print(X);
  #define LOGLN(X) Serial.println(X);
#else
  #define LOG(X)
  #define LOGLN(X)
#endif

// High voltage system enable
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
// How long between NTP retrys (millis)
#define NTP_RESEND_INTERVAL 2000

// How long to wait for wifi to connect
#define WIFI_RECONNECT_INTERVAL 10000

// How long between display updates (millis)
#define DISPLAY_UPDATE_INTERVAL 100

#define TIME_SERVER "time.nist.gov"
#define NTP_PORT 123
#define LOCAL_PORT 2390

// Wifi details
#define MAX_SSID_LEN 64
boolean ssidLoaded = false;
uint8_t EEMEM_SSID_LEN EEMEM = 0;
uint8_t EEMEM_PASS_LEN EEMEM = 0;
char EEMEM_SSID[MAX_SSID_LEN] EEMEM = "your_ssid_here";
char EEMEM_PASS[MAX_SSID_LEN] EEMEM = "your_password_here";
char ssid[MAX_SSID_LEN] = {0};
char pass[MAX_SSID_LEN] = {0};

// WIFI101 internals
WiFiUDP Udp;
int wifiStatus = WL_IDLE_STATUS;
unsigned long lastWifiBeginTime = WIFI_RECONNECT_INTERVAL;
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

// Timezone rules
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
Timezone usEastern(usEDT, usEST);

// State
unsigned long millis_offset = 0;
unsigned long lastNtpPacketSent = NTP_RESEND_INTERVAL;
unsigned long lastNtpSyncSuccess = NTP_SYNC_INTERVAL;
unsigned long lastDisplayUpdate = 0;

/**
 * Set pinmodes, initialize WiFi chipset, enable the high voltage system
 */
void setup() {
    // Initialize serial
    Serial.begin(9600);

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

    // Turn on display
    write595Time(0, 0, 0);
    digitalWrite(HV_EN, HIGH);
}

/**
 * On each loop, there are three things we want to check:
 * - Whether we need to re-sync with the NTP server
 * - Whether there is a pending UDB datagram to handle
 * - Whether it's time to update the display
 */
void loop() {
    // Ensure that the wifi remains connected
    ensureWifi();

    // If it's been more than NTP_SYNC_INTERVAL since we last updated our time
    // offset, then issue a new NTP sync request.
    if (WiFi.status() == WL_CONNECTED
     && millis() - lastNtpSyncSuccess > NTP_SYNC_INTERVAL
     && millis() - lastNtpPacketSent > NTP_RESEND_INTERVAL) {
        LOGLN("Sending new NTP packet");
        sendNTPpacket();
        lastNtpPacketSent = millis();
    }

    // If we have a pending NTP response datagram, handle it
    if (Udp.parsePacket()) {
        LOGLN("Got NTP response");
        parseNtpResponse();
    }

    // If it's been too long since our last refresh of the display, trigger one
    if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = millis();
        updateDisplay();
    }

    while (Serial.available()) {
        handleSerial();
    }
}

#define READY 1
#define READ_SSID 2
#define READ_PASS 3

uint8_t serialState = READY;

char serialBuffer[96];
uint8_t cmdBufferIndex = 0;

void handleSerial() {
    // Read the next character
    int c = Serial.read();

    // If we read a -1, there's no data available
    if (c < 0) {
      return;
    }

    Serial.print(((char)c));

    // Handle the byte depending on our current state
    switch(serialState) {
        case READY:
            // Add the char to the command buffer
            serialBuffer[cmdBufferIndex++] = c;

            // If the buffer has 4 characters, check if it's a valid command
            if (cmdBufferIndex == 5) {
                if (!strncmp("ssid ", serialBuffer, 5)) {
                  serialState = READ_SSID;
                } else if (!strncmp("pass ", serialBuffer, 5)) {
                  serialState = READ_PASS;
                } else {
                  serialState = READY;
                  Serial.println("Invalid cmd");
                }
                cmdBufferIndex = 0;
            }
            break;
        case READ_PASS:
            if (c == '\n' || c == '\r') {
                serialBuffer[cmdBufferIndex++] = 0;
                setWifiPass(serialBuffer, cmdBufferIndex);
                Serial.print("Set Wifi pass to ");
                Serial.println(serialBuffer);
                cmdBufferIndex = 0;
                serialState = READY;
            }
            serialBuffer[cmdBufferIndex++] = c;
            break;
        case READ_SSID:
            if (c == '\n' || c == '\r') {
                serialBuffer[cmdBufferIndex++] = 0;
                setWifiSsid(serialBuffer, cmdBufferIndex);
                Serial.print("Set Wifi ssid to ");
                Serial.println(serialBuffer);
                cmdBufferIndex = 0;
                serialState = READY;
            }
            serialBuffer[cmdBufferIndex++] = c;
            break;
    }
}

void setWifiSsid(char *newSSID, uint8_t len) {
    eeprom_write_block(newSSID, EEMEM_SSID, len);
    eeprom_write_byte(&EEMEM_SSID_LEN, len);
}

void setWifiPass(char *newPass, uint8_t len) {
    eeprom_write_block(newPass, EEMEM_PASS, len);
    eeprom_write_byte(&EEMEM_PASS_LEN, len);
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
void ensureWifi() {
    // If the wifi is connected, and we thought it was connected, do nothing
    if (WiFi.status() == WL_CONNECTED && wifiStatus == WL_CONNECTED) {
        return;
    }

    // Check if we have loaded the SSID details from EEPROM
    if (!ssidLoaded) {
        uint8_t ssidLen = eeprom_read_byte(&EEMEM_SSID_LEN);
        LOG("SSid length: ");
        LOGLN(ssidLen);
        uint8_t passLen = eeprom_read_byte(&EEMEM_PASS_LEN);
        LOG("Pass length: ");
        LOGLN(passLen);
        eeprom_read_block(ssid, EEMEM_SSID, ssidLen);
        eeprom_read_block(pass, EEMEM_PASS, passLen);
        LOG("Loaded wifi details ");
        LOG(ssid);
        LOG(" / ");
        LOGLN(pass);
        ssidLoaded = true;
    }

    // If we're not connected, and it's been long enough since the last attempt,
    // re-begin the wifi connection.
    if (millis() - lastWifiBeginTime > WIFI_RECONNECT_INTERVAL) {
        lastWifiBeginTime = millis();
        LOG("(Re)Connecting to WiFi");
        wifiStatus = WiFi.begin(ssid, pass);
    }

    // If the wifi is connected but we didn't think it was, then we must just have
    // connected, so start a UDP socket up.
    if (wifiStatus == WL_CONNECTED || WiFi.status() == WL_CONNECTED) {
        LOG("Wifi connected");
        wifiStatus = WL_CONNECTED;
        Udp.begin(LOCAL_PORT);
        return;
    }
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
    LOG("Updated time offset: ");
    LOGLN(localTime);

    // And update our millisecond offset
    updateOffset(localTime);

    lastNtpSyncSuccess = millis();
}


/**
 * Issue an NTP sync request to our timeserver.
 * This involves building up a packet by hand - reference to RFC5905 is key
 * https://tools.ietf.org/html/rfc5905#section-7.3
 */
unsigned long sendNTPpacket() {
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
    Udp.beginPacket(TIME_SERVER, NTP_PORT);
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
