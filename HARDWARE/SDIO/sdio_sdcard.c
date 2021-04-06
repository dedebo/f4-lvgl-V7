#include "sdio_sdcard.h"
#include "string.h"	 
#include "sys.h"	 
#include "usart.h"	 
//////////////////////////////////////////////////////////////////////////////////	 
//±¾³ÌĞòÖ»¹©Ñ§Ï°Ê¹ÓÃ£¬Î´¾­×÷ÕßĞí¿É£¬²»µÃÓÃÓÚÆäËüÈÎºÎÓÃÍ¾
//ALIENTEK STM32F407¿ª·¢°å
//SDIO Çı¶¯´úÂë	   
//ÕıµãÔ­×Ó@ALIENTEK
//¼¼ÊõÂÛÌ³:www.openedv.com
//´´½¨ÈÕÆÚ:2014/5/14
//°æ±¾£ºV1.2
//°æÈ¨ËùÓĞ£¬µÁ°æ±Ø¾¿¡£
//Copyright(C) ¹ãÖİÊĞĞÇÒíµç×Ó¿Æ¼¼ÓĞÏŞ¹«Ë¾ 2014-2024
//All rights reserved		
//********************************************************************************
//ĞŞ¸ÄËµÃ÷
//V1.1	20140522
//1,¼ÓÈë³¬Ê±ÅĞ¶Ï,½â¾öÂÖÑ¯½ÓÊÕËÀ»úµÄÎÊÌâ.
//V1.2 	20140715
//1,ĞÂÔöSD_GetStateºÍSD_SendStatusº¯Êı.
////////////////////////////////////////////////////////////////////////////////// 	 

/*ÓÃÓÚsdio³õÊ¼»¯µÄ½á¹¹Ìå*/
SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;   

SD_Error CmdError(void);  
SD_Error CmdResp7Error(void);
SD_Error CmdResp1Error(u8 cmd);
SD_Error CmdResp3Error(void);
SD_Error CmdResp2Error(void);
SD_Error CmdResp6Error(u8 cmd,u16*prca);  
SD_Error SDEnWideBus(u8 enx);	  
SD_Error IsCardProgramming(u8 *pstatus); 
SD_Error FindSCR(u16 rca,u32 *pscr);
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes); 


static u8 CardType=SDIO_STD_CAPACITY_SD_CARD_V1_1;		//SD¿¨ÀàĞÍ£¨Ä¬ÈÏÎª1.x¿¨£©
static u32 CSD_Tab[4],CID_Tab[4],RCA=0;					//SD¿¨CSD,CIDÒÔ¼°Ïà¶ÔµØÖ·(RCA)Êı¾İ
static u8 DeviceMode=SD_DMA_MODE;		   				//¹¤×÷Ä£Ê½,×¢Òâ,¹¤×÷Ä£Ê½±ØĞëÍ¨¹ıSD_SetDeviceMode,ºó²ÅËãÊı.ÕâÀïÖ»ÊÇ¶¨ÒåÒ»¸öÄ¬ÈÏµÄÄ£Ê½(SD_DMA_MODE)
static u8 StopCondition=0; 								//ÊÇ·ñ·¢ËÍÍ£Ö¹´«Êä±êÖ¾Î»,DMA¶à¿é¶ÁĞ´µÄÊ±ºòÓÃµ½  
volatile SD_Error TransferError=SD_OK;					//Êı¾İ´«Êä´íÎó±êÖ¾,DMA¶ÁĞ´Ê±Ê¹ÓÃ	    
volatile u8 TransferEnd=0;								//´«Êä½áÊø±êÖ¾,DMA¶ÁĞ´Ê±Ê¹ÓÃ
SD_CardInfo SDCardInfo;									//SD¿¨ĞÅÏ¢

//SD_ReadDisk/SD_WriteDiskº¯Êı×¨ÓÃbuf,µ±ÕâÁ½¸öº¯ÊıµÄÊı¾İ»º´æÇøµØÖ·²»ÊÇ4×Ö½Ú¶ÔÆëµÄÊ±ºò,
//ĞèÒªÓÃµ½¸ÃÊı×é,È·±£Êı¾İ»º´æÇøµØÖ·ÊÇ4×Ö½Ú¶ÔÆëµÄ.
__align(4) u8 SDIO_DATA_BUFFER[512];						  
 
 
void SDIO_Register_Deinit()
{
	SDIO->POWER=0x00000000;
	SDIO->CLKCR=0x00000000;
	SDIO->ARG=0x00000000;
	SDIO->CMD=0x00000000;
	SDIO->DTIMER=0x00000000;
	SDIO->DLEN=0x00000000;
	SDIO->DCTRL=0x00000000;
	SDIO->ICR=0x00C007FF;
	SDIO->MASK=0x00000000;	 
}

//åˆå§‹åŒ–SD
//è¿”å›å€¼ ï¼šé”™è¯¯ä»£ç ï¼ˆè¿”å›0,æ— é”™è¯¯ï¼‰
SD_Error SD_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	SD_Error errorstatus=SD_OK;	 
  u8 clkdiv=0;
	
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_DMA2, ENABLE);//Ê¹ÄÜGPIOC,GPIOD DMA2Ê±ÖÓ
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIOÊ±ÖÓÊ¹ÄÜ
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE);//SDIO¸´Î»
	
	
  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12; 	//PC8,9,10,11,12¸´ÓÃ¹¦ÄÜÊä³ö	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//¸´ÓÃ¹¦ÄÜ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//100M
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//ÉÏÀ­
  GPIO_Init(GPIOC, &GPIO_InitStructure);// PC8,9,10,11,12¸´ÓÃ¹¦ÄÜÊä³ö

	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;
  GPIO_Init(GPIOD, &GPIO_InitStructure);//PD2¸´ÓÃ¹¦ÄÜÊä³ö
	
	 //Òı½Å¸´ÓÃÓ³ÉäÉèÖÃ
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_SDIO); //PC8,AF12
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_SDIO);
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SDIO);
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SDIO);
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SDIO);	
  GPIO_PinAFConfig(GPIOD,GPIO_PinSource2,GPIO_AF_SDIO);	
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE);//SDIO½áÊø¸´Î»
		
 	//SDIOÍâÉè¼Ä´æÆ÷ÉèÖÃÎªÄ¬ÈÏÖµ 			   
	SDIO_Register_Deinit();
	
  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//ÇÀÕ¼ÓÅÏÈ¼¶3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//×ÓÓÅÏÈ¼¶3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQÍ¨µÀÊ¹ÄÜ
	NVIC_Init(&NVIC_InitStructure);	//¸ù¾İÖ¸¶¨µÄ²ÎÊı³õÊ¼»¯VIC¼Ä´æÆ÷¡¢
	
   	errorstatus=SD_PowerON();			//SD¿¨ÉÏµç
 	if(errorstatus==SD_OK)errorstatus=SD_InitializeCards();			//³õÊ¼»¯SD¿¨														  
  	if(errorstatus==SD_OK)errorstatus=SD_GetCardInfo(&SDCardInfo);	//»ñÈ¡¿¨ĞÅÏ¢
 	if(errorstatus==SD_OK)errorstatus=SD_SelectDeselect((u32)(SDCardInfo.RCA<<16));//Ñ¡ÖĞSD¿¨   
   	if(errorstatus==SD_OK)errorstatus=SD_EnableWideBusOperation(SDIO_BusWide_4b);	//4Î»¿í¶È,Èç¹ûÊÇMMC¿¨,Ôò²»ÄÜÓÃ4Î»Ä£Ê½ 
  	if((errorstatus==SD_OK)||(SDIO_MULTIMEDIA_CARD==CardType))
	{  		    
		if(SDCardInfo.CardType==SDIO_STD_CAPACITY_SD_CARD_V1_1||SDCardInfo.CardType==SDIO_STD_CAPACITY_SD_CARD_V2_0)
		{
			clkdiv=SDIO_TRANSFER_CLK_DIV+2;	//V1.1/V2.0¿¨£¬ÉèÖÃ×î¸ß48/4=12Mhz
		}else clkdiv=SDIO_TRANSFER_CLK_DIV;	//SDHCµÈÆäËû¿¨£¬ÉèÖÃ×î¸ß48/2=24Mhz
		SDIO_Clock_Set(clkdiv);	//ÉèÖÃÊ±ÖÓÆµÂÊ,SDIOÊ±ÖÓ¼ÆËã¹«Ê½:SDIO_CKÊ±ÖÓ=SDIOCLK/[clkdiv+2];ÆäÖĞ,SDIOCLK¹Ì¶¨Îª48Mhz 
		//errorstatus=SD_SetDeviceMode(SD_DMA_MODE);	//ÉèÖÃÎªDMAÄ£Ê½
		errorstatus=SD_SetDeviceMode(SD_POLLING_MODE);//ÉèÖÃÎª²éÑ¯Ä£Ê½
 	}
	return errorstatus;		 
}
//SDIOÊ±ÖÓ³õÊ¼»¯ÉèÖÃ
//clkdiv:Ê±ÖÓ·ÖÆµÏµÊı
//CKÊ±ÖÓ=SDIOCLK/[clkdiv+2];(SDIOCLKÊ±ÖÓ¹Ì¶¨Îª48Mhz)
void SDIO_Clock_Set(u8 clkdiv)
{
	u32 tmpreg=SDIO->CLKCR; 
  	tmpreg&=0XFFFFFF00; 
 	tmpreg|=clkdiv;   
	SDIO->CLKCR=tmpreg;
} 


