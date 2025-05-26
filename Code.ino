#include <Servo.h>  
#include <Keypad.h> 
#include <LiquidCrystal.h> 
#include <EEPROM.h> 

#define greenled 13 
#define redled 10
#define buzzer 11
#define lcdBacklight 9 
#define FIRMWARE_VERSION "v1.0.0" 

Servo servo; 

const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {7, 6, 5, 4};
byte colPins[COLS] = {3, 2, 1, 0};
Keypad kp = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const int rs = A0, en = A1, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

bool isLockedOut = false;
unsigned long lockoutStartTime = 0;
const unsigned long lockoutDuration = 30000; 

const int maxLen = 4;
char pass[maxLen + 1]; 
char correctPass[maxLen + 1] = "0000";
int i = 0, counter = 0;

bool door = 1;

bool autoCloseEnabled = false;
unsigned long doorOpenTime = 0;
const unsigned long autoCloseDelay = 30000;

unsigned long lastAccessTime = 0; 
#define MAX_ACCESS_LOG 5
unsigned long accessLog[MAX_ACCESS_LOG] = {0, 0, 0, 0, 0};
int accessLogCount = 0;

int wrongAttemptsCount = 0;

int brightnessLevel = 255; 

enum Mode { NORMAL, MENU }; 
enum MenuSubState { MENU_LIST, NO_FUNCTION_PAGE };
MenuSubState menuSubState = MENU_LIST; 

Mode currentMode = NORMAL;
const char* menuItems[] = {
  "Change Pass",
  "Auto Close",
  "Audit",
  "Wrong Attempts",
  "Brightness",
  "Fw Ver.",
  "Info"
};

const int totalMenuItems = sizeof(menuItems) / sizeof(menuItems[0]); 
int menuIndex = 0; 

void beepTone(int freq, int duration) {
  tone(buzzer, freq); 
  delay(duration); 
  noTone(buzzer);
}

void animateKeyEntry(int pos) {
  lcd.setCursor(pos, 1); 
  lcd.print('*'); 
  beepTone(900, 50); 
}

void doorOpenAnimation() {
  lcd.clear();
  lcd.print("Opening Door...");
  lcd.setCursor(0, 1);
  for (int pos = 0; pos <= 15; pos++) {
    lcd.setCursor(pos, 1);
    lcd.write(byte(255)); 
  }
  for (int pos = 15; pos >= 0; pos--) {
    lcd.setCursor(pos, 1);
    lcd.print(" "); 
    delay(50);
  }
}

void doorCloseAnimation() {
  lcd.clear(); lcd.print("Closing Door...");
  lcd.setCursor(0, 1);
  for (int pos = 0; pos <= 15; pos++) {
    lcd.setCursor(pos, 1);
    lcd.write(byte(255));
    delay(50);
  }
}

void setup() {
  pinMode(lcdBacklight, OUTPUT);
  analogWrite(lcdBacklight, brightnessLevel); 
  pinMode(greenled, OUTPUT);
  pinMode(redled, OUTPUT);
  pinMode(buzzer, OUTPUT);
  servo.attach(8); 
  lcd.begin(16, 2);
  bool emptyEEPROM = true; 
  for (int i = 0; i < maxLen; i++) {
    byte val = EEPROM.read(i); 
    if (val < '0' || val > '9') {
      emptyEEPROM = true; 
      break;
    } else {
      correctPass[i] = val; 
      emptyEEPROM = false;
    }
  }
  correctPass[maxLen] = '\0'; 
  if (emptyEEPROM) {
    strcpy(correctPass, "0000"); 
    for (int i = 0; i < maxLen; i++) {
      EEPROM.update(i, correctPass[i]); 
    }
  }
	showWelcomeMessage();
	lcd.clear();
  	lcd.print("Enter Password:");
  	lcd.setCursor(0, 1);
  	servo.write(0); 
}

