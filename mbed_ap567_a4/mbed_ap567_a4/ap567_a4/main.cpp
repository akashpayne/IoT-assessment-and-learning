/**
 *  Project Name: ap567_a4.cpp 
 *  Assignment 4 for CO657 (Internet of Things)
 *  Author: Akash Payne - ap567
 *  Date: 27/11/2015 - Update 59
 **/
 // LIBRARIES
#include "mbed.h"
#include "C12832.h"
#include "LM75B.h"
#include "MMA7660.h"
#include "FXOS8700Q.h"
#include "MQTTClient.h"
#include "MQTTEthernet.h"
#include "Websocket.h"
#include "EthernetInterface.h"
#include "NTPClient.h"
#
//#include HTTPClient.h"
//#include "SimpleSMTPClient.h"

// DEFINITIONS
#define MQTT_BROKER_IP "129.12.3.210"
#define MQTT_CLIENT_ID "app567"
#define MQTTCLIENT_QOS2 1


#ifndef PI
#define PI           3.14159265358979323846
#endif

// MQTT 
int arrivedcount = 0;

// OUTPUTS
DigitalOut lr(PTB22), lg(PTE26), lb(PTB21);
PwmOut xr(D5), xg(D9), xb(D8);
C12832 lcd(D11, D13, D12, D7, D10);

// INPUTS
LM75B temp(D14, D15);
MMA7660 accel(D14, D15);
//FXOS8700CQ magAccel(D14, D15, FXOS8700CQ_SLAVE_ADDR1);
MotionSensorDataUnits acc_data;
MotionSensorDataUnits mag_data;
FXOS8700Q_acc acc( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); 
FXOS8700Q_mag mag( PTE25, PTE24, FXOS8700CQ_SLAVE_ADDR1); 

// INTERRUPTS 
InterruptIn sw2_int(SW2), sw3_int(SW3), up_int(A2), down_int(A3), left_int(A4), right_int(A5), fire_int(D4);
DigitalIn sw2(SW2), sw3(SW3), up(A2), down(A3), left(A4), right(A5), fire(D4);

// ANALOGS (POT_1 & _2)
AnalogIn pot1(A0);
AnalogIn pot2(A1);

// INTERRUPT FLAGS
uint16_t flags;
#define SW2_DOWN   0x0001
#define SW2_UP     0x0002
#define SW3_UP     0x0004
#define SW3_DOWN   0x0008
#define UP_UP      0x0010
#define UP_DOWN    0x0020
#define DOWN_UP    0x0040
#define DOWN_DOWN  0x0080
#define LEFT_UP    0x0100
#define LEFT_DOWN  0x0200
#define RIGHT_UP   0x0400
#define RIGHT_DOWN 0x0800
#define FIRE_UP    0x1000
#define FIRE_DOWN  0x2000

// Ticker and Timeout 
Ticker waitTicker;
Ticker send_info;
Ticker check_move;
int ctr; // counter 
Timeout flipper; 
static volatile int menu = 0; // Menu Counter

// DATA PROCESSING

// COM SERIAL
Serial pc(USBTX, USBRX); // Com 5 - laptop // Com 8 if PC

// WEB SOCKET
char dataOut[128];

// LED 
float brightness = 0.0;

float standard_deviation(float data[], int n, float *mean );
void CalaulateXYZStatisticValue();

// LED  colours
void off() {lr = lg = lb = xr = xg = xb = 1.0;}
void red() { lr = brightness; lg = 1.0; lb = 1.0; }
void yellow() { lr = 0.7; lg = 0.7; lb = 1.0; }
void green() { lr = 1.0; lg = brightness; lb = 1.0; }

// INTERRUPT HANDLERS (Down, Rise and Up)
void sw2Down() { flags |= SW2_DOWN; }
void sw2Up()  { flags |= SW2_UP; }

void sw3Down() { flags |= SW3_DOWN; }
void sw3Up()  { flags |= SW3_UP; }