//¿¨ÉÏµç
//²éÑ¯ËùÓĞSDIO½Ó¿ÚÉÏµÄ¿¨Éè±¸,²¢²éÑ¯ÆäµçÑ¹ºÍÅäÖÃÊ±ÖÓ
//·µ»ØÖµ:´íÎó´úÂë;(0,ÎŞ´íÎó)
SD_Error SD_PowerON(void)
{
 	u8 i=0;
	SD_Error errorstatus=SD_OK;
	u32 response=0,count=0,validvoltage=0;
	u32 SDType=SD_STD_CAPACITY;
	
	 /*³õÊ¼»¯Ê±µÄÊ±ÖÓ²»ÄÜ´óÓÚ400KHz*/ 
  SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;	/* HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz */
  SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
  SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;  //²»Ê¹ÓÃbypassÄ£Ê½£¬Ö±½ÓÓÃHCLK½øĞĞ·ÖÆµµÃµ½SDIO_CK
  SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;	// ¿ÕÏĞÊ±²»¹Ø±ÕÊ±ÖÓµçÔ´
  SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;	 				//1Î»Êı¾İÏß
  SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;//Ó²¼şÁ÷
  SDIO_Init(&SDIO_InitStructure);

	SDIO_SetPowerState(SDIO_PowerState_ON);	//ÉÏµç×´Ì¬,¿ªÆô¿¨Ê±ÖÓ   
  SDIO->CLKCR|=1<<8;			//SDIOCKÊ¹ÄÜ  
 
 	for(i=0;i<74;i++)
	{
 
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;//·¢ËÍCMD0½øÈëIDLE STAGEÄ£Ê½ÃüÁî.
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE; //cmd0
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;  //ÎŞÏìÓ¦
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;  //ÔòCPSMÔÚ¿ªÊ¼·¢ËÍÃüÁîÖ®Ç°µÈ´ıÊı¾İ´«Êä½áÊø¡£ 
    SDIO_SendCommand(&SDIO_CmdInitStructure);	  		//Ğ´ÃüÁî½øÃüÁî¼Ä´æÆ÷
		
		errorstatus=CmdError();
		
		if(errorstatus==SD_OK)break;
 	}
 	if(errorstatus)return errorstatus;//·µ»Ø´íÎó×´Ì¬
	
  SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;	//·¢ËÍCMD8,¶ÌÏìÓ¦,¼ì²éSD¿¨½Ó¿ÚÌØĞÔ
  SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;	//cmd8
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;	 //r7
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;			 //¹Ø±ÕµÈ´ıÖĞ¶Ï
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);
	
  errorstatus=CmdResp7Error();						//µÈ´ıR7ÏìÓ¦
	
 	if(errorstatus==SD_OK) 								//R7ÏìÓ¦Õı³£
	{
		CardType=SDIO_STD_CAPACITY_SD_CARD_V2_0;		//SD 2.0¿¨
		SDType=SD_HIGH_CAPACITY;			   			//¸ßÈİÁ¿¿¨
	}
	  
	  SDIO_CmdInitStructure.SDIO_Argument = 0x00;//·¢ËÍCMD55,¶ÌÏìÓ¦	
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);		//·¢ËÍCMD55,¶ÌÏìÓ¦	 
	
	 errorstatus=CmdResp1Error(SD_CMD_APP_CMD); 		 	//µÈ´ıR1ÏìÓ¦   
	
	if(errorstatus==SD_OK)//SD2.0/SD 1.1,·ñÔòÎªMMC¿¨
	{																  
		//SD¿¨,·¢ËÍACMD41 SD_APP_OP_COND,²ÎÊıÎª:0x80100000 
		while((!validvoltage)&&(count<SD_MAX_VOLT_TRIAL))
		{	   										   
		  SDIO_CmdInitStructure.SDIO_Argument = 0x00;//·¢ËÍCMD55,¶ÌÏìÓ¦
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;	  //CMD55
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);			//·¢ËÍCMD55,¶ÌÏìÓ¦	 
			
			errorstatus=CmdResp1Error(SD_CMD_APP_CMD); 	 	//µÈ´ıR1ÏìÓ¦  
			
 			if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó

      //acmd41£¬ÃüÁî²ÎÊıÓÉÖ§³ÖµÄµçÑ¹·¶Î§¼°HCSÎ»×é³É£¬HCSÎ»ÖÃÒ»À´Çø·Ö¿¨ÊÇSDSc»¹ÊÇsdhc
      SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;	//·¢ËÍACMD41,¶ÌÏìÓ¦	
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r3
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus=CmdResp3Error(); 					//µÈ´ıR3ÏìÓ¦   
			
 			if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó 
			response=SDIO->RESP1;;			   				//µÃµ½ÏìÓ¦
			validvoltage=(((response>>31)==1)?1:0);			//ÅĞ¶ÏSD¿¨ÉÏµçÊÇ·ñÍê³É
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL)
		{
			errorstatus=SD_INVALID_VOLTRANGE;
			return errorstatus;
		}	 
		if(response&=SD_HIGH_CAPACITY)
		{
			CardType=SDIO_HIGH_CAPACITY_SD_CARD;
		}
 	}else//MMC¿¨
	{
		//MMC¿¨,·¢ËÍCMD1 SDIO_SEND_OP_COND,²ÎÊıÎª:0x80FF8000 
		while((!validvoltage)&&(count<SD_MAX_VOLT_TRIAL))
		{	   										   				   
			SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_MMC;//·¢ËÍCMD1,¶ÌÏìÓ¦	   
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r3
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus=CmdResp3Error(); 					//µÈ´ıR3ÏìÓ¦   
			
 			if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó  
			response=SDIO->RESP1;;			   				//µÃµ½ÏìÓ¦
			validvoltage=(((response>>31)==1)?1:0);
			count++;
		}
		if(count>=SD_MAX_VOLT_TRIAL)
		{
			errorstatus=SD_INVALID_VOLTRANGE;
			return errorstatus;
		}	 			    
		CardType=SDIO_MULTIMEDIA_CARD;	  
  	}  
  	return(errorstatus);		
}
//SD¿¨ Power OFF
//·µ»ØÖµ:´íÎó´úÂë;(0,ÎŞ´íÎó)
SD_Error SD_PowerOFF(void)
{
 
  SDIO_SetPowerState(SDIO_PowerState_OFF);//SDIOµçÔ´¹Ø±Õ,Ê±ÖÓÍ£Ö¹	

  return SD_OK;	  
}   
//³õÊ¼»¯ËùÓĞµÄ¿¨,²¢ÈÃ¿¨½øÈë¾ÍĞ÷×´Ì¬
//·µ»ØÖµ:´íÎó´úÂë
SD_Error SD_InitializeCards(void)
{
 	SD_Error errorstatus=SD_OK;
	u16 rca = 0x01;
	
  if (SDIO_GetPowerState() == SDIO_PowerState_OFF)	//¼ì²éµçÔ´×´Ì¬,È·±£ÎªÉÏµç×´Ì¬
  {
    errorstatus = SD_REQUEST_NOT_APPLICABLE;
    return(errorstatus);
  }

 	if(SDIO_SECURE_DIGITAL_IO_CARD!=CardType)			//·ÇSECURE_DIGITAL_IO_CARD
	{
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;//·¢ËÍCMD2,È¡µÃCID,³¤ÏìÓ¦
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);//·¢ËÍCMD2,È¡µÃCID,³¤ÏìÓ¦	
		
		errorstatus=CmdResp2Error(); 					//µÈ´ıR2ÏìÓ¦ 
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó		 
		
 		CID_Tab[0]=SDIO->RESP1;
		CID_Tab[1]=SDIO->RESP2;
		CID_Tab[2]=SDIO->RESP3;
		CID_Tab[3]=SDIO->RESP4;
	}
	if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_SECURE_DIGITAL_IO_COMBO_CARD==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))//ÅĞ¶Ï¿¨ÀàĞÍ
	{
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;//·¢ËÍCMD3,¶ÌÏìÓ¦ 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;	//cmd3
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; //r6
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);	//·¢ËÍCMD3,¶ÌÏìÓ¦ 
		
		errorstatus=CmdResp6Error(SD_CMD_SET_REL_ADDR,&rca);//µÈ´ıR6ÏìÓ¦ 
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó		    
	}   
    if (SDIO_MULTIMEDIA_CARD==CardType)
    {

		  SDIO_CmdInitStructure.SDIO_Argument = (u32)(rca<<16);//·¢ËÍCMD3,¶ÌÏìÓ¦ 
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;	//cmd3
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short; //r6
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);	//·¢ËÍCMD3,¶ÌÏìÓ¦ 	
			
      errorstatus=CmdResp2Error(); 					//µÈ´ıR2ÏìÓ¦   
			
		  if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó	 
    }
	if (SDIO_SECURE_DIGITAL_IO_CARD!=CardType)			//·ÇSECURE_DIGITAL_IO_CARD
	{
		RCA = rca;
		
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16);//·¢ËÍCMD9+¿¨RCA,È¡µÃCSD,³¤ÏìÓ¦ 
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus=CmdResp2Error(); 					//µÈ´ıR2ÏìÓ¦   
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó		    
  		
		CSD_Tab[0]=SDIO->RESP1;
	  CSD_Tab[1]=SDIO->RESP2;
		CSD_Tab[2]=SDIO->RESP3;						
		CSD_Tab[3]=SDIO->RESP4;					    
	}
	return SD_OK;//¿¨³õÊ¼»¯³É¹¦
} 
//µÃµ½¿¨ĞÅÏ¢
//cardinfo:¿¨ĞÅÏ¢´æ´¢Çø
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
 	SD_Error errorstatus=SD_OK;
	u8 tmp=0;	   
	cardinfo->CardType=(u8)CardType; 				//¿¨ÀàĞÍ
	cardinfo->RCA=(u16)RCA;							//¿¨RCAÖµ
	tmp=(u8)((CSD_Tab[0]&0xFF000000)>>24);
	cardinfo->SD_csd.CSDStruct=(tmp&0xC0)>>6;		//CSD½á¹¹
	cardinfo->SD_csd.SysSpecVersion=(tmp&0x3C)>>2;	//2.0Ğ­Òé»¹Ã»¶¨ÒåÕâ²¿·Ö(Îª±£Áô),Ó¦¸ÃÊÇºóĞøĞ­Òé¶¨ÒåµÄ
	cardinfo->SD_csd.Reserved1=tmp&0x03;			//2¸ö±£ÁôÎ»  
	tmp=(u8)((CSD_Tab[0]&0x00FF0000)>>16);			//µÚ1¸ö×Ö½Ú
	cardinfo->SD_csd.TAAC=tmp;				   		//Êı¾İ¶ÁÊ±¼ä1
	tmp=(u8)((CSD_Tab[0]&0x0000FF00)>>8);	  		//µÚ2¸ö×Ö½Ú
	cardinfo->SD_csd.NSAC=tmp;		  				//Êı¾İ¶ÁÊ±¼ä2
	tmp=(u8)(CSD_Tab[0]&0x000000FF);				//µÚ3¸ö×Ö½Ú
	cardinfo->SD_csd.MaxBusClkFrec=tmp;		  		//´«ÊäËÙ¶È	   
	tmp=(u8)((CSD_Tab[1]&0xFF000000)>>24);			//µÚ4¸ö×Ö½Ú
	cardinfo->SD_csd.CardComdClasses=tmp<<4;    	//¿¨Ö¸ÁîÀà¸ßËÄÎ»
	tmp=(u8)((CSD_Tab[1]&0x00FF0000)>>16);	 		//µÚ5¸ö×Ö½Ú
	cardinfo->SD_csd.CardComdClasses|=(tmp&0xF0)>>4;//¿¨Ö¸ÁîÀàµÍËÄÎ»
	cardinfo->SD_csd.RdBlockLen=tmp&0x0F;	    	//×î´ó¶ÁÈ¡Êı¾İ³¤¶È
	tmp=(u8)((CSD_Tab[1]&0x0000FF00)>>8);			//µÚ6¸ö×Ö½Ú
	cardinfo->SD_csd.PartBlockRead=(tmp&0x80)>>7;	//ÔÊĞí·Ö¿é¶Á
	cardinfo->SD_csd.WrBlockMisalign=(tmp&0x40)>>6;	//Ğ´¿é´íÎ»
	cardinfo->SD_csd.RdBlockMisalign=(tmp&0x20)>>5;	//¶Á¿é´íÎ»
	cardinfo->SD_csd.DSRImpl=(tmp&0x10)>>4;
	cardinfo->SD_csd.Reserved2=0; 					//±£Áô
 	if((CardType==SDIO_STD_CAPACITY_SD_CARD_V1_1)||(CardType==SDIO_STD_CAPACITY_SD_CARD_V2_0)||(SDIO_MULTIMEDIA_CARD==CardType))//±ê×¼1.1/2.0¿¨/MMC¿¨
	{
		cardinfo->SD_csd.DeviceSize=(tmp&0x03)<<10;	//C_SIZE(12Î»)
	 	tmp=(u8)(CSD_Tab[1]&0x000000FF); 			//µÚ7¸ö×Ö½Ú	
		cardinfo->SD_csd.DeviceSize|=(tmp)<<2;
 		tmp=(u8)((CSD_Tab[2]&0xFF000000)>>24);		//µÚ8¸ö×Ö½Ú	
		cardinfo->SD_csd.DeviceSize|=(tmp&0xC0)>>6;
 		cardinfo->SD_csd.MaxRdCurrentVDDMin=(tmp&0x38)>>3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax=(tmp&0x07);
 		tmp=(u8)((CSD_Tab[2]&0x00FF0000)>>16);		//µÚ9¸ö×Ö½Ú	
		cardinfo->SD_csd.MaxWrCurrentVDDMin=(tmp&0xE0)>>5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax=(tmp&0x1C)>>2;
		cardinfo->SD_csd.DeviceSizeMul=(tmp&0x03)<<1;//C_SIZE_MULT
 		tmp=(u8)((CSD_Tab[2]&0x0000FF00)>>8);	  	//µÚ10¸ö×Ö½Ú	
		cardinfo->SD_csd.DeviceSizeMul|=(tmp&0x80)>>7;
 		cardinfo->CardCapacity=(cardinfo->SD_csd.DeviceSize+1);//¼ÆËã¿¨ÈİÁ¿
		cardinfo->CardCapacity*=(1<<(cardinfo->SD_csd.DeviceSizeMul+2));
		cardinfo->CardBlockSize=1<<(cardinfo->SD_csd.RdBlockLen);//¿é´óĞ¡
		cardinfo->CardCapacity*=cardinfo->CardBlockSize;
	}else if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)	//¸ßÈİÁ¿¿¨
	{
 		tmp=(u8)(CSD_Tab[1]&0x000000FF); 		//µÚ7¸ö×Ö½Ú	
		cardinfo->SD_csd.DeviceSize=(tmp&0x3F)<<16;//C_SIZE
 		tmp=(u8)((CSD_Tab[2]&0xFF000000)>>24); 	//µÚ8¸ö×Ö½Ú	
 		cardinfo->SD_csd.DeviceSize|=(tmp<<8);
 		tmp=(u8)((CSD_Tab[2]&0x00FF0000)>>16);	//µÚ9¸ö×Ö½Ú	
 		cardinfo->SD_csd.DeviceSize|=(tmp);
 		tmp=(u8)((CSD_Tab[2]&0x0000FF00)>>8); 	//µÚ10¸ö×Ö½Ú	
 		cardinfo->CardCapacity=(long long)(cardinfo->SD_csd.DeviceSize+1)*512*1024;//¼ÆËã¿¨ÈİÁ¿
		cardinfo->CardBlockSize=512; 			//¿é´óĞ¡¹Ì¶¨Îª512×Ö½Ú
	}	  
	cardinfo->SD_csd.EraseGrSize=(tmp&0x40)>>6;
	cardinfo->SD_csd.EraseGrMul=(tmp&0x3F)<<1;	   
	tmp=(u8)(CSD_Tab[2]&0x000000FF);			//µÚ11¸ö×Ö½Ú	
	cardinfo->SD_csd.EraseGrMul|=(tmp&0x80)>>7;
	cardinfo->SD_csd.WrProtectGrSize=(tmp&0x7F);
 	tmp=(u8)((CSD_Tab[3]&0xFF000000)>>24);		//µÚ12¸ö×Ö½Ú	
	cardinfo->SD_csd.WrProtectGrEnable=(tmp&0x80)>>7;
	cardinfo->SD_csd.ManDeflECC=(tmp&0x60)>>5;
	cardinfo->SD_csd.WrSpeedFact=(tmp&0x1C)>>2;
	cardinfo->SD_csd.MaxWrBlockLen=(tmp&0x03)<<2;	 
	tmp=(u8)((CSD_Tab[3]&0x00FF0000)>>16);		//µÚ13¸ö×Ö½Ú
	cardinfo->SD_csd.MaxWrBlockLen|=(tmp&0xC0)>>6;
	cardinfo->SD_csd.WriteBlockPaPartial=(tmp&0x20)>>5;
	cardinfo->SD_csd.Reserved3=0;
	cardinfo->SD_csd.ContentProtectAppli=(tmp&0x01);  
	tmp=(u8)((CSD_Tab[3]&0x0000FF00)>>8);		//µÚ14¸ö×Ö½Ú
	cardinfo->SD_csd.FileFormatGrouop=(tmp&0x80)>>7;
	cardinfo->SD_csd.CopyFlag=(tmp&0x40)>>6;
	cardinfo->SD_csd.PermWrProtect=(tmp&0x20)>>5;
	cardinfo->SD_csd.TempWrProtect=(tmp&0x10)>>4;
	cardinfo->SD_csd.FileFormat=(tmp&0x0C)>>2;
	cardinfo->SD_csd.ECC=(tmp&0x03);  
	tmp=(u8)(CSD_Tab[3]&0x000000FF);			//µÚ15¸ö×Ö½Ú
	cardinfo->SD_csd.CSD_CRC=(tmp&0xFE)>>1;
	cardinfo->SD_csd.Reserved4=1;		 
	tmp=(u8)((CID_Tab[0]&0xFF000000)>>24);		//µÚ0¸ö×Ö½Ú
	cardinfo->SD_cid.ManufacturerID=tmp;		    
	tmp=(u8)((CID_Tab[0]&0x00FF0000)>>16);		//µÚ1¸ö×Ö½Ú
	cardinfo->SD_cid.OEM_AppliID=tmp<<8;	  
	tmp=(u8)((CID_Tab[0]&0x000000FF00)>>8);		//µÚ2¸ö×Ö½Ú
	cardinfo->SD_cid.OEM_AppliID|=tmp;	    
	tmp=(u8)(CID_Tab[0]&0x000000FF);			//µÚ3¸ö×Ö½Ú	
	cardinfo->SD_cid.ProdName1=tmp<<24;				  
	tmp=(u8)((CID_Tab[1]&0xFF000000)>>24); 		//µÚ4¸ö×Ö½Ú
	cardinfo->SD_cid.ProdName1|=tmp<<16;	  
	tmp=(u8)((CID_Tab[1]&0x00FF0000)>>16);	   	//µÚ5¸ö×Ö½Ú
	cardinfo->SD_cid.ProdName1|=tmp<<8;		 
	tmp=(u8)((CID_Tab[1]&0x0000FF00)>>8);		//µÚ6¸ö×Ö½Ú
	cardinfo->SD_cid.ProdName1|=tmp;		   
	tmp=(u8)(CID_Tab[1]&0x000000FF);	  		//µÚ7¸ö×Ö½Ú
	cardinfo->SD_cid.ProdName2=tmp;			  
	tmp=(u8)((CID_Tab[2]&0xFF000000)>>24); 		//µÚ8¸ö×Ö½Ú
	cardinfo->SD_cid.ProdRev=tmp;		 
	tmp=(u8)((CID_Tab[2]&0x00FF0000)>>16);		//µÚ9¸ö×Ö½Ú
	cardinfo->SD_cid.ProdSN=tmp<<24;	   
	tmp=(u8)((CID_Tab[2]&0x0000FF00)>>8); 		//µÚ10¸ö×Ö½Ú
	cardinfo->SD_cid.ProdSN|=tmp<<16;	   
	tmp=(u8)(CID_Tab[2]&0x000000FF);   			//µÚ11¸ö×Ö½Ú
	cardinfo->SD_cid.ProdSN|=tmp<<8;		   
	tmp=(u8)((CID_Tab[3]&0xFF000000)>>24); 		//µÚ12¸ö×Ö½Ú
	cardinfo->SD_cid.ProdSN|=tmp;			     
	tmp=(u8)((CID_Tab[3]&0x00FF0000)>>16);	 	//µÚ13¸ö×Ö½Ú
	cardinfo->SD_cid.Reserved1|=(tmp&0xF0)>>4;
	cardinfo->SD_cid.ManufactDate=(tmp&0x0F)<<8;    
	tmp=(u8)((CID_Tab[3]&0x0000FF00)>>8);		//µÚ14¸ö×Ö½Ú
	cardinfo->SD_cid.ManufactDate|=tmp;		 	  
	tmp=(u8)(CID_Tab[3]&0x000000FF);			//µÚ15¸ö×Ö½Ú
	cardinfo->SD_cid.CID_CRC=(tmp&0xFE)>>1;
	cardinfo->SD_cid.Reserved2=1;	 
	return errorstatus;
}
//ÉèÖÃSDIO×ÜÏß¿í¶È(MMC¿¨²»Ö§³Ö4bitÄ£Ê½)
//wmode:Î»¿íÄ£Ê½.0,1Î»Êı¾İ¿í¶È;1,4Î»Êı¾İ¿í¶È;2,8Î»Êı¾İ¿í¶È
//·µ»ØÖµ:SD¿¨´íÎó×´Ì¬

