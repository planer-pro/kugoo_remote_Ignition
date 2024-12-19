#include <Arduino.h>
#include <GyverButton.h>
#include <EEPROM.h>

#define CONTROL_PIN 13
#define BTN_PIN 3
#define BT_POWER_PIN 4

enum modes
{
    off,
    on,
    invert
};

bool lockFlag = true;
bool activeFlag = false;
bool repairFlag = false;

GButton btn(BTN_PIN, HIGH_PULL, NORM_OPEN);

void sendState_scoot()
{
    Serial.print(String(lockFlag) + String(activeFlag) + String(repairFlag)); // lock activ format
}

void activatioMode_scoot(uint8_t m)
{
    switch (m)
    {
    case off:
        digitalWrite(CONTROL_PIN, LOW);
        activeFlag = false;
        break;
    case on:
        digitalWrite(CONTROL_PIN, HIGH);
        activeFlag = true;
        break;
    case invert:
        digitalWrite(CONTROL_PIN, !digitalRead(CONTROL_PIN));
        activeFlag = !activeFlag;
        break;
    }

    EEPROM.put(1, activeFlag);
    sendState_scoot();
}

void lock_scoot()
{
    lockFlag = true;
    EEPROM.put(0, lockFlag);
    activatioMode_scoot(off);
}

void unlock_scoot()
{
    lockFlag = false;
    EEPROM.put(0, lockFlag);
    sendState_scoot();
}

// void restart_bt(uint16_t t)
// {
//     digitalWrite(BT_POWER_PIN, LOW);
//     delay(t);
//     digitalWrite(BT_POWER_PIN, HIGH);
// }

void btnHeadler()
{
    btn.tick();

    if (btn.isSingle())
    {
        if (!lockFlag)
            activatioMode_scoot(invert); // lock invertion
    }
    else if (btn.isHolded())
    {
        if (!lockFlag)
            // {
            if (!repairFlag)
                lock_scoot(); // lock
        // }
        // else
        //     restart_bt(2000);
    }

    btn.resetStates();
}

void setup(void)
{
    Serial.begin(115200);

    pinMode(CONTROL_PIN, OUTPUT);
    pinMode(BT_POWER_PIN, OUTPUT);

    digitalWrite(CONTROL_PIN, LOW);
    digitalWrite(BT_POWER_PIN, HIGH);

    // EEPROM.put(0, lockFlag);
    // EEPROM.put(1, activeFlag);
    // EEPROM.put(2, repairFlag);

    EEPROM.get(0, lockFlag);
    EEPROM.get(1, activeFlag);
    EEPROM.get(2, repairFlag);

    // Serial.println(lockFlag);
    // Serial.println(activeFlag);
    // Serial.println(repairFlag);

    activeFlag ? digitalWrite(CONTROL_PIN, HIGH) : digitalWrite(CONTROL_PIN, LOW);
}

void loop(void)
{
    btnHeadler();

    if (Serial.available())
    {
        char n = Serial.read();

        switch (n)
        {
        case 'l': // lock
            if (!repairFlag)
                lock_scoot();
            break;

        case 'u': // unlock
            if (!repairFlag)
                unlock_scoot();
            break;

        case 'a': // activate
            if (!repairFlag)
            {
                lockFlag = false;
                EEPROM.put(0, lockFlag);
            }
            activatioMode_scoot(on);
            break;

        case 'd': // deactivate
            activatioMode_scoot(off);
            break;

        case 'r': // repair
            repairFlag = !repairFlag;
            EEPROM.put(2, repairFlag);

            if (lockFlag)
                unlock_scoot();
            else
                sendState_scoot();
            break;

        case 's': // status
            sendState_scoot();
            break;
        }
    }
}