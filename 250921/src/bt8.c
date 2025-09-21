#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "stm32f10x_spi.h"
#include "stm32f10x_usart.h"

#define LORA_NSS_LOW()   GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define LORA_NSS_HIGH()  GPIO_SetBits(GPIOA, GPIO_Pin_4)

// ================= Delay ===================
void delay(int time_ms) {
    while (time_ms--) {
        SysTick->LOAD = 72000 - 1;   // 1 ms @ 72MHz
        SysTick->VAL  = 0;
        SysTick->CTRL = 5;
        while (!(SysTick->CTRL & (1 << 16)));
        SysTick->CTRL = 0;
    }
}

// ================= UART ===================
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // TX (PA9), RX (PA10)
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

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

// ================= SPI ===================
void SPI1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

    // SCK, MOSI
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // MISO
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // NSS
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI1_Transfer(uint8_t data) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

// ================= LoRa ===================
void LoRa_WriteReg(uint8_t addr, uint8_t value) {
    LORA_NSS_LOW();
    SPI1_Transfer(addr | 0x80);
    SPI1_Transfer(value);
    LORA_NSS_HIGH();
}

uint8_t LoRa_ReadReg(uint8_t addr) {
    uint8_t val;
    LORA_NSS_LOW();
    SPI1_Transfer(addr & 0x7F);
    val = SPI1_Transfer(0x00);
    LORA_NSS_HIGH();
    return val;
}

void LoRa_Init(void) {
    uint8_t version = LoRa_ReadReg(0x42);
    char msg[32];
    sprintf(msg, "LoRa Version: 0x%02X\r\n", version);
    USART1_SendString(msg);

    LoRa_WriteReg(0x01, 0x81);
}

void LoRa_SendAndEcho(uint8_t *data, uint8_t length) {
    uint8_t i;
    char buffer[64];

    LoRa_WriteReg(0x0D, 0x00); // FIFO addr = 0
    for (i = 0; i < length; i++) {
        LoRa_WriteReg(0x00, data[i]);
    }

    LoRa_WriteReg(0x22, length);

    sprintf(buffer, "Sent: %s\r\n", data);
    USART1_SendString(buffer);

    LoRa_WriteReg(0x0D, 0x00); 
    for (i = 0; i < length; i++) {
        buffer[i] = LoRa_ReadReg(0x00);
    }
    buffer[length] = '\0';

    USART1_SendString("Echo: ");
    USART1_SendString(buffer);
    USART1_SendString("\r\n");
}

// ================= MAIN ===================
int main(void) {
    uint8_t msg[] = "Hello LoRa!";

    SystemInit();
    USART1_Init();
    SPI1_Init();

    USART1_SendString("STM32 LoRa Test Init...\r\n");
    LoRa_Init();

    while (1) {
        LoRa_SendAndEcho(msg, sizeof(msg) - 1);
        delay(1000);
    }
}
