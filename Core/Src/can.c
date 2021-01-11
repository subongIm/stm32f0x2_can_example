#include <string.h>
#include <stdlib.h>
#include "can.h"
#include "main.h"

#define TEST_CAN_TX 1

int flag;
CAN_HandleTypeDef    		CanHandle;
uint32_t TxMailBox;
CAN_TxHeaderTypeDef        TxMessage;
CAN_RxHeaderTypeDef        RxMessage;

//void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
void CAN_Pin_Init(void)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE(); 
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_REMAP_PIN_ENABLE(HAL_REMAP_PA11_PA12);
	
	//CAN RX,TX PIN Config.  
	GPIO_InitStruct.Pin = GPIO_PIN_12| GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Alternate =  GPIO_AF4_CAN;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
	
	//CAN transceiver STB PIN Config  
	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);   
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);   
}

void fnCan_InitCan(void)
{ 
	CAN_FilterTypeDef  sFilterConfig;
	uint32_t uiFuel_ID;
	uiFuel_ID = 0;
	
	CanHandle.Instance = CAN;
	CanHandle.Init.TimeTriggeredMode = DISABLE;                 //Time triggered communication mode
	CanHandle.Init.AutoBusOff = DISABLE;                //Automatic bus-off management
	CanHandle.Init.AutoWakeUp = DISABLE;                //Automatic wakeup mode
	CanHandle.Init.AutoRetransmission = DISABLE;                //No automatic retransmission
	CanHandle.Init.ReceiveFifoLocked = DISABLE;                //Receive FIFO locked mode
	CanHandle.Init.TransmitFifoPriority = DISABLE;                //Transmit FIFO priority
	CanHandle.Init.Mode = CAN_MODE_NORMAL;
	CanHandle.Init.SyncJumpWidth = CAN_SJW_1TQ;             //??????
	
	//CAN 500khz
	CanHandle.Init.TimeSeg1 = CAN_BS1_4TQ;             //Sample point
	CanHandle.Init.TimeSeg2 = CAN_BS2_3TQ;             //Transmit point
	CanHandle.Init.Prescaler = 12;             
	if (HAL_CAN_Init(&CanHandle) != HAL_OK)
	{ 
		/* Initialization Error */
		Error_Handler();
	}
	
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = (uiFuel_ID << 3) >> 16;
	sFilterConfig.FilterIdLow = (0xFFFF & (uiFuel_ID << 3));
	sFilterConfig.FilterMaskIdHigh = (uiFuel_ID << 3) >> 16;
	sFilterConfig.FilterMaskIdLow = (0xFFFF & (uiFuel_ID << 3));
	sFilterConfig.FilterFIFOAssignment = 0;
	sFilterConfig.FilterActivation = ENABLE;
	if (HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

void fnCan_Init(void)
{
	CAN_Pin_Init();
	
	fnCan_InitCan();
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);  
	
	HAL_Delay(50);
	
	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn); 
	
	HAL_CAN_ActivateNotification(&CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING);	// fifo0 interrupt check
		
	HAL_CAN_Start(&CanHandle);
}

void fnCan_Deinit(void)
{
	HAL_NVIC_DisableIRQ(CEC_CAN_IRQn); 
	
	HAL_CAN_DeInit(&CanHandle);
	__HAL_RCC_CAN1_CLK_DISABLE();
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{ 
	uint8_t	CANRxData[8] = {0, };
	if(hcan->Instance == CAN)
	{
		HAL_CAN_GetRxMessage(&CanHandle, CAN_RX_FIFO0, &RxMessage, &CANRxData[0]);
		flag = 1;
	}
}

int fnCan_TestTx(void)
{
	uint32_t EXTID = 0x1C5071E4;
	uint8_t	CANData[8] = {0,1,2,3,4,5,6,7};
	
	TxMessage.ExtId = EXTID;
	TxMessage.IDE = CAN_ID_EXT;
	TxMessage.DLC = 8;
	TxMessage.RTR = CAN_RTR_DATA;

	TxMailBox = HAL_CAN_GetTxMailboxesFreeLevel(&CanHandle);
	HAL_CAN_AddTxMessage(&CanHandle, &TxMessage, &CANData[0], &TxMailBox);
	
	return 0;
}
