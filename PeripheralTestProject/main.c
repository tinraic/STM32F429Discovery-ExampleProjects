#include "main.h"


GPIO_InitTypeDef GPIO_InitStructure;

static __IO uint32_t TimingDelay;


static void Delay(__IO uint32_t nTime);   

static __IO uint32_t TimingDelay;


#define ABS(x)                     (x < 0) ? (-x) : x
#define L3G_Sensitivity_250dps     (float)114.285f        /*!< gyroscope sensitivity with 250 dps full scale [LSB/dps]  */
#define L3G_Sensitivity_500dps     (float)57.1429f        /*!< gyroscope sensitivity with 500 dps full scale [LSB/dps]  */
#define L3G_Sensitivity_2000dps    (float)14.285f         /*!< gyroscope sensitivity with 2000 dps full scale [LSB/dps] */
  

float Buffer[6];
float Gyro[3];
float X_BiasError, Y_BiasError, Z_BiasError = 0.0;
uint8_t Xval, Yval = 0x00;
static __IO uint32_t TimingDelay;


static void Demo_MEMS(void);
static void Demo_GyroConfig(void);
static void Demo_GyroReadAngRate (float* pfData);
static void Gyro_SimpleCalibration(float* GyroData);


RCC_ClocksTypeDef RCC_Clocks;

int main(void)
{
  /* SysTick end of count event each 10ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
  
  /* Initialize the LCD */
  LCD_Init();
  /* Initialize the LCD Layers*/
  LCD_LayerInit();
  
  /* Enable the LTDC */
  LTDC_Cmd(ENABLE);
  
  /* Set LCD Background Layer  */
  LCD_SetLayer(LCD_FOREGROUND_LAYER);
  
  /* Clear the Background Layer */ 
  LCD_Clear(LCD_COLOR_WHITE);

  /* Gyroscope configuration */
  Demo_GyroConfig();

  /* Gyroscope calibration */
  Gyro_SimpleCalibration(Gyro);
  
  /* Infinite loop */
  while (1)
  {
    Demo_MEMS();
  }
}

static void Demo_MEMS(void)
{   
  
  /* Read Gyro Angular data */
  Demo_GyroReadAngRate(Buffer);

  Buffer[0] = (int8_t)Buffer[0] - (int8_t)Gyro[0];
  Buffer[1] = (int8_t)Buffer[1] - (int8_t)Gyro[1];
  
  /* Update autoreload and capture compare registers value*/
  Xval = ABS((int8_t)(Buffer[0]));
  Yval = ABS((int8_t)(Buffer[1])); 
  
  if ( Xval>Yval)
  {
    if ((int16_t)Buffer[0] > 35)
    {
      /* Clear the LCD */
      LCD_Clear(LCD_COLOR_WHITE);
      LCD_SetTextColor(LCD_COLOR_MAGENTA);
      LCD_SetFont(&Font16x24);
			/*Print sting on LCD line 5*/
			LCD_DisplayStringLine(LINE(6), (uint8_t*)"  Nagib dolje");
      Delay(50);
    }
    if ((int16_t)Buffer[0] < -35)
    {
      /* Clear the LCD */
      LCD_Clear(LCD_COLOR_WHITE);
      LCD_SetTextColor(LCD_COLOR_RED);
      LCD_SetFont(&Font16x24);
			/*Print sting on LCD line 5*/
			LCD_DisplayStringLine(LINE(6), (uint8_t*)"  Nagib gore");
      Delay(50);
    }
  }
  else
  {
    if ((int16_t)Buffer[1] < -35)
    {
      /* Clear the LCD */
      LCD_Clear(LCD_COLOR_WHITE);
      LCD_SetTextColor(LCD_COLOR_GREEN);
      LCD_SetFont(&Font16x24);
			/*Print sting on LCD line 5*/
			LCD_DisplayStringLine(LINE(6), (uint8_t*)"  Nagib lijevo");      
      Delay(50);
    }
    if ((int16_t)Buffer[1] > 35)
    {      
      /* Clear the LCD */ 
      LCD_Clear(LCD_COLOR_WHITE);
      LCD_SetTextColor(LCD_COLOR_BLUE);
      LCD_SetFont(&Font16x24);
			/*Print sting on LCD line 5*/
			LCD_DisplayStringLine(LINE(6), (uint8_t*)"  Nagib desno");
      Delay(50);
    } 
  } 
}

