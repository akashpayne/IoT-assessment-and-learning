import java.applet.Applet;
import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.*;
import java.lang.Math.*;

import javax.swing.Timer;

import org.mbed.RPC.*;
/**
 * Example applet showing how to use RPC to interface with a range of sensors connected to mbed.
 * @author Michael Walker
 * @license
 * Copyright (c) 2010 ARM Ltd
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 * <br>
 *The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 * <br>
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *THE SOFTWARE.
 * 
 */

public class CommPortTest extends Applet implements mbedRPC, MouseListener{
	//mbed Variables
	mbed mbed;
	DigitalOut LightOut;
	RPCFunction getTemp;
	RPCFunction getAccel;
	RPCFunction getAcc;
	RPCFunction getMag;

	AnalogIn pot1;
	AnalogIn pot2;
	
	DigitalIn PIRin;
	PwmOut RedLed;
	PwmOut BlueLed;
	PwmOut GreenLed;

	boolean threadSuspended;
	
	//Sensor Values
	float temps = 40;
	float LightClear, LightRed, LightGreen, LightBlue;
	double AmbientLight = 45;
	boolean PIR = false;
	
	//Sensor History
	int noReadings = 0;
	float[] temp;
	float[] accel;
	float[] acc;
	float[] mag;
	
	
	//Output Settings
	boolean Light, fan, heater = false;
	
	
	//Applet Variables
	final static BasicStroke wideStroke = new BasicStroke(8.0f);
	final static BasicStroke thinstroke = new BasicStroke(2.0f);
	final static BasicStroke linestroke = new BasicStroke(1.0f);

	Timer refresh_timer;
	private static final long serialVersionUID = 1L;

	public void init() {
		mbed = new HTTPRPC(this);
		//mbed = new SerialRxTxRPC("COM5", 9600);
		System.out.println("Created mbed Conenction");
		temps = new float[270];
		accels = new float[270];
		accs = new float[270];
		pot1s = new float[270];
		pot2s = new float[270];
		LightLevels = new float[270];
		
		getTemp = RPCFunction(mbed, "Temperature");
		getAccel = RPCFunction(mbed, "Accelerometer_MMA");
		getAcc = RPCFunction(mbed, "Accelerometer");
		getMag = RPCFunction(mbed, "Magnetometer");
		getPot_1 = RPCFunction(mbed, "Potentiometer_1");
		getPot_2 = RPCFunction(mbed, "Potentiometer_2");
		
		
		LightOut = new DigitalOut(mbed, LED1);
		RedLed = new PwmOut(mbed, D5);
		GreenLed = new PwmOut(mbed, D9);
		BlueLed = new PwmOut(mbed, D8);
		
		pot_1 = new AnalogIn(mbed, A0);
		pot_2 = new AnalogIn(mbed, A1);
		
		addMouseListener(this); 
		
		int rate = 1000; 
		try{
			rate =Integer.parseInt(getParameter("rate"));
		}catch(Exception e){
			System.err.println("No parameter found");
			rate = 1000;
		}
		refresh_timer = new Timer(rate, timerListener);
		refresh_timer.start();
		System.out.println("Initialised");
	}

	public void start(){
		refresh_timer.start();
		System.out.println("Started");
		
	}
	public void stop(){
		System.out.println("Stopped");
		refresh_timer.stop();
	}
	public void destroy(){
		System.out.println("Destroyed");
		
		//Delete any of the objects which were created by this applet 
		//This will prevent a build up of objects everytime someone loads the applet
		//Don't delete objects the applet tied to.
		LightOut.delete();
		RedLed.delete();
		GreenLed.delete();
		BlueLed.delete();
		LightSens.delete();
		PIRin.delete();
		
		//Close the connection - this isn't really required for HTTP but is for testing with serial and
		//so should be included for completeness and to ensure portability across different transport mechanisms
		mbed.delete();
	}
	
	ActionListener timerListener = new ActionListener() {
		public void actionPerformed(ActionEvent ev) {
			// TODO Auto-generated method stub
			refresh();
		}
        // Define an action listener to respond to events

  };
	
