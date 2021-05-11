#include <Wire.h>
   
int PhotoTakenCount = 0;

int PHOTO_WIDTH;   
int PHOTO_HEIGHT; 
int PHOTO_BYTES_PER_PIXEL;
//int ImageHeight = 40;
//int ImageWidth  = 80;
unsigned char ImageData[20][160];

unsigned int sumL = 0;
unsigned int meanL = 0;
unsigned int sumM = 0;
unsigned int meanM = 0;
unsigned int sumR = 0;
unsigned int meanR = 0; 

// Command and Parameter related Strings
String RawCommandLine = "t";
String Command  = "QQVGA";
String FPSParam = "ThirtyFPS";
String AWBParam = "SAWB";
String AECParam = "HistAEC";
String YUVMatrixParam = "YUVMatrixOn";
String DenoiseParam = "DenoiseNo";
String EdgeParam = "EdgeNo";
String ABLCParam = "AblcON";

enum ResolutionType
{
  QQVGA,
  None
};

ResolutionType Resolution = None; 

// Camera input/output pin connection to Arduino
#define WRST  22      // Output Write Pointer Reset
#define RRST  23      // Output Read Pointer Reset
#define WEN   24      // Output Write Enable
#define VSYNC 25      // Input Vertical Sync marking frame capture
#define RCLK  26      // Output FIFO buffer output clock
// set OE to low gnd

// FIFO Ram input pins
#define DO7   28     
#define DO6   29   
#define DO5   30   
#define DO4   31   
#define DO3   32   
#define DO2   33   
#define DO1   34
#define DO0   35 

// Register addresses and values
#define CLKRC                 0x11 
#define CLKRC_VALUE_VGA       0x01  // Raw Bayer
#define CLKRC_VALUE_QVGA      0x01
#define CLKRC_VALUE_QQVGA     0x01
#define CLKRC_VALUE_NIGHTMODE_FIXED   0x03 // Fixed Frame
#define CLKRC_VALUE_NIGHTMODE_AUTO    0x80 // Auto Frame Rate Adjust

#define COM7                                   0x12 
#define COM7_VALUE_VGA                         0x01   // Raw Bayer
#define COM7_VALUE_VGA_COLOR_BAR               0x03   // Raw Bayer
#define COM7_VALUE_VGA_PROCESSED_BAYER         0x05   // Processed Bayer
#define COM7_VALUE_QVGA                        0x00
#define COM7_VALUE_QVGA_COLOR_BAR              0x02
#define COM7_VALUE_QVGA_PREDEFINED_COLOR_BAR   0x12
#define COM7_VALUE_QQVGA                       0x00
#define COM7_VALUE_QQVGA_COLOR_BAR             0x02   
#define COM7_VALUE_QCIF                        0x08     // Predefined QCIF format
#define COM7_VALUE_COLOR_BAR_QCIF              0x0A     // Predefined QCIF Format with ColorBar
#define COM7_VALUE_RESET                       0x80

#define COM3                            0x0C 
#define COM3_VALUE_VGA                  0x00 // Raw Bayer
#define COM3_VALUE_QVGA                 0x04
#define COM3_VALUE_QQVGA                0x04  // From Docs
#define COM3_VALUE_QQVGA_SCALE_ENABLED  0x0C  // Enable Scale and DCW
#define COM3_VALUE_QCIF                 0x0C  // Enable Scaling and enable DCW

#define COM14                            0x3E 
#define COM14_VALUE_VGA                  0x00 // Raw Bayer
#define COM14_VALUE_QVGA                 0x19
#define COM14_VALUE_QQVGA                0x1A
#define COM14_VALUE_MANUAL_SCALING       0x08   // Manual Scaling Enabled
#define COM14_VALUE_NO_MANUAL_SCALING    0x00   // Manual Scaling DisEnabled

#define SCALING_XSC                                  0x70
#define SCALING_XSC_VALUE_VGA                        0x3A  // Raw Bayer
#define SCALING_XSC_VALUE_QVGA                       0x3A
#define SCALING_XSC_VALUE_QQVGA                      0x3A
#define SCALING_XSC_VALUE_QQVGA_SHIFT1               0x3A
#define SCALING_XSC_VALUE_COLOR_BAR                  0xBA  
#define SCALING_XSC_VALUE_QCIF_COLOR_BAR_NO_SCALE    0x80 // Predefined QCIF with Color Bar and NO Scaling

#define SCALING_YSC                                   0x71 
#define SCALING_YSC_VALUE_VGA                         0x35 // Raw Bayer 
#define SCALING_YSC_VALUE_QVGA                        0x35
#define SCALING_YSC_VALUE_QQVGA                       0x35
#define SCALING_YSC_VALUE_COLOR_BAR                   0x35  // 8 bar color bar
#define SCALING_YSC_VALUE_COLOR_BAR_GREY              0xB5  // fade to grey color bar
#define SCALING_YSC_VALUE_COLOR_BAR_SHIFT1            0xB5  // fade to grey color bar
#define SCALING_YSC_VALUE_QCIF_COLOR_BAR_NO_SCALE     0x00  // Predefined QCIF with Color Bar and NO Scaling

#define SCALING_DCWCTR               0x72 
#define SCALING_DCWCTR_VALUE_VGA     0x11  // Raw Bayer
#define SCALING_DCWCTR_VALUE_QVGA    0x11
#define SCALING_DCWCTR_VALUE_QQVGA   0x22  

#define SCALING_PCLK_DIV              0x73  
#define SCALING_PCLK_DIV_VALUE_VGA    0xF0 // Raw Bayer
#define SCALING_PCLK_DIV_VALUE_QVGA   0xF1
#define SCALING_PCLK_DIV_VALUE_QQVGA  0xF2

#define SCALING_PCLK_DELAY              0xA2
#define SCALING_PCLK_DELAY_VALUE_VGA    0x02 // Raw Bayer
#define SCALING_PCLK_DELAY_VALUE_QVGA   0x02
#define SCALING_PCLK_DELAY_VALUE_QQVGA  0x02


