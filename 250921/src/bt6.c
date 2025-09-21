#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include <string.h>

#define LED_PORT GPIOC
#define LED_PIN  GPIO_Pin_13
#define LED_ON()   GPIO_ResetBits(LED_PORT, LED_PIN)
#define LED_OFF()  GPIO_SetBits(LED_PORT, LED_PIN)
char rx_buffer[20];
uint8_t rx_index = 0;
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStruct);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_Cmd(USART1, ENABLE);
}


void USART1_SendChar(char c) {
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, c);
}


void USART1_SendString(char *s) {
    while (*s) {
        USART1_SendChar(*s++);
    }
}

void LED_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStruct.GPIO_Pin = LED_PIN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_PORT, &GPIO_InitStruct);

    LED_OFF(); 
}


void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char c = USART_ReceiveData(USART1);

        if (c == '\n' || c == '\r') {  
            rx_buffer[rx_index] = '\0';  
            if (strcmp(rx_buffer, "ON") == 0) {
                LED_ON();
                USART1_SendString("LED ON\r\n");
            } else if (strcmp(rx_buffer, "OFF") == 0) {
                LED_OFF();
                USART1_SendString("LED OFF\r\n");
            }
            rx_index = 0;  
        } else {
            if (rx_index < sizeof(rx_buffer) - 1) {
                rx_buffer[rx_index++] = c;
            }
        }
    }
}
int main(void) {
    LED_Init();
    USART1_Init();
    USART1_SendString("Hello from STM32\r\n");
    while (1) {
        
    }
}