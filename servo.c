/*********************************************************************************************
    /////// /////  ///////   //  //////////   ////   //         //  //       //              *
   //   // //     //   //   //      //     //    //   //       //  //       //               *
  /////// ////   ///////   //      //    //       //   //    //   //       //                *
 //      //     //  //    //      //      //    //      // //    //       //                 *
//      ////// //    //  //      //        ////          //     //////   //////              *
**********************************************************************************************
Title:                Servo.c                                                                *
Autor:                PeritoValligiano                                                       *
Date:                 23.07.2020                                                             *
Device Compatibility: STM8 & HAL_Library                                                     *
Description:          Library for servomotor (Tested with SG90 and STM8S105C6T6) 						 *
Compliler:						Cosmic																																 *
Read_me:              User is invited to modify the parameters of the following code         *
											in relation to his own hardware confguration.                          *
											Pay attention to the writing"HARDWARE_DEPENDENT"and"FIRMWARE_DEPENDENT"*
																																														 *
Read_Me_a_Lot:		  I wrote the following code as a contribution to open source community.   *
					  I don't respond in any way to physical or material damage that the use 					 *
					  of this code can create.                                               					*
**********************************************************************************************/
#include "servo.h"

//HARDWARE_DEPENDENT!
//User must check his own servomotor datasheet.
//The following parameters may be changed to get
//a good device operation.
#define SERVO_MIDDLE_IMPULSE      1500  //(uS)
#define SERVO_LEFT_IMPULSE_MAX    1000  //(uS)
#define SERVO_RIGHT_IMPULSE_MAX   2000  //(uS)
#define SERVO_PERIOD              20000 //(uS)
#define ONE_MILLISEC						  20		//50us * 20 = 1ms
#define FYFTY_us                  0x0190//50us TIM2 tick @ 16Mhz
//HARDWARE_DEPENDENT!
//User add his own port and pin configuration
//to the following definitions:SERVO_PORT,SERVO_PIN  
#define SERVO_PORT                GPIOB           
#define SERVO_PIN                 GPIO_PIN_0

static volatile uint16_t servo_impulse    = 0; 
static volatile uint16_t servo_period     = 0;
static volatile uint16_t delay_ms_counter = 0;
static volatile fiftyus_counter           = 0;

void servopin_config(void);
void clock_init(void);

//Configure Clock,Servo pin and TIM2
void system_init(void)
{
	clock_init();
	servopin_config();
	
	TIM2_DeInit();
	TIM2_TimeBaseInit(TIM2_PRESCALER_2,0x00);
	TIM2_SetAutoreload(FYFTY_us);
	TIM2_SetCounter(0x0000);
	TIM2_ITConfig(TIM2_IT_UPDATE, DISABLE);
	TIM2_Cmd(DISABLE);
}

//Setup 16Mhz clock
void clock_init(void)
{
	CLK_HSICmd(ENABLE); 
	CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);
	CLK->CMSR = 0xE1;
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2, ENABLE);
}

void servopin_config(void)
{
	//Servo pin is configured push pull,initial state low.
	GPIO_Init(SERVO_PORT, SERVO_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_Init(SERVO_PORT, GPIO_PIN_1, GPIO_MODE_OUT_PP_LOW_FAST);
}

void servo_start(void)
{
	//Enable TIM2 interrupt.
	TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
	//Timer enable
	TIM2_Cmd(ENABLE);
}

void servo_stop(void)
{
	//Stop the timer.
	TIM2_Cmd(DISABLE);
	//Disable TIM2 interrupt.
	TIM2_ITConfig(TIM2_IT_UPDATE, DISABLE);
	//Reset the counter
	TIM2->CNTRH = 0;
	TIM2->CNTRL = 0;
	//Servo pin to low logic level
	GPIO_WriteLow(SERVO_PORT , SERVO_PIN);
}

void servo_set_impulse(uint16_t myservo_impulse)
{
	//myservo_impulse must be a value between 1000 and 2000. If parameter is out of range
	//myservo_impulse will be equal to middle position impulse (1500).
	if ((myservo_impulse < SERVO_LEFT_IMPULSE_MAX) && (myservo_impulse > SERVO_RIGHT_IMPULSE_MAX))
	{
		servo_impulse = (uint16_t) SERVO_MIDDLE_IMPULSE;
	}
	else
		servo_impulse = (uint16_t)(myservo_impulse / 50);
	servo_period = (uint16_t)(SERVO_PERIOD / 50);
	//50us is the TIM2 tick
}

void delay_ms(uint16_t ms)
{
	uint16_t ms_to_wait = ms;
	delay_ms_counter = 0;
	//Variable ms_counter is incremented in interrupt
	//handler function
	while(delay_ms_counter <= ms_to_wait);
	_asm("nop\n");
  
}

//FIRMWARE_DEPENDENT!
//1)_asm("rim\n"); must be called in main function
//2) INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)
//{
//}
//Be sure that you have comment it in the file stm8s_it.c
  
INTERRUPT_HANDLER(TIM2_UPD_OVF_BRK_IRQHandler, 13)
{
    TIM2_ClearITPendingBit(TIM2_IT_UPDATE);
    fiftyus_counter++;
		GPIO_WriteReverse(GPIOB,GPIO_PIN_1);
		if((fiftyus_counter % ONE_MILLISEC) == 0)
		{
			delay_ms_counter++;
		}
    if (fiftyus_counter <= servo_impulse)
    {
			GPIO_WriteHigh(SERVO_PORT , SERVO_PIN);
    }
    else
    {
        GPIO_WriteLow(SERVO_PORT , SERVO_PIN);  
    }
    if (fiftyus_counter >= servo_period)
			fiftyus_counter = 0;
}