	public void paint(Graphics g) {
		Graphics2D g2 = (Graphics2D)g;
		//Draw layout
		g.setColor(Color.blue);
		g2.setStroke(wideStroke);
		g.drawRoundRect(4, 4, 696, 396, 30, 30);
		
		//Draw Titles
		
		Font mbedFont = new Font("Arial",Font.BOLD,24);
		g.setFont(mbedFont);
	    g.drawString("mbed Remote Sensors", 14 ,37);
	    
	    g.setColor(Color.black);
	    Font smallerFont = new Font("Arial",Font.PLAIN,16);
		g.setFont(smallerFont);
		g.drawString("Temperature", 14,60);
		g.drawString("Accelerometer_MMA", 175,60);
		g.drawString("Accelerometer", 14,200);
		g.drawString("Magnetometer", 14,300);
		g.drawString("Potentometer", 14,350);
		
		//Draw Sensor Results
			//Draw temp scales
		g.setColor(Color.black);
		g2.setStroke(linestroke);
		Font littleFont = new Font("Arial",Font.PLAIN,10);
		g.setFont(littleFont);
		for(int i = 0; i < 100; i = i + 10){
			g2.drawLine(60, 70 + i, 75, 70 + i);
			g.drawString(String.valueOf(60 - i), 82,70 + i + 4);
			g2.drawLine(100, 70 + i, 115, 70 + i);
		}
		g.setColor(Color.orange);
		int t = (int) temps;
		g.fillRect(20, (160 - 30 - t), 35, (t + 30));
		t = (int) ScpTemp_Value;
		g.fillRect(120, (160 - 30 - t), 35, (t + 30));
		g.setColor(Color.black);
		g.drawString(String.valueOf(temps) + " C", 25,175);
		g.drawString(String.valueOf(ScpTemp_Value) + " C", 125,175);
		
		
		//Draw the Pressure					- Need to work out scaling
		for(int i = 0; i < 100; i = i + 10){
			g2.drawLine(215, 70 + i, 220, 70 + i);
			g.drawString(String.valueOf(150 - i), 225,70 + i + 4);
		}
		g.setColor(Color.GREEN);
		int p = (int) Pressure;
		g.fillRect(175, (160 + 60 - p), 35, (p - 60));
		g.setColor(Color.black);
		g.drawString(String.valueOf(Pressure) + " kPa", 180,175);
		
		//Draw Light Sensor
		Color red = new Color( LightRed/100 ,0,0,1);
		Color green = new Color(0,LightGreen/100,0,1);
		Color blue = new Color(0,0,LightBlue/100,1);
		Color ActualColour = new Color(LightRed/100,LightGreen/100,LightBlue/100,1);
		g.setColor(red);
		g.fillRoundRect(20, 210, 50, 50,5,5);
		g.setColor(green);
		g.fillRoundRect(80, 210, 50, 50,5,5);
		g.setColor(blue);
		g.fillRoundRect(140, 210, 50, 50,5,5);
		g.setColor(ActualColour);
		g.fillRoundRect(200, 210, 50, 50,5,5);
		
		g.setColor(Color.black);
		g.drawString(String.valueOf(LightRed) + " %", 25,275);
		g.drawString(String.valueOf(LightGreen) + " %", 85,275);
		g.drawString(String.valueOf(LightBlue) + " %", 145,275);
		g.drawString(String.valueOf(LightClear) + " %", 205,275);
		
		//Draw Ambient Light Sensor
		g.setColor(Color.black);
		g2.setStroke(linestroke);
		for (int i = 0; i<100; i = i + 10){
			g2.drawLine(40 + i, 315, 40+i, 320);
		}
		g.setFont(littleFont);
		g.drawString(String.valueOf(AmbientLight) + " %", 150,315);
		g.setColor(Color.YELLOW);
		g.fillRect(40, 305, (int)AmbientLight, 10);
		
		
		//Draw PIR
		g.setFont(smallerFont);
		if(PIR == true){
			g.setColor(Color.red);
			g2.fillRoundRect(20, 355, 175, 35, 10,10);
			g.setColor(Color.black);
			g.drawString("PIR Triggered", 45, 375);
		}else{
			g.setColor(Color.green);
			g2.fillRoundRect(20, 355, 175, 35, 10,10);
			g.setColor(Color.black);
			g.drawString("PIR Not Triggered", 45, 375);
		}
		
		
		//Draw Controls
		g.setColor(Color.lightGray);
		g2.setStroke(thinstroke);
		g.drawRoundRect(350, 270, 320, 120, 20, 20);
		g.setColor(Color.black);
		g.setFont(smallerFont);
		g.drawString("Control Panel", 360,285);
		
		//Light box
		if(Light == false){
			g.setColor(Color.RED);
		}else{
			g.setColor(Color.GREEN);
		}
		g.fillRoundRect(360, 300, 75, 75, 15,15);

		
		// colour wheel
		g.setColor(Color.black);
		g2.setStroke(linestroke);
		for(float r = 0; r < (75/2); r++){
			for(float theta = 0; theta < 360; theta = theta + 2){
			 //Centered on 567.5
				g.setColor(Color.getHSBColor(theta/360, r/(75/2), 1));
				g.fillOval((int)(r * Math.cos(theta / 180 * Math.PI) + 517), (int)(r * Math.sin(theta/ 180 * Math.PI) + 337), 1, 1);
			}
		}
		
		g.setColor(Color.black);
		g.drawString("Light", 380,340);
		//g.drawString("Fan", 465 ,340);
		//g.drawString("Heater", 545,340);
		
		//Draw graphs of recent data
		g.setColor(Color.CYAN);
		g2.setStroke(thinstroke);
		g.drawRoundRect(350, 55, 320, 200, 20, 20);
		g.setColor(Color.black);
		g.setFont(smallerFont);
		g.drawString("History", 360,75);
		g2.setStroke(linestroke);
		g.drawLine(370, 230, 635, 230);
		g.setColor(Color.RED);
		g.drawLine(370, 230, 370, 85);
		g.setColor(Color.GREEN);
		g.drawLine(635, 230, 635, 85);
		//Add Scales - note temperature goes negative
		
		
		//Draw lines on graphs  - as either ovals or as a lines which connect the previous point to the next
		g.setColor(Color.ORANGE);
		if(temperatures != null){
			for(int n = 1; n < noReadings; n++){
				//g.fillOval(n + 370, (int)(230 - temperatures[n] - 30), 1, 1);
				g.drawLine(n + 369, (int)(230 - temperatures[n-1] -30), n + 370, (int)(230 - temperatures[n] -30));
			}
			g.setColor(Color.green);
			for(int n = 1; n < noReadings; n++){
				//g.fillOval(n + 370, (int)(230 - pressures[n] + 60), 1, 1);
				g.drawLine(n + 369, (int)(230 - pressures[n-1] + 60), n + 370, (int)(230 - pressures[n] + 60));
			}
			g.setColor(Color.BLACK);
			for(int n = 1; n < noReadings; n++){
				//g.fillOval(n + 370, (int)(230 - LightLevels[n] * 1.5), 2, 2);
				g.drawLine(n + 369, (int)(230 - LightLevels[n-1] * 1.5), n + 370, (int)(230 - LightLevels[n] * 1.5));
			}
		}
	}
	

