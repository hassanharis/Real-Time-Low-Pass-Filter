#include <SPI.h>
#include <RingBuf.h>
#include <ADC.h>
#include <ADC_util.h>

SPISettings SettingsDAC(300000, MSBFIRST, SPI_MODE0);

//Buffer & Timer
RingBuf<unsigned int, 256> Buffer1; //max buffer 65535
//Create an IntervalTimer object
IntervalTimer myTimer;
const int numberoflevels = 256;
const int updaterate = 5;


// DAC MCP4922 Pins:
const int selectDAC = 34;
const int LDAC = 24;
const int SHDN = 25;


/////////// Filter Parameters ///////////
int Nc=9; // Number of Cooficients
const int nc = 9;
// Filter Cofficients
float b[nc] = {1e-35 *0.0045, 1e-35 * 0.0356, 1e-35 * 0.1247, 1e-35 * 0.2494, 1e-35 * 0.3118, 1e-35 * 0.2494, 1e-35 * 0.1247, 1e-35 * 0.0356, 1e-35 * 0.0045};
float a[nc] = { 1, -7.2169, 23.4949, -45.0120, 55.4682, -45.012, 23.4949, -7.2169, 1.0000};
float y[nc],x[nc];
float value, sal1, sal2;

int j, k;

void setup() {
  // SPI1 of Teensy 4.1 (MISO > pin 1, MOSI > pin 26, SCK > pin 27)
  SPI1.begin(); 

  // DAC MCP4922 Pins Settings:
  pinMode(selectDAC, OUTPUT);
  digitalWrite(selectDAC, HIGH); 

  pinMode(LDAC, OUTPUT);//LDAC
  digitalWrite(LDAC, LOW); //LDAC
  
  pinMode(SHDN, OUTPUT);//SHDN
  digitalWrite(SHDN, HIGH); //SHDN

  Serial.begin(11500);
  analogReadResolution(10);
  analogWriteResolution(10); 
  
  uint8_t i = 0;
  int amp=800;
  
  while(Buffer1.push(int(amp+amp*cos(2*10*3.14159*i++/numberoflevels))));
  myTimer.begin(updatevalue,updaterate);
}

void updatevalue(){
  MCP4922_write(1, Buffer1[j]); //test signal
  
  value = analogRead(24);
  Serial.println(value);

  //Mapping the value to input command interval of MCP4922
  value=map(value,0,768,0,4095);
  
  sal1=0;
  sal2=0;
  x[0] = value;

  for(int i=0 ; i<Nc ; i++){
    sal1=sal1 + x[i]*b[i];
  }
  
  for(int i=0; i<Nc ; i++){
    sal2=sal2 + y[i]*a[i];
  }
  y[0]=sal1-sal2;
   
  for(int i=0; i<Nc ; i++){
    y [Nc-i] = y [Nc-1-i];
    x [Nc-i] = x [Nc-1-i];
  }
  //_output=map(_output,0,50,1024,2048);
  MCP4922_write(0, y[0]);
   
  j++;
  if(j>256){
    j=0;
  }
}

void loop() {
  
}


void MCP4922_write(byte dac, int value) {
  byte low = value & 0xff; 
  byte high = (value >> 8) & 0x0f; 
  dac = (dac & 1) << 7; 
  _beginDACTransmission(selectDAC);
  SPI1.transfer(dac | 0x30 | high); 
  SPI1.transfer(low); 
  _endDACTransmission(selectDAC);
}

void _beginDACTransmission(int chipSelectPin) {
  ::digitalWrite(chipSelectPin, 0);
  SPI1.beginTransaction(SettingsDAC);
}

void _endDACTransmission(int chipSelectPin) {
  SPI1.endTransaction();
  ::digitalWrite(chipSelectPin, 1);
}
