//=====================================================================
//  Flickering Candle Application
//
//	Aspect Shield with a Spark Core
//=====================================================================
//	Copyright (c) 2014 Tangibit Studios LLC.  All rights reserved.
//
//	This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//	License as published by the Free Software Foundation, either
//	version 3 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public
//	License along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//=====================================================================
#define LED0 A7
#define LED1 A6
#define LED2 A5
#define LED3 A4

// touch sensor pins
    int sPin = D3;
    int rPin = D2;
    
// timestamps
    unsigned long tS;
    unsigned long tR;
    
// reading and baseline
    long tReading;
    long tBaseline;


//=====================================================================
//  Setup
//=====================================================================
void setup()
{
    // Make sure your Serial Terminal app is closed before powering your Core
    //Serial.begin(9600);
    
    // Now open your Serial Terminal, and hit any key to continue!
    //while(!Serial.available()) SPARK_WLAN_Loop();
  
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(sPin, OUTPUT);
    
    // intialize conditions for touch sensor
    pinMode(sPin,OUTPUT);
    attachInterrupt(rPin,touchSense,RISING);
    
    // calibrate touch sensor
    LampsFlash(3);
    tBaseline = touchSampling();    // initialize to first reading
}


//=====================================================================
//  Loop
//=====================================================================
void loop()
{
    static unsigned long lastCheck = 0;
    unsigned long checkRate = 1800000;  // 30 minutes
    static int speed = 25;
    static boolean CandleOn = true;
    static unsigned long tTimer = millis();
    
    
    // timer to check touch user interface
    if (millis() > tTimer + 50)
    {
        // reset timer
        tTimer = millis();
    
        if (touchEventCheck() == 1)
        {
            CandleOn = !CandleOn;
        }
    }
    
    if (CandleOn)
    {
        // update wind speed
        if (millis()>lastCheck + checkRate)
        {
            // signal update happening
            LampsFlash(3);
        
            speed = GetWindSpeed();
            //speed = 25; // debug
            lastCheck = millis();
        
            //Serial.println("Rev 0850");
            //Serial.print("Wind speed is ");
            //Serial.println(speed);
        }
    
        // simulate candle flame
        Flicker(speed);
    }
    else
    {
        LampsOff();
    }
}


//=====================================================================
//  Read wind speed
//=====================================================================
int GetWindSpeed()
{
    int speed = 100;
    String jSON = "";
    TCPClient client;
    byte server[] = { 192, 168, 1, 1 }; 		// CHANGE TO YOUR SERVER IP
    
    
    //Serial.println("connecting...");

    if (client.connect(server, 80))
    {
        //Serial.println("connected");
        client.println("GET /api/weather/wind.php HTTP/1.0");
        client.println("Connection: close");
        client.println("Host: EXAMPLE.COM");	// CHANGE TO YOUR HOST
        client.println("Accept: text/html, text/plain");
        client.println("Content-Length: 0");
        client.println();
    }
    else
    {
        //Serial.println("connection failed");
    }
    
    // read response
    LampsFull();
    delay(5000);
    while (client.available())
    {
        char c = client.read();
        //Serial.print(c);
        jSON += c;
    }
    
    // parse out wind speed from JSON
    if (!client.connected())
    {
        //Serial.println();
        //Serial.println("disconnecting.");
        //Serial.println(jSON);
        int pos0 = jSON.indexOf('{');
        int pos1 = jSON.indexOf(':', pos0 + 1);
        int pos2 = jSON.indexOf('"', pos1 + 2);
        //Serial.print("Parsed wind speed is ");
        String temp = jSON.substring(pos1 + 2,pos2);
        speed = temp.toInt();
        //Serial.println(speed);
        client.flush();
        client.stop();
    }
    
    return speed;
}


//=====================================================================
//  Flickering flame simulation
//=====================================================================
void Flicker(int Windy)
{
    // last brightness levels
    static byte lastBright[4] = {0, 0, 0, 0};
    int Lamps[4] = {LED0, LED1, LED2, LED3};
    
    // random movement
    int i = rand() % 1000;   // random # 0 to 999
    int j = rand() % 1000;
    double u = (double)i;
    double v = (double)j;
    u = (u-500)/2000;
    v = (v-500)/2000;
    
    // boundary checks
    if (u < -0.25) {u = -0.25;}
    if (u >  0.25) {u =  0.25;}
    if (v < -0.25) {v = -0.25;}
    if (v >  0.25) {v =  0.25;}  
    /*
    Serial.print("u= ");
    Serial.print(u);
    Serial.print(" v= ");
    Serial.println(v);
    */
  
    // adjust for wind speed on scale of 0 to 25 mph
    if (Windy > 25) {Windy = 25;}
    u = u*Windy/25;
    v = v*Windy/25;
  
    // get resulting brightness levels
    byte newBright[4] = {0, 0, 0, 0};
    newBright[0] = LightLevel(-u, -v);
    newBright[1] = LightLevel( v, -u);
    newBright[2] = LightLevel( u,  v);
    newBright[3] = LightLevel(-v,  u);

    /*
    Serial.print(level0);
    Serial.print(" ");
    Serial.print(level1);
    Serial.print(" ");
    Serial.print(level2);
    Serial.print(" ");
    Serial.println(level3);
    */
    
    // fade to new brightness level
    boolean fadeDone = false;
    while (!fadeDone)
    {
        int fadeCheck = 0;
        
        // slow down loop for low wind speeds
        if (Windy > 10)
        {
            delay(2);
        }
        else
        {
            delay(20);
        }
        
        for (int i = 0; i<4; i++)
        {
            if (newBright[i]>lastBright[i])
            {
                lastBright[i] += 1;
            }
        
            if (newBright[i]<lastBright[i])
            {
                lastBright[i] -= 1;
            }
            
            if (newBright[i] == lastBright[i])
            {
                fadeCheck += 1;    
            }
            
            analogWrite(Lamps[i],lastBright[i]);
        }
        
        if (fadeCheck == 4) {fadeDone = true;}
    }
}


