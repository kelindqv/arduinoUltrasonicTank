// -- ARDUINO IN WELL/TANK --
// Define physical setup for sensor
int inputPin=2;  //ECHO pin 
int outputPin=4; //TRIG pin 

// Start serial port and setup the pins
void setup() 
{ 
    Serial.begin(2400);
    pinMode(inputPin, INPUT);
    pinMode(outputPin, OUTPUT);
}

// Start listening on serial interface
void loop() 
{
    unsigned long int total=0; // for storing result from sensor, 4 bytes long
    if (Serial.available())
    {
        if (Serial.read() == 162)
        {
            // We got a trig! Do measurement:
            digitalWrite(outputPin, HIGH);      //Trigger ultrasonic detection 
            delayMicroseconds(10); 
            digitalWrite(outputPin, LOW); 
            total = pulseIn(inputPin, HIGH);    //Read ultrasonic reflection
            Serial.write((total >> 24) & 0xFF); // Send first byte on serial
            delay(100);
            Serial.write((total >> 16) & 0xFF); // Send second byte on serial
            delay(100);
            Serial.write((total >> 8) & 0xFF);  // Send third byte on serial
            delay(100);
            Serial.write(total & 0xFF);         // Send fourth byte on serial
            delay(100);
        }
    }
} 

