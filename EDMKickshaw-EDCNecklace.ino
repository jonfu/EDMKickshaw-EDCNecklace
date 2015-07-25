#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
 
#define VCC 9
#define PIN A5
#define STRIPSIZE 24
#define GND1 3
#define GND2 10

const unsigned long maxTime = 4294967295;

const long interlude = 16; //default should be 20
const int totalInterludeMode = 5;
const int maxColorWipe = 5;
const int totalColorChase = 10;

const byte SCANNERINTERVAL = 40;
const byte WIPEINTERVAL = 10;
const byte CHASEINTERVAL = 10;
const byte BRI_SCANNER = 100;
const byte BRI_THEATER = 20;
const byte BRI_RAINBOW = 5;
const byte BRI_WIPE = 10;
const byte BRI_WAVE = 5;
const byte BRI_CHASE = 20;


long count = 0;
int interludeMode = 0;
int colorWipeCount = 0;
int G_flag = 1;
int RGB = 0;
int RGB_val[3];

boolean enableInterlude = false;

byte mode;

 
// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
    
    
    void OnCompleteLocal() {
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycle(random(0,10));
                    interludeMode++;
                    enableInterlude = false;
                    break;
                case THEATER_CHASE:
                    //Reverse();
                    interludeMode++;
                    enableInterlude = false;
                    break;
                case COLOR_WIPE:
                    Color1 = Wheel(random(255));
                    //Interval = 20000;
                    if ( ++colorWipeCount % maxColorWipe == 0) {
                      colorWipeCount = 0;
                      interludeMode++;
                      enableInterlude = false;
                    }
                    break;
                case SCANNER:
                    Color1 = Wheel(random(255));
                    count++;
                    if (count%interlude==0) {
                      enableInterlude = true;
                    }                    
                    break;
                case FADE:
                    Color1 = Wheel(random(255));
                    //Interval = 20000;
                    break;
                default:
                    break;
            }      
      
    }
	
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
                else
                {
                    OnCompleteLocal();
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
                else
                {
                    OnCompleteLocal();
                }
            }
        }
    }
    
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
        
        setBrightness(BRI_RAINBOW);
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
        
        setBrightness(BRI_THEATER);
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
        
        setBrightness(BRI_WIPE);
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
        
        setBrightness(BRI_SCANNER);
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        //uint32_t dimColor = Color(Red(color)*88/100, Green(color)*88/100, Blue(color)*88/100);
        
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};





 
 
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPSIZE, PIN, NEO_GRB + NEO_KHZ800);

//NeoPatterns strip(16, 7, NEO_GRB + NEO_KHZ800, &StickComplete);

NeoPatterns strip(STRIPSIZE, PIN, NEO_GRB + NEO_KHZ800, NULL);
 
void setup() {
  
  mode = EEPROM.read(0);
  
  if (mode == 0) {
    
    EEPROM.write(0, 1);

    pinMode(GND1, OUTPUT);
    pinMode(GND2, OUTPUT);
    pinMode(VCC, OUTPUT);
    digitalWrite(GND1, LOW);
    digitalWrite(GND2, LOW);
    digitalWrite(VCC, HIGH);
  
   
    strip.begin();
    
    strip.Scanner(strip.Color(255,0,0), SCANNERINTERVAL);
  
  } else {
    
    EEPROM.write(0, 0);
    
  }


}
 