// Controls YUV order Used with COM13
// Need YUYV format for Android Decoding- Default value is 0xD
#define TSLB                                         0x3A
#define TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_ENABLED   0x01 // No custom scaling
#define TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_DISABLED  0x00 // For adjusting HSTART, etc. YUYV format
#define TSLB_VALUE_UYVY_AUTO_OUTPUT_WINDOW_DISABLED  0x08 
#define TSLB_VALUE_TESTVALUE                         0x04 // From YCbCr Reference 

// Default value is 0x88
// ok if you want YUYV order, no need to change
#define COM13                      0x3D
#define COM13_VALUE_DEFAULT        0x88
#define COM13_VALUE_NOGAMMA_YUYV   0x00
#define COM13_VALUE_GAMMA_YUYV     0x80
#define COM13_VALUE_GAMMA_YVYU     0x82
#define COM13_VALUE_YUYV_UVSATAUTOADJ_ON 0x40

// Works with COM4
#define COM17                                 0x42
#define COM17_VALUE_AEC_NORMAL_NO_COLOR_BAR   0x00
#define COM17_VALUE_AEC_NORMAL_COLOR_BAR      0x08 // Activate Color Bar for DSP

#define COM4   0x0D

// RGB Settings and Data format
#define COM15    0x40

// Night Mode
#define COM11                             0x3B
#define COM11_VALUE_NIGHTMODE_ON          0x80     // Night Mode
#define COM11_VALUE_NIGHTMODE_OFF         0x00 
#define COM11_VALUE_NIGHTMODE_ON_EIGHTH   0xE0     // Night Mode 1/8 frame rate minimum
#define COM11_VALUE_NIGHTMODE_FIXED       0x0A 
#define COM11_VALUE_NIGHTMODE_AUTO        0xEA     // Night Mode Auto Frame Rate Adjust

// Color Matrix Control YUV
#define MTX1      0x4f 
#define MTX1_VALUE  0x80

#define MTX2      0x50 
#define MTX2_VALUE  0x80

#define MTX3      0x51 
#define MTX3_VALUE  0x00

#define MTX4      0x52 
#define MTX4_VALUE  0x22

#define MTX5      0x53 
#define MTX5_VALUE  0x5e

#define MTX6      0x54 
#define MTX6_VALUE  0x80

#define CONTRAS     0x56 
#define CONTRAS_VALUE 0x40

#define MTXS      0x58 
#define MTXS_VALUE  0x9e

// COM8
#define COM8                    0x13
#define COM8_VALUE_AWB_OFF      0xE5
#define COM8_VALUE_AWB_ON       0xE7

// Automatic White Balance
#define AWBC1     0x43 
#define AWBC1_VALUE 0x14

#define AWBC2     0x44 
#define AWBC2_VALUE 0xf0

#define AWBC3     0x45 
#define AWBC3_VALUE   0x34

#define AWBC4     0x46 
#define AWBC4_VALUE 0x58

#define AWBC5         0x47 
#define AWBC5_VALUE 0x28

#define AWBC6     0x48 
#define AWBC6_VALUE 0x3a

#define AWBC7           0x59
#define AWBC7_VALUE     0x88

#define AWBC8          0x5A
#define AWBC8_VALUE    0x88

#define AWBC9          0x5B
#define AWBC9_VALUE    0x44

#define AWBC10         0x5C
#define AWBC10_VALUE   0x67

#define AWBC11         0x5D
#define AWBC11_VALUE   0x49

#define AWBC12         0x5E
#define AWBC12_VALUE   0x0E

#define AWBCTR3        0x6C
#define AWBCTR3_VALUE  0x0A

#define AWBCTR2        0x6D
#define AWBCTR2_VALUE  0x55

#define AWBCTR1        0x6E
#define AWBCTR1_VALUE  0x11

#define AWBCTR0                0x6F
#define AWBCTR0_VALUE_NORMAL   0x9F
#define AWBCTR0_VALUE_ADVANCED 0x9E

// Gain
#define COM9                        0x14
#define COM9_VALUE_MAX_GAIN_128X    0x6A
#define COM9_VALUE_4XGAIN           0x10    // 0001 0000

#define BLUE          0x01    // AWB Blue Channel Gain
#define BLUE_VALUE    0x40

#define RED            0x02    // AWB Red Channel Gain
#define RED_VALUE      0x40

#define GGAIN            0x6A   // AWB Green Channel Gain
#define GGAIN_VALUE      0x40

#define COM16     0x41 
#define COM16_VALUE 0x08 // AWB Gain on

#define GFIX      0x69 
#define GFIX_VALUE  0x00

// Edge Enhancement Adjustment
#define EDGE      0x3f 
#define EDGE_VALUE  0x00

#define REG75     0x75 
#define REG75_VALUE 0x03

#define REG76     0x76 
#define REG76_VALUE 0xe1

// DeNoise 
#define DNSTH     0x4c 
#define DNSTH_VALUE 0x00

#define REG77     0x77 
#define REG77_VALUE 0x00

// Denoise and Edge Enhancement
#define COM16_VALUE_DENOISE_OFF_EDGE_ENHANCEMENT_OFF_AWBGAIN_ON     0x08 // Denoise off, AWB Gain on
#define COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_OFF__AWBGAIN_ON    0x18
#define COM16_VALUE_DENOISE_OFF__EDGE_ENHANCEMENT_ON__AWBGAIN_ON    0x28
#define COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_ON__AWBGAIN_ON     0x38 // Denoise on,  Edge Enhancement on, AWB Gain on

// 30FPS Frame Rate , PCLK = 24Mhz
#define CLKRC_VALUE_30FPS  0x80

#define DBLV               0x6b
#define DBLV_VALUE_30FPS   0x0A

#define EXHCH              0x2A
#define EXHCH_VALUE_30FPS  0x00

