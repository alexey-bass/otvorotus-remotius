
#include <LBattery.h>
#include <LCheckSIM.h>
#include <LGSM.h>
#include <LDateTime.h>

char num[20] = {0};
String incomingNumber;
datetimeInfo t;

int statusCheckCount = 0;
int statusCheckInterval = 600; // number of loops
String statusReportPhone = "0540000000";
String statusText;

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
        LVoiceCall.retrieveCallingNumber(num, 20);
        incomingNumber = String(num);
        if (DEBUG_SERIAL) Serial.printf("Incoming call from %s", num);
        if (DEBUG_SERIAL) Serial.println();
        
        if (
               incomingNumber.startsWith("0540000000") // alexey bass
            ) {
            if (DEBUG_SERIAL) Serial.println("Opening the gate");
            digitalWrite(RELAY_COMMAND_PIN, LOW); // on
            delay(1000);
            digitalWrite(RELAY_COMMAND_PIN, HIGH); // off
        }
        LVoiceCall.hangCall();
        
        // send confirmation and it will also be counted at mobile operator for stats
        LSMS.beginSMS(num);
        LSMS.print("Welcome back, "+ incomingNumber);
        LSMS.endSMS();
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
    
    statusCheckCount = (statusCheckCount + 1) % statusCheckInterval;
    if (statusCheckCount == 0) {
        statusText = "";

        if (!LBattery.isCharging() && LBattery.level() < 30) {
            statusText += "Battery level is less than 30%! "
        }
        
        if (statusText.length() > 0) {
            LSMS.beginSMS(statusReportPhone);
            LSMS.print(statusText);
            LSMS.endSMS();
        }
    }
  
    delay(500);
}
