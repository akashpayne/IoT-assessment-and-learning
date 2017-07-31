#include "mbed.h"
#include "C12832.h"  /* for the LCD */
#include "FXOS8700Q.h"

DigitalOut lr(PTB22), lg(PTE26), lb(PTB21), xr(D5), xg(D9), xb(PTC12);

Serial xbee(D1, D0);

C12832 lcd (D11, D13, D12, D7, D10);   /* LCD */

FXOS8700Q_acc acc( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // Proper Ports and I2C Address for K64F Freedom board
FXOS8700Q_mag mag( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); // Proper Ports and I2C Address for K64F Freedom board

DigitalIn up(A2);                       /* connections for Joystick */
DigitalIn down(A3);
DigitalIn left(A4);
AnalogIn right(A5);
DigitalIn fire(D4);

MotionSensorDataUnits acc_data;
MotionSensorDataUnits mag_data;
bool alarm = false; 

float xPosMax, xPosMin, yPosMax, yPosMin;
float x = 0, y = 0;

void xySet(){
    acc.getAxis(acc_data);
    x = (x + acc_data.x * 32)/2;
    y = (y -(acc_data.y * 16))/2;
}

void offLED() {
    wait(0.5f); lr = lb = lg = xr = xg = xb = 1.0; 
}

void togglexg() {
    xg = !xg; 
}

void togglexr() {
    xr = !xr; 
}

void togglelb() {
    lb = !lb; 
}

void togglexb() {
    xb = !xb;
}

void togglelg() {
    lg = !lg; 
}
 
void togglelr() {
    lr = !lr; 
}
   
void clearLCD() {
    lcd.cls();
    lcd.locate(1,1);
}


void doorGuage() 
{
    lcd.locate(1,1);     
    lcd.printf("Acc: ");
    xySet();
    lcd.locate(25,1); 
    lcd.printf("%0.2f, %0.2f \n", x, y);
    togglexg();
}

void setPos() 
{
    bool alarm = false; 
    while(!alarm) {
        doorGuage(); 
        lcd.locate(10, 10);
        lcd.printf("Hold the joystick button");
        lcd.locate(25, 21);
        lcd.printf("(fire) to set.");
        if ( fire )
        {
            xPosMax = x + 0.5f; 
            xPosMin = x - 0.5f;
            togglexg();
            alarm = true;
        }
         
    }
    togglexb();
}

bool closedDoor(float xTemp) {
    if (xTemp > xPosMax) {
        return false;
    }
    else if (xTemp < xPosMin) {
        return false; 
    } 
    else 
    {
        return true; 
    }     
}

void sendDeviceID()
{
    lcd.locate(15, 23);
    lcd.printf("Device : 1 ");
    xbee.printf("1");
    wait(0.2f);
    togglelb();   
}

void doorSensor() 
{
    int cnt = 0;  
    offLED();
    while (true) 
    {
        xySet();
        offLED();
        doorGuage();
        if ( closedDoor(x) ) 
        { 
            xbee.printf("C");
            lcd.locate(15,15); 
            lcd.printf("Door is Closed.");
            togglelg(); 
            offLED();
            sendDeviceID();
        } 
        else
        {
            xbee.printf("O");
            lcd.locate(15,15);
            lcd.printf("Door is Open.");
            togglelr();
            offLED();
            sendDeviceID();
        }  
    }
}

int main()
{
    offLED();
    offLED();
    acc.enable();
    // sets the alarm 
    setPos();
    clearLCD();
    // main loop
    doorSensor();
}