void loop() {
  char key = kp.getKey(); 
  if (isLockedOut) {
  unsigned long elapsed = millis() - lockoutStartTime;
  if (elapsed >= lockoutDuration) {
    isLockedOut = false;
    wrongAttemptsCount = 0;
    lcd.clear();
    showWelcomeMessage();
    lcd.clear();
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
  	} 
    else {
    lcd.clear();
    lcd.print("LOCKED OUT");
    lcd.setCursor(0, 1);
    int secondsLeft = (lockoutDuration - elapsed) / 1000;
    lcd.print("Wait ");
    lcd.print(secondsLeft);
    lcd.print(" sec");
    delay(500); 
  	}
  return; 
}
  if (key == '*') {
    if (door == 0) {
      lcd.clear();
      lcd.print("Closing Door...");
      doorCloseAnimation();
      servo.write(0);
      digitalWrite(greenled, LOW);
      door = 1;
    }
    currentMode = NORMAL;
    menuSubState = MENU_LIST;
    menuIndex = 0;

    lcd.clear();
    showWelcomeMessage();
    lcd.clear();
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    resetInput();
    return; 
  }
  if (key == 'C') {
    if (currentMode == MENU) {
      if (menuSubState == NO_FUNCTION_PAGE) {
        menuSubState = MENU_LIST;
        updateMenuDisplay();
        
      } 
      else if (menuSubState == MENU_LIST) {
        currentMode = NORMAL;
        lcd.clear();
        lcd.print("<D> Menu  ");
        lcd.setCursor(0, 1);
        lcd.print("<*> ClOSE");
        menuSubState = MENU_LIST;
      }
    } 
    else {
    }
    return; 
  }
  if (autoCloseEnabled && door == 0 && millis() - doorOpenTime >= autoCloseDelay) {
    lcd.clear();
    lcd.print("Auto Closing...");
    doorCloseAnimation();
    servo.write(0);
    digitalWrite(greenled, LOW);
    door = 1;
    currentMode = NORMAL;
    menuSubState = MENU_LIST;
    menuIndex = 0;
    lcd.clear();
    showWelcomeMessage();
	lcd.clear();
    lcd.print("Enter Password:");
    lcd.setCursor(0, 1);
    resetInput();
	}
  if (currentMode == MENU) {
    if (key == 'A') {
      menuIndex--;
      if (menuIndex < 0) menuIndex = totalMenuItems - 1;
      updateMenuDisplay();
      menuSubState = MENU_LIST;
    }
    else if (key == 'B') {
      menuIndex++;
      if (menuIndex >= totalMenuItems) menuIndex = 0;
      updateMenuDisplay();
      menuSubState = MENU_LIST;
    }
    else if (key == 'C') {
      if (menuSubState == NO_FUNCTION_PAGE) {
        updateMenuDisplay();
        menuSubState = MENU_LIST;
      }
      else if (menuSubState == MENU_LIST) {
        currentMode = NORMAL;
        lcd.clear();
        lcd.print("<D> Menu  ");
        lcd.setCursor(0, 1);
        lcd.print("<*> ClOSE");
        menuSubState = MENU_LIST;
      }
    } 
    else if (key == 'D') {
  	if (menuIndex == 0) {
    changePassword();
  	} 
	else if (menuIndex == 1) {
    toggleAutoClose();  
  	} 
    else if (menuIndex == 2) { 
    viewLastAccessTime();
   	}
    else if (menuIndex == 3) {
    viewWrongAttempts();
  	}
    else if (menuIndex == 4) {
    adjustBrightness();
  	} 
    else if (menuIndex == 5) {
    showFirmwareVersion();
  	}
 	else {
  	showHowToUse();
	}
	}
    return;
  }
  if (door == 1) {
    if (key >= '0' && key <= '9' && i < maxLen) { 
      pass[i] = key;
      animateKeyEntry(i); 
      i++;
    } 
    else if (key == 'D') {
      if (i == maxLen) {
        counter = 0;
        for (int j = 0; j < maxLen; j++) {
          if (pass[j] == correctPass[j]) counter++;
        }
        if (counter == maxLen) {
          correct();
        }
        else {
          wrong();
        }
      	}
      else {
       wrong();
      }
   	 }
  	} 
  else if (door == 0) {
    if (key == 'D') {
      currentMode = MENU;
      menuIndex = 0;
      updateMenuDisplay();
    }
    else if (key == '*') {
      lcd.clear();
      lcd.print("Closing Door...");
      doorCloseAnimation();
      servo.write(0);
      digitalWrite(greenled, LOW);
      door = 1;
      lcd.clear();
      showWelcomeMessage();
      lcd.clear();
      lcd.print("Enter Password:");
      lcd.setCursor(0, 1);
      resetInput();
    }
  }
}

void updateMenuDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("> ");
  lcd.print(menuItems[menuIndex]);
  lcd.setCursor(0, 1);
  lcd.print("  ");
  lcd.print(menuItems[(menuIndex + 1) % totalMenuItems]);
}