//ÉèÖÃSDIO×ÜÏß¿í¶È(MMC¿¨²»Ö§³Ö4bitÄ£Ê½)
//   @arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
//   @arg SDIO_BusWide_4b: 4-bit data transfer
//   @arg SDIO_BusWide_1b: 1-bit data transfer (Ä¬ÈÏ)
//·µ»ØÖµ:SD¿¨´íÎó×´Ì¬


SD_Error SD_EnableWideBusOperation(u32 WideMode)
{
  	SD_Error errorstatus=SD_OK;
  if (SDIO_MULTIMEDIA_CARD == CardType)
  {
    errorstatus = SD_UNSUPPORTED_FEATURE;
    return(errorstatus);
  }
	
 	else if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
	{
		 if (SDIO_BusWide_8b == WideMode)   //2.0 sd²»Ö§³Ö8bits
    {
      errorstatus = SD_UNSUPPORTED_FEATURE;
      return(errorstatus);
    }
 		else   
		{
			errorstatus=SDEnWideBus(WideMode);
 			if(SD_OK==errorstatus)
			{
				SDIO->CLKCR&=~(3<<11);		//Çå³ıÖ®Ç°µÄÎ»¿íÉèÖÃ    
				SDIO->CLKCR|=WideMode;//1Î»/4Î»×ÜÏß¿í¶È 
				SDIO->CLKCR|=0<<14;			//²»¿ªÆôÓ²¼şÁ÷¿ØÖÆ
			}
		}  
	}
	return errorstatus; 
}
//ÉèÖÃSD¿¨¹¤×÷Ä£Ê½
//Mode:
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error SD_SetDeviceMode(u32 Mode)
{
	SD_Error errorstatus = SD_OK;
 	if((Mode==SD_DMA_MODE)||(Mode==SD_POLLING_MODE))DeviceMode=Mode;
	else errorstatus=SD_INVALID_PARAMETER;
	return errorstatus;	    
}
//Ñ¡¿¨
//·¢ËÍCMD7,Ñ¡ÔñÏà¶ÔµØÖ·(rca)ÎªaddrµÄ¿¨,È¡ÏûÆäËû¿¨.Èç¹ûÎª0,Ôò¶¼²»Ñ¡Ôñ.
//addr:¿¨µÄRCAµØÖ·
SD_Error SD_SelectDeselect(u32 addr)
{

  SDIO_CmdInitStructure.SDIO_Argument =  addr;//·¢ËÍCMD7,Ñ¡Ôñ¿¨,¶ÌÏìÓ¦	
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);//·¢ËÍCMD7,Ñ¡Ôñ¿¨,¶ÌÏìÓ¦
	
 	return CmdResp1Error(SD_CMD_SEL_DESEL_CARD);	  
}
//SD¿¨¶ÁÈ¡Ò»¸ö¿é 
//buf:¶ÁÊı¾İ»º´æÇø(±ØĞë4×Ö½Ú¶ÔÆë!!)
//addr:¶ÁÈ¡µØÖ·
//blksize:¿é´óĞ¡
SD_Error SD_ReadBlock(u8 *buf,long long addr,u16 blksize)
{	  
	SD_Error errorstatus=SD_OK;
	u8 power;
  u32 count=0,*tempbuff=(u32*)buf;//×ª»»Îªu32Ö¸Õë 
	u32 timeout=SDIO_DATATIMEOUT;   
  if(NULL==buf)
		return SD_INVALID_PARAMETER; 
  SDIO->DCTRL=0x0;	//Êı¾İ¿ØÖÆ¼Ä´æÆ÷ÇåÁã(¹ØDMA) 
  
	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)//´óÈİÁ¿¿¨
	{
		blksize=512;
		addr>>=9;
	}   
  	SDIO_DataInitStructure.SDIO_DataBlockSize= SDIO_DataBlockSize_1b ;//Çå³ıDPSM×´Ì¬»úÅäÖÃ
	  SDIO_DataInitStructure.SDIO_DataLength= 0 ;
	  SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
	  SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
	  SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToCard;
	  SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
	
	
	if(SDIO->RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//¿¨ËøÁË
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0))
	{
		power=convert_from_bytes_to_power_of_two(blksize);	
		
   
		SDIO_CmdInitStructure.SDIO_Argument =  blksize;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);//·¢ËÍCMD16+ÉèÖÃÊı¾İ³¤¶ÈÎªblksize,¶ÌÏìÓ¦
		
		
		errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);	//µÈ´ıR1ÏìÓ¦ 
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó	
		
	}else return SD_INVALID_PARAMETER;	  	 
	
	  SDIO_DataInitStructure.SDIO_DataBlockSize= power<<4 ;//Çå³ıDPSM×´Ì¬»úÅäÖÃ
	  SDIO_DataInitStructure.SDIO_DataLength= blksize ;
	  SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
	  SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
	  SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToSDIO;
	  SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
	
	  SDIO_CmdInitStructure.SDIO_Argument =  addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);//·¢ËÍCMD17+´ÓaddrµØÖ·³ö¶ÁÈ¡Êı¾İ,¶ÌÏìÓ¦ 
	
	errorstatus=CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);//µÈ´ıR1ÏìÓ¦   
	if(errorstatus!=SD_OK)return errorstatus;   		//ÏìÓ¦´íÎó	 
 	if(DeviceMode==SD_POLLING_MODE)						//²éÑ¯Ä£Ê½,ÂÖÑ¯Êı¾İ	 
	{
 		INTX_DISABLE();//¹Ø±Õ×ÜÖĞ¶Ï(POLLINGÄ£Ê½,ÑÏ½ûÖĞ¶Ï´ò¶ÏSDIO¶ÁĞ´²Ù×÷!!!)
		while(!(SDIO->STA&((1<<5)|(1<<1)|(1<<3)|(1<<10)|(1<<9))))//ÎŞÉÏÒç/CRC/³¬Ê±/Íê³É(±êÖ¾)/ÆğÊ¼Î»´íÎó
		{
			if(SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)						//½ÓÊÕÇø°ëÂú,±íÊ¾ÖÁÉÙ´æÁË8¸ö×Ö
			{
				for(count=0;count<8;count++)			//Ñ­»·¶ÁÈ¡Êı¾İ
				{
					*(tempbuff+count)=SDIO->FIFO;
				}
				tempbuff+=8;	 
				timeout=0X7FFFFF; 	//¶ÁÊı¾İÒç³öÊ±¼ä
			}else 	//´¦Àí³¬Ê±
			{
				if(timeout==0)return SD_DATA_TIMEOUT;
				timeout--;
			}
		} 
		if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)		//Êı¾İ³¬Ê±´íÎó
		{										   
	 		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); 	//Çå´íÎó±êÖ¾
			return SD_DATA_TIMEOUT;
	 	}else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)	//Êı¾İ¿éCRC´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);  		//Çå´íÎó±êÖ¾
			return SD_DATA_CRC_FAIL;		   
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) 	//½ÓÊÕfifoÉÏÒç´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);		//Çå´íÎó±êÖ¾
			return SD_RX_OVERRUN;		 
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) 	//½ÓÊÕÆğÊ¼Î»´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_STBITERR);//Çå´íÎó±êÖ¾
			return SD_START_BIT_ERR;		 
		}   
		while(SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)	//FIFOÀïÃæ,»¹´æÔÚ¿ÉÓÃÊı¾İ
		{
			*tempbuff=SDIO->FIFO;	//Ñ­»·¶ÁÈ¡Êı¾İ
			tempbuff++;
		}
		INTX_ENABLE();//¿ªÆô×ÜÖĞ¶Ï
		SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	 
	}else if(DeviceMode==SD_DMA_MODE)
	{
 		TransferError=SD_OK;
		StopCondition=0;			//µ¥¿é¶Á,²»ĞèÒª·¢ËÍÍ£Ö¹´«ÊäÖ¸Áî
		TransferEnd=0;				//´«Êä½áÊø±êÖÃÎ»£¬ÔÚÖĞ¶Ï·şÎñÖÃ1
		SDIO->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<5)|(1<<9);	//ÅäÖÃĞèÒªµÄÖĞ¶Ï 
	 	SDIO->DCTRL|=1<<3;		 	//SDIO DMAÊ¹ÄÜ 
 	    SD_DMA_Config((u32*)buf,blksize,DMA_DIR_PeripheralToMemory); 
 		while(((DMA2->LISR&(1<<27))==RESET)&&(TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;//µÈ´ı´«ÊäÍê³É 
		if(timeout==0)return SD_DATA_TIMEOUT;//³¬Ê±
		if(TransferError!=SD_OK)errorstatus=TransferError;  
    }   
 	return errorstatus; 
}
//SD¿¨¶ÁÈ¡¶à¸ö¿é 
//buf:¶ÁÊı¾İ»º´æÇø
//addr:¶ÁÈ¡µØÖ·
//blksize:¿é´óĞ¡
//nblks:Òª¶ÁÈ¡µÄ¿éÊı
//·µ»ØÖµ:´íÎó×´Ì¬
__align(4) u32 *tempbuff;
SD_Error SD_ReadMultiBlocks(u8 *buf,long long addr,u16 blksize,u32 nblks)
{
  SD_Error errorstatus=SD_OK;
	u8 power;
  u32 count=0;
	u32 timeout=SDIO_DATATIMEOUT;  
	tempbuff=(u32*)buf;//×ª»»Îªu32Ö¸Õë
	
  SDIO->DCTRL=0x0;		//Êı¾İ¿ØÖÆ¼Ä´æÆ÷ÇåÁã(¹ØDMA)   
	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)//´óÈİÁ¿¿¨
	{
		blksize=512;
		addr>>=9;
	}  
	
	  SDIO_DataInitStructure.SDIO_DataBlockSize= 0; ;//Çå³ıDPSM×´Ì¬»úÅäÖÃ
	  SDIO_DataInitStructure.SDIO_DataLength= 0 ;
	  SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
	  SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
	  SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToCard;
	  SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
	
	if(SDIO->RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//¿¨ËøÁË
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0))
	{
		power=convert_from_bytes_to_power_of_two(blksize);	    
		
	  SDIO_CmdInitStructure.SDIO_Argument =  blksize;//·¢ËÍCMD16+ÉèÖÃÊı¾İ³¤¶ÈÎªblksize,¶ÌÏìÓ¦ 
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);	//µÈ´ıR1ÏìÓ¦  
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó	 
		
	}else return SD_INVALID_PARAMETER;	  
	
	if(nblks>1)											//¶à¿é¶Á  
	{									    
 	  	if(nblks*blksize>SD_MAX_DATA_LENGTH)return SD_INVALID_PARAMETER;//ÅĞ¶ÏÊÇ·ñ³¬¹ı×î´ó½ÓÊÕ³¤¶È 
		
		   SDIO_DataInitStructure.SDIO_DataBlockSize= power<<4; ;//nblks*blksize,512¿é´óĞ¡,¿¨µ½¿ØÖÆÆ÷
			 SDIO_DataInitStructure.SDIO_DataLength= nblks*blksize ;
			 SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
			 SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
			 SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToSDIO;
			 SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
			 SDIO_DataConfig(&SDIO_DataInitStructure);

       SDIO_CmdInitStructure.SDIO_Argument =  addr;//·¢ËÍCMD18+´ÓaddrµØÖ·³ö¶ÁÈ¡Êı¾İ,¶ÌÏìÓ¦ 
	     SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
		   SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		   SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		   SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		   SDIO_SendCommand(&SDIO_CmdInitStructure);	
		
		errorstatus=CmdResp1Error(SD_CMD_READ_MULT_BLOCK);//µÈ´ıR1ÏìÓ¦ 
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó	 
		
 		if(DeviceMode==SD_POLLING_MODE)
		{
			INTX_DISABLE();//¹Ø±Õ×ÜÖĞ¶Ï(POLLINGÄ£Ê½,ÑÏ½ûÖĞ¶Ï´ò¶ÏSDIO¶ÁĞ´²Ù×÷!!!)
			while(!(SDIO->STA&((1<<5)|(1<<1)|(1<<3)|(1<<8)|(1<<9))))//ÎŞÉÏÒç/CRC/³¬Ê±/Íê³É(±êÖ¾)/ÆğÊ¼Î»´íÎó
			{
				if(SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)						//½ÓÊÕÇø°ëÂú,±íÊ¾ÖÁÉÙ´æÁË8¸ö×Ö
				{
					for(count=0;count<8;count++)			//Ñ­»·¶ÁÈ¡Êı¾İ
					{
						*(tempbuff+count)=SDIO->FIFO;
					}
					tempbuff+=8;	 
					timeout=0X7FFFFF; 	//¶ÁÊı¾İÒç³öÊ±¼ä
				}else 	//´¦Àí³¬Ê±
				{
					if(timeout==0)return SD_DATA_TIMEOUT;
					timeout--;
				}
			}  
		if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)		//Êı¾İ³¬Ê±´íÎó
		{										   
	 		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); 	//Çå´íÎó±êÖ¾
			return SD_DATA_TIMEOUT;
	 	}else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)	//Êı¾İ¿éCRC´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);  		//Çå´íÎó±êÖ¾
			return SD_DATA_CRC_FAIL;		   
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) 	//½ÓÊÕfifoÉÏÒç´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);		//Çå´íÎó±êÖ¾
			return SD_RX_OVERRUN;		 
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) 	//½ÓÊÕÆğÊ¼Î»´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_STBITERR);//Çå´íÎó±êÖ¾
			return SD_START_BIT_ERR;		 
		}   
	    
		while(SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)	//FIFOÀïÃæ,»¹´æÔÚ¿ÉÓÃÊı¾İ
		{
			*tempbuff=SDIO->FIFO;	//Ñ­»·¶ÁÈ¡Êı¾İ
			tempbuff++;
		}
	 		if(SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)		//½ÓÊÕ½áÊø
			{
				if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
				{				
					SDIO_CmdInitStructure.SDIO_Argument =  0;//·¢ËÍCMD12+½áÊø´«Êä
				  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
					SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
					SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
					SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
					SDIO_SendCommand(&SDIO_CmdInitStructure);	
					
					errorstatus=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);//µÈ´ıR1ÏìÓ¦   
					
					if(errorstatus!=SD_OK)return errorstatus;	 
				}
 			}
			INTX_ENABLE();//¿ªÆô×ÜÖĞ¶Ï
	 		SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
 		}else if(DeviceMode==SD_DMA_MODE)
		{
	   		TransferError=SD_OK;
			StopCondition=1;			//¶à¿é¶Á,ĞèÒª·¢ËÍÍ£Ö¹´«ÊäÖ¸Áî 
			TransferEnd=0;				//´«Êä½áÊø±êÖÃÎ»£¬ÔÚÖĞ¶Ï·şÎñÖÃ1
			SDIO->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<5)|(1<<9);	//ÅäÖÃĞèÒªµÄÖĞ¶Ï 
		 	SDIO->DCTRL|=1<<3;		 						//SDIO DMAÊ¹ÄÜ 
	 	    SD_DMA_Config((u32*)buf,nblks*blksize,DMA_DIR_PeripheralToMemory); 
	 		while(((DMA2->LISR&(1<<27))==RESET)&&timeout)timeout--;//µÈ´ı´«ÊäÍê³É 
			if(timeout==0)return SD_DATA_TIMEOUT;//³¬Ê±
			while((TransferEnd==0)&&(TransferError==SD_OK)); 
			if(TransferError!=SD_OK)errorstatus=TransferError;  	 
		}		 
  	}
	return errorstatus;
}			    																  
//SD¿¨Ğ´1¸ö¿é 
//buf:Êı¾İ»º´æÇø
//addr:Ğ´µØÖ·
//blksize:¿é´óĞ¡	  
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error SD_WriteBlock(u8 *buf,long long addr,  u16 blksize)
{
	SD_Error errorstatus = SD_OK;
	
	u8  power=0,cardstate=0;
	
	u32 timeout=0,bytestransferred=0;
	
	u32 cardstatus=0,count=0,restwords=0;
	
	u32	tlen=blksize;						//×Ü³¤¶È(×Ö½Ú)
	
	u32*tempbuff=(u32*)buf;					
	
 	if(buf==NULL)return SD_INVALID_PARAMETER;//²ÎÊı´íÎó  
	
  SDIO->DCTRL=0x0;							//Êı¾İ¿ØÖÆ¼Ä´æÆ÷ÇåÁã(¹ØDMA)
	
	SDIO_DataInitStructure.SDIO_DataBlockSize= 0; ;//Çå³ıDPSM×´Ì¬»úÅäÖÃ
	SDIO_DataInitStructure.SDIO_DataLength= 0 ;
	SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
	SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
	
	if(SDIO->RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//¿¨ËøÁË
 	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)	//´óÈİÁ¿¿¨
	{
		blksize=512;
		addr>>=9;
	}    
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0))
	{
		power=convert_from_bytes_to_power_of_two(blksize);	
		
		SDIO_CmdInitStructure.SDIO_Argument = blksize;//·¢ËÍCMD16+ÉèÖÃÊı¾İ³¤¶ÈÎªblksize,¶ÌÏìÓ¦ 	
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);	
		
		errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);	//µÈ´ıR1ÏìÓ¦  
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó	 
		
	}else return SD_INVALID_PARAMETER;	
	
			SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA<<16;//·¢ËÍCMD13,²éÑ¯¿¨µÄ×´Ì¬,¶ÌÏìÓ¦ 	
		  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);	

	  errorstatus=CmdResp1Error(SD_CMD_SEND_STATUS);		//µÈ´ıR1ÏìÓ¦  
	
	if(errorstatus!=SD_OK)return errorstatus;
	cardstatus=SDIO->RESP1;													  
	timeout=SD_DATATIMEOUT;
   	while(((cardstatus&0x00000100)==0)&&(timeout>0)) 	//¼ì²éREADY_FOR_DATAÎ»ÊÇ·ñÖÃÎ»
	{
		timeout--;  
		
		SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA<<16;//·¢ËÍCMD13,²éÑ¯¿¨µÄ×´Ì¬,¶ÌÏìÓ¦
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);	
		
		errorstatus=CmdResp1Error(SD_CMD_SEND_STATUS);	//µÈ´ıR1ÏìÓ¦   
		
		if(errorstatus!=SD_OK)return errorstatus;		
		
		cardstatus=SDIO->RESP1;													  
	}
	if(timeout==0)return SD_ERROR;

			SDIO_CmdInitStructure.SDIO_Argument = addr;//·¢ËÍCMD24,Ğ´µ¥¿éÖ¸Áî,¶ÌÏìÓ¦ 	
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);	
	
	errorstatus=CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);//µÈ´ıR1ÏìÓ¦  
	
	if(errorstatus!=SD_OK)return errorstatus;   	 
	
	StopCondition=0;									//µ¥¿éĞ´,²»ĞèÒª·¢ËÍÍ£Ö¹´«ÊäÖ¸Áî 

	SDIO_DataInitStructure.SDIO_DataBlockSize= power<<4; ;	//blksize, ¿ØÖÆÆ÷µ½¿¨	
	SDIO_DataInitStructure.SDIO_DataLength= blksize ;
	SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
	SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
	
	timeout=SDIO_DATATIMEOUT;
	
	if (DeviceMode == SD_POLLING_MODE)
	{
		INTX_DISABLE();//¹Ø±Õ×ÜÖĞ¶Ï(POLLINGÄ£Ê½,ÑÏ½ûÖĞ¶Ï´ò¶ÏSDIO¶ÁĞ´²Ù×÷!!!)
		while(!(SDIO->STA&((1<<10)|(1<<4)|(1<<1)|(1<<3)|(1<<9))))//Êı¾İ¿é·¢ËÍ³É¹¦/ÏÂÒç/CRC/³¬Ê±/ÆğÊ¼Î»´íÎó
		{
			if(SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)							//·¢ËÍÇø°ë¿Õ,±íÊ¾ÖÁÉÙ´æÁË8¸ö×Ö
			{
				if((tlen-bytestransferred)<SD_HALFFIFOBYTES)//²»¹»32×Ö½ÚÁË
				{
					restwords=((tlen-bytestransferred)%4==0)?((tlen-bytestransferred)/4):((tlen-bytestransferred)/4+1);
					
					for(count=0;count<restwords;count++,tempbuff++,bytestransferred+=4)
					{
						SDIO->FIFO=*tempbuff;
					}
				}else
				{
					for(count=0;count<8;count++)
					{
						SDIO->FIFO=*(tempbuff+count);
					}
					tempbuff+=8;
					bytestransferred+=32;
				}
				timeout=0X3FFFFFFF;	//Ğ´Êı¾İÒç³öÊ±¼ä
			}else
			{
				if(timeout==0)return SD_DATA_TIMEOUT;
				timeout--;
			}
		} 
		if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)		//Êı¾İ³¬Ê±´íÎó
		{										   
	 		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); 	//Çå´íÎó±êÖ¾
			return SD_DATA_TIMEOUT;
	 	}else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)	//Êı¾İ¿éCRC´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);  		//Çå´íÎó±êÖ¾
			return SD_DATA_CRC_FAIL;		   
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) 	//½ÓÊÕfifoÏÂÒç´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);		//Çå´íÎó±êÖ¾
			return SD_TX_UNDERRUN;		 
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) 	//½ÓÊÕÆğÊ¼Î»´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_STBITERR);//Çå´íÎó±êÖ¾
			return SD_START_BIT_ERR;		 
		}   
	      
		INTX_ENABLE();//¿ªÆô×ÜÖĞ¶Ï
		SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç  
	}else if(DeviceMode==SD_DMA_MODE)
	{
   		TransferError=SD_OK;
		StopCondition=0;			//µ¥¿éĞ´,²»ĞèÒª·¢ËÍÍ£Ö¹´«ÊäÖ¸Áî 
		TransferEnd=0;				//´«Êä½áÊø±êÖÃÎ»£¬ÔÚÖĞ¶Ï·şÎñÖÃ1
		SDIO->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<4)|(1<<9);	//ÅäÖÃ²úÉúÊı¾İ½ÓÊÕÍê³ÉÖĞ¶Ï
		SD_DMA_Config((u32*)buf,blksize,DMA_DIR_MemoryToPeripheral);				//SDIO DMAÅäÖÃ
 	 	SDIO->DCTRL|=1<<3;								//SDIO DMAÊ¹ÄÜ.  
 		while(((DMA2->LISR&(1<<27))==RESET)&&timeout)timeout--;//µÈ´ı´«ÊäÍê³É 
		if(timeout==0)
		{
  			SD_Init();	 					//ÖØĞÂ³õÊ¼»¯SD¿¨,¿ÉÒÔ½â¾öĞ´ÈëËÀ»úµÄÎÊÌâ
			return SD_DATA_TIMEOUT;			//³¬Ê±	 
 		}
		timeout=SDIO_DATATIMEOUT;
		while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
 		if(timeout==0)return SD_DATA_TIMEOUT;			//³¬Ê±	 
  		if(TransferError!=SD_OK)return TransferError;
 	}  
 	SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
 	errorstatus=IsCardProgramming(&cardstate);
 	while((errorstatus==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		errorstatus=IsCardProgramming(&cardstate);
	}   
	return errorstatus;
}
//SD¿¨Ğ´¶à¸ö¿é 
//buf:Êı¾İ»º´æÇø
//addr:Ğ´µØÖ·
//blksize:¿é´óĞ¡
//nblks:ÒªĞ´ÈëµÄ¿éÊı
//·µ»ØÖµ:´íÎó×´Ì¬												   
SD_Error SD_WriteMultiBlocks(u8 *buf,long long addr,u16 blksize,u32 nblks)
{
	SD_Error errorstatus = SD_OK;
	u8  power = 0, cardstate = 0;
	u32 timeout=0,bytestransferred=0;
	u32 count = 0, restwords = 0;
	u32 tlen=nblks*blksize;				//×Ü³¤¶È(×Ö½Ú)
	u32 *tempbuff = (u32*)buf;  
  if(buf==NULL)return SD_INVALID_PARAMETER; //²ÎÊı´íÎó  
  SDIO->DCTRL=0x0;							//Êı¾İ¿ØÖÆ¼Ä´æÆ÷ÇåÁã(¹ØDMA)   
	
	SDIO_DataInitStructure.SDIO_DataBlockSize= 0; ;	//Çå³ıDPSM×´Ì¬»úÅäÖÃ	
	SDIO_DataInitStructure.SDIO_DataLength= 0 ;
	SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
	SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
	SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
  SDIO_DataConfig(&SDIO_DataInitStructure);
	
	if(SDIO->RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//¿¨ËøÁË
 	if(CardType==SDIO_HIGH_CAPACITY_SD_CARD)//´óÈİÁ¿¿¨
	{
		blksize=512;
		addr>>=9;
	}    
	if((blksize>0)&&(blksize<=2048)&&((blksize&(blksize-1))==0))
	{
		power=convert_from_bytes_to_power_of_two(blksize);
		
		SDIO_CmdInitStructure.SDIO_Argument = blksize;	//·¢ËÍCMD16+ÉèÖÃÊı¾İ³¤¶ÈÎªblksize,¶ÌÏìÓ¦
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);	
		
		errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);	//µÈ´ıR1ÏìÓ¦  
		
		if(errorstatus!=SD_OK)return errorstatus;   	//ÏìÓ¦´íÎó	 
		
	}else return SD_INVALID_PARAMETER;	 
	if(nblks>1)
	{					  
		if(nblks*blksize>SD_MAX_DATA_LENGTH)return SD_INVALID_PARAMETER;   
     	if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
    	{
			//Ìá¸ßĞÔÄÜ
				SDIO_CmdInitStructure.SDIO_Argument = (u32)RCA<<16;		//·¢ËÍACMD55,¶ÌÏìÓ¦ 	
				SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
				SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
				SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
				SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
				SDIO_SendCommand(&SDIO_CmdInitStructure);	
				
			errorstatus=CmdResp1Error(SD_CMD_APP_CMD);		//µÈ´ıR1ÏìÓ¦ 
				
			if(errorstatus!=SD_OK)return errorstatus;				 
				
				SDIO_CmdInitStructure.SDIO_Argument =nblks;		//·¢ËÍCMD23,ÉèÖÃ¿éÊıÁ¿,¶ÌÏìÓ¦ 	 
				SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
				SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
				SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
				SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
				SDIO_SendCommand(&SDIO_CmdInitStructure);
			  
				errorstatus=CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);//µÈ´ıR1ÏìÓ¦ 
				
			if(errorstatus!=SD_OK)return errorstatus;		
		    
		} 

				SDIO_CmdInitStructure.SDIO_Argument =addr;	//·¢ËÍCMD25,¶à¿éĞ´Ö¸Áî,¶ÌÏìÓ¦ 	  
				SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
				SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
				SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
				SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
				SDIO_SendCommand(&SDIO_CmdInitStructure);	

 		errorstatus=CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);	//µÈ´ıR1ÏìÓ¦   		   
	
		if(errorstatus!=SD_OK)return errorstatus;

        SDIO_DataInitStructure.SDIO_DataBlockSize= power<<4; ;	//blksize, ¿ØÖÆÆ÷µ½¿¨	
				SDIO_DataInitStructure.SDIO_DataLength= nblks*blksize ;
				SDIO_DataInitStructure.SDIO_DataTimeOut=SD_DATATIMEOUT ;
				SDIO_DataInitStructure.SDIO_DPSM=SDIO_DPSM_Enable;
				SDIO_DataInitStructure.SDIO_TransferDir=SDIO_TransferDir_ToCard;
				SDIO_DataInitStructure.SDIO_TransferMode=SDIO_TransferMode_Block;
				SDIO_DataConfig(&SDIO_DataInitStructure);
				
		if(DeviceMode==SD_POLLING_MODE)
	    {
			timeout=SDIO_DATATIMEOUT;
			INTX_DISABLE();//¹Ø±Õ×ÜÖĞ¶Ï(POLLINGÄ£Ê½,ÑÏ½ûÖĞ¶Ï´ò¶ÏSDIO¶ÁĞ´²Ù×÷!!!)
			while(!(SDIO->STA&((1<<4)|(1<<1)|(1<<8)|(1<<3)|(1<<9))))//ÏÂÒç/CRC/Êı¾İ½áÊø/³¬Ê±/ÆğÊ¼Î»´íÎó
			{
				if(SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)							//·¢ËÍÇø°ë¿Õ,±íÊ¾ÖÁÉÙ´æÁË8×Ö(32×Ö½Ú)
				{	  
					if((tlen-bytestransferred)<SD_HALFFIFOBYTES)//²»¹»32×Ö½ÚÁË
					{
						restwords=((tlen-bytestransferred)%4==0)?((tlen-bytestransferred)/4):((tlen-bytestransferred)/4+1);
						for(count=0;count<restwords;count++,tempbuff++,bytestransferred+=4)
						{
							SDIO->FIFO=*tempbuff;
						}
					}else 										//·¢ËÍÇø°ë¿Õ,¿ÉÒÔ·¢ËÍÖÁÉÙ8×Ö(32×Ö½Ú)Êı¾İ
					{
						for(count=0;count<SD_HALFFIFO;count++)
						{
							SDIO->FIFO=*(tempbuff+count);
						}
						tempbuff+=SD_HALFFIFO;
						bytestransferred+=SD_HALFFIFOBYTES;
					}
					timeout=0X3FFFFFFF;	//Ğ´Êı¾İÒç³öÊ±¼ä
				}else
				{
					if(timeout==0)return SD_DATA_TIMEOUT; 
					timeout--;
				}
			} 
		if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)		//Êı¾İ³¬Ê±´íÎó
		{										   
	 		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); 	//Çå´íÎó±êÖ¾
			return SD_DATA_TIMEOUT;
	 	}else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)	//Êı¾İ¿éCRC´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);  		//Çå´íÎó±êÖ¾
			return SD_DATA_CRC_FAIL;		   
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) 	//½ÓÊÕfifoÏÂÒç´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);		//Çå´íÎó±êÖ¾
			return SD_TX_UNDERRUN;		 
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) 	//½ÓÊÕÆğÊ¼Î»´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_STBITERR);//Çå´íÎó±êÖ¾
			return SD_START_BIT_ERR;		 
		}   
	      										   
			if(SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)		//·¢ËÍ½áÊø
			{															 
				if((SDIO_STD_CAPACITY_SD_CARD_V1_1==CardType)||(SDIO_STD_CAPACITY_SD_CARD_V2_0==CardType)||(SDIO_HIGH_CAPACITY_SD_CARD==CardType))
				{   
					SDIO_CmdInitStructure.SDIO_Argument =0;//·¢ËÍCMD12+½áÊø´«Êä 	  
					SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
					SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
					SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
					SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
					SDIO_SendCommand(&SDIO_CmdInitStructure);	
					
					errorstatus=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);//µÈ´ıR1ÏìÓ¦   
					if(errorstatus!=SD_OK)return errorstatus;	 
				}
			}
			INTX_ENABLE();//¿ªÆô×ÜÖĞ¶Ï
	 		SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	    }else if(DeviceMode==SD_DMA_MODE)
		{
	   	TransferError=SD_OK;
			StopCondition=1;			//¶à¿éĞ´,ĞèÒª·¢ËÍÍ£Ö¹´«ÊäÖ¸Áî 
			TransferEnd=0;				//´«Êä½áÊø±êÖÃÎ»£¬ÔÚÖĞ¶Ï·şÎñÖÃ1
			SDIO->MASK|=(1<<1)|(1<<3)|(1<<8)|(1<<4)|(1<<9);	//ÅäÖÃ²úÉúÊı¾İ½ÓÊÕÍê³ÉÖĞ¶Ï
			SD_DMA_Config((u32*)buf,nblks*blksize,DMA_DIR_MemoryToPeripheral);		//SDIO DMAÅäÖÃ
	 	 	SDIO->DCTRL|=1<<3;								//SDIO DMAÊ¹ÄÜ. 
			timeout=SDIO_DATATIMEOUT;
	 		while(((DMA2->LISR&(1<<27))==RESET)&&timeout)timeout--;//µÈ´ı´«ÊäÍê³É 
			if(timeout==0)	 								//³¬Ê±
			{									  
  				SD_Init();	 					//ÖØĞÂ³õÊ¼»¯SD¿¨,¿ÉÒÔ½â¾öĞ´ÈëËÀ»úµÄÎÊÌâ
	 			return SD_DATA_TIMEOUT;			//³¬Ê±	 
	 		}
			timeout=SDIO_DATATIMEOUT;
			while((TransferEnd==0)&&(TransferError==SD_OK)&&timeout)timeout--;
	 		if(timeout==0)return SD_DATA_TIMEOUT;			//³¬Ê±	 
	 		if(TransferError!=SD_OK)return TransferError;	 
		}
  	}
 	SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
 	errorstatus=IsCardProgramming(&cardstate);
 	while((errorstatus==SD_OK)&&((cardstate==SD_CARD_PROGRAMMING)||(cardstate==SD_CARD_RECEIVING)))
	{
		errorstatus=IsCardProgramming(&cardstate);
	}   
	return errorstatus;	   
}
//SDIOÖĞ¶Ï·şÎñº¯Êı		  
void SDIO_IRQHandler(void) 
{											
 	SD_ProcessIRQSrc();//´¦ÀíËùÓĞSDIOÏà¹ØÖĞ¶Ï
}	 																    
//SDIOÖĞ¶Ï´¦Àíº¯Êı
//´¦ÀíSDIO´«Êä¹ı³ÌÖĞµÄ¸÷ÖÖÖĞ¶ÏÊÂÎñ
//·µ»ØÖµ:´íÎó´úÂë
SD_Error SD_ProcessIRQSrc(void)
{
	if(SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)//½ÓÊÕÍê³ÉÖĞ¶Ï
	{	 
		if (StopCondition==1)
		{  
				SDIO_CmdInitStructure.SDIO_Argument =0;//·¢ËÍCMD12+½áÊø´«Êä 	  
				SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
				SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
				SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
				SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
				SDIO_SendCommand(&SDIO_CmdInitStructure);	
					
			TransferError=CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
		}else TransferError = SD_OK;	
 		SDIO->ICR|=1<<8;//Çå³ıÍê³ÉÖĞ¶Ï±ê¼Ç
		SDIO->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//¹Ø±ÕÏà¹ØÖĞ¶Ï
 		TransferEnd = 1;
		return(TransferError);
	}
 	if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)//Êı¾İCRC´íÎó
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);  		//Çå´íÎó±êÖ¾
		SDIO->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//¹Ø±ÕÏà¹ØÖĞ¶Ï
	    TransferError = SD_DATA_CRC_FAIL;
	    return(SD_DATA_CRC_FAIL);
	}
 	if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)//Êı¾İ³¬Ê±´íÎó
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);  			//ÇåÖĞ¶Ï±êÖ¾
		SDIO->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//¹Ø±ÕÏà¹ØÖĞ¶Ï
	    TransferError = SD_DATA_TIMEOUT;
	    return(SD_DATA_TIMEOUT);
	}
  	if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)//FIFOÉÏÒç´íÎó
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);  			//ÇåÖĞ¶Ï±êÖ¾
		SDIO->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//¹Ø±ÕÏà¹ØÖĞ¶Ï
	    TransferError = SD_RX_OVERRUN;
	    return(SD_RX_OVERRUN);
	}
   	if(SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)//FIFOÏÂÒç´íÎó
	{
		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);  			//ÇåÖĞ¶Ï±êÖ¾
		SDIO->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//¹Ø±ÕÏà¹ØÖĞ¶Ï
	    TransferError = SD_TX_UNDERRUN;
	    return(SD_TX_UNDERRUN);
	}
	if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)//ÆğÊ¼Î»´íÎó
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);  		//ÇåÖĞ¶Ï±êÖ¾
		SDIO->MASK&=~((1<<1)|(1<<3)|(1<<8)|(1<<14)|(1<<15)|(1<<4)|(1<<5)|(1<<9));//¹Ø±ÕÏà¹ØÖĞ¶Ï
	    TransferError = SD_START_BIT_ERR;
	    return(SD_START_BIT_ERR);
	}
	return(SD_OK);
}
  
