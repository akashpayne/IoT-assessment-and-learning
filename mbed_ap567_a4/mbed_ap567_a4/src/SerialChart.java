import java.awt.Color;  
import java.io.BufferedReader;  
import java.io.InputStreamReader;  
import java.io.OutputStream;  
import java.util.Enumeration;  
  
import gnu.io.CommPortIdentifier;  
import gnu.io.SerialPort;  
import gnu.io.SerialPortEvent;  
import gnu.io.SerialPortEventListener;  
  
import org.jfree.chart.ChartFactory;  
import org.jfree.chart.ChartPanel;  
import org.jfree.chart.JFreeChart;  
import org.jfree.chart.axis.DateAxis;  
import org.jfree.chart.plot.XYPlot;  
import org.jfree.data.time.Millisecond;  
import org.jfree.data.time.TimeSeries;  
import org.jfree.data.time.TimeSeriesCollection;  
import org.jfree.data.xy.XYDataset;  
import org.jfree.ui.ApplicationFrame;  
import org.jfree.ui.RefineryUtilities;  
  
public class SerialChart extends ApplicationFrame implements SerialPortEventListener {  
  
 private TimeSeriesCollection timeSeriesCollection; // Collection of time series data  
 private XYDataset xyDataset; // dataset that will be used for the chart  
 private TimeSeries seriesX; // X series data  
 private TimeSeries seriesY; // Y series data  
 private TimeSeries seriesZ; // X series data  
   
 private BufferedReader input; // input reader  
 private OutputStream output; //output reader  
 private SerialPort serialPort; // serial port object  
   
 private String [] PORT_NAMES = {"COM7"}; // available ports  
   
 public SerialChart(String title) {  
  super(title);  
    
  initializeSerial();  
    
  timeSeriesCollection = new TimeSeriesCollection();  
  seriesX = new TimeSeries("SensorX");  
  seriesY = new TimeSeries("SensorY");  
  seriesZ = new TimeSeries("SensorZ");  
    
  timeSeriesCollection.addSeries(seriesX);  
  timeSeriesCollection.addSeries(seriesY);   
  timeSeriesCollection.addSeries(seriesZ);  
    
  JFreeChart chart = createChart();  
  ChartPanel chartPanel = new ChartPanel(chart);  
  chartPanel.setFillZoomRectangle(true);  
  chartPanel.setMouseWheelEnabled(true);  
  chartPanel.setPreferredSize(new java.awt.Dimension(1000,500));  
  setContentPane(chartPanel);  
 }  
  
 @Override  
 public synchronized void serialEvent(SerialPortEvent event) {  
  if(event.getEventType() == SerialPortEvent.DATA_AVAILABLE)  
  {  
   try  
   {  
    String inputLine = input.readLine();  
    String [] inputValues = inputLine.split(",");  
      
    if(inputValues[0].equals("STX") && inputValues[inputValues.length-1].equals("ETX"))  
    {  
     float in_x = new Float(inputValues[1]).floatValue();  
     float in_y = new Float(inputValues[2]).floatValue();  
     float in_z = new Float(inputValues[3]).floatValue();  
       
     this.timeSeriesCollection.getSeries(0).add(new Millisecond(),in_x);  
     this.timeSeriesCollection.getSeries(1).add(new Millisecond(),in_y);  
     this.timeSeriesCollection.getSeries(2).add(new Millisecond(),in_z);  
    }  
      
    System.out.println(inputLine);  
   }  
   catch(Exception ex)  
   {  
    ex.printStackTrace();  
   }  
  }  
 }  
   
 private void initializeSerial()  
 {  
  CommPortIdentifier portId = null;  
  Enumeration portEnum = CommPortIdentifier.getPortIdentifiers();  
    
  while(portEnum.hasMoreElements())  
  {  
   CommPortIdentifier currentPortIdentifier = (CommPortIdentifier)portEnum.nextElement();  
   for(String portName : PORT_NAMES)  
   {  
    if(currentPortIdentifier.getName().equals(portName))  
    {  
     portId = currentPortIdentifier;  
     break;  
    }  
   }     
  }  
    
  if(portId == null)  
  {  
   System.out.println("Port not found");  
   return;  
  }  
    
  try {  
     
   serialPort = (SerialPort)portId.open(this.getClass().getName(), 2000);  
   serialPort.setSerialPortParams(9600, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);  
     
   input = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));  
   output = serialPort.getOutputStream();  
     
   serialPort.addEventListener(this);  
   serialPort.notifyOnDataAvailable(true);  
     
  } catch (Exception e) {  
   System.err.println("Initialization failed : " + e.toString());  
  }  
 }  
  
 private JFreeChart createChart() {  
  
        JFreeChart chart = ChartFactory.createTimeSeriesChart(  
            "Sensor Data Display",  // title  
            "Time",             // x-axis label  
            "Sensor Value",   // y-axis label  
            timeSeriesCollection,            // data  
            true,               // create legend?  
            true,               // generate tooltips?  
            false               // generate URLs?  
        );  
  
        chart.setBackgroundPaint(Color.white);  
  
        XYPlot plot = (XYPlot) chart.getPlot();  
  
        DateAxis axis = (DateAxis) plot.getDomainAxis();  
        axis.setAutoRange(true);  
  axis.setFixedAutoRange(60000.0);  
  
        return chart;  
 }  
  
 public synchronized void close()  
 {  
  if(serialPort != null)  
  {  
   serialPort.removeEventListener();  
   serialPort.close();  
  }  
 }  
   
 public static void main(String[] args) {  
  
  SerialChart serialChartDemo = new SerialChart("Time Series Chart Demo");  
        serialChartDemo.pack();  
        RefineryUtilities.centerFrameOnScreen(serialChartDemo);  
        serialChartDemo.setVisible(true);  
  
    }  
}  