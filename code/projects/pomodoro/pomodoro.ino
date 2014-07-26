//============================================================
//	Pomodoro Timer application
//
//	Aspect Shield with a Spark Core
//============================================================
//	Copyright (c) 2014 Tangibit Studios LLC.  All rights reserved.
//
//	This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
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
//============================================================
#include <string.h>

// Lamp ports
#define LAMP0 A7
#define LAMP1 A6
#define LAMP2 A5
#define LAMP3 A3

// Haptic port
#define HAPTIC D5

// Lamp modes
#define LAMPS_OFF 0
#define LAMPS_FULL 1
#define LAMPS_BREATHE 2
#define LAMPS_LISSAJOUS 3

// State machine states
#define STATE_IDLE 0
#define STATE_POMODORO 1

// Polling times in [ms]
#define POLL_LAMP 5
#define POLL_LISSAJOUS 50
#define POLL_STATEMACHINE 20

// Touch events
#define tEVENT_NONE 0
#define tEVENT_TOUCH 1
#define tEVENT_RELEASE 2

// Pomodoro definitions
#define POMODORO 1500000  //25 min = 25*60*1000 = 1,500,000 ms
#define POMODORO_START 0
#define POMODORO_DONE 1
#define POMODORO_INTERUPT 2

// Reporting definitions
#define REPORT_ID "Office"      // Unit or location reporting pomodoros-- NO SPACES!
#define REPORT_TOKEN "1111"     // Access token to web PHP script-- CHANGE TO YOUR OWN TOKEN

// touch sensor pins
    int sPin = D3;
    int rPin = D2;
    
// timestamps
    unsigned long tS;
    unsigned long tR;
    
// reading and baseline
    long tReading;
    long tBaseline;


//------------------------------------------------------------
//	Setup
//------------------------------------------------------------
void setup()
{
	// Lamp pins
	pinMode(LAMP0, OUTPUT);
	pinMode(LAMP1, OUTPUT);
	pinMode(LAMP2, OUTPUT);
	pinMode(LAMP3, OUTPUT);
    
    // Haptic pin
    pinMode(HAPTIC, OUTPUT);
    
    // intialize conditions for touch sensor
    pinMode(sPin,OUTPUT);
    attachInterrupt(rPin,touchSense,RISING);
    
    // calibrate touch sensor
    tBaseline = touchSampling();    // initialize to first reading
	
	/*
	// Make sure your Serial Terminal app is closed before powering your Core
    Serial.begin(9600);
    // Now open your Serial Terminal, and hit any key to continue!
    while(!Serial.available()) SPARK_WLAN_Loop();
    */
    
}

//------------------------------------------------------------
//	Loop
//------------------------------------------------------------
void loop()
{
	// update state machine;
	int lampMode = PomodoroUpdate();
	
	// update lamps
	LampsUpdate(lampMode);
}


//============================================================
//	State machine
//============================================================
int PomodoroUpdate()
{
	// State of machine
	static int state = STATE_IDLE;
	
	// Lamp mode setting
	static int lampMode = LAMPS_LISSAJOUS;
	
	// time stamps
	static unsigned long lastUpdate = 0;
	static unsigned long startPomodoro = 0;
	
	// check Touch UI
	int touchEvent = touchEventCheck();
	
	//Timing check to update state machine
	if (millis() > lastUpdate + POLL_STATEMACHINE)
	{
		// State machine
		switch (state)
		{
			case STATE_IDLE:
				if (touchEvent == tEVENT_TOUCH) 
				{ 
					LampsFlash();
					state = STATE_POMODORO;
					startPomodoro = millis();
					ReportPomodoro(POMODORO_START, 0);
				}
				lampMode = LAMPS_LISSAJOUS;
			break;
			
			case STATE_POMODORO:
				if (touchEvent == tEVENT_TOUCH)
				{
				    LampsFlash();
				    state = STATE_IDLE;
				    unsigned long lengthPomodoro = 0;
				    if (millis() > startPomodoro) { lengthPomodoro = (millis()-startPomodoro)/60000; }
				    ReportPomodoro(POMODORO_INTERUPT, lengthPomodoro);
				}
				if (millis() > startPomodoro + POMODORO) 
				{
				    state = STATE_IDLE;
				    unsigned long lengthPomodoro = 0;
				    if (millis() > startPomodoro) { lengthPomodoro = (millis()-startPomodoro)/60000; }
				    ReportPomodoro(POMODORO_DONE, lengthPomodoro);
				}
				lampMode = LAMPS_BREATHE;
			break;
		}
		
		// time stamp updated
		lastUpdate = millis();
	}
	
	return lampMode;
}


