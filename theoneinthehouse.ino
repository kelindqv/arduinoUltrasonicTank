// -- ARDUINO IN HOUSE --
// Load driver for LED display
#include "LedControl.h"
LedControl lc=LedControl(12,11, 10,1);

// Define how many measurements to take when starting the device
//  to establish the water level
#define NUM_SAMPLES 5
// Define delay between measurements. This will allow echoes to ring out
//  before making a new measurement
#define M_DELAY 7000 // [ms]

int state = 0; // Switches between send, receive, process + wait states
unsigned long int measurement = 0;          // placeholder for sensor results
unsigned long int latest_measurement = 0;   // placeholder for previous result
int count = 0;                              // counter for samples

/* we always wait a bit between updates of the display */
unsigned long delaytime=250;

// Configure LED display and serial port
void setup() {
  delay(1000);
  Serial.begin(2400); 
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
}

// Define a function that writes a number to the display. Append
//  the letter 'L', since we will be showing results in litre.
void led_write(uint16_t number) {
  lc.clearDisplay(0);
  char chars[8] = {0};
  unsigned int len = sprintf(chars,"%u",number);
  for (int i=0;i<len;i++)
  {
    lc.setChar(0,7-i,chars[i],false);
  }

  if (len <7)
  {
    lc.setChar(0,7-len-1,'L',false);
  }
} 

void loop() { 
  // Cycle through states: send, receive, process + wait
  if (state == 0)
  {
    // Send trig to the one in the well - has to be 162
    Serial.write(162);
    state = 1;
  }
  else if (state > 0 && state < 5)
  {
    // Receive 4 packets of data, each correspond to state 1,2,3,4
    if (Serial.available())
    {
      unsigned long int r = Serial.read();
      // bit magic: put bits in r in the correct place in measurement
      measurement |= (r&0xFF) << (unsigned long int)((4-state)*8);
      state+=1;
    }
  }
  else
  {
    // Do something with measurement
    if (count < NUM_SAMPLES)
    {
      // We are initializing - gather measurements
      if (count == 0)
      {
        // It is the first time we run - write 'init.'
        lc.setLed(0,7,3,true);    // i
        lc.setRow(0,6,B00010101); // n
        lc.setLed(0,5,3,true);    // i
        lc.setRow(0,4,B10001111); // t.
      }
      // update best guess of water level
      latest_measurement = max(measurement,latest_measurement);
      count++;
      // Blink led
      lc.setLed(0,0,0,true);
      delay(100);
      lc.setLed(0,0,0,false);
    }
    else
    {
      // We are not initializing. Check for reasonable value
      if (((latest_measurement*95/100) < measurement) &&  ((latest_measurement*105/100) > measurement))
      {
        // Measurement is within +-5% of the current mean. Accept measurement,
        //  update with a small fraction of the new value
        latest_measurement = (latest_measurement*9/10) + (measurement*1/10);

        // Subtract measurement (air depth) from total well capacity, print value in litre
        //  Total depth:      344cm -> 3440*5.8 = 19 952 magic units
        //  Litre conversion: 0.554 litre per millimetre
        //  mm conversion:    5.8 magic units per millimetre
        led_write((19952 - latest_measurement)*554/5800); // litre of water
        //led_write((19952 - latest_measurement)*10/58); // millimetre of water
        //led_write(latest_measurement*554/5800);      // litre of air
        //led_write(latest_measurement*10/58);         // millimetre of air
        //led_write(latest_measurement);

        // Blink led
        lc.setLed(0,0,0,true);
        delay(100);
        lc.setLed(0,0,0,false);
      }
      else
      {
        // Not reasonable value - flash the value and all dots, then go back showing mean
        for (int i=0;i<8;i++)
        {
          lc.setLed(0,i,0,true);
        }
        delay(500);
        for (int i=0;i<8;i++)
        {
          lc.setLed(0,i,0,false);
        }
      }
    }
    
    // Return to original state
    state = 0;
    measurement = 0;
    delay(M_DELAY);
  }
}
