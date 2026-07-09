# Visible Light Communication (VLC) for Wireless Transmission

> Data Transmission Through Illumination — a laser-based VLC system for transmitting text, audio, and image data wirelessly between two PCs using visible light instead of RF.

Based on the research paper *"Revolutionizing Connectivity: Visible Light Communication for Wireless Transmission"* by Kavya Vishal Sikri & Vedant Santosh Jadhav, Lovely Professional University.

## 📡 Overview

Visible Light Communication (VLC) transmits data using the visible light spectrum (380–780 nm) instead of the radio frequency spectrum used by Wi-Fi, Bluetooth, and similar technologies. This project implements a working VLC link between two PCs using a red LASER as the transmitter and a Solar Cell as the receiver, with Arduino UNO boards handling data processing at both ends.

**Why VLC?**
- Much larger available bandwidth (430–750 THz) than RF (3 KHz–300 GHz), enabling higher data rates
- Safe for use in RF-sensitive environments (hospitals, mines, aircraft, space stations)
- Inherently more secure — light doesn't pass through walls, so signals can't be intercepted from outside the room
- Low-cost to implement using ubiquitous LED/LASER light sources

## 🔧 Hardware Used
- 2× Arduino UNO microcontrollers (one per end)
- Red LASER diode (transmitter)
- Solar Cell (receiver)
- PAM (Pulse Amplitude Modulation) module + speaker (for audio output)
- 2× PCs running Arduino IDE

## ⚙️ System Architecture

```
PC 1 → Arduino UNO 1 → LASER ---(light)---> Solar Cell → Arduino UNO 2 → PC 2
      (Transmitter)                                        (Receiver)
```

### Transmitter
- User enters a string (up to 50 characters) via the Arduino IDE serial monitor on PC 1
- The Arduino converts each character to its ASCII value, then to an 8-bit binary stream
- The LASER (connected to digital pin 7) flickers HIGH/LOW according to the bit stream, using Pulse Width Modulation — a longer pulse duration represents `1`, a shorter one represents `0`
- Flicker rate is fast enough (human eye resolves ~10 frames/sec) to be completely imperceptible
- For audio, the signal is similarly converted into a stream of 1s and 0s before transmission

### Receiver
- A Solar Cell (analog input) detects the light intensity variations from the LASER
- `pulseIn()` measures the duration of each HIGH/LOW pulse; a threshold distinguishes bit `1` from bit `0`
- The received bit stream (arriving in reverse order) is reversed, split into 8-bit groups, and converted back to ASCII characters
- Reconstructed text is displayed on PC 2's serial monitor
- For audio: the Solar Cell converts the light signal into weak electric current, which is converted to sound via a speaker and amplified using the PAM module

## 📁 Repository Structure
```
VLC-Wireless-Transmission/
├── transmitter/
│   └── transmitter.ino
├── receiver/
│   └── receiver.ino
├── circuit-diagrams/
│   └── (schematics, breadboard images)
├── docs/
│   └── research-paper.pdf
└── README.md
```

## 🚀 Getting Started
1. Wire the transmitter: Arduino UNO 1 → LASER diode on digital pin 7
2. Wire the receiver: Arduino UNO 2 → Solar Cell on an analog pin, positioned in direct line of sight with the LASER (tested up to 3m)
3. Upload `transmitter/transmitter.ino` to Arduino UNO 1, connected to PC 1
4. Upload `receiver/receiver.ino` to Arduino UNO 2, connected to PC 2
5. Open the Serial Monitor on PC 1, set baud rate to **9600**, and enter a string (up to 50 characters)
6. Open the Serial Monitor on PC 2 to view the received string
7. Test in low-ambient-light conditions for best accuracy — ambient light affects the Solar Cell's threshold detection

## 📈 Results
- Successfully transmitted and received strings up to 50 characters
- Reliable operation at a distance of up to 3 meters between LASER and Solar Cell
- Audio transmission successfully demonstrated using PAM-based amplification
- Pulse durations (and thus detection accuracy) are sensitive to ambient lighting conditions

## 🔮 Future Improvements
- Add error correction (not just detection) for more robust data transfer
- Explore higher baud rates / modulation schemes to increase data throughput
- Extend range using focusing optics or higher-power LEDs
- Add image transmission support as outlined in the original project scope

## 📄 Reference
K. V. Sikri and V. S. Jadhav, "Revolutionizing Connectivity: Visible Light Communication for Wireless Transmission," Electronics and Communication Engineering, Lovely Professional University, Phagwara, Punjab.

## 👤 Author
**Kavya Sikri**
- LinkedIn: [kavya-sikri](https://www.linkedin.com/in/kavya-sikri)
- GitHub: [KVSIKRI](https://github.com/KVSIKRI)