void upDown() { flags |= UP_DOWN; }
void upUp()  { flags |= UP_UP; }

void downDown() { flags |= DOWN_DOWN; }
void downUp()  { flags |= DOWN_UP; }

void leftDown(){ 
    flags |= LEFT_DOWN;
    menu--;
    if(menu < 0) {
        menu = 0; 
    }       
}
void leftUp()  { flags |= LEFT_UP; }

void rightDown() { 
    flags |= RIGHT_DOWN;
    menu++;
    if(menu > 5) {
        menu = 4;
    }
}
void rightUp()  { flags |= RIGHT_UP; }

void fireDown() { flags |= FIRE_DOWN; }
void fireUp()  { flags |= FIRE_UP; }

// TIMEOUT/TICKERS HANDLERS
void flip() { lg = !lg; lr = 1; lb = 1; }

void waitTick() {
    ctr++;
    if (ctr >= 150) {
        yellow(); 
    } else if (ctr <= 300) { 
        red();
    } else { ctr == 0; off(); }
}

// 
void cleanup() {
    lr = lg = lb = 1;
    //xr = xg = xb = 1;
    temp.read();
    lcd.cls();
    pc.putc('+');
}

//  
void checkFlags() {
    waitTick();
    if(flags&SW2_UP) {
        flags&=!SW2_UP; pc.printf("!2u"); red();
    }
    if(flags&SW2_DOWN) {
        flags&=!SW2_DOWN; pc.printf("!2d"); red();
    }
    if(flags&SW3_UP) {
        flags&=!SW3_UP; pc.printf("!3u"); red();

    }
    if(flags&SW3_DOWN) {
        flags&=!SW3_DOWN; pc.printf("!3d"); red();
    }
    if(flags&LEFT_UP) {
        flags&=!LEFT_UP; pc.printf("!lu"); green();
    }
    if(flags&LEFT_DOWN) {
        flags&=!LEFT_DOWN; pc.printf("!ld"); red();
    }
    if(flags&RIGHT_UP) {
        flags&=!RIGHT_UP; pc.printf("!ru"); green();
    }
    if(flags&RIGHT_DOWN) {
        flags&=!RIGHT_DOWN; pc.printf("!rd"); red();
    }
    if(flags&FIRE_UP) {
        flags&=!FIRE_UP; pc.printf("!fu"); red();
    }
    if(flags&FIRE_DOWN) {
        flags&=!FIRE_DOWN; pc.printf("!fd"); red();
    }
}

// 
void setInterrupts() {
    sw2_int.mode (PullUp); sw2_int.fall(&sw2Down); sw2_int.rise(&sw2Up);
    sw3_int.mode (PullUp); sw3_int.fall(&sw3Down); sw3_int.rise(&sw3Up);
    left_int.mode (PullUp); left_int.fall(&leftDown); left_int.rise(&leftUp);
    right_int.mode (PullUp); right_int.fall(&rightDown); right_int.rise(&rightUp);
    fire_int.mode (PullUp); fire_int.fall(&fireDown); fire_int.rise(&fireUp);
}

// Data Processing Methods: 
float standard_deviation(float data[], int n, float *Mean )
{
    float mean=0.0;
    float sum_deviation=0.0;
    int i;
    for(i=0; i<n;++i)
    {
        mean+=data[i];
    }
    mean=mean/n;
    *Mean = mean;
    for(i=0; i<n;++i)
    sum_deviation+=(data[i]-mean)*(data[i]-mean);
    return sqrt(sum_deviation/n);           
}

// PRINT METHODS
void printWelcomeLCD() {
    lcd.locate(0,0);  lcd.printf("Akash's MBED Device:");
    lcd.locate(1,10); lcd.printf("  MQTT, eCompass, Weather");
    lcd.locate(1,20); lcd.printf("  Station, LCD Drawer. -> ");
}
void printWelcomePC() { printf("/n/n/n Please Type '-' to refresh Connection, "); }
void printMenuPC() { printf("-->    Menu: ( q, -, e )\n"); }