	public void refresh(){
		//Load new variables
		try{
			temps = Float.parseFloat(getTemperature.run(" "));
			ScpTemp_Value = Float.parseFloat(getSCPTemperature.run(" "));
			Pressure = Float.parseFloat(getPressure.run(" ")) / 1000;
			
			String colourCSV = getColour.run(" ");
			
			String[] ColourArray = colourCSV.split(",");
			
			//Will be returned from mbed in range 0 to 1, need them as a percentage
			LightRed = Coerce(Float.parseFloat(ColourArray[1]),0,1) * 100; 
			LightGreen = Coerce(Float.parseFloat(ColourArray[2]),0,1)  * 100; 
			LightBlue = Coerce(Float.parseFloat(ColourArray[3]),0,1) * 100; 
			LightClear = Coerce(Float.parseFloat(ColourArray[0]),0,1) * 100; 
			//need to force them to be within range
			
			
			AmbientLight = LightSens.read() * 100;
			if(PIRin.read() == 1){
				PIR = true;
			}else{
				PIR = false;
			}
			
			//Add values to graph
			temperatures[noReadings] = temps;
			pressures[noReadings] = Pressure;
			LightLevels[noReadings] = (float) AmbientLight;
			noReadings++;
			if(noReadings >= 266)noReadings = 0;
				
		}catch(Exception e){
			System.err.println("Error Refreshing data from mbed");
		}
		//Redraw display
		repaint();
	}
	
	public void mousePressed (MouseEvent me){}
	public void mouseEntered (MouseEvent me) {}
	public void mouseClicked (MouseEvent me) {
		 int xpos = me.getX();
		 int ypos = me.getY();
		 
		 if(xpos > 480 && xpos < 555 && ypos > 300 && ypos < 375){
			 int xp = xpos - 517;
			 int yp = ypos - 337;
			 
			 double x = ((float)xp / 75) * 2;
			 double y = ((float)yp / 75) * 2;
			 double r =  Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2));
			 
			 double theta = Math.asin(y/r);
			 

			 if(y > 0 && x < 0)theta = (Math.PI) - theta;
			 
			 if(y < 0 && x > 0)theta = 2 * Math.PI + theta;
			 if(y < 0 && x < 0)theta = Math.PI - theta;
			 
			 double red = 1;
			 double green = 1;
			 double blue = 1;
			 //System.out.println(r + " " + Math.toDegrees(theta));
			 
			 float h = (float)Math.toDegrees(theta) / 360;
			 float s = (float) r;
			 int rgb = Color.HSBtoRGB(h,s, 1); 
			 red = (rgb >>16)& 0xFF; 
			 green = (rgb >> 8) &0xFF; 
			 blue = rgb & 0xFF; 
			 //System.out.println(red + " " + green +" " +  blue);
			 
			 RedLed.write(Coerce(red / 255, 0 ,1));
			 GreenLed.write(Coerce(green / 255, 0 ,1));
			 BlueLed.write(Coerce(blue/255, 0 ,1));
			 
			 //System.out.println(red/255 + " " + green/255 +" " +  blue/255);
			 
		 }
		 if(xpos > 360 && xpos < 435 && ypos > 300 && ypos < 375){
			 Light = !Light;
			 if(Light == true)LightOut.write(1);
			 if(Light == false)LightOut.write(0);
			 
			 
		 }
		repaint();
	}
	public void mouseReleased (MouseEvent me) {} 
	public void mouseExited (MouseEvent me) {}

	private float Coerce(float value, float min, float max){
		float result = value;
		if(value > max)result = max;
		if(value < min)result = min;
		return (result);
	}
	private double Coerce(double value, double min, double max){
		double result = value;
		if(value > max)result = max;
		if(value < min)result = min;
		return (result);
	}
}