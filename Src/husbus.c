/*
 * husbus.c
 *
 *  Created on: 08.09.2023
 *      Author: Husarion
 */

#include "husbus.h"
#include "string.h"
static inline void outputAppendByte(uint8_t byte);

extern CAN_HandleTypeDef hcan;
extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern uint8_t husMsgLen;
extern uint8_t activeBuffer;
extern uart_buffer uart_buffers[2];
extern uint8_t rxFullFlag;
static uint8_t* outBuffer = NULL;
static uint8_t cursor = 0;
static uint32_t rxTimestamp = 0;
static HusBus_Frame_t rxFrame = {0}, txFrame = {0};
void Husbus_Init(uint8_t* output)
{
	outBuffer = output;
}

void HusBus_Update()
{
	HAL_UART_Receive_DMA(&huart2, uart_buffers[activeBuffer].data, 14);
	if(rxFullFlag)
	{
		rxFullFlag = 0;
		Husbus_ParseUART((HusBus_Frame_t*) &uart_buffers[activeBuffer].data);
		memset(&uart_buffers[activeBuffer].data, 0, sizeof(uart_buffers[0].data));
		memset(&rxFrame, 0, sizeof(rxFrame));
	}
}

void Husbus_ParseCanFrame(CanRxMsgTypeDef *pRxMsg)
{
	uint8_t i;
	HusBus_Header_t header = {0};
	hcrc.Instance->CR |= CRC_CR_RESET;
	if(outBuffer == NULL)return;
	memset(outBuffer, 0, HUSBUS_BUF_SIZE );
	memset(txFrame.bytes, 0, 14);
	header.fields.DLC = (uint8_t)pRxMsg->DLC;
	if (pRxMsg->RTR == 1)
	{
		header.fields.RTR = 1;
	}
	else
	{
		header.fields.RTR = 0;
	}
	if (pRxMsg->IDE == CAN_ID_EXT)
	{
		header.fields.IDEXT = 1;
		// id
		for (i = 4; i != 0; i--)
		{
			txFrame.fields.id[4-i] = ((uint8_t*)&pRxMsg->ExtId)[i - 1];
		}
	}
	else
	{
		header.fields.IDEXT = 0;
		//id
		txFrame.fields.id[0] = ((uint8_t*)&pRxMsg->StdId)[0];
		txFrame.fields.id[1] = ((uint8_t*)&pRxMsg->StdId)[1];
	}

	//data
	if ((pRxMsg->DLC > 0) && (pRxMsg->RTR == CAN_RTR_DATA))
	{
		memcpy(txFrame.fields.data, pRxMsg->Data, pRxMsg->DLC);
	}
	txFrame.fields.header = header.byte;
	uint32_t crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)txFrame.bytes, 13);
	crc = ~crc;
	txFrame.fields.crc = crc;
	memcpy(outBuffer, txFrame.bytes, 14);
}

void Husbus_ParseUART(const HusBus_Frame_t* frame)
{
//	if(strlen((const char*)input) == 0)return;
	if(frame == NULL)return;
	uint32_t crc = 0;
	hcrc.Instance->CR |= CRC_CR_RESET;
	HusBus_Header_t header;
	header.byte = frame->fields.header;
	if(header.fields.RTR)
	{
		hcan.pTxMsg->RTR = CAN_RTR_REMOTE;
	}
	else
	{
		hcan.pTxMsg->RTR = CAN_RTR_DATA;
	}
	if (header.fields.IDEXT) {
		hcan.pTxMsg->IDE = CAN_ID_EXT;
		hcan.pTxMsg->ExtId = (frame->fields.id[3]<<24) | (frame->fields.id[2]<<16) | (frame->fields.id[1])<<8 | frame->fields.id[0];
	} else {
		hcan.pTxMsg->IDE = CAN_ID_STD;
		hcan.pTxMsg->StdId = (frame->fields.id[1]<<8) | frame->fields.id[0];
	}
	hcan.pTxMsg->DLC = header.fields.DLC;
	if (hcan.pTxMsg->RTR == CAN_RTR_DATA)
	{
		memcpy(hcan.pTxMsg->Data, (uint32_t*)&frame->fields.data, hcan.pTxMsg->DLC);
	}
	crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)&frame->bytes, 13);
	crc = ~crc;
	if((crc & 0xFF) == frame->fields.crc)
	{
		HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
		HAL_CAN_Transmit(&hcan, 0);
		HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
	}
}

static inline void outputAppendByte(uint8_t byte)
{
	outBuffer[cursor] = byte;
	cursor++;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint32_t err = huart->ErrorCode;
	UNUSED(err);
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) // peripheral error
{
	uint32_t err = huart->ErrorCode;
//	HAL_UART_Transmit(&huart2, sl_frame, sl_frame_len, 100);
//	NVIC_SystemReset();
//	HAL_UART_Receive_DMA(huart, uart_buffers[activeBuffer].data, UART_RX_BUFFER_SIZE); //ignore and continue to receive data
	UNUSED(err);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart) //new command received on UART
{
//	activeBuffer = !activeBuffer; //swap rx buffers
	rxFullFlag = 1; //indicate that data is ready to be parsed
//	_state = DataReceived;
	rxTimestamp = HAL_GetTick();
}