#define EXHCL              0x2B
#define EXHCL_VALUE_30FPS  0x00

#define DM_LNL               0x92
#define DM_LNL_VALUE_30FPS   0x00

#define DM_LNH               0x93
#define DM_LNH_VALUE_30FPS   0x00

#define COM11_VALUE_30FPS    0x0A   

// Saturation Control
#define SATCTR      0xc9 
#define SATCTR_VALUE  0x60

// AEC/AGC - Automatic Exposure/Gain Control
#define GAIN    0x00 
#define GAIN_VALUE  0x00

#define AEW     0x24 
#define AEW_VALUE 0x95

#define AEB     0x25 
#define AEB_VALUE 0x33

#define VPT     0x26 
#define VPT_VALUE 0xe3

// AEC/AGC Control- Histogram
#define HAECC1      0x9f 
#define HAECC1_VALUE  0x78

#define HAECC2      0xa0 
#define HAECC2_VALUE  0x68

#define HAECC3      0xa6 
#define HAECC3_VALUE  0xd8

#define HAECC4      0xa7 
#define HAECC4_VALUE  0xd8

#define HAECC5      0xa8 
#define HAECC5_VALUE  0xf0

#define HAECC6      0xa9 
#define HAECC6_VALUE  0x90

#define HAECC7                          0xaa  // AEC Algorithm selection
#define HAECC7_VALUE_HISTOGRAM_AEC_ON 0x94 
#define HAECC7_VALUE_AVERAGE_AEC_ON     0x00

// Array Control
#define CHLF      0x33 
#define CHLF_VALUE  0x0b

#define ARBLM     0x34 
#define ARBLM_VALUE 0x11

// ADC Control
#define ADCCTR1     0x21 
#define ADCCTR1_VALUE 0x02

#define ADCCTR2     0x22 
#define ADCCTR2_VALUE 0x91

#define ADC     0x37 
#define ADC_VALUE       0x1d

#define ACOM      0x38 
#define ACOM_VALUE  0x71

#define OFON      0x39 
#define OFON_VALUE  0x2a

// Black Level Calibration
#define ABLC1     0xb1 
#define ABLC1_VALUE 0x0c

#define THL_ST    0xb3 
#define THL_ST_VALUE  0x82

// Window Output 
#define HSTART               0x17
#define HSTART_VALUE_DEFAULT 0x11
#define HSTART_VALUE_VGA     0x13     
#define HSTART_VALUE_QVGA    0x13   
#define HSTART_VALUE_QQVGA   0x13   // Works

#define HSTOP                0x18
#define HSTOP_VALUE_DEFAULT  0x61
#define HSTOP_VALUE_VGA      0x01   
#define HSTOP_VALUE_QVGA     0x01  
#define HSTOP_VALUE_QQVGA    0x01   // Works 

#define HREF                  0x32
#define HREF_VALUE_DEFAULT    0x80
#define HREF_VALUE_VGA        0xB6   
#define HREF_VALUE_QVGA       0x24
#define HREF_VALUE_QQVGA      0xA4  

#define VSTRT                0x19
#define VSTRT_VALUE_DEFAULT  0x03
#define VSTRT_VALUE_VGA      0x02
#define VSTRT_VALUE_QVGA     0x02
#define VSTRT_VALUE_QQVGA    0x02  
 
#define VSTOP                0x1A
#define VSTOP_VALUE_DEFAULT  0x7B
#define VSTOP_VALUE_VGA      0x7A
#define VSTOP_VALUE_QVGA     0x7A
#define VSTOP_VALUE_QQVGA    0x7A  

#define VREF                 0x03
#define VREF_VALUE_DEFAULT   0x03
#define VREF_VALUE_VGA       0x0A   
#define VREF_VALUE_QVGA      0x0A
#define VREF_VALUE_QQVGA     0x0A  

// I2C 
#define OV7670_I2C_ADDRESS                 0x21
#define I2C_ERROR_WRITING_START_ADDRESS      11
#define I2C_ERROR_WRITING_DATA               22

#define DATA_TOO_LONG                  1      // data too long to fit in transmit buffer 
#define NACK_ON_TRANSMIT_OF_ADDRESS    2      // received NACK on transmit of address 
#define NACK_ON_TRANSMIT_OF_DATA       3      // received NACK on transmit of data 
#define OTHER_ERROR                    4      // other error 

#define I2C_READ_START_ADDRESS_ERROR        33
#define I2C_READ_DATA_SIZE_MISMATCH_ERROR   44

byte ReadRegisterValue(int RegisterAddress)
{
  byte data = 0;
  
  Wire.beginTransmission(OV7670_I2C_ADDRESS);         
  Wire.write(RegisterAddress);                        
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS, 1);            
  while(Wire.available() < 1);              
  data = Wire.read(); 

  return data;  
}

void ReadRegisters()
{
  byte data = 0;
  data = ReadRegisterValue(CLKRC);
  data = ReadRegisterValue(COM7);
  data = ReadRegisterValue(COM3);
  data = ReadRegisterValue(COM14); 
  data = ReadRegisterValue(SCALING_XSC);
  data = ReadRegisterValue(SCALING_YSC);
  data = ReadRegisterValue(SCALING_DCWCTR);
  data = ReadRegisterValue(SCALING_PCLK_DIV);
  data = ReadRegisterValue(SCALING_PCLK_DELAY);
  data = ReadRegisterValue(TSLB);
  data = ReadRegisterValue(COM13);
  data = ReadRegisterValue(COM17);
  data = ReadRegisterValue(COM4);
  data = ReadRegisterValue(COM15);
  data = ReadRegisterValue(COM11);
  data = ReadRegisterValue(COM8);
  data = ReadRegisterValue(HAECC7);
  data = ReadRegisterValue(GFIX);
  data = ReadRegisterValue(HSTART);
  data = ReadRegisterValue(HSTOP);
  data = ReadRegisterValue(HREF);
  data = ReadRegisterValue(VSTRT);
  data = ReadRegisterValue(VSTOP);
  data = ReadRegisterValue(VREF);
}


