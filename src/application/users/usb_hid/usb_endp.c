/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "common.h"

#include "server_comm_control.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
	g_bUsbRecvReady = 1;
	CommControl_RecvThread();

  /* Read received data (2 bytes) */  
  /*   USB_ReceiveFlg = TRUE;
     PMAToUserBufferCopy(Receive_Buffer, ENDP1_RXADDR, nUSBReportCnt);
     SetEPRxStatus(ENDP1, EP_RX_VALID);	*/
 
}

void EP2_IN_Callback(void)
{
	if (g_bUsbSendReady == 1) {
		if (GetEPTxStatus(ENDP2) == EP_TX_NAK) //每次发送前应使用GetEPTxStatus(ENDP1)检测上次发送是否完成。如果端点状态处于EP_TX_VALID，说明发送未结束，如果端点状态处于EP_TX_NAK，说明发送结束。
		{
			USB_SIL_Write(EP2_IN, g_usbSendBuf, USB_BUF_SIZE);  // 数据要复制到USB发送缓冲区中
			SetEPTxStatus(ENDP2, EP_TX_VALID);
		}
		g_bUsbSendReady = 0;
	}

     /*u8 ii;
     for (ii=0; ii<nUSBReportCnt; ii++) Transi_Buffer[ii] = 0x00;
     //for LED test
     if (GPIOA->ODR & 0x0c )  GPIOA->ODR &= (~0x0c);
     else GPIOA->ODR |= 0x0c;*/
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

