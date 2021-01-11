#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_can.h"

#define		CAN_WAIT_TIME	500

typedef struct
{
  uint32_t uiHead;
  uint32_t uiTail;
  uint8_t abyBuffer[1024];
}stRing_Buf;

extern void fnCan_Init(void);
extern int fnCan_Check(void);
extern void fnCan_Deinit(void);
extern int fnCan_TestTx(void);
extern int fnCan_Transmit(uint8_t* pData);

extern CAN_HandleTypeDef    CanHandle;
extern uint32_t CanCheckTimer;
