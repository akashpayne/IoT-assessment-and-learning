#include "mbed.h"
#include "C12832.h"  /* for the LCD */

DigitalOut lr(PTB22), lg(PTE26), lb(PTB21), xr(D5), xg(D9), xb(PTC12);

Serial xbee(D1, D0);

C12832 lcd (D11, D13, D12, D7, D10);   /* LCD */

DigitalIn fire(D4);

bool alarm, xbeeFound = false;  
char c,g, tempc, tempg;

void offLED() { wait(0.4f); lr = lb = lg = xr = xg = xb = 1.0; }
void togglelg() { lg = !lg; } 
void togglelr() { lr = !lr; }
void togglexg() { xg = !xg; }
void togglexr() { xr = !xr; }
void togglexb() { xb = !xb; }
void clearLCD() { lcd.cls(); lcd.locate(12,1); }

bool openDoor() 
{
    lcd.locate(15,12);
    lcd.printf("Door is open!"); 
    togglelr();
    if (fire) { return false; }
    else { return true; }
}

void closeDoor()
{
    togglelg();
    lcd.locate(15,12);
    lcd.printf("Door is closed."); 
    
}

void deviceID() 
{
    togglexg();
    lcd.locate(18,20); 
    lcd.printf("Device : ", g);
    /** 
     * 
     * Send data via ethernet Data/Time stamp and Device number 
     * of the device that is open. Display on Graph?
     * 
     **/    
}

bool gMenu(char x) {
    switch (x) {
        case '1' : { 
            deviceID();
            return true;
        }
        case '2' : {
            deviceID();
            return true;
        }
        case 'e' : {
            tempc = c; 
            tempg = g;
            /** 
             * 
             * Send data via ethernet Data/Time stamp and Device number 
             * of the device that is open. Display on Graph?
             * 
             **/  
             offLED();
             return true;
        }
        default : break;  
    }
    return false;
}

void checkDevice() 
{
    if (xbee.readable()) 
    { 
        while (!xbeeFound) {   
            g = xbee.getc(); 
            xbeeFound = gMenu(g); 
            offLED(); 
        }     
    } 
}

void cMenu(char c) {
    switch (c) {
        case 'R' : { lr = 0.0; lg= 1.0; lb = 1.0; offLED(); }
        case 'G' : { lr = 1.0; lg= 0.0; lb = 1.0; offLED(); }
        case 'B' : { lr = 1.0; lg= 1.0; lb = 0.0; offLED(); }
        case 'O' : { 
            alarm = true; // change this so web variable  
            checkDevice();
            while (alarm) {
                alarm = openDoor(); 
            }
        }
        case 'C' : { 
            closeDoor();
            checkDevice();         
        }
        default : break;  
    }
}

int main()
{   
    lcd.locate(1,1);
    lcd.printf("Welcome. No readable xbee."); 
    while (true) {
        if ( xbee.readable() ) {
            c = xbee.getc();
            clearLCD();
            lcd.locate(5,1);
            lcd.printf("XBee Device says : ", c);
            cMenu(c);  
        }
        offLED();
    }            
}