//============================================================
//	Lamp functions
//============================================================
//------------------------------------------------------------
//	Update lamps
//------------------------------------------------------------
void LampsUpdate(int lampMode)
{
	switch (lampMode)
	{
		case LAMPS_OFF:
			LampsFade(1,0,0,0,0);
		break;
	
		case LAMPS_FULL:
			LampsFade(1,255,255,255,255);
		break;
		
		case LAMPS_BREATHE:
			LampsBreathe();
		break;
		
		case LAMPS_LISSAJOUS:
			LampsLissajous(32,48);
		break;
	}
}

//------------------------------------------------------------
//	100ms full brightness flash
//------------------------------------------------------------
void LampsFlash()
{
    analogWrite(LAMP0, 255);
    analogWrite(LAMP1, 255);
    analogWrite(LAMP2, 255);
    analogWrite(LAMP3, 255);
    
    digitalWrite(HAPTIC, HIGH);
    
    delay(500);
    
    digitalWrite(HAPTIC, LOW);
    
}

//------------------------------------------------------------
//	Lamp fade
//
//	rate= multiplier for extending base fade rate (1= base)
//	returns true if all lamp fades are completed
//------------------------------------------------------------
boolean LampsFade(int rate, int bright0, int bright1, int bright2, int bright3)
{
	// last brightness level
	static int level0 = 0;
	static int level1 = 0;
	static int level2 = 0;
	static int level3 = 0;
	
	// last update time
	static unsigned long lastUpdate = 0;
	
	
	//Timing check to update fade
	boolean fadeDone = false;
	if (millis() > lastUpdate + rate*POLL_LAMP)
	{
		// step toward target brightness if not equal
		if (bright0 > level0) {level0++;}
		if (bright0 < level0) {level0--;}
		if (bright1 > level1) {level1++;}
		if (bright1 < level1) {level1--;}
		if (bright2 > level2) {level2++;}
		if (bright2 < level2) {level2--;}
		if (bright3 > level3) {level3++;}
		if (bright3 < level3) {level3--;}
		
		// flag completed fade on all lamps
		if ((bright0 == level0)&&(bright1 == level1)&&(bright2 == level2)&&(bright3 == level3)) {fadeDone = true;}
		
		// update lamps
		analogWrite(LAMP0, level0);
		analogWrite(LAMP1, level1);
		analogWrite(LAMP2, level2);
		analogWrite(LAMP3, level3);
		
		// time stamp updated
		lastUpdate = millis();
	}
	
	return fadeDone;
}

//------------------------------------------------------------
//	Breathing lamp pattern
//------------------------------------------------------------
void LampsBreathe()
{
	static boolean breathIn = true;
	boolean fadeDone;
	
	if (breathIn)
	{
		fadeDone = LampsFade(1,192,192,192,192);
		if (fadeDone) {breathIn = false;}
	}
	else
	{
		fadeDone = LampsFade(1,32,32,32,32);
		if (fadeDone) {breathIn = true;}
	}
}

//------------------------------------------------------------
//	Triangle wave generator [-1,+1] output
//	
//	based on y = | x mod 4 - 2| -1 (see Wikipedia)
//------------------------------------------------------------
double TriangleWave(double t, double period, double phase)
{
	double wave;
	double z;
	
	// the base function has a fixed period of 4, so scale t
	// to adjust period after adding phase offset first
	// phase is in fraction of a period 
	z = 4*((t + phase*period)/period);
	
	// note: modulo operator doesn't work on doubles
	z = z - (4*((int)(z/4))) - 2;
	if (z >= 0) {wave =  z -1;}
    if (z < 0)  {wave = -z -1;}
	
	return wave;
}

//------------------------------------------------------------
//	Corner illumination model
//
//	Calculate brightness for each corner lamp
//------------------------------------------------------------
int CornerModel(int lamp, double u, double v)
{
	double x;
	double y;
	double xt;
	double yt;
	double z;			// brightness dimension
	double d = 2; 	    // x and y intercepts
	
	// transform coordinates to corner origin
	// Lamp0 (+1,-1)
	// Lamp1 (-1,-1)
	// Lamp2 (-1,+1)
	// Lamp3 (+1,+1)
	// Lissajous has its origin in center of [-1, +1] box
	switch (lamp)
	{
		case 0:
			// translate
			xt = u - 1;
			yt = v + 1;
			
			// rotate
			x = yt;
			y = -xt;
		break;
		
		case 1:
			// translate
			xt = u + 1;
			yt = v + 1;
			
			// rotate
			x = xt;
			y = yt;
		break;
		
		case 2:
			// translate
			xt = u + 1;
			yt = v - 1;
			
			// rotate
			x = -yt;
			y = xt;
		break;
		
		case 3:
			// translate
			xt = u - 1;
			yt = v - 1;
			
			// rotate
			x = -xt;
			y = -yt;
		break;
	}
	
	// calculate brightness based on plane
	z = 255*(1-(x+y)/d);
	if (z < 0) { z = 0;}
	if(z > 255) { z = 255;}
	
	return (int)z;
}