static void Demo_GyroConfig(void)
{
  L3GD20_InitTypeDef L3GD20_InitStructure;
  L3GD20_FilterConfigTypeDef L3GD20_FilterStructure;

  /* Configure Mems L3GD20 */
  L3GD20_InitStructure.Power_Mode = L3GD20_MODE_ACTIVE;
  L3GD20_InitStructure.Output_DataRate = L3GD20_OUTPUT_DATARATE_1;
  L3GD20_InitStructure.Axes_Enable = L3GD20_AXES_ENABLE;
  L3GD20_InitStructure.Band_Width = L3GD20_BANDWIDTH_4;
  L3GD20_InitStructure.BlockData_Update = L3GD20_BlockDataUpdate_Continous;
  L3GD20_InitStructure.Endianness = L3GD20_BLE_LSB;
  L3GD20_InitStructure.Full_Scale = L3GD20_FULLSCALE_500; 
  L3GD20_Init(&L3GD20_InitStructure);
  
  L3GD20_FilterStructure.HighPassFilter_Mode_Selection =L3GD20_HPM_NORMAL_MODE_RES;
  L3GD20_FilterStructure.HighPassFilter_CutOff_Frequency = L3GD20_HPFCF_0;
  L3GD20_FilterConfig(&L3GD20_FilterStructure) ;
  
  L3GD20_FilterCmd(L3GD20_HIGHPASSFILTER_ENABLE);
}


static void Demo_GyroReadAngRate (float* pfData)
{
  uint8_t tmpbuffer[6] ={0};
  int16_t RawData[3] = {0};
  uint8_t tmpreg = 0;
  float sensitivity = 0;
  int i =0;
  
  L3GD20_Read(&tmpreg,L3GD20_CTRL_REG4_ADDR,1);
  
  L3GD20_Read(tmpbuffer,L3GD20_OUT_X_L_ADDR,6);
  
  /* check in the control register 4 the data alignment (Big Endian or Little Endian)*/
  if(!(tmpreg & 0x40))
  {
    for(i=0; i<3; i++)
    {
      RawData[i]=(int16_t)(((uint16_t)tmpbuffer[2*i+1] << 8) + tmpbuffer[2*i]);
    }
  }
  else
  {
    for(i=0; i<3; i++)
    {
      RawData[i]=(int16_t)(((uint16_t)tmpbuffer[2*i] << 8) + tmpbuffer[2*i+1]);
    }
  }
  
  /* Switch the sensitivity value set in the CRTL4 */
  switch(tmpreg & 0x30)
  {
  case 0x00:
    sensitivity=L3G_Sensitivity_250dps;
    break;
    
  case 0x10:
    sensitivity=L3G_Sensitivity_500dps;
    break;
    
  case 0x20:
    sensitivity=L3G_Sensitivity_2000dps;
    break;
  }
  /* divide by sensitivity */
  for(i=0; i<3; i++)
  {
  pfData[i]=(float)RawData[i]/sensitivity;
  }
}

static void Gyro_SimpleCalibration(float* GyroData)
{
  uint32_t BiasErrorSplNbr = 500;
  int i = 0;
  
  for (i = 0; i < BiasErrorSplNbr; i++)
  {
    Demo_GyroReadAngRate(GyroData);
    X_BiasError += GyroData[0];
    Y_BiasError += GyroData[1];
    Z_BiasError += GyroData[2];
  }
  /* Set bias errors */
  X_BiasError /= BiasErrorSplNbr;
  Y_BiasError /= BiasErrorSplNbr;
  Z_BiasError /= BiasErrorSplNbr;
  
  /* Get offset value on X, Y and Z */
  GyroData[0] = X_BiasError;
  GyroData[1] = Y_BiasError;
  GyroData[2] = Z_BiasError;
}


uint32_t L3GD20_TIMEOUT_UserCallback(void)
{
  return 0;
}

void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}


void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}


