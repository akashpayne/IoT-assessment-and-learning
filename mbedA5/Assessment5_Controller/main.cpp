#include "mbed.h"
#include "C12832.h"  /* for the LCD */
#include "EthernetNetIf.h"
#include "HTTPServer.h"

LocalFireSystem fs("websf");

EthernetNetIf eth(
    IpAddr(192,168,1,10), //IP Address
    IpAddr(255,255,255,0), //Network Mask
    IpAddr(192,168,1,1), //Gateway
    IpAddr(192,168,1,1)  //DNS
);
HTTPServer svr; 

DigitalOut lr(PTB22), lg(PTE26), lb(PTB21), xr(D5), xg(D9), xb(PTC12);

RpcDigitalOut myled(xb,"blueLED");

Serial xbee(D1, D0);
//Serial pc(USBTX, USBRX); 

C12832 lcd (D11, D13, D12, D7, D10);   /* LCD */

DigitalIn fire(D4);

RpcDigitalOut lr1(PTB22, "lr1");

//RPCVariable<float> RPCAcc(&GetAcc, "acc");
//RPCVariable<float> RPCdevice(&GetDev, "dev");


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
    //lcd.printf(": ", g);
    /** 
     * 
     * Send data via ethernet Data/Time stamp and Device number 
     * of the device that is open. Display on Graph?
     * 
     **/    
}

bool gMenu(char x) {
    lcd.locate(18,20); 
    lcd.printf("Device : ");
    switch (x) {
        case '1' : { 
            lcd.printf(": 1");
            return true;
        }
        case '2' : {
            lcd.printf(": 2");
            return true;
        }
        case 'e' : {
            lcd.printf(": e");
            tempc = c; 
            tempg = g;
            deviceID();
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
    togglexr(); 
    offLED();
    return false;
}

void cMenu(char c) {
    clearLCD();
    lcd.locate(5,1);
    lcd.printf("XBee Device says : ");
    switch (c) {
        case 'R' : { lr = 0.0; lg= 1.0; lb = 1.0; offLED(); }
        case 'G' : { lr = 1.0; lg= 0.0; lb = 1.0; offLED(); }
        case 'B' : { lr = 1.0; lg= 1.0; lb = 0.0; offLED(); }
        case 'O' : { 
            alarm = true; // change this - web variable  
            while (alarm) {
                alarm = openDoor(); 
                g = xbee.getc(); 
                xbeeFound = gMenu(g); 
                offLED(); 
                if (fire) {alarm = false; }
            }
        }
        case 'C' : { 
            closeDoor();
            g = xbee.getc(); 
            xbeeFound = gMenu(g);    
        }
        default : break;  
    }
}

void httpServer() {
    
        printf("\r\nSetup OK\r\n");
        
        FSHandler::mount("/webfs". "/files"); 
        FSHandler::mount("/webfs", "/");
        
        svr.addHandler<SimpleHandler>("/hello");
        svr.addHandler<RPCHandler>("/rpc");
        svr.addHandler<FSHandler>("/files");
        svr.addHandler<FSHandler>("/"); 
        // Access to mbed.htm :  http//a.b.c.d/mbed.htm or http://a.b.c.d/files/mbed.htm
        
        sv.bind(80);
        
        printf("Listening...\r\n");
        
        Timer tm;
        t,.start();
        //Listen..\r\n
    }
}

int main()
{   
    Base::add_rpc_class<DigitalOut>();
    
    printf("\r\nSetting up...\r\n");
    EthernetErr ethErr = eth.setup();
    if (ethErr) {
        httpServer();
        lcd.locate(1,1);
        lcd.printf("Welcome. No readable xbee."); 
        while (true) {
            if ( xbee.readable() ) {
                c = xbee.getc();
                cMenu(c);  
            }
            //clearLCD();
            offLED();
            if(tm.read()>.5)
                {
                    togglexb();
                    tm.start();
                }
            }
        }
    }    
    return 0;           
}
