/*
 * husbus.h
 *
 *  Created on: 08.09.2023
 *      Author: Husarion
 */

#ifndef HUSBUS_H_
#define HUSBUS_H_

#include "stdint.h"
#include "stm32f0xx_hal.h"

#define HUSBUS_BUF_SIZE 20

typedef enum
{
	Setup,
	WaitForHeader,
	HeaderReceived,
	WaitForData,
	DataReceived,
	Timeout,
	Error
}HusBus_State_t;

typedef union HusBus_Header
{
	uint8_t byte;
	struct
	{
		uint8_t IDEXT:1;
		uint8_t RTR:1;
		uint8_t DLC:4;
		uint8_t reserved:2;
	}fields;

}HusBus_Header_t;

typedef union HusBus_Frame
{
	uint8_t bytes[14];
	struct __attribute__((packed))
	{
		uint8_t  header;
		uint8_t id[4];
		uint8_t data[8];
		uint8_t crc;
	}fields;
}HusBus_Frame_t;

typedef struct
{
	uint8_t bufferParsed  :1;
	uint8_t bufferCleared :1;
	uint8_t data[UART_RX_BUFFER_SIZE];
}uart_buffer;

void Husbus_Init(uint8_t* output);
void HusBus_Update(void);
void Husbus_ParseCanFrame(CanRxMsgTypeDef *pRxMsg);
void Husbus_ParseUART(const HusBus_Frame_t* frame);

#endif /* HUSBUS_H_ */
