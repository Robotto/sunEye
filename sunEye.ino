#include <Adafruit_GFX.h>    // Core graphics library
#include "Adafruit_ILI9340.h" // Hardware-specific library - Get the ESP8266 compatible fork from here: https://github.com/glennirwin/Adafruit_ILI9340
#include "FS.h"

#include <ESP8266WiFi.h>

const char* ssid     = "Nope";
const char* password = "not happening";

const char* host = "192.168.0.5"; //could be global, but this is just lan
const int hostPort = 1338;

#define TFT_RST D8
#define TFT_DC D0
#define TFT_CS D3

#define TFT_BACKLIGHT D2

#define LDR_PIN A0
#define filter_alpha 16

Adafruit_ILI9340 tft = Adafruit_ILI9340(TFT_CS, TFT_DC, TFT_RST);

int ldr_val = 512;
int lastLDRval = ldr_val;
bool inputStage1Triggered=false;

unsigned long lastGet = 0;
const unsigned long loopDelay = 1000*60*30; //every 30 minutes

void setup()
{

    Serial.begin(115200);

    pinMode(TFT_BACKLIGHT,OUTPUT);

    //get actual brightness:
    for(int i=0;i<1023;i++) ldr_val = (((long)ldr_val*filter_alpha)+analogRead(LDR_PIN))/(filter_alpha+1); //low pass
    analogWrite(TFT_BACKLIGHT,map(ldr_val,1023,10,0,PWMRANGE)); //invert and scale ADC->PWM
    lastLDRval=ldr_val;

    delay(1000);

    if(SPIFFS.begin()) Serial.println("SPIFFS opened.");
    else { Serial.println("Error: doing while(1)..."); while(1);}

    //FORMAT SPIFFS:
    //if(SPIFFS.format()) Serial.println("Sucessfully formatted SPIFFS"); //takes about a minute. Don't panic.
    //else Serial.println("SPIFFS format failed..."); 
    //while(1) yield();

      tft.begin();
      tft.fillScreen(ILI9340_BLACK);
      tft.setRotation(1);

      tft.setTextColor(ILI9340_RED);
      tft.setTextSize(3);

      if(SPIFFS.exists("/test.bmp")) bmpDraw("/test.bmp",0,0);
      
      else{
      tft.setCursor(48, 48);
      tft.println("no file!");
      inputStage1Triggered=true;

      }
}

unsigned long tick;

void loop()
{
  yield();

  ldr_val = (((long)ldr_val*filter_alpha)+analogRead(LDR_PIN))/(filter_alpha+1); //low pass
  
  if(millis()>tick+500){ //every 500mS:
  tick=millis();
  //if light suddenly increases by 64:
  if(ldr_val<lastLDRval-64) inputStage1Triggered=true; 
  else inputStage1Triggered=false;
  lastLDRval=ldr_val;
  }

  if(inputStage1Triggered) {
    inputStage1Triggered=false;
    Serial.println("triggered!");
    lastGet=millis();
    tft.setCursor(48, 96);
    tft.println("Refreshing...");

    SPIFFS.remove("/test.bmp");

    if(!SPIFFS.exists("/test.bmp")) Serial.println("successfully removed old test.bmp");

    getIt();
    
    tft.fillScreen(ILI9340_BLACK);
      
    if(SPIFFS.exists("/test.bmp")) bmpDraw("/test.bmp",0,0);
    else{
      tft.setCursor(48, 48);
      tft.println("no file!");
    }

    //reset LDR:
    for(int i=0;i<1023;i++) ldr_val = (((long)ldr_val*filter_alpha)+analogRead(LDR_PIN))/(filter_alpha+1); //low pass
    lastLDRval=ldr_val;
  }

                     //calibrated from: 1023 0
  analogWrite(TFT_BACKLIGHT,map(ldr_val,1023,10,0,PWMRANGE)); //invert and scale ADC->PWM

  if(millis()>lastGet+loopDelay) {//every $loopDelay milliseconds
    lastGet=millis();
    getIt();
  	//tft.fillScreen(ILI9340_BLACK);
    bmpDraw("/test.bmp",0,0);
  }
}


void getIt()
{

//Establish connection

  int wifiTries=0;      
      // Use WiFiClient class to create TCP connections
  WiFiClient client;

  unsigned long startTime=millis();

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    wifiTries++;
    Serial.print(".");
    if(wifiTries==60) { //after 30 seconds of trying to connect.
      Serial.println("WiFi connection failed.. will retry next round.");
      WiFi.disconnect();
      return;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(": ");

  if (!client.connect(host, hostPort)) {
    Serial.println("connection failed");
    return;
  }

  Serial.println("Success!");

  Serial.println("Awaiting data");

  while(!client.available()) {delay(1000); Serial.print(".");}
  Serial.println();
  Serial.print("Getting image");
  File f = SPIFFS.open("/test.bmp", "w");

      //client.setNoDelay(true); //don't pool small packages
       /*
        while (client.available())
        {
          f.write(client.read());
          if(!client.available()) delay(10);
        }
        */
        const int bufSize = 64; 
        byte tcpBuf[bufSize];
        int incomingCount = 0;
        int remainingByteCount = 230538; //since it's a BMP it never actually changes size :D

        //while (client.available())
        while(remainingByteCount>0)
        {
          tcpBuf[incomingCount] = client.read();
          incomingCount++;
          remainingByteCount--;
          //delayMicroseconds(10);

          if (incomingCount > bufSize-1) 
          {          
            f.write((const uint8_t *)tcpBuf, bufSize);
            incomingCount = 0;
            delayMicroseconds(200);
            
            float progress=100.0-((float)remainingByteCount/230538.0*100.0);
            for(int i=140;i<150;i++) tft.drawPixel((int)(progress*3.2),i,ILI9340_RED); //10 px wide progress bar
            
          }

          if(!client.available()) {Serial.print("!"); delay(50);} //wait a bit to see if more data should happen to be on the way.

        }
        // final < bufSize byte cleanup packet
        if (incomingCount > 0) f.write((const uint8_t *)tcpBuf, incomingCount);

        Serial.println(" Done!");

        Serial.println("Download took: " + String(millis()-startTime) + "mS");
        
        delay(1000); //is file write done?

        f.close();
        Serial.println();
        Serial.println("disconnecting.");
        client.stop();
        WiFi.disconnect();

        delay(1000); //just trying random things now...
} 


// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

//#define BUFFPIXEL 20 //we could go crazy with the ram, since were on an ESP8266! with 96K memory, rather than a 328 with only 2K!
#define BUFFPIXEL 20 //we could go crazy with the ram, since were on an ESP8266! with 96K memory, rather than a 328 with only 2K!

void bmpDraw(char *filename, uint16_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  //uint16_t  buffidx = sizeof(sdbuffer); // make big, to account for the enormous buffer size!
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if(!SPIFFS.exists(filename)) {Serial.println("File not found"); return;}
  bmpFile = SPIFFS.open(filename, "r");
  if (!bmpFile) {
    Serial.println("file open failed");
    return;
  }
  Serial.println("File opened..");


  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print("File size: "); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print("Image Offset: "); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print("Header size: "); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print("Bit Depth: "); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print("Image size: ");
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos,SeekSet);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.Color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print("Loaded in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println("BMP format not recognized.");
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File & f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File & f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
