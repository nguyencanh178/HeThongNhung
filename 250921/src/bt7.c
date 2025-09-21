#include "stm32f10x.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_usart.h"
#include <stdio.h>

#define DS3231_ADDR 0xD0   // 0x68 << 1

void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
    // TX - PA9
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // RX - PA10
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &USART_InitStruct);

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

/* ========== I2C ========== */
void I2C1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // PB6 - SCL, PB7 - SDA
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    I2C_InitStruct.I2C_ClockSpeed = 100000;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitStruct);

    I2C_Cmd(I2C1, ENABLE);
}

/* ========== I2C Read/Write ========== */
void I2C_WriteReg(uint8_t devAddr, uint8_t regAddr, uint8_t data) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t I2C_ReadReg(uint8_t devAddr, uint8_t regAddr) {
    uint8_t data;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, regAddr);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, devAddr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data = I2C_ReceiveData(I2C1);
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return data;
}

/* ========== BCD Convert ========== */
uint8_t DEC2BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

uint8_t BCD2DEC(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

/* ========== DS3231 Function ========== */
void DS3231_SetTime(uint8_t hour, uint8_t min, uint8_t sec) {
    I2C_WriteReg(DS3231_ADDR, 0x00, DEC2BCD(sec));
    I2C_WriteReg(DS3231_ADDR, 0x01, DEC2BCD(min));
    I2C_WriteReg(DS3231_ADDR, 0x02, DEC2BCD(hour));
}

void DS3231_GetTime(uint8_t *hour, uint8_t *min, uint8_t *sec) {
    *sec  = BCD2DEC(I2C_ReadReg(DS3231_ADDR, 0x00));
    *min  = BCD2DEC(I2C_ReadReg(DS3231_ADDR, 0x01));
    *hour = BCD2DEC(I2C_ReadReg(DS3231_ADDR, 0x02));
}

/* ========== MAIN ========== */
int main(void) {
    I2C1_Init();
    USART1_Init();

     DS3231_SetTime(12, 29, 0);  // Set 21:45:00

    uint8_t h, m, s;
    char buffer[32];

    while (1) {
        DS3231_GetTime(&h, &m, &s);
        sprintf(buffer, "Time: %02d:%02d:%02d\r\n", h, m, s);
        USART1_SendString(buffer);

        for (volatile int i = 0; i < 800000; i++); // delay ~1s
    }
}