//¼ì²éCMD0µÄÖ´ĞĞ×´Ì¬
//·µ»ØÖµ:sd¿¨´íÎóÂë
SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	u32 timeout=SDIO_CMD0TIMEOUT;	   
	while(timeout--)
	{
		if(SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) != RESET)break;	//ÃüÁîÒÑ·¢ËÍ(ÎŞĞèÏìÓ¦)	 
	}	    
	if(timeout==0)return SD_CMD_RSP_TIMEOUT;  
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	return errorstatus;
}	 
//¼ì²éR7ÏìÓ¦µÄ´íÎó×´Ì¬
//·µ»ØÖµ:sd¿¨´íÎóÂë
SD_Error CmdResp7Error(void)
{
	SD_Error errorstatus=SD_OK;
	u32 status;
	u32 timeout=SDIO_CMD0TIMEOUT;
 	while(timeout--)
	{
		status=SDIO->STA;
		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC´íÎó/ÃüÁîÏìÓ¦³¬Ê±/ÒÑ¾­ÊÕµ½ÏìÓ¦(CRCĞ£Ñé³É¹¦)	
	}
 	if((timeout==0)||(status&(1<<2)))	//ÏìÓ¦³¬Ê±
	{																				    
		errorstatus=SD_CMD_RSP_TIMEOUT;	//µ±Ç°¿¨²»ÊÇ2.0¼æÈİ¿¨,»òÕß²»Ö§³ÖÉè¶¨µÄµçÑ¹·¶Î§
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); 			//Çå³ıÃüÁîÏìÓ¦³¬Ê±±êÖ¾
		return errorstatus;
	}	 
	if(status&1<<6)						//³É¹¦½ÓÊÕµ½ÏìÓ¦
	{								   
		errorstatus=SD_OK;
		SDIO_ClearFlag(SDIO_FLAG_CMDREND); 				//Çå³ıÏìÓ¦±êÖ¾
 	}
	return errorstatus;
}	   
//¼ì²éR1ÏìÓ¦µÄ´íÎó×´Ì¬
//cmd:µ±Ç°ÃüÁî
//·µ»ØÖµ:sd¿¨´íÎóÂë
SD_Error CmdResp1Error(u8 cmd)
{	  
   	u32 status; 
	while(1)
	{
		status=SDIO->STA;
		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC´íÎó/ÃüÁîÏìÓ¦³¬Ê±/ÒÑ¾­ÊÕµ½ÏìÓ¦(CRCĞ£Ñé³É¹¦)
	} 
	if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)					//ÏìÓ¦³¬Ê±
	{																				    
 		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); 				//Çå³ıÃüÁîÏìÓ¦³¬Ê±±êÖ¾
		return SD_CMD_RSP_TIMEOUT;
	}	
 	if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)					//CRC´íÎó
	{																				    
 		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL); 				//Çå³ı±êÖ¾
		return SD_CMD_CRC_FAIL;
	}		
	if(SDIO->RESPCMD!=cmd)return SD_ILLEGAL_CMD;//ÃüÁî²»Æ¥Åä 
  SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	return (SD_Error)(SDIO->RESP1&SD_OCR_ERRORBITS);//·µ»Ø¿¨ÏìÓ¦
}
//¼ì²éR3ÏìÓ¦µÄ´íÎó×´Ì¬
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error CmdResp3Error(void)
{
	u32 status;						 
 	while(1)
	{
		status=SDIO->STA;
		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC´íÎó/ÃüÁîÏìÓ¦³¬Ê±/ÒÑ¾­ÊÕµ½ÏìÓ¦(CRCĞ£Ñé³É¹¦)	
	}
 	if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)					//ÏìÓ¦³¬Ê±
	{											 
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);			//Çå³ıÃüÁîÏìÓ¦³¬Ê±±êÖ¾
		return SD_CMD_RSP_TIMEOUT;
	}	 
   SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
 	return SD_OK;								  
}
//¼ì²éR2ÏìÓ¦µÄ´íÎó×´Ì¬
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error CmdResp2Error(void)
{
	SD_Error errorstatus=SD_OK;
	u32 status;
	u32 timeout=SDIO_CMD0TIMEOUT;
 	while(timeout--)
	{
		status=SDIO->STA;
		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC´íÎó/ÃüÁîÏìÓ¦³¬Ê±/ÒÑ¾­ÊÕµ½ÏìÓ¦(CRCĞ£Ñé³É¹¦)	
	}
  	if((timeout==0)||(status&(1<<2)))	//ÏìÓ¦³¬Ê±
	{																				    
		errorstatus=SD_CMD_RSP_TIMEOUT; 
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT); 		//Çå³ıÃüÁîÏìÓ¦³¬Ê±±êÖ¾
		return errorstatus;
	}	 
	if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)						//CRC´íÎó
	{								   
		errorstatus=SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);		//Çå³ıÏìÓ¦±êÖ¾
 	}
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
 	return errorstatus;								    		 
} 
//¼ì²éR6ÏìÓ¦µÄ´íÎó×´Ì¬
//cmd:Ö®Ç°·¢ËÍµÄÃüÁî
//prca:¿¨·µ»ØµÄRCAµØÖ·
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error CmdResp6Error(u8 cmd,u16*prca)
{
	SD_Error errorstatus=SD_OK;
	u32 status;					    
	u32 rspr1;
 	while(1)
	{
		status=SDIO->STA;
		if(status&((1<<0)|(1<<2)|(1<<6)))break;//CRC´íÎó/ÃüÁîÏìÓ¦³¬Ê±/ÒÑ¾­ÊÕµ½ÏìÓ¦(CRCĞ£Ñé³É¹¦)	
	}
	if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)					//ÏìÓ¦³¬Ê±
	{																				    
 		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);			//Çå³ıÃüÁîÏìÓ¦³¬Ê±±êÖ¾
		return SD_CMD_RSP_TIMEOUT;
	}	 	 
	if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)						//CRC´íÎó
	{								   
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);					//Çå³ıÏìÓ¦±êÖ¾
 		return SD_CMD_CRC_FAIL;
	}
	if(SDIO->RESPCMD!=cmd)				//ÅĞ¶ÏÊÇ·ñÏìÓ¦cmdÃüÁî
	{
 		return SD_ILLEGAL_CMD; 		
	}	    
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	rspr1=SDIO->RESP1;					//µÃµ½ÏìÓ¦ 	 
	if(SD_ALLZERO==(rspr1&(SD_R6_GENERAL_UNKNOWN_ERROR|SD_R6_ILLEGAL_CMD|SD_R6_COM_CRC_FAILED)))
	{
		*prca=(u16)(rspr1>>16);			//ÓÒÒÆ16Î»µÃµ½,rca
		return errorstatus;
	}
   	if(rspr1&SD_R6_GENERAL_UNKNOWN_ERROR)return SD_GENERAL_UNKNOWN_ERROR;
   	if(rspr1&SD_R6_ILLEGAL_CMD)return SD_ILLEGAL_CMD;
   	if(rspr1&SD_R6_COM_CRC_FAILED)return SD_COM_CRC_FAILED;
	return errorstatus;
}