void ResetCameraRegisters()
{
  // Reset Camera Registers
  // Reading needed to prevent error
  byte data = ReadRegisterValue(COM7);
  
  int result = OV7670WriteReg(COM7, COM7_VALUE_RESET );
  String sresult = ParseI2CResult(result);
  //Serial.println("RESETTING ALL REGISTERS BY SETTING COM7 REGISTER to 0x80: " + sresult);

  // Delay at least 500ms 
  delay(500);
}

// Main Call to Setup the ov7670 Camera
void SetupCamera()
{
  //Serial.println(F("In SetupCamera() ..."));
  InitializeOV7670Camera();
}

void InitializeOV7670Camera()
{
  //Serial.println(F("Initializing OV7670 Camera ..."));
  
  //Set WRST to 0 and RRST to 0 , 0.1ms after power on.
  int DurationMicroSecs =  1;
  
  // Set mode for pins wither input or output
  pinMode(WRST , OUTPUT);
  pinMode(RRST , OUTPUT);
  pinMode(WEN  , OUTPUT);
  pinMode(VSYNC, INPUT);
  pinMode(RCLK , OUTPUT);
  
  // FIFO Ram output pins
  pinMode(DO7 , INPUT);
  pinMode(DO6 , INPUT);
  pinMode(DO5 , INPUT);
  pinMode(DO4 , INPUT);
  pinMode(DO3 , INPUT);
  pinMode(DO2 , INPUT);
  pinMode(DO1 , INPUT);
  pinMode(DO0 , INPUT);
  
  // Delay 1 ms 
  delay(1); 
  
  PulseLowEnabledPin(WRST, DurationMicroSecs); 
  
  //PulseLowEnabledPin(RRST, DurationMicroSecs); 
  // Need to clock the fifo manually to get it to reset
  digitalWrite(RRST, LOW);
  PulsePin(RCLK, DurationMicroSecs); 
  PulsePin(RCLK, DurationMicroSecs); 
  digitalWrite(RRST, HIGH);  
}

void SetupCameraAdvancedAutoWhiteBalanceConfig()
{
   int result = 0;
   String sresult = "";
  
   result = OV7670WriteReg(AWBC1, AWBC1_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC2, AWBC2_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC3, AWBC3_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC4, AWBC4_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC5, AWBC5_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC6, AWBC6_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC7, AWBC7_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC8, AWBC8_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC9, AWBC9_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC10, AWBC10_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC11, AWBC11_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBC12, AWBC12_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBCTR3, AWBCTR3_VALUE);
   sresult = ParseI2CResult(result);
   //Serial.println(sresult);
 
   result = OV7670WriteReg(AWBCTR2, AWBCTR2_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AWBCTR1, AWBCTR1_VALUE);
   sresult = ParseI2CResult(result);
}

void SetupCameraUndocumentedRegisters()
{ 
   // Write(0xb0,0x84); //adding this improve the color a little bit
   int result = 0;
   String sresult = "";
   
   result = OV7670WriteReg(0xB0, 0x84);
   sresult = ParseI2CResult(result);
}

