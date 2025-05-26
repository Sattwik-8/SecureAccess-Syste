# SmartLock-Arduino

A fully-featured **Arduino-based Smart Lock System**, designed and simulated in **Tinkercad**. This project combines hardware security, user interface, and persistent storage to create a secure and user-friendly door lock system.

---

##  Features

-  **4-digit Password Protection**
-  **Servo-controlled door mechanism**
-  **EEPROM Password Storage** – Retains password after power loss
-  **Auto-Close Door Feature** – Closes door automatically after set time
-  **Lockout After 3 Failed Attempts** – Temporary lock for 30 seconds
-  **Access Logging** – Stores timestamps of last 5 access events
-  **View Wrong Attempt Count**
-  **Brightness Adjustment** – Control LCD backlight intensity
-  **Firmware Version Info**
-  **How-to-Use Instructions Menu**
-  **Hierarchical Menu Navigation** via 4x4 keypad

---

##  Components Used

| Component         | Quantity |
|------------------|----------|
| Arduino UNO       | 1        |
| 4x4 Keypad        | 1        |
| LCD 16x2 Display  | 1        |
| Servo Motor       | 1        |
| Red LED           | 1        |
| Green LED         | 1        |
| Buzzer            | 1        |
| Potentiometer (for LCD brightness, optional) | 1 |
| Resistors         | As needed |
| Breadboard        | 1        |
| Jumper Wires      | Several  |

---

## 🔧 How It Works

1. **Startup**: Displays a welcome message.
2. **Login**: User enters a 4-digit password using the keypad.
3. **Correct Password**: Servo rotates to open position; green LED turns on; access time is logged.
4. **Wrong Password**: Red LED blinks, error tones played.
5. **3 Wrong Attempts**: System enters lockout for 30 seconds.
6. **In-Menu Options**:
   - Change password
   - Toggle auto-close
   - View access logs
   - See wrong attempts
   - Adjust brightness
   - Firmware info
   - How-to-use guide
## 🧠 EEPROM Usage

The password is stored in EEPROM, so it persists even after power loss. On reset, the system reads the saved password from EEPROM.
---

## 👤 Author

**Sattwik**  
