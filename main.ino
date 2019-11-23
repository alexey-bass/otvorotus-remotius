#include <LBattery.h>
#include <LCheckSIM.h>
#include <LGSM.h>
#include <LDateTime.h>

char num[20] = {0};
String CALLING_NUMBER;
datetimeInfo t;

unsigned long lastStatusCheckTime = 0;
unsigned long statusCheckIntervalMs = 60 * 60 * 1000; // 1 hour
String statusReportPhone = "0540000000";

bool DEBUG_SERIAL = false;

// http://wiki.seeedstudio.com/Xadow_GSMPlusBLE/ - see pin numbers on picture
// https://forum.seeedstudio.com/viewtopic.php?t=6793&start=10
int RELAY_COMMAND_PIN = 3; // A1



void setup() 
{
    if (DEBUG_SERIAL) delay(5000);    
    if (DEBUG_SERIAL) Serial.begin(115200);
    if (DEBUG_SERIAL) Serial.println("Otvorotus-Remotius");
    if (DEBUG_SERIAL) Serial.println();
    
    pinMode(RELAY_COMMAND_PIN, OUTPUT);
    digitalWrite(RELAY_COMMAND_PIN, HIGH); // LOW
    
    // This makes sure the modem notifies correctly incoming events
    LVoiceCall.hangCall();
}

void loop()
{
    if (LVoiceCall.getVoiceCallStatus() == RECEIVINGCALL) {
        LVoiceCall.retrieveCallingNumber(num, 20);
        delay(100);
        // hangup and do the business, no need to occupy the line
        LVoiceCall.hangCall();
        
        CALLING_NUMBER = String(num);
        if (DEBUG_SERIAL) Serial.printf("Incoming call from %s", num);
        if (DEBUG_SERIAL) Serial.println();

        if (
               CALLING_NUMBER.startsWith("0540000000") // alexey bass
        ) {
            if (DEBUG_SERIAL) Serial.println("Opening the gate");
            digitalWrite(RELAY_COMMAND_PIN, LOW); // on
            delay(1000);
            digitalWrite(RELAY_COMMAND_PIN, HIGH); // off
            
            // send confirmation and it will also be counted at mobile operator for stats
            if (CALLING_NUMBER.startsWith("05")) { // never send sms to not israeli local numbers
                // we want to know battery charging state
                // in case of power outage, our relay may not function
                sendSMS("Welcome back, "+ CALLING_NUMBER +"!\nBC="+ LBattery.isCharging() +" BL="+ LBattery.level(), CALLING_NUMBER);
            }
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

        int currentLevel = LBattery.level()
        if (!LBattery.isCharging() && currentLevel < 30) {
            statusText += "Battery level is " + currentLevel + "%";
        }
        
        if (statusText.length() > 0) {
            sendSMS(statusText, statusReportPhone);
        }
    }
  
    delay(1000);
}

void sendSMS(String message, String phoneNumber)
{
    LSMS.beginSMS(phoneNumber.c_str());
    LSMS.print(message.c_str());
    LSMS.endSMS();
}