byte LightLevel(double u, double v)
{
    /*
    Serial.print("u= ");
    Serial.print(u);
    Serial.print(" v= ");
    Serial.println(v);
    */
    
    double F = 255*(1-5*u*u/3)/((2-4*v)*(2-4*v));
    if (F<0) {F = 0;}
    if (F>255) {F = 255;}
    byte lightLevel = (byte) F;
    
    /*
    Serial.print(F);
    Serial.print(" ");
    Serial.println(lightLevel);
    */
    
    return lightLevel;
}

//=====================================================================
//  Flash lamps slowly for n cycles
//=====================================================================
void LampsFlash(int cycles)
{
    for (int i = 0; i < cycles; i++)
    {
        // ramp up lamps
        LampsFull();
        
        // ramp down lamps
        LampsOff();
    }
}

//=====================================================================
//  Lamps on full
//=====================================================================
void LampsFull()
{
    // ramp up lamps
    for (int j = 0; j<255; j++)
    {
        delay(2);
        analogWrite(LED0, j);
        analogWrite(LED1, j);
        analogWrite(LED2, j);
        analogWrite(LED3, j);
    }
    
    analogWrite(LED0, 255);
    analogWrite(LED1, 255);
    analogWrite(LED2, 255);
    analogWrite(LED3, 255);
}

//=====================================================================
//  Lamps off
//=====================================================================
void LampsOff()
{
    // ramp down lamps
    for (int j = 255; j<1; j--)
    {
        delay(2);
        analogWrite(LED0, j);
        analogWrite(LED1, j);
        analogWrite(LED2, j);
        analogWrite(LED3, j);
    }
    
    analogWrite(LED0, 0);
    analogWrite(LED1, 0);
    analogWrite(LED2, 0);
    analogWrite(LED3, 0);
}

//=========================================================
// Touch sensing
//=========================================================

//=========================================================
// ISR for touch sensing
//=========================================================
void touchSense()
{
    tR = micros();
}

//=========================================================
// touch sampling
//
// sample touch sensor 32 times and get average RC delay [usec]
//=========================================================
long touchSampling()
{
    long tDelay = 0;
    int mSample = 0;
    
    for (int i=0; i<32; i++)
    {
        // discharge capacitance at rPin
        pinMode(rPin, OUTPUT);
        digitalWrite(sPin,LOW);
        digitalWrite(rPin,LOW);
        
        // revert to high impedance input
        pinMode(rPin,INPUT);
        
        // timestamp & transition sPin to HIGH and wait for interrupt in a read loop
        tS = micros();
        tR = tS;
        digitalWrite(sPin,HIGH);
        do
        {
            // wait for transition
        } while (digitalRead(rPin)==LOW);
        
        // accumulate the RC delay samples
        // ignore readings when micros() overflows
        if (tR>tS)
        {
            tDelay = tDelay + (tR - tS);
            mSample++;
        }
        
    }
    
    // calculate average RC delay [usec]
    if (mSample>0)
    {
        tDelay = tDelay/mSample;
    }
    else
    {
        tDelay = 0;     // this is an error condition!
    }

    //autocalibration using exponential moving average on data below trigger point
    if (tDelay<(tBaseline + tBaseline/4))
    {
        tBaseline = tBaseline + (tDelay - tBaseline)/8;
    }
    
    return tDelay;
    
}

//=========================================================
// touch event check
//
// check touch sensor for events:
//      tEVENT_NONE     no change
//      tEVENT_TOUCH    sensor is touched (Low to High)
//      tEVENT_RELEASE  sensor is released (High to Low)
//
//=========================================================

// state definitions
#define tEVENT_NONE 0
#define tEVENT_TOUCH 1
#define tEVENT_RELEASE 2

int touchEventCheck()
{
    int touchSense;                     // current reading
    static int touchSenseLast = LOW;    // last reading
    
    static unsigned long touchDebounceTimeLast = 0; // debounce timer
    int touchDebounceTime = 50;                     // debounce time
    
    static int touchNow = LOW;  // current debounced state
    static int touchLast = LOW; // last debounced state
    
    int tEvent = tEVENT_NONE;   // default event
    
    
    // read touch sensor
    tReading = touchSampling();
    
    // touch sensor is HIGH if trigger point 1.25*Baseline
    if (tReading>(tBaseline + tBaseline/4)) 
    {
        touchSense = HIGH; 
    }
    else
    {
        touchSense = LOW; 
    }
    
    // debounce touch sensor
    // if state changed then reset debounce timer
    if (touchSense != touchSenseLast)
    {
        touchDebounceTimeLast = millis();
    }
    
    touchSenseLast = touchSense;
    
    
    // accept as a stable sensor reading if the debounce time is exceeded without reset
    if (millis() > touchDebounceTimeLast + touchDebounceTime)
    {
        touchNow = touchSense;
    }
    
    
    // set events based on transitions between readings
    if (!touchLast && touchNow)
    {
        tEvent = tEVENT_TOUCH;
    }
    
    if (touchLast && !touchNow)
    {
        tEvent = tEVENT_RELEASE;
    }
    
    
    // update last reading
    touchLast = touchNow;
    
    return tEvent;
}