//------------------------------------------------------------
//	Lissajous lamp pattern
//
//	Move a virtual source around based on a 2:3 Lissajous (32:48)
//	pattern using triangle waves to approximate sine/cosine
//  Circle approximation is 1:1 Lissajous (32:32)
//  Meander over entire space is (32:35)
//------------------------------------------------------------
void LampsLissajous(double freq1, double freq2)
{
	static unsigned long lastUpdate = 0;
	static int bright0 = 0;
	static int bright1 = 0;
	static int bright2 = 0;
	static int bright3 = 0;
	static int step = 0;
	
	
	// Timing check to update Lissajous pattern
	if (millis() > lastUpdate + POLL_LISSAJOUS)
	{
		// Calculate a source location (u,v) in x [-1,1] and y [-1,1]
		double u = TriangleWave((double)step, 32, 0.0);
		double v = TriangleWave((double)step, 32, 0.75);    // Circle
		//double v = TriangleWave((double)step, 35, 0.75);    // 2:3 Lissajous
		//double v = TriangleWave((double)step, 48, 0.75);    // Meander
		
		// Calculate corresponding lamp brightness levels
		bright0 = CornerModel(0, u, v);
		bright1 = CornerModel(1, u, v);
		bright2 = CornerModel(2, u, v);
		bright3 = CornerModel(3, u, v);
		
		// time stamps updated
		lastUpdate = millis();
		step++;
	}
	
	// Update lamps
	boolean fadeDone = LampsFade(1, bright0, bright1, bright2, bright3);
}


//============================================================
//	Touch UI
//============================================================
//------------------------------------------------------------
// ISR for touch sensing
//------------------------------------------------------------
void touchSense()
{
    tR = micros();
}

//------------------------------------------------------------
// touch sampling
//
// sample touch sensor 32 times and get average RC delay [usec]
//------------------------------------------------------------
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

//------------------------------------------------------------
// touch event check
//
// check touch sensor for events:
//      tEVENT_NONE     no change
//      tEVENT_TOUCH    sensor is touched (Low to High)
//      tEVENT_RELEASE  sensor is released (High to Low)
//
//------------------------------------------------------------
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

//============================================================
//	Reporting
//============================================================
//------------------------------------------------------------
// Report on Pomodoro
//------------------------------------------------------------
void ReportPomodoro(int pomodoroLog, unsigned long pomodoroMinutes)
{
    String pomodoroStatus = "";
    TCPClient client;
    byte server[] = { 192, 168, 1, 1 };		// CHANGE TO YOUR SERVER IP
    
    switch (pomodoroLog)
    {
        case POMODORO_START:
            //Not using this currently
        break;
        
        case POMODORO_DONE:
            pomodoroStatus = "DONE";
        break;
        
        case POMODORO_INTERUPT:
            pomodoroStatus = "INTERUPT";
        break;
    }
    
    String pomodoroLength = String(pomodoroMinutes);

    if (client.connect(server, 80)&&(pomodoroLog != POMODORO_START))
    {
        // POST Header
        client.println("POST /api/iot/pomodoro.php HTTP/1.1");		// CHANGE TO YOUR PATH
        client.println("Host: EXAMPLE.COM");						// CHANGE TO YOUR HOST
        client.println("Accept: */*");
        client.println("Content-Type: application/x-www-form-urlencoded");
        
        client.print("Content-Length: ");
        client.println(strlen(REPORT_ID)+strlen(REPORT_TOKEN)+pomodoroStatus.length()+pomodoroLength.length()+26);
        client.println();
        
        // POST Data
        client.print("ID=");                // 3 characters
        client.print(REPORT_ID);
        client.print("&Token=");            // 7 characters
        client.print(REPORT_TOKEN);
        client.print("&Status=");           // 8 characters
        client.print(pomodoroStatus);
        client.print("&Length=");           // 8 characters
        client.println(pomodoroLength);
        delay(1000);
    }
    
    while (client.available())
    {
        char c = client.read();
        //Serial.print(c);
    }
    
    if (!client.connected())
    {
        //Serial.println();
        //Serial.println("disconnecting.");
        client.flush();
        delay(100);
        client.stop();
        delay(250);
    }
  
}