//SDIOÊ¹ÄÜ¿í×ÜÏßÄ£Ê½
//enx:0,²»Ê¹ÄÜ;1,Ê¹ÄÜ;
//·µ»ØÖµ:´íÎó×´Ì¬
SD_Error SDEnWideBus(u8 enx)
{
	SD_Error errorstatus = SD_OK;
 	u32 scr[2]={0,0};
	u8 arg=0X00;
	if(enx)arg=0X02;
	else arg=0X00;
 	if(SDIO->RESP1&SD_CARD_LOCKED)return SD_LOCK_UNLOCK_FAILED;//SD¿¨´¦ÓÚLOCKED×´Ì¬		    
 	errorstatus=FindSCR(RCA,scr);						//µÃµ½SCR¼Ä´æÆ÷Êı¾İ
 	if(errorstatus!=SD_OK)return errorstatus;
	if((scr[1]&SD_WIDE_BUS_SUPPORT)!=SD_ALLZERO)		//Ö§³Ö¿í×ÜÏß
	{
		  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;//·¢ËÍCMD55+RCA,¶ÌÏìÓ¦	
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);
		
	 	errorstatus=CmdResp1Error(SD_CMD_APP_CMD);
		
	 	if(errorstatus!=SD_OK)return errorstatus; 
		
		  SDIO_CmdInitStructure.SDIO_Argument = arg;//·¢ËÍACMD6,¶ÌÏìÓ¦,²ÎÊı:10,4Î»;00,1Î».	
      SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
      SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
      SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
      SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
      SDIO_SendCommand(&SDIO_CmdInitStructure);
			
     errorstatus=CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
		
		return errorstatus;
	}else return SD_REQUEST_NOT_APPLICABLE;				//²»Ö§³Ö¿í×ÜÏßÉèÖÃ 	 
}												   
//¼ì²é¿¨ÊÇ·ñÕıÔÚÖ´ĞĞĞ´²Ù×÷
//pstatus:µ±Ç°×´Ì¬.
//·µ»ØÖµ:´íÎó´úÂë
SD_Error IsCardProgramming(u8 *pstatus)
{
 	vu32 respR1 = 0, status = 0;  
  
  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16; //¿¨Ïà¶ÔµØÖ·²ÎÊı
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;//·¢ËÍCMD13 	
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);	
 	
	status=SDIO->STA;
	
	while(!(status&((1<<0)|(1<<6)|(1<<2))))status=SDIO->STA;//µÈ´ı²Ù×÷Íê³É
   	if(SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)			//CRC¼ì²âÊ§°Ü
	{  
	  SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);	//Çå³ı´íÎó±ê¼Ç
		return SD_CMD_CRC_FAIL;
	}
   	if(SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)			//ÃüÁî³¬Ê± 
	{
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);			//Çå³ı´íÎó±ê¼Ç
		return SD_CMD_RSP_TIMEOUT;
	}
 	if(SDIO->RESPCMD!=SD_CMD_SEND_STATUS)return SD_ILLEGAL_CMD;
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	respR1=SDIO->RESP1;
	*pstatus=(u8)((respR1>>9)&0x0000000F);
	return SD_OK;
}
//¶ÁÈ¡µ±Ç°¿¨×´Ì¬
//pcardstatus:¿¨×´Ì¬
//·µ»ØÖµ:´íÎó´úÂë
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;
	if(pcardstatus==NULL)
	{
		errorstatus=SD_INVALID_PARAMETER;
		return errorstatus;
	}
	
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;//·¢ËÍCMD13,¶ÌÏìÓ¦		 
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);	
	
	errorstatus=CmdResp1Error(SD_CMD_SEND_STATUS);	//²éÑ¯ÏìÓ¦×´Ì¬ 
	if(errorstatus!=SD_OK)return errorstatus;
	*pcardstatus=SDIO->RESP1;//¶ÁÈ¡ÏìÓ¦Öµ
	return errorstatus;
} 
//·µ»ØSD¿¨µÄ×´Ì¬
//·µ»ØÖµ:SD¿¨×´Ì¬
SDCardState SD_GetState(void)
{
	u32 resp1=0;
	if(SD_SendStatus(&resp1)!=SD_OK)return SD_CARD_ERROR;
	else return (SDCardState)((resp1>>9) & 0x0F);
}
//²éÕÒSD¿¨µÄSCR¼Ä´æÆ÷Öµ
//rca:¿¨Ïà¶ÔµØÖ·
//pscr:Êı¾İ»º´æÇø(´æ´¢SCRÄÚÈİ)
//·µ»ØÖµ:´íÎó×´Ì¬		   
SD_Error FindSCR(u16 rca,u32 *pscr)
{ 
	u32 index = 0; 
	SD_Error errorstatus = SD_OK;
	u32 tempscr[2]={0,0};  
	
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8;	 //·¢ËÍCMD16,¶ÌÏìÓ¦,ÉèÖÃBlock SizeÎª8×Ö½Ú	
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN; //	 cmd16
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r1
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);
	
 	errorstatus=CmdResp1Error(SD_CMD_SET_BLOCKLEN);
	
 	if(errorstatus!=SD_OK)return errorstatus;	 
	
  SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16; 
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;//·¢ËÍCMD55,¶ÌÏìÓ¦ 	
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);
	
 	errorstatus=CmdResp1Error(SD_CMD_APP_CMD);
 	if(errorstatus!=SD_OK)return errorstatus;
	
  SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
  SDIO_DataInitStructure.SDIO_DataLength = 8;  //8¸ö×Ö½Ú³¤¶È,blockÎª8×Ö½Ú,SD¿¨µ½SDIO.
  SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b  ;  //¿é´óĞ¡8byte 
  SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
  SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
  SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
  SDIO_DataConfig(&SDIO_DataInitStructure);		

  SDIO_CmdInitStructure.SDIO_Argument = 0x0;
  SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;	//·¢ËÍACMD51,¶ÌÏìÓ¦,²ÎÊıÎª0	
  SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;  //r1
  SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
  SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
  SDIO_SendCommand(&SDIO_CmdInitStructure);
	
 	errorstatus=CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
 	if(errorstatus!=SD_OK)return errorstatus;							   
 	while(!(SDIO->STA&(SDIO_FLAG_RXOVERR|SDIO_FLAG_DCRCFAIL|SDIO_FLAG_DTIMEOUT|SDIO_FLAG_DBCKEND|SDIO_FLAG_STBITERR)))
	{ 
		if(SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)//½ÓÊÕFIFOÊı¾İ¿ÉÓÃ
		{
			*(tempscr+index)=SDIO->FIFO;	//¶ÁÈ¡FIFOÄÚÈİ
			index++;
			if(index>=2)break;
		}
	}
		if(SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)		//Êı¾İ³¬Ê±´íÎó
		{										   
	 		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); 	//Çå´íÎó±êÖ¾
			return SD_DATA_TIMEOUT;
	 	}else if(SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)	//Êı¾İ¿éCRC´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);  		//Çå´íÎó±êÖ¾
			return SD_DATA_CRC_FAIL;		   
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) 	//½ÓÊÕfifoÉÏÒç´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);		//Çå´íÎó±êÖ¾
			return SD_RX_OVERRUN;		 
		}else if(SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) 	//½ÓÊÕÆğÊ¼Î»´íÎó
		{
	 		SDIO_ClearFlag(SDIO_FLAG_STBITERR);//Çå´íÎó±êÖ¾
			return SD_START_BIT_ERR;		 
		}  
   SDIO_ClearFlag(SDIO_STATIC_FLAGS);//Çå³ıËùÓĞ±ê¼Ç
	//°ÑÊı¾İË³Ğò°´8Î»Îªµ¥Î»µ¹¹ıÀ´.   	
	*(pscr+1)=((tempscr[0]&SD_0TO7BITS)<<24)|((tempscr[0]&SD_8TO15BITS)<<8)|((tempscr[0]&SD_16TO23BITS)>>8)|((tempscr[0]&SD_24TO31BITS)>>24);
	*(pscr)=((tempscr[1]&SD_0TO7BITS)<<24)|((tempscr[1]&SD_8TO15BITS)<<8)|((tempscr[1]&SD_16TO23BITS)>>8)|((tempscr[1]&SD_24TO31BITS)>>24);
 	return errorstatus;
}
//µÃµ½NumberOfBytesÒÔ2Îªµ×µÄÖ¸Êı.
//NumberOfBytes:×Ö½ÚÊı.
//·µ»ØÖµ:ÒÔ2Îªµ×µÄÖ¸ÊıÖµ
u8 convert_from_bytes_to_power_of_two(u16 NumberOfBytes)
{
	u8 count=0;
	while(NumberOfBytes!=1)
	{
		NumberOfBytes>>=1;
		count++;
	}
	return count;
} 	 