void correct() {
  beepTone(1000, 100);
  delay(100);
  beepTone(1500, 100);
  doorOpenAnimation();
  servo.write(90); 
  digitalWrite(greenled, HIGH); 
  door = 0;
  unsigned long currentTime = millis(); 
  addAccessTime(currentTime); 
  if (autoCloseEnabled) {
    doorOpenTime = millis();
  }
  lcd.clear();
  lcd.print("<D> Menu  ");
  lcd.setCursor(0, 1);
  lcd.print("<*> ClOSE");
  resetInput(); 
}

void wrong() {
  wrongAttemptsCount++;  
  if (wrongAttemptsCount >= 3) {
    isLockedOut = true;
    lockoutStartTime = millis();
    lcd.clear();
    lcd.print("LOCKED OUT");
    lcd.setCursor(0, 1);
    lcd.print("Wait 30 sec");
    digitalWrite(redled, HIGH);
    beepTone(300, 300);
    delay(1000);
  	}
  	else {
    lcd.clear();
    lcd.print("Wrong Pass");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    digitalWrite(redled, HIGH);
    beepTone(400, 150);
    delay(100);
    beepTone(400, 150);
    delay(1000);
  	}
    digitalWrite(redled, LOW); 
    resetInput();
    lcd.clear();
    if (!isLockedOut) {
    lcd.clear();
    lcd.print("Enter Password");
    lcd.setCursor(0, 1);
  	}
}

void resetInput() {
  i = 0; 
  counter = 0; 
  memset(pass, 0, sizeof(pass));
}

void changePassword() {
  char newPass[maxLen + 1] = {0}; 
  char confirmPass[maxLen + 1] = {0};
  lcd.clear();
  lcd.print("New Password:");
  lcd.setCursor(0, 1);
  int idx = 0;
  while (idx < maxLen) {
    char key = kp.getKey(); 
    if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      return;
    } 
    else if (key >= '0' && key <= '9') {
      newPass[idx] = key;
      lcd.print('*');
      beepTone(900, 50);
      idx++;
    }
  }
  delay(300);
  lcd.clear();
  lcd.print("Confirm Password:");
  lcd.setCursor(0, 1);
  idx = 0;
  while (idx < maxLen) {
    char key = kp.getKey();
    if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      return;
    }
    else if (key >= '0' && key <= '9') { 
      confirmPass[idx] = key;
      lcd.print('*');
      beepTone(900, 50);
      idx++;
    }
  }
  if (strcmp(newPass, confirmPass) == 0) { 
    strcpy(correctPass, newPass); 
    for (int i = 0; i < maxLen; i++) {
      EEPROM.update(i, newPass[i]); 
    }
    lcd.clear();
    lcd.print("Password Changed");
    beepTone(1200, 100);
    delay(1000);
  	}
  else {
    lcd.clear();
    lcd.print("Mismatch!");
    lcd.setCursor(0, 1);
    lcd.print("Retry");
    beepTone(300, 150);
    delay(1500);
  }
  menuSubState = MENU_LIST;
  updateMenuDisplay();
}

void toggleAutoClose() {
  int option = autoCloseEnabled ? 0 : 1; 
  int lastOption = -1; 
  while (true) {
    char key = kp.getKey();
    if (key == 'A') {
      option = (option - 1 + 2) % 2; 
      beepTone(800, 50);
    }
    else if (key == 'B') {
      option = (option + 1) % 2; 
      beepTone(800, 50);
    }
    else if (key == 'D') {
      autoCloseEnabled = (option == 0); 
      lcd.clear();
      lcd.print("Auto-Close:");
      lcd.setCursor(0, 1);
      lcd.print(autoCloseEnabled ? "ON" : "OFF");
      if (autoCloseEnabled) {
        doorOpenTime = millis();
      }
      beepTone(1000, 100);
      delay(1500);
      break;
    }
    else if (key == 'C') {
      break;
    }
    if (option != lastOption) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(option == 0 ? "> ON" : "  ON");
      lcd.setCursor(0, 1);
      lcd.print(option == 1 ? "> OFF" : "  OFF");
      lastOption = option;
    }
  }
  menuSubState = MENU_LIST;
  updateMenuDisplay();
}