// WebSocket
int web_Socket() {
    int ws_ret, eth_ret, ret; 
    EthernetInterface eth;
    eth.init(); // DHCP
    //lcd.cls();
    pc.printf("\n\n/*  \n");
    pc.printf("Setting up ...\n");

    eth.connect();
    ret = eth.connect(); 
    if ((ret = eth.connect()) != 0) {
        pc.printf("Error eth.connect() - ret = %d\n", ret);
        lcd.cls();
        lcd.printf("Ethernet not connected.");
        wait(0.5);
        return ret;
    } 
    pc.printf("Connected OK\n");
        
    // IP ADDRESS
    pc.printf("IP Address is %s\n", eth.getIPAddress());
    lcd.locate(0,1);
    lcd.printf("%s", eth.getIPAddress());
    wait(1);
    
    // NTP Client
    NTPClient ntp;
    ntp.setTime("time.nist.gov");                                    
    //lcd.cls(); 

    
    Websocket ws("ws://sockets.mbed.org:443/co657_ap567/wo");
    printf("Connecting to mbed websocket server channel 443, co567_ap567/wo");
    ws_ret = ws.connect();
    if (!ws_ret) {
            lcd.cls(); 
            lcd.printf("Not connected to URL.");
    } 
    printf("\n Ticker set: \n\r");
    //myTicker.attach(&callback, 0.5); // send data every half a second
    float p1, p2;
    
    ctr=0;
    while(1) {
        if (xr) {
            ctr++;
            if (ctr>=20) {ctr=0;};
            p1 = (float)pot1;
            p2 = (float)pot2;
            sprintf(dataOut, "{\"X\":%.2f, \"Y\":2f}",p1, p2); 
            ws.send(dataOut);
            xr=0;
        }
    }
}

// MQTT 
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

int MQTTConnect(const char *ip, char *uid)
{   
    MQTTEthernet ipstack = MQTTEthernet();
    float version = 0.5;
    char* topic = "mbed-co657_ap567";
    
    printf("HelloMQTT: version is %f\n", version);
              
    MQTT::Client<MQTTEthernet, Countdown> client = MQTT::Client<MQTTEthernet, Countdown>(ipstack);
    
    char* hostname = "doughnut.kent.ac.uk";
    int port = 1883;
    printf("Connecting to %s:%d\n", hostname, port);
    int rc = ipstack.connect(hostname, port);
    if (rc != 0)
        printf("rc from TCP connect is %d\n", rc);
 
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
    data.MQTTVersion = 3;
    data.clientID.cstring = uid;
    data.username.cstring = "ap567";
    data.password.cstring = "testpassword";
    if ((rc = client.connect(data)) != 0)
        printf("rc from MQTT connect is %d\n", rc);
    
    if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0)
        printf("rc from MQTT subscribe is %d\n", rc);
 
    MQTT::Message message;
 
    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f\n", version);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    while (arrivedcount < 1)
        client.yield(100);
        
    // QoS 1
    sprintf(buf, "Hello World!  QoS 1 message from app version %f\n", version);
    message.qos = MQTT::QOS1;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    while (arrivedcount < 2)
        client.yield(100);
        
    // QoS 2
    sprintf(buf, "Hello World!  QoS 2 message from app version %f\n", version);
    message.qos = MQTT::QOS2;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    while (arrivedcount < 3) client.yield(100);
    if ((rc = client.unsubscribe(topic)) != 0) printf("rc from unsubscribe was %d\n", rc);
    if ((rc = client.disconnect()) != 0) printf("rc from disconnect was %d\n", rc);
    ipstack.disconnect();
    printf("Version %.2f: finish %d msgs\n", version, arrivedcount);
    return 0;
}

// MAIN 
int main() {          
    float x = 0, y = 0;
    setInterrupts();
    ctr=0; 
    acc.enable();
    lcd.cls();     lcd.locate(1,1);     lcd.printf("Connect via Serial: ...");
    waitTicker.attach(&waitTick,0.02);
    while( (pc.getc()!='-') );
    waitTicker.detach();
    cleanup();
    union { float f; int i; char c[4]; } val;
    for(;;) {
        checkFlags(); wait(0.25); off(); //Check if any interrupts fired
        switch (menu) {

            case 4 : {
                lcd.cls();     lcd.locate(1,1);     lcd.printf("Serial Window");
                lcd.locate(1,10); lcd.printf(" <- ");
                printMenuPC();
                while(menu == 4) {
                    checkFlags(); wait(0.1); off();
                    if(pc.readable()) {
                        char cmd = pc.getc();
                        switch(cmd) {
                            case 'u': { // sound up
                                brightness += 0.001;
                                if (brightness >= 1.000) {brightness = 1.000;} 
                                pc.printf("brightness: %.2f\n", brightness);
                            }
                            case 'd': { // sound down
                                brightness-=0.001;
                                if (brightness <= 0.000) {brightness = 0.000;}
                                pc.printf("brightness: %.2f\n", brightness);
                            }
                            case 't': {} // connect mqtt }
                            case 'w': { web_Socket(); }
                            case 'j': {
                                acc.getAxis(acc_data);
                                lcd.cls();     lcd.locate(1,1);     lcd.printf("Menu: Level Guage ");
                                wait(0.5);
                                x = (x + acc_data.x * 32)/2;
                                y = (y -(acc_data.y * 16))/2;
                                lcd.printf("Acc : %0.2f, %0.2f \n", x, y);
                            }
                            case 'c': {
                                lcd.locate(1, 10);
                                double heading = atan2(mag_data.y, mag_data.x);
                                if(heading < 0)
                                    heading += 2*PI;
                                if(heading > 2*PI)
                                    heading -= 2*PI;
                                heading = heading * 180 / PI;
                                lcd.printf("Heading %1.2f", heading);
                            }
                            case 'q': {break;}
                            case 'e': {break;}
                            case '-': { cleanup(); }
                            default: break;
                        }
                    }
                }
                wait(0.5);
            }
            
            case 3 : {
                while (menu == 3) {
                    lcd.cls(); lcd.locate(1, 1); lcd.printf("LCD Window 3"); 
                    lcd.locate(1, 10); lcd.printf("      <->");
                    wait(1);
                }
            }
            
            case 2 : {
                while (menu == 2) {
                    wait(0.25);
                    mag.getAxis(mag_data);
                    lcd.cls(); lcd.locate(1, 1); lcd.printf("Mode: LCD eCompass");  
                    lcd.locate(8, 10);
                    double heading = atan2(mag_data.y, mag_data.x);
                    if(heading < 0)
                        heading += 2*PI;
                    if(heading > 2*PI)
                        heading -= 2*PI;
                    heading = heading * 180 / PI;
                    lcd.printf("Heading %1.2f", heading);
                    //lcd.printf("X: %1.2f Y: %1.2f", mag_data.x, mag_data.y);
                     lcd.locate(1, 20); lcd.printf("      <->");
                }
            }
            
            case 1 : {
                int p1, p2; 
                lcd.cls();     lcd.locate(1,1);     lcd.printf("Menu: Draw with Pot1 & 2");
                lcd.locate(1,10); lcd.printf("Use Potentiometers to draw");
                lcd.locate(1, 20); lcd.printf("      <->");
                wait(2);
                lcd.cls();
                while(menu == 1) {
                    float p1 = (float)pot1;
                    float p2 = (float)pot2;
                    int x = floor((p1 * 127) + 0.5);
                    int y = floor((p2 * 32) + 0.5);
                    lcd.pixel(x,y,1);
                    lcd.copy_to_lcd();
                    wait(.005);
                }
            }
            
            case 0 : {
                lcd.locate(1, 10); lcd.printf("      ->");
                lcd.cls();
                while (menu == 0) {
                    printWelcomeLCD();
                    wait(1);
                }
            }
        
        }
    }   
}