//ÅäÖÃSDIO DMA  
//mbuf:´æ´¢Æ÷µØÖ·
//bufsize:´«ÊäÊı¾İÁ¿
//dir:·½Ïò;DMA_DIR_MemoryToPeripheral  ´æ´¢Æ÷-->SDIO(Ğ´Êı¾İ);DMA_DIR_PeripheralToMemory SDIO-->´æ´¢Æ÷(¶ÁÊı¾İ);
void SD_DMA_Config(u32*mbuf,u32 bufsize,u32 dir)
{		 

  DMA_InitTypeDef  DMA_InitStructure;
	
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE){}//µÈ´ıDMA¿ÉÅäÖÃ 
		
  DMA_DeInit(DMA2_Stream3);//Çå¿ÕÖ®Ç°¸Ãstream3ÉÏµÄËùÓĞÖĞ¶Ï±êÖ¾
	
 
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  //Í¨µÀÑ¡Ôñ
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SDIO->FIFO;//DMAÍâÉèµØÖ·
  DMA_InitStructure.DMA_Memory0BaseAddr = (u32)mbuf;//DMA ´æ´¢Æ÷0µØÖ·
  DMA_InitStructure.DMA_DIR = dir;//´æ´¢Æ÷µ½ÍâÉèÄ£Ê½
  DMA_InitStructure.DMA_BufferSize = 0;//Êı¾İ´«ÊäÁ¿ 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//ÍâÉè·ÇÔöÁ¿Ä£Ê½
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//´æ´¢Æ÷ÔöÁ¿Ä£Ê½
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;//ÍâÉèÊı¾İ³¤¶È:32Î»
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;//´æ´¢Æ÷Êı¾İ³¤¶È:32Î»
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// Ê¹ÓÃÆÕÍ¨Ä£Ê½ 
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//×î¸ßÓÅÏÈ¼¶
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;   //FIFOÊ¹ÄÜ      
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;//È«FIFO
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;//ÍâÉèÍ»·¢4´Î´«Êä
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;//´æ´¢Æ÷Í»·¢4´Î´«Êä
  DMA_Init(DMA2_Stream3, &DMA_InitStructure);//³õÊ¼»¯DMA Stream

	DMA_FlowControllerConfig(DMA2_Stream3,DMA_FlowCtrl_Peripheral);//ÍâÉèÁ÷¿ØÖÆ 
	 
  DMA_Cmd(DMA2_Stream3 ,ENABLE);//¿ªÆôDMA´«Êä	 

}   


