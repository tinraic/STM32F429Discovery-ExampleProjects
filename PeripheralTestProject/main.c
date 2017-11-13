#include "main.h"


GPIO_InitTypeDef GPIO_InitStructure;

static __IO uint32_t TimingDelay;


static void Delay(__IO uint32_t nTime);   


int main(void)
{

  STM_EVAL_LEDInit(LED3);
	
  STM_EVAL_LEDInit(LED4);

  STM_EVAL_LEDOn(LED3);

  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    while (1);
  }

  while (1)
  {

    STM_EVAL_LEDToggle(LED4);

    Delay(2500);

    STM_EVAL_LEDToggle(LED3);

    Delay(1000);
  }
}

void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0)
  { 
    TimingDelay--;
  }
}
