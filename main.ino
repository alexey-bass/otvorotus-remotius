
#include <LBattery.h>
#include <LCheckSIM.h>
#include <LGSM.h>
#include <LDateTime.h>

char num[20] = {0};
datetimeInfo t;

unsigned long lastStatusCheckTime = 0;
unsigned long statusCheckIntervalMs = 300000;
String statusReportPhone = "0540000000";

bool DEBUG_SERIAL = false;

// http://wiki.seeedstudio.com/Xadow_GSMPlusBLE/ - see pin numbers on icture
// https://forum.seeedstudio.com/viewtopic.php?t=6793&start=10
int RELAY_COMMAND_PIN = 3; // A1

bool test = false;

void setup() 
{
    if (DEBUG_SERIAL) delay(5000);    
    if (DEBUG_SERIAL) Serial.begin(115200);
    if (DEBUG_SERIAL) Serial.println("Otvorotus-Remotius");
    if (DEBUG_SERIAL) Serial.println();
    
    pinMode(RELAY_COMMAND_PIN, OUTPUT);
    digitalWrite(RELAY_COMMAND_PIN, HIGH); // LOW
}

void loop() 
{
    if (LVoiceCall.getVoiceCallStatus() == RECEIVINGCALL) {
        LVoiceCall.retrieveCallingNumber(num, sizeof(num));
        String incomingNumber = String(num);
        if (DEBUG_SERIAL) Serial.printf("Incoming call from %s", num);
        if (DEBUG_SERIAL) Serial.println();

        bool authorizedUser = incomingNumber.startsWith("0540000000"); // alexey bass
        
        if (authorizedUser) {
            if (DEBUG_SERIAL) Serial.println("Opening the gate");
            digitalWrite(RELAY_COMMAND_PIN, LOW); // on
            delay(1000);
            digitalWrite(RELAY_COMMAND_PIN, HIGH); // off

            LVoiceCall.hangCall();

            // send confirmation and it will also be counted at mobile operator for stats
            sendSMS("Welcome back, " + incomingNumber, num);

        } else {
            LVoiceCall.hangCall();
        }
    }
  
    if (DEBUG_SERIAL) {
        LDateTime.getTime(&t);
        Serial.printf("%d-%d-%d %d:%d:%d", t.year, t.mon, t.day, t.hour, t.min, t.sec);
        Serial.println();
    
        Serial.printf("Battery: LVL=%d CHRG=%d", LBattery.level(), LBattery.isCharging());
        Serial.println();
        
        Serial.printf("SIM: CHECK=%d", LCheckSIM.isCheck());
        Serial.println();
      
        Serial.println();
    }
    
    unsigned long timeSinceStart = millis();
    if (!lastStatusCheckTime
        || timeSinceStart < lastStatusCheckTime // millis() overflow
        || timeSinceStart - lastStatusCheckTime >= statusCheckIntervalMs) {

        String statusText = "";
        lastStatusCheckTime = timeSinceStart;

        if (!LBattery.isCharging() && LBattery.level() < 30) {
            statusText += "Battery level is less than 30%! "
        }
        
        if (statusText.length() > 0) {
            sendSMS(statusText, statusReportPhone);
        }
    }
  
    delay(500);
}

void sendSMS(String message, String phoneNumber)
{
    LSMS.beginSMS(phoneNumber.c_str());
    LSMS.print(message.c_str());
    LSMS.endSMS();
}