//¶ÁSD¿¨
//buf:¶ÁÊı¾İ»º´æÇø
//sector:ÉÈÇøµØÖ·
//cnt:ÉÈÇø¸öÊı	
//·µ»ØÖµ:´íÎó×´Ì¬;0,Õı³£;ÆäËû,´íÎó´úÂë;				  				 
u8 SD_ReadDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 sta=SD_OK;
	long long lsector=sector;
	u8 n;
	lsector<<=9;
	if((u32)buf%4!=0)
	{
	 	for(n=0;n<cnt;n++)
		{
		 	sta=SD_ReadBlock(SDIO_DATA_BUFFER,lsector+512*n,512);//µ¥¸ösectorµÄ¶Á²Ù×÷
			memcpy(buf,SDIO_DATA_BUFFER,512);
			buf+=512;
		} 
	}else
	{
		if(cnt==1)sta=SD_ReadBlock(buf,lsector,512);    	//µ¥¸ösectorµÄ¶Á²Ù×÷
		else sta=SD_ReadMultiBlocks(buf,lsector,512,cnt);//¶à¸ösector  
	}
	return sta;
}
//Ğ´SD¿¨
//buf:Ğ´Êı¾İ»º´æÇø
//sector:ÉÈÇøµØÖ·
//cnt:ÉÈÇø¸öÊı	
//·µ»ØÖµ:´íÎó×´Ì¬;0,Õı³£;ÆäËû,´íÎó´úÂë;	
u8 SD_WriteDisk(u8*buf,u32 sector,u8 cnt)
{
	u8 sta=SD_OK;
	u8 n;
	long long lsector=sector;
	lsector<<=9;
	if((u32)buf%4!=0)
	{
	 	for(n=0;n<cnt;n++)
		{
			memcpy(SDIO_DATA_BUFFER,buf,512);
		 	sta=SD_WriteBlock(SDIO_DATA_BUFFER,lsector+512*n,512);//µ¥¸ösectorµÄĞ´²Ù×÷
			buf+=512;
		} 
	}else
	{
		if(cnt==1)sta=SD_WriteBlock(buf,lsector,512);    	//µ¥¸ösectorµÄĞ´²Ù×÷
		else sta=SD_WriteMultiBlocks(buf,lsector,512,cnt);	//¶à¸ösector  
	}
	return sta;
}







