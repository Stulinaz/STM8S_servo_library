#include "stm8s.h"
#include "servo.h"

//Before use this application make sure that:
//In "Project"->"Settings"->"C Compler"->"Optimizations"
//make sure that is set to "Disable for Debugging"

void main(void)
{
	system_init();
	
	//Enable interrupts
	_asm("rim\n");
	
	servo_set_impulse(1500);
	servo_start();
	
  while (1)
  {
		delay_ms(1000);
		servo_set_impulse(1800);
		delay_ms(1000);
		servo_set_impulse(1200);
  } 
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
  {
  }
}
#endif