void viewLastAccessTime() {
  if (accessLogCount == 0) {
    lcd.clear();
    lcd.print("No access yet");
    delay(2000);
    menuSubState = MENU_LIST;
    updateMenuDisplay();
    return;
  }
  int currentIndex = 0;
  int lastDisplayedIndex = -1; 
  while (true) {
    char key = kp.getKey();
    if (key == 'A') {
      currentIndex--;
      if (currentIndex < 0) currentIndex = accessLogCount - 1;
    } 
    else if (key == 'B') {
      currentIndex++;
      if (currentIndex >= accessLogCount) currentIndex = 0;
    } 
    else if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      break;
    }
    if (currentIndex != lastDisplayedIndex) {
      lcd.clear();
      lcd.print("Access ");
      lcd.print(currentIndex + 1);
      lcd.print(":");
      unsigned long elapsed = (millis() - accessLog[currentIndex]) / 1000; 
      lcd.setCursor(0, 1);
      if (elapsed < 60) {
        lcd.print(elapsed);
        lcd.print(" sec ago");
      } else if (elapsed < 3600) {
        lcd.print(elapsed / 60);
        lcd.print(" min ago");
      } else {
        lcd.print(elapsed / 3600);
        lcd.print(" hr ago");
      }
      lastDisplayedIndex = currentIndex;
    }
  }
}

void addAccessTime(unsigned long time) {
  for (int i = MAX_ACCESS_LOG - 1; i > 0; i--) {
    accessLog[i] = accessLog[i - 1];
  }
  accessLog[0] = time; 
  if (accessLogCount < MAX_ACCESS_LOG) accessLogCount++; 
}
void viewWrongAttempts() {
  lcd.clear();
  lcd.print("Wrong Attempts:");
  lcd.setCursor(0, 1);
  lcd.print(wrongAttemptsCount);
  while (true) {
    char key = kp.getKey();
    if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      break;
    }
  }
}

void adjustBrightness() {
  int lastLevel = -1; 
  lcd.clear();
  lcd.print("Adjust Brightness");
  while (true) {
    char key = kp.getKey();
    if (key == 'B') {
      brightnessLevel -= 25;
      if (brightnessLevel < 0) brightnessLevel = 0;
      analogWrite(lcdBacklight, brightnessLevel);
    }
    else if (key == 'A') {
      brightnessLevel += 25;
      if (brightnessLevel > 255) brightnessLevel = 255; 
      analogWrite(lcdBacklight, brightnessLevel); 
    }
    else if (key == 'D') {
      lcd.clear();
      lcd.print("Saved Brightness");
      delay(1000);
      updateMenuDisplay();
      menuSubState = MENU_LIST;
      return;
    }
    if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      break;
    }
    if (brightnessLevel != lastLevel) {
      lcd.setCursor(0, 1);
      lcd.print("Level: ");
      lcd.print(brightnessLevel);
      lcd.print("   "); 
      lastLevel = brightnessLevel;
    }
    delay(150); 
  }
}

void showFirmwareVersion() {
  lcd.clear();
  lcd.print("Firmware:");
  lcd.setCursor(0, 1);
  lcd.print(FIRMWARE_VERSION);
  while (true) {
    char key = kp.getKey();
    if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      break;
    }
  }
}

void showWelcomeMessage() {
  servo.write(0);
  lcd.clear();
  lcd.print("Welcome !");
  delay(2000);
}

void showHowToUse() {
  const int totalMessages = 5; 
  int currentMessage = 0; 
  int lastMessage = -1;
  String messages[totalMessages][2] = {
    {"Use keypad to", "enter password"},
    {"Use <A>/<B> to", "Scroll"},
    {"Press <C> to", "Go Back"},
    {"Press <D> to", "select"},
    {"Press <*> to", "Close Door"}
  };
  while (true) {
    if (currentMessage != lastMessage) {
      lcd.clear();
      lcd.print(messages[currentMessage][0]);
      lcd.setCursor(0, 1);
      lcd.print(messages[currentMessage][1]);
      lastMessage = currentMessage;
    }
    char key = kp.getKey();
    if (key == 'A') {
      currentMessage = (currentMessage - 1 + totalMessages) % totalMessages;
    }
    else if (key == 'B') {
      currentMessage = (currentMessage + 1) % totalMessages;
    }
    else if (key == 'C') {
      menuSubState = MENU_LIST;
      updateMenuDisplay();
      break;
    }
    delay(100);
  }
}
