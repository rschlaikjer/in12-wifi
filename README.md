# Wifi-enabled clock with IN-12 nixies

### Nixie clock that synchronizes over Network Time Protocol

Relatively simple Nixie clock, but with an automatic time detection twist.
On boot, and then every hour after, the clock will synchronize with a network
time server to ensure that it is still correct. No need to manually set the
time, no need to worry about RE-setting the time for daylight savings.
The core components are the MCU (ATMEGA328P), WiFi chipset (ATWINC1500)
and boost converter for the nixie tubes (MAX1771).

![Picture](/Media/done.jpg?raw=true "All set")

This project builds on a
[previous clock](https://github.com/rschlaikjer/ntp-nixie)
, which was also NTP-powered but
required a wired internet connection. After the ethernet chipset on that clock
died after a few years continuous service, I resolved to build a second
generation design with two improvements - a seconds place, and wifi enabled to
avoid yet another cable snaking across the desk.

![Picture](/Media/counting.gif?raw=true "In operation")

The design is split into three PCBs:
- The control board has three sections - one for the boost logic (12V -> 170V),
  one for the wifi logic (at 3.3V), and one for the MCU itsef at 5V.
- The shift register board, middle, simply converts the serial output from the
  MCU on the control board into a parallel output enabling various sinks on the
  nixie pins through MPSA42 transistors.
- The front PCB simply breaks the nixie pins out into even rows at the edges,
  so that it meshes well with the shift register board. Dealing with routing
  the shift registers, current limiting resistors and transistors all on the
  same board as the nixies was too troublesome compared with simply adding
  another layer.

![Picture](/Media/testing.jpg?raw=true "Verifying shift register outputs")

## BOMs

### Control board

| Part | Value | Device | Package | Description | PROD_ID | VALUE |
| -- | -- | -- | -- | -- | -- | -- |
| C1 | 10uF | CAP0805 | 0805 | Capacitor |  |  |
| C2 | 10uF,25V | CAP_POLPTH2 | CPOL-RADIAL-10UF-25V | Capacitor Polarized |  |  |
| C3 | 10uF/250V | CAP_POLPTH2 | CPOL-RADIAL-10UF-25V | Capacitor Polarized |  |  |
| C4 | 100n | CAPPTH | CAP-PTH-SMALL | Capacitor |  |  |
| C5 | 100n | CAPPTH | CAP-PTH-SMALL | Capacitor |  |  |
| C6 | 100nF | CAP0805 | 0805 | Capacitor |  |  |
| C7 | 10nF | CAP0805 | 0805 | Capacitor |  |  |
| C8 |  | CPOL-USE2,5-6E | E2,5-6E | POLARIZED CAPACITOR, American symbol |  |  |
| C9 |  | CPOL-USE2,5-6E | E2,5-6E | POLARIZED CAPACITOR, American symbol |  |  |
| C10 | 100uF,35V | CAP_POLPTH2 | CPOL-RADIAL-10UF-25V | Capacitor Polarized |  |  |
| C11 | 100nF | CAP0805 | 0805 | Capacitor |  |  |
| C12 | 22pF | CAP0805 | 0805 | Capacitor |  |  |
| C13 | 22pF | CAP0805 | 0805 | Capacitor |  |  |
| C14 | 10uF | CAP0805 | 0805 | Capacitor |  |  |
| C15 |  | CPOL-USE2,5-6E | E2,5-6E | POLARIZED CAPACITOR, American symbol |  |  |
| C16 | 10uF | CAP0805 | 0805 | Capacitor |  |  |
| C17 |  | CPOL-USE2,5-6E | E2,5-6E | POLARIZED CAPACITOR, American symbol |  |  |
| D1 | 1N4007 | 1N4004 | DO41-10 | DIODE |  |  |
| FTDI |  | ARDUINO_SERIAL_PROGRAMPTH | 1X06 | FTDI connector footprints |  |  |
| H1 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| H2 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| H3 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| H4 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| IC1 | 7805T | 7805T | TO220H | Positive VOLTAGE REGULATOR |  |  |
| IC3 | ATWINC1500 | ATWINC1500 | ATWINC1500-MR210 | Atmel ATWINC1500-MR210 WiFi/WLAN 802.11 b/g/n |  |  |
| IC4 | LD117AV33 | LD117AV33 | TO220L1 | Low drop fixed and adjustable positive voltage regulators 1 A |  |  |
| ISP1 | AVRISP-6 | AVRISP-6 | AVRISP | AVR ISP HEADER |  |  |
| J1 |  | M04PTH | 1X04 | Header 4 | CONN-09696 |  |
| J2 |  | M02PTH | 1X02 | Standard 2-pin 0.1" header. Use with |  |  |
| J7 |  | M02PTH | 1X02 | Standard 2-pin 0.1" header. Use with |  |  |
| J9 |  | M02PTH | 1X02 | Standard 2-pin 0.1" header. Use with |  |  |
| L1 | 100uH | L-SMT-POWER-B82479 | L-B82479 | SMT power inductors |  |  |
| LED_ACT |  | LED0603 | LED-0603 | LEDs |  |  |
| LED_ERR |  | LED0603 | LED-0603 | LEDs |  |  |
| LED_WIFI |  | LED0603 | LED-0603 | LEDs |  |  |
| Q1 | IRF840 | IRF840 | TO220 | N-Channel Enhancement MOSFET (HEXFET); 500V; 8A; 0,85Ohm |  |  |
| Q2 | MOSFET-NCHANNEL2N7000 | MOSFET-NCHANNEL2N7000 | TO-92 | Common NMOSFET Parts |  |  |
| R1 | 1k | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R2 | 100k | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R3 | 1.5M | RESISTORPTH-1/4W | AXIAL-0.4 | Resistor |  |  |
| R4 | 10k | RESISTORPTH-1/4W | AXIAL-0.4 | Resistor |  |  |
| R5 | 0R050 | RESISTORPTH-1/4W | AXIAL-0.4 | Resistor |  |  |
| R6 | 1lk | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R7 | 1k | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R11 | 100k | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R12 | 100k | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R13 | 100k | RESISTOR0805-RES | 0805 | Resistor |  |  |
| RESET_SW1 | SPST | TACTILE-SWITCH-PTH | TAC-SWITCH-PTH | Tactile Switch 4 Pins, SPST NO |  |  |
| U$3 | MAX1771 | MAX1771 | 8-DIP |  |  |  |
| U$4 | 5k | RTRIM-3362P | RTRIM3362P | BOURNS 3362 - 1/4" Square Trimpot® Trimming Potentiometer |  |  |
| U$19 | PJ-044A | PJ-044A | PJ-044A |  |  |  |
| U1 |  | TXB0104PWRSOIC14 | SO14 | 4-Bit Bi-Directional Level Shifter | IC-11329 |  |
| U2 |  | TXB0104PWRSOIC14 | SO14 | 4-Bit Bi-Directional Level Shifter | IC-11329 |  |
| U3 | ATMEGA328P_TQFP | ATMEGA328P_TQFP | TQFP32-08 |  | IC-09069 | ATMEGA328P_TQFP |
| Y3 | 16MHz | CRYSTALHC49US | HC49US | Various standard crystals. Proven footprints. |  |  |


### Shift register board


| Part | Value | Device | Package | Description | PROD_ID | VALUE |
| -- | -- | -- | -- | -- | -- | -- |
| C1 |  | CAP0805 | 0805 | Capacitor |  |  |
| C2 |  | CAP0805 | 0805 | Capacitor |  |  |
| C3 |  | CAP0805 | 0805 | Capacitor |  |  |
| C4 |  | CAP0805 | 0805 | Capacitor |  |  |
| C5 |  | CAP0805 | 0805 | Capacitor |  |  |
| C6 |  | CAP0805 | 0805 | Capacitor |  |  |
| H1 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| H2 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| H3 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| H4 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |  |
| IC1 | 74LS595D | 74LS595D | SO16 | 8-bit SHIFT REGISTER, output latch |  |  |
| IC2 | 74LS595D | 74LS595D | SO16 | 8-bit SHIFT REGISTER, output latch |  |  |
| IC3 | 74LS595D | 74LS595D | SO16 | 8-bit SHIFT REGISTER, output latch |  |  |
| IC4 | 74LS595D | 74LS595D | SO16 | 8-bit SHIFT REGISTER, output latch |  |  |
| IC5 | 74LS595D | 74LS595D | SO16 | 8-bit SHIFT REGISTER, output latch |  |  |
| IC9 | 74LS595D | 74LS595D | SO16 | 8-bit SHIFT REGISTER, output latch |  |  |
| J1 |  | M04PTH | 1X04 | Header 4 | CONN-09696 |  |
| J2 |  | M05PTH | 1X05 | Header 5 |  |  |
| J3 |  | M05PTH | 1X05 | Header 5 |  |  |
| J4 |  | M05PTH | 1X05 | Header 5 |  |  |
| J5 |  | M05PTH | 1X05 | Header 5 |  |  |
| J6 |  | M05PTH | 1X05 | Header 5 |  |  |
| J7 |  | M02PTH | 1X02 | Standard 2-pin 0.1" header. Use with |  |  |
| J8 |  | M05PTH | 1X05 | Header 5 |  |  |
| J9 |  | M02PTH | 1X02 | Standard 2-pin 0.1" header. Use with |  |  |
| J10 |  | M05PTH | 1X05 | Header 5 |  |  |
| J11 |  | M05PTH | 1X05 | Header 5 |  |  |
| J12 |  | M05PTH | 1X05 | Header 5 |  |  |
| J13 |  | M05PTH | 1X05 | Header 5 |  |  |
| J14 |  | M05PTH | 1X05 | Header 5 |  |  |
| J15 |  | M05PTH | 1X05 | Header 5 |  |  |
| Q8 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q9 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q10 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q11 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q12 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q13 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q14 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q15 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q16 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q17 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q18 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q19 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q20 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q25 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q26 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q27 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q28 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q29 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q30 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q31 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q32 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q33 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q34 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q35 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q36 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q37 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q38 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q39 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q40 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q45 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q46 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q47 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q48 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q49 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q50 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q51 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q52 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q53 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q54 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q55 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q56 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q57 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q58 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q59 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| Q60 | MPSA42 | TRANSISTOR_NPNMPSA42 | SOT23-3 | Generic NPN BJT | TRANS-09116 | MPSA42 |
| R3 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R4 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R5 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R6 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R7 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R8 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R9 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R10 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R11 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| R12 |  | RESISTOR0805-RES | 0805 | Resistor |  |  |
| U$1 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$10 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$11 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$12 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$14 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$15 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$16 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$18 | CTS744 | CTS744 | CT740-746 |  |  |  |
| U$19 | CTS744 | CTS744 | CT740-746 |  |  |  |


### Display board


| Part | Value | Device | Package | Description |  |
| -- | -- | -- | -- | -- | -- |
| H1 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |
| H2 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |
| H3 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |
| H4 | STAND-OFF | STAND-OFF | STAND-OFF | #4 Stand Off |  |
| J1 |  | M02PTH | 1X02 | Standard 2-pin 0.1" header. Use with |  |
| J2 |  | M05PTH | 1X05 | Header 5 |  |
| J3 |  | M05PTH | 1X05 | Header 5 |  |
| J4 |  | M05PTH | 1X05 | Header 5 |  |
| J5 |  | M05PTH | 1X05 | Header 5 |  |
| J6 |  | M05PTH | 1X05 | Header 5 |  |
| J7 |  | M05PTH | 1X05 | Header 5 |  |
| J8 |  | M05PTH | 1X05 | Header 5 |  |
| J9 |  | M05PTH | 1X05 | Header 5 |  |
| J10 |  | M05PTH | 1X05 | Header 5 |  |
| J11 |  | M05PTH | 1X05 | Header 5 |  |
| J12 |  | M05PTH | 1X05 | Header 5 |  |
| J13 |  | M05PTH | 1X05 | Header 5 |  |
| N1 | IN-12A | IN-12A | IN-12 | IN-12A: medium numeric topview nixie tube |  |
| N2 | IN-12A | IN-12A | IN-12 | IN-12A: medium numeric topview nixie tube |  |
| N3 | IN-12A | IN-12A | IN-12 | IN-12A: medium numeric topview nixie tube |  |
| N4 | IN-12A | IN-12A | IN-12 | IN-12A: medium numeric topview nixie tube |  |
| N5 | IN-12A | IN-12A | IN-12 | IN-12A: medium numeric topview nixie tube |  |
| N6 | IN-12A | IN-12A | IN-12 | IN-12A: medium numeric topview nixie tube |  |
| R1 |  | RESISTOR0805-RES | 0805 | Resistor |  |
| R2 |  | RESISTOR0805-RES | 0805 | Resistor |  |
| R3 |  | RESISTOR0805-RES | 0805 | Resistor |  |
| R4 |  | RESISTOR0805-RES | 0805 | Resistor |  |
| R5 |  | RESISTOR0805-RES | 0805 | Resistor |  |
| R6 |  | RESISTOR0805-RES | 0805 | Resistor |  |