void loop() {
  
  
  
 if (mode == 0) {
  
  if (enableInterlude) {
    
    switch (interludeMode) {

      case 0:
        if (strip.ActivePattern == THEATER_CHASE) {
          strip.Update();
        } else {
          strip.TheaterChase(strip.Color(random(255),random(255),0), strip.Color(0,0,random(100)), 100);
        }
      break;
      
      case 1:
        if (strip.ActivePattern == RAINBOW_CYCLE) {
          strip.Update();
        } else {
          strip.RainbowCycle(12);
        }
      break;      
      
      case 2:
        if (strip.ActivePattern == COLOR_WIPE) {
          strip.Update();
        } else {
          strip.ColorWipe(strip.Color(random(255),random(255),random(255)), WIPEINTERVAL);
        }
      break;    
      
      case 3:
        strip.setBrightness(BRI_WAVE);
        colorWave(10);
        interludeMode++;
        enableInterlude = false;
        strip.setBrightness(BRI_SCANNER);
      break;
      
      case 4:
        strip.setBrightness(BRI_CHASE);
        for (int i=0; i<totalColorChase; i++) {
          colorChase(strip.Color(random(255), random(255), random(255)), 1, CHASEINTERVAL);
          colorChaseReverse(strip.Color(random(255), random(255), random(255)), 1, CHASEINTERVAL);    
        } 
        interludeMode++;
        enableInterlude = false;
        strip.setBrightness(BRI_SCANNER);
      break;      
      
      /*
      
      case 4:
        strip.setBrightness(25);
        rainbow(10);
        interludeMode++;
        enableInterlude = false;
      break;
      

      
      case 5:
        colorChase(strip.Color(random(255), random(255), random(255)), 5, 65);
        colorChaseReverse(strip.Color(random(255), random(255), random(255)), 5, 65);     
        interludeMode++;
        enableInterlude = false;
      break;
      
      case 6:
        colorWipe(strip.Color(random(255), random(255), random(255)), 50);
        colorWipe(strip.Color(0,0,0), 50); // Black      
        interludeMode++;
        enableInterlude = false;
      break;
      
      case 7:
        colorWipeReverse(strip.Color(random(255), random(255), random(255)), 50);
        colorWipeReverse(strip.Color(0,0,0), 50); // Black     
        interludeMode++;
        enableInterlude = false;
      break;
      
      */
      
      
      
      
    }
    

    
    if (interludeMode == totalInterludeMode) {
        interludeMode = 0;
    }
    


  }
  else {
    
    if (strip.ActivePattern != SCANNER) {
        strip.Scanner(strip.Color(255,0,0), SCANNERINTERVAL);
    }
    //strip.setBrightness(100);
    strip.Update();
  }
  
  
  
  
 } else {
   
   delay(maxTime);
   
 }
  
  
  

}

void colorChase(uint32_t c, int maxDots, uint8_t wait) {
  for (int a=0; a<maxDots; a++) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      if (i>a) {
        strip.setPixelColor((i-a-1), strip.Color(0,0,0));
      }
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
    }
  }
}

void colorChaseReverse(uint32_t c, int maxDots, uint8_t wait) {
  for (int a=0; a<maxDots; a++) {
    for(int i=strip.numPixels()-1; i>=0; i--) {
      if ((strip.numPixels()-i)>(a+1)) {
        strip.setPixelColor((i+a+1), strip.Color(0,0,0));
      }
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
    }
  }
}
 
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
for(uint16_t i=0; i<strip.numPixels(); i++) {
strip.setPixelColor(i, c);
strip.show();
delay(wait);
}
}

void colorWipeReverse(uint32_t c, uint8_t wait) {
  int pos = strip.numPixels();
  while (--pos >= 0) {
    strip.setPixelColor(pos, c);
    strip.show();
    delay(wait);
  }
}
 
void rainbow(uint8_t wait) {
uint16_t i, j;
 
for(j=0; j<32; j++) { //original value 256
for(i=0; i<strip.numPixels(); i++) {
strip.setPixelColor(i, Wheel((i+j) & 255));
}
strip.show();
delay(wait);
}
}
 
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
uint16_t i, j;
 
for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
for(i=0; i< strip.numPixels(); i++) {
strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
}
strip.show();
delay(wait);
}
}
 
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
if(WheelPos < 85) {
return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
} else if(WheelPos < 170) {
WheelPos -= 85;
return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
} else {
WheelPos -= 170;
return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
}
}
 
/**
* ^ ^ ^
* ~~~~~ ColorWave ~~~~~
* V V V
*/
void colorWave(uint8_t wait) {
int i, j, stripsize, cycle;
float ang, rsin, gsin, bsin, offset;
 
static int tick = 0;
stripsize = strip.numPixels();
cycle = stripsize * 4; // times around the circle...
 
while (++tick % cycle) {
offset = map2PI(tick);
 
for (i = 0; i < stripsize; i++) {
ang = map2PI(i) - offset;
rsin = sin(ang);
gsin = sin(2.0 * ang / 3.0 + map2PI(int(stripsize/6)));
bsin = sin(4.0 * ang / 5.0 + map2PI(int(stripsize/3)));
strip.setPixelColor(i, strip.Color(trigScale(rsin), trigScale(gsin), trigScale(bsin)));
}
 
strip.show();
delay(wait);
}
 
}
 
/**
* Scale a value returned from a trig function to a byte value.
* [-1, +1] -> [0, 254]
* Note that we ignore the possible value of 255, for efficiency,
* and because nobody will be able to differentiate between the
* brightness levels of 254 and 255.
*/
byte trigScale(float val) {
val += 1.0; // move range to [0.0, 2.0]
val *= 127.0; // move range to [0.0, 254.0]
 
return int(val) & 255;
}
 
/**
* Map an integer so that [0, striplength] -> [0, 2PI]
*/
float map2PI(int i) {
return PI*2.0*float(i) / float(strip.numPixels());
}


