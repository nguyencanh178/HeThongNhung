#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void delay_ms(unsigned int time)
{
    unsigned int i, j;
    for (i = 0; i < time; i++)
    {
        for (j = 0; j < 0x2aff; j++) {}
    }
}
void GPIO_init(void)
{
    GPIO_InitTypeDef GPIO;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;   
    GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO);
}
int main(void)
{
    GPIO_init();
    while (1)
    {  
        GPIO_SetBits(GPIOA, GPIO_Pin_0);
        GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        delay_ms(1000);
        GPIO_SetBits(GPIOA, GPIO_Pin_1);
        GPIO_ResetBits(GPIOA, GPIO_Pin_0);
        delay_ms(1000);
    }
}