void SetupCameraFor30FPS()
{
   int result = 0;
   String sresult = "";
   
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_30FPS);
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(DBLV, DBLV_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(EXHCH, EXHCH_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(EXHCL, EXHCL_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(DM_LNL, DM_LNL_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(DM_LNH, DM_LNH_VALUE_30FPS);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(COM11, COM11_VALUE_30FPS);
   sresult = ParseI2CResult(result);
}

void SetupCameraABLC()
{
   int result = 0;
   String sresult = "";
   
   // If ABLC is off then return otherwise
   // turn on ABLC.
   if (ABLCParam == "AblcOFF")
   {
     return;
   }
   
   result = OV7670WriteReg(ABLC1, ABLC1_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(THL_ST, THL_ST_VALUE);
   sresult = ParseI2CResult(result);
}



void SetupCameraAverageBasedAECAGC()
{
   int result = 0;
   String sresult = "";
   
   //Serial.println(F("-------------- Setting Camera Average Based AEC/AGC Registers ---------------"));
  
   result = OV7670WriteReg(AEW, AEW_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AEB, AEB_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(VPT, VPT_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(HAECC7, HAECC7_VALUE_AVERAGE_AEC_ON);
   sresult = ParseI2CResult(result);   
}

void SetCameraHistogramBasedAECAGC()
{
   int result = 0;
   String sresult = "";
    
   result = OV7670WriteReg(AEW, AEW_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(AEB, AEB_VALUE);
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(HAECC1, HAECC1_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(HAECC2, HAECC2_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(HAECC3, HAECC3_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(HAECC4, HAECC4_VALUE);
   sresult = ParseI2CResult(result);
    
   result = OV7670WriteReg(HAECC5, HAECC5_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(HAECC6, HAECC6_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(HAECC7, HAECC7_VALUE_HISTOGRAM_AEC_ON);
   sresult = ParseI2CResult(result);
}



void SetupCameraNightMode()
{
   int result = 0;
   String sresult = "";
   
   //Serial.println(F("......... Turning NIGHT MODE ON ........"));
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_NIGHTMODE_AUTO);
   sresult = ParseI2CResult(result);
   //Serial.print(F("CLKRC: "));
   //Serial.println(sresult);
 
   result = OV7670WriteReg(COM11, COM11_VALUE_NIGHTMODE_AUTO);
   sresult = ParseI2CResult(result);
   //Serial.print(F("COM11: "));
   //Serial.println(sresult); 
}

void SetupCameraSimpleAutomaticWhiteBalance()
{
 /*
   i2c_salve_Address = 0x42;
   write_i2c(0x13, 0xe7); //AWB on
   write_i2c(0x6f, 0x9f); // Simple AWB
 */
 
   int result = 0;
   String sresult = "";
   
   // COM8
   //result = OV7670WriteReg(0x13, 0xE7);
   result = OV7670WriteReg(COM8, COM8_VALUE_AWB_ON);
   sresult = ParseI2CResult(result);
   
   // AWBCTR0
   //result = OV7670WriteReg(0x6f, 0x9f);
   result = OV7670WriteReg(AWBCTR0, AWBCTR0_VALUE_NORMAL);
   sresult = ParseI2CResult(result);
}

void SetupCameraAdvancedAutomaticWhiteBalance()
{
   int result = 0;
   String sresult = "";
   
   //Serial.println(F("........... Setting Camera to Advanced AWB ........"));
  
   // AGC, AWB, and AEC Enable
   result = OV7670WriteReg(0x13, 0xE7);
   sresult = ParseI2CResult(result);
   //Serial.print(F("COM8(0x13): "));
   //Serial.println(sresult);
 
   // AWBCTR0 
   result = OV7670WriteReg(0x6f, 0x9E);
   sresult = ParseI2CResult(result);
   //Serial.print(F("AWB Control Register 0(0x6F): "));
   //Serial.println(sresult);
}

void SetupCameraGain()
{
   int result = 0;
   String sresult = "";
  
   // Set Maximum Gain
   //result = OV7670WriteReg(COM9, COM9_VALUE_MAX_GAIN_128X);
   result = OV7670WriteReg(COM9, COM9_VALUE_4XGAIN);
   //result = OV7670WriteReg(COM9, 0x18);
   sresult = ParseI2CResult(result);
   
   // Set Blue Gain
   //{ REG_BLUE, 0x40 },
   result = OV7670WriteReg(BLUE, BLUE_VALUE);
   sresult = ParseI2CResult(result);
   
   // Set Red Gain
   //{ REG_RED, 0x60 },
   result = OV7670WriteReg(RED, RED_VALUE);
   sresult = ParseI2CResult(result);
   
   // Set Green Gain
   //{ 0x6a, 0x40 }, 
   result = OV7670WriteReg(GGAIN, GGAIN_VALUE);
   sresult = ParseI2CResult(result);
  
   // Enable AWB Gain
   // REG_COM16 0x41  /* Control 16 */
   // COM16_AWBGAIN   0x08    /* AWB gain enable */
   // { REG_COM16, COM16_AWBGAIN }, 
   result = OV7670WriteReg(COM16, COM16_VALUE);
   sresult = ParseI2CResult(result);
}

void SetCameraSaturationControl()
{
  int result = 0;
  String sresult = "";
  
  result = OV7670WriteReg(SATCTR, SATCTR_VALUE);
  sresult = ParseI2CResult(result);
}

void SetCameraColorMatrixYUV()
{
  int result = 0;
  String sresult = "";
 
  result = OV7670WriteReg(MTX1, MTX1_VALUE);
  sresult = ParseI2CResult(result);
 
  result = OV7670WriteReg(MTX2, MTX2_VALUE);
  sresult = ParseI2CResult(result);
 
  result = OV7670WriteReg(MTX3, MTX3_VALUE);
  sresult = ParseI2CResult(result);
  
  result = OV7670WriteReg(MTX4, MTX4_VALUE);
  sresult = ParseI2CResult(result);
 
  result = OV7670WriteReg(MTX5, MTX5_VALUE);
  sresult = ParseI2CResult(result);
  
  result = OV7670WriteReg(MTX6, MTX6_VALUE);
  sresult = ParseI2CResult(result);
  
  result = OV7670WriteReg(CONTRAS, CONTRAS_VALUE);
  sresult = ParseI2CResult(result);
 
  result = OV7670WriteReg(MTXS, MTXS_VALUE);
  sresult = ParseI2CResult(result);  
}

void SetCameraFPSMode()
{
   // Set FPS for Camera
   if (FPSParam == "ThirtyFPS")
   {
     SetupCameraFor30FPS();
   }    
   else
   if (FPSParam == "NightMode")
   {
     SetupCameraNightMode();
   } 
}

void SetCameraAEC()
{
    // Process AEC
   if (AECParam == "AveAEC")
   {
     // Set Camera's Average AEC/AGC Parameters  
     SetupCameraAverageBasedAECAGC();  
   }
   else
   if (AECParam == "HistAEC")
   { 
     // Set Camera AEC algorithim to Histogram
     SetCameraHistogramBasedAECAGC();
   }
}

void SetupCameraAWB()
{
   // Set AWB Mode
   if (AWBParam == "SAWB")
   {
     // Set Simple Automatic White Balance
     SetupCameraSimpleAutomaticWhiteBalance(); // OK
      
     // Set Gain Config
     SetupCameraGain();
   }
   else
   if (AWBParam == "AAWB")
   {
     // Set Advanced Automatic White Balance
     SetupCameraAdvancedAutomaticWhiteBalance(); // ok
   
     // Set Camera Automatic White Balance Configuration
     SetupCameraAdvancedAutoWhiteBalanceConfig(); // ok
     
     // Set Gain Config
     SetupCameraGain();
   }
}


void SetupCameraDenoise()
{  
   int result = 0;
   String sresult = "";
   
   //Serial.println(F("........... Setting Camera Denoise  ........"));
  
   result = OV7670WriteReg(DNSTH, DNSTH_VALUE);
   sresult = ParseI2CResult(result);
   //Serial.print(F("DNSTH: "));
   //Serial.println(sresult);
 
   result = OV7670WriteReg(REG77, REG77_VALUE);
   sresult = ParseI2CResult(result);
   //Serial.print(F("REG77: "));
   //Serial.println(sresult);
}


void SetupCameraEdgeEnhancement()
{
   int result = 0;
   String sresult = "";
   
   //Serial.println(F("........... Setting Camera Edge Enhancement  ........"));
  
   result = OV7670WriteReg(EDGE, EDGE_VALUE);
   sresult = ParseI2CResult(result);
   //Serial.print(F("EDGE: "));
   //Serial.println(sresult);
 
   result = OV7670WriteReg(REG75, REG75_VALUE);
   sresult = ParseI2CResult(result);
   //Serial.print(F("REG75: "));
   //Serial.println(sresult);
 
   result = OV7670WriteReg(REG76, REG76_VALUE);
   sresult = ParseI2CResult(result);
   //Serial.print(F("REG76: "));
   //Serial.println(sresult);
}

void SetupCameraDenoiseEdgeEnhancement()
{
   int result = 0;
   String sresult = "";
   
   if ((DenoiseParam == "DenoiseYes")&& 
       (EdgeParam == "EdgeYes"))
      {
        SetupCameraDenoise();
        SetupCameraEdgeEnhancement();
        result = OV7670WriteReg(COM16, COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_ON__AWBGAIN_ON);
        sresult = ParseI2CResult(result);
        //Serial.print(F("COM16: "));
        //Serial.println(sresult);
      }
      else
      if ((DenoiseParam == "DenoiseYes")&& 
          (EdgeParam == "EdgeNo"))
       {
         SetupCameraDenoise();
         result = OV7670WriteReg(COM16, COM16_VALUE_DENOISE_ON__EDGE_ENHANCEMENT_OFF__AWBGAIN_ON);
         sresult = ParseI2CResult(result);
         //Serial.print(F("COM16: "));
         //Serial.println(sresult);
       }
       else
       if ((DenoiseParam == "DenoiseNo")&& 
          (EdgeParam == "EdgeYes"))
          {
            SetupCameraEdgeEnhancement();
            result = OV7670WriteReg(COM16, COM16_VALUE_DENOISE_OFF__EDGE_ENHANCEMENT_ON__AWBGAIN_ON);
            sresult = ParseI2CResult(result);
            //Serial.print(F("COM16: "));
            //Serial.println(sresult);
          }
}


void SetupCameraArrayControl()
{
   int result = 0;
   String sresult = "";
  
   result = OV7670WriteReg(CHLF, CHLF_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(ARBLM, ARBLM_VALUE);
   sresult = ParseI2CResult(result);
}


void SetupCameraADCControl()
{
   int result = 0;
   String sresult = "";
   
   result = OV7670WriteReg(ADCCTR1, ADCCTR1_VALUE);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(ADCCTR2, ADCCTR2_VALUE);
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(ADC, ADC_VALUE);
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(ACOM, ACOM_VALUE);
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(OFON, OFON_VALUE);
   sresult = ParseI2CResult(result); 
}


void SetupOV7670ForQQVGAYUV()
{
   int result = 0;
   String sresult = "";
   
   //Serial.println(F("--------------------------- Setting Camera for QQVGA YUV ---------------------------"));
  
   PHOTO_WIDTH  = 160;
   PHOTO_HEIGHT = 20; 
   PHOTO_BYTES_PER_PIXEL = 2;

   //Serial.print(F("Photo Width = "));
   //Serial.println(PHOTO_WIDTH);
   
   //Serial.print(F("Photo Height = "));
   //Serial.println(PHOTO_HEIGHT);
   
   //Serial.print(F("Bytes Per Pixel = "));
   //Serial.println(PHOTO_BYTES_PER_PIXEL);
   
   result = OV7670WriteReg(CLKRC, CLKRC_VALUE_QQVGA);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(COM7, COM7_VALUE_QQVGA );
   //result = OV7670WriteReg(COM7, COM7_VALUE_QQVGA_COLOR_BAR );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(COM3, COM3_VALUE_QQVGA); 
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(COM14, COM14_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(SCALING_XSC,SCALING_XSC_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(SCALING_YSC,SCALING_YSC_VALUE_QQVGA );    
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(SCALING_DCWCTR, SCALING_DCWCTR_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(SCALING_PCLK_DIV, SCALING_PCLK_DIV_VALUE_QQVGA);
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(SCALING_PCLK_DELAY,SCALING_PCLK_DELAY_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   // YUV order control change from default use with COM13
   result = OV7670WriteReg(TSLB, TSLB_VALUE_YUYV_AUTO_OUTPUT_WINDOW_DISABLED); // Works
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(COM13, 0xC8);  // Gamma Enabled, UV Auto Adj On
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(COM17, COM17_VALUE_AEC_NORMAL_NO_COLOR_BAR);
   sresult = ParseI2CResult(result);
   
   // Set Additional Parameters
   // Set Camera Frames per second
   SetCameraFPSMode();
    
   // Set Camera Automatic Exposure Control
   SetCameraAEC();
   
   // Set Camera Automatic White Balance
   SetupCameraAWB();
   
   // Setup Undocumented Registers - Needed Minimum
   SetupCameraUndocumentedRegisters();
  
 
   // Set Color Matrix for YUV
   if (YUVMatrixParam == "YUVMatrixOn")
   {
     SetCameraColorMatrixYUV();
   }
  
   // Set Camera Saturation
   SetCameraSaturationControl();
   
   // Denoise and Edge Enhancement
   SetupCameraDenoiseEdgeEnhancement();
   
   // Set New Gamma Values
   //SetCameraGamma();
   
   // Set Array Control
   SetupCameraArrayControl();
   
   // Set ADC Control
   SetupCameraADCControl();

   // Set Automatic Black Level Calibration
   SetupCameraABLC();
    
   // Change Window Output parameters after custom scaling
   result = OV7670WriteReg(HSTART, HSTART_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(HSTOP, HSTOP_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
  
   result = OV7670WriteReg(HREF, HREF_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(VSTRT, VSTRT_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(VSTOP, VSTOP_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
   
   result = OV7670WriteReg(VREF, VREF_VALUE_QQVGA );
   sresult = ParseI2CResult(result);
}

void CaptureOV7670Frame()
{
   unsigned long DurationStart = 0;
   unsigned long DurationStop = 0;
   unsigned long TimeForCaptureStart = 0;
   unsigned long TimeForCaptureEnd = 0;
   unsigned long ElapsedTime = 0;
   
   //Capture one frame into FIFO memory
   
   // 0. Initialization. 
   //Serial.println();
   //Serial.println(F("Starting Capture of Photo ..."));
   TimeForCaptureStart = millis();
 
   // 1. Wait for VSync to pulse to indicate the start of the image
   DurationStart = pulseIn(VSYNC, HIGH);
 
   // 2. Reset Write Pointer to 0. Which is the beginning of frame
   PulseLowEnabledPin(WRST, 6); // 3 microseconds + 3 microseconds for error factor on Arduino

   // 3. Set FIFO Write Enable to active (high) so that image can be written to ram
   digitalWrite(WEN, HIGH);
  
   // 4. Wait for VSync to pulse again to indicate the end of the frame capture
   DurationStop = pulseIn(VSYNC, HIGH);
  
   // 5. Set FIFO Write Enable to nonactive (low) so that no more images can be written to the ram
   digitalWrite(WEN, LOW);
     
   // 6. Print out Stats
   TimeForCaptureEnd = millis();
   ElapsedTime = TimeForCaptureEnd - TimeForCaptureStart;
   
   //Serial.print(F("Time for Frame Capture (milliseconds) = "));
   //Serial.println(ElapsedTime);
   
   // 7. WAIT so that new data can appear on output pins Read new data.
   delay(2);
}


// Converts pin HIGH/LOW values on pins at positions 0-7 to a corresponding byte value
byte ConvertPinValueToByteValue(int PinValue, int PinPosition)
{
  byte ByteValue = 0;
  if (PinValue == HIGH)
  {
    ByteValue = 1 << PinPosition;
  }
  
  return ByteValue;
}

void ReadTransmitCapturedFrame()
{
   unsigned long Yvalue = 0;
   byte PixelData = 0;
   byte PinVal7 = 0;
   byte PinVal6 = 0;
   byte PinVal5 = 0;
   byte PinVal4 = 0;
   byte PinVal3 = 0;
   byte PinVal2 = 0;
   byte PinVal1 = 0;
   byte PinVal0 = 0;
   
   
   // Set Read Buffer Pointer to start of frame
   digitalWrite(RRST, LOW);
   PulsePin(RCLK, 1); 
   PulsePin(RCLK, 1);
   PulsePin(RCLK, 1);
   digitalWrite(RRST, HIGH);
   
   unsigned long  ByteCounter = 0;
  
   for (int height = 0; height < PHOTO_HEIGHT; height++)
   {
     unsigned long counter = 0;
     for (int width = 0; width < PHOTO_WIDTH; width++)
     {
       for (int bytenumber = 0; bytenumber < PHOTO_BYTES_PER_PIXEL; bytenumber++)
       {
         // Pulse the read clock RCLK to bring in new byte of data.
         // 7ns for RCLK High Pulse Width and Low Pulse Width .007 micro secs
         PulsePin(RCLK, 1); 
             
         // Convert Pin values to byte values for pins 0-7 of incoming pixel byte
         PinVal7 = ConvertPinValueToByteValue(digitalRead(DO7), 7);
         PinVal6 = ConvertPinValueToByteValue(digitalRead(DO6), 6);
         PinVal5 = ConvertPinValueToByteValue(digitalRead(DO5), 5);
         PinVal4 = ConvertPinValueToByteValue(digitalRead(DO4), 4);
         PinVal3 = ConvertPinValueToByteValue(digitalRead(DO3), 3);
         PinVal2 = ConvertPinValueToByteValue(digitalRead(DO2), 2);
         PinVal1 = ConvertPinValueToByteValue(digitalRead(DO1), 1);
         PinVal0 = ConvertPinValueToByteValue(digitalRead(DO0), 0);
     
         // Combine individual data from each pin into composite data in the form of a single byte
         PixelData = PinVal7 | PinVal6 | PinVal5 | PinVal4 | PinVal3 | PinVal2 | PinVal1 | PinVal0;
         Yvalue++;
         //Serial.println(PixelData, DEC);
         if(Yvalue%2 != 0 )
         {
         ImageData[height][counter++] = PixelData;
         //Serial.print(PixelData);
         //Serial.print(" ");
         }
         /////////////////////////////  SD Card ////////////////////////////////
         ByteCounter++;      
         ///////////////////////////////////////////////////////////////////////

       }
       
     }
     //Serial.println();
     //Serial.println(counter);
   }
   /*
   for(int i=0; i<40; i++)
   {
    for(int j=0; j<80; j++)
      {
       Serial.println(ImageData[i][j],HEX);
      }
    }
    */
   //Serial.print(F("Total Bytes = "));
   //Serial.println(ByteCounter);
    
        
       delay(100);
}

void TakePhoto()
{
  // Take Photo using the ov7670 camera and transmit the image to the Android controller via 
  // Bluetooth
  unsigned long StartTime   = 0;
  unsigned long EndTime     = 0;
  unsigned long ElapsedTime = 0;
  
  StartTime = millis();
  
  CaptureOV7670Frame(); 
  ReadTransmitCapturedFrame();
 
  EndTime = millis(); 
  ElapsedTime = (EndTime - StartTime)/1000; // Convert to seconds
  
  //Serial.print(F("Elapsed Time for Taking and Sending Photo(secs) = "));
  //Serial.println(ElapsedTime);
}

void PulseLowEnabledPin(int PinNumber, int DurationMicroSecs)
{
  // For Low Enabled Pins , 0 = on and 1 = off
  digitalWrite(PinNumber, LOW);            // Sets the pin on
  delayMicroseconds(DurationMicroSecs);    // Pauses for DurationMicroSecs microseconds      
  
  digitalWrite(PinNumber, HIGH);            // Sets the pin off
  delayMicroseconds(DurationMicroSecs);     // Pauses for DurationMicroSecs microseconds  
}

void PulsePin(int PinNumber, int DurationMicroSecs)
{
  digitalWrite(PinNumber, HIGH);           // Sets the pin on
  delayMicroseconds(DurationMicroSecs);    // Pauses for DurationMicroSecs microseconds      
  
  digitalWrite(PinNumber, LOW);            // Sets the pin off
  delayMicroseconds(DurationMicroSecs);    // Pauses for DurationMicroSecs microseconds  
}

String ParseI2CResult(int result)
{
  String sresult = "";
  switch(result)
  {
    case 0:
     sresult = "I2C Operation OK ...";
    break;
    
    case  I2C_ERROR_WRITING_START_ADDRESS:
     sresult = "I2C_ERROR_WRITING_START_ADDRESS";
    break;
    
    case I2C_ERROR_WRITING_DATA:
     sresult = "I2C_ERROR_WRITING_DATA";
    break;
      
    case DATA_TOO_LONG:
     sresult = "DATA_TOO_LONG";
    break;   
    
    case NACK_ON_TRANSMIT_OF_ADDRESS:
     sresult = "NACK_ON_TRANSMIT_OF_ADDRESS";
    break;
    
    case NACK_ON_TRANSMIT_OF_DATA:
     sresult = "NACK_ON_TRANSMIT_OF_DATA";
    break;
    
    case OTHER_ERROR:
     sresult = "OTHER_ERROR";
    break;
       
    default:
     sresult = "I2C ERROR TYPE NOT FOUND...";
    break;
  }
 
  return sresult;
}


// Parameters:
//   start : Start address, use a define for the register
//   pData : A pointer to the data to write.
//   size  : The number of bytes to write.
//
int OV7670Write(int start, const byte *pData, int size)
{
  int n, error;

  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  n = Wire.write(start);        // write the start address
  if (n != 1)
  {
    return (I2C_ERROR_WRITING_START_ADDRESS);
  }
  
  n = Wire.write(pData, size);  // write data bytes
  if (n != size)
  {
    return (I2C_ERROR_WRITING_DATA);
  }
  
  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
  {
    return (error);
  }
  
  return 0;         // return : no error
}


//
// A function to write a single register
//
int OV7670WriteReg(int reg, byte data)
{
  int error;

  error = OV7670Write(reg, &data, 1);

  return (error);
}

//
// This is a common function to read multiple bytes 
// from an I2C device.
//
// It uses the boolean parameter for Wire.endTransMission()
// to be able to hold or release the I2C-bus. 
// This is implemented in Arduino 1.0.1.
//
int OV7670Read(int start, byte *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
  {
    return (I2C_READ_START_ADDRESS_ERROR);
  }
  
  n = Wire.endTransmission(false);    // hold the I2C-bus
  if (n != 0)
  {
    return (n);
  }
  
  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(OV7670_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++] = Wire.read();
  }
  if ( i != size)
  {
    return (I2C_READ_DATA_SIZE_MISMATCH_ERROR);
  }
  
  return (0);  // return no error
}

//
// A function to read a single register
//
int OV7670ReadReg(int reg, byte *data)
{
  int error;

  error = OV7670Read(reg, data, 1);

  return (error);
}

void setup()
{
     // Initialize Serial
     Serial.begin(19200); 
     //Serial1.begin(9600);
     // Setup the OV7670 Camera for use in taking still photos
     Wire.begin();
     ResetCameraRegisters();
     ReadRegisters();
     SetupCamera();    
     //Serial.println(F("FINISHED INITIALIZING CAMERA ..."));
     //Serial.println();
}

void ExecuteCommand(String Command)
{
  // Set up Camera for VGA, QVGA, or QQVGA Modes
  
   if (Command == "QQVGA")
  {
     //Serial.println(F("Taking a QQVGA Photo...")); 
     if (Resolution != QQVGA)
     {
       // If current resolution is not QQVGA then set camera for QQVGA
       Resolution = QQVGA;
       SetupOV7670ForQQVGAYUV();
       ReadRegisters();
     }
  }
  else
  {
     //Serial.print(F("The command ")); 
     //Serial.print(Command);
     //Serial.println(F(" is not recognized ..."));
  }
  
  
  // Delay for registers to settle
  delay(100);
  
  // Take Photo 
  TakePhoto();
}


void loop()
{   

  
     if (RawCommandLine == "t")
     {
         // Take Photo
         //Serial.println(F("\nGoing to take photo with current command:")); 
                 
         // Take Photo
         ExecuteCommand(Command);    
         //Serial.println(F("Photo Taken seccessfully")); 
         PhotoTakenCount++;
         //Process taken image
         
         for(int i=0; i<20; i++)
         {
          for(int j=2; j<60; j++)
          {
            //Serial.println(ImageData[i][j]);
            sumL = sumL + ImageData[i][j];
           }
          }
           meanL = ImageData[19][5];//sumL/1160;
            //Serial.println(meanL);

           for(int z=0; z<20; z++)
           {
            for(int w=60; w<100; w++)
            {
              sumM = sumM + ImageData[z][w];
             }
            }
            meanM = ImageData[19][80];//sumM/800;
            
          for(int y=0; y<20; y++)
          {
            for(int x=100; x<158; x++)
            {
              sumR = sumR + ImageData[y][x];
             }
           }
           meanR = ImageData[19][155];//sumR/1160;
           /*
           Serial.print(meanL);
           Serial.print("\t");
           Serial.print(meanM);
           Serial.print("\t");
           Serial.println(meanR);
           */
           
          
      if(meanL>meanM && meanL>meanR)
      {
        Serial.write("l");//108
        Serial.write("20");
        //delay(1500);
        }
         
          else if(meanM>meanL && meanM>meanR)
          {
            Serial.write("f");//102
            Serial.write("0");
            //delay(1500);
          }
           
            else if(meanR>meanL && meanR>meanM)
            {
              Serial.write("r");//114
              Serial.write("20");
              //delay(1500);
              }
         //delay(1000);
         sumL = 0;
         sumM = 0;
         sumR = 0;    
     }
    
}
