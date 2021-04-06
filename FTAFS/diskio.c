/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sys.h"
#include "sdio_sdcard.h"



#define SD_CARD	 0  //SDå¡ï¼Œå·æ ‡ä¸º0
#define EX_FLASH 1

#define FLASH_SECTOR_SIZE 	512			  
//¶ÔÓÚW25Q128
//Ç°12M×Ö½Ú¸øfatfsÓÃ,12M×Ö½Úºó,ÓÃÓÚ´æ·Å×Ö¿â,×Ö¿âÕ¼ÓÃ3.09M.	Ê£Óà²¿·Ö,¸ø¿Í»§×Ô¼ºÓÃ	 			    
u16	    FLASH_SECTOR_COUNT=2048*12;	//W25Q128,Ç°12M×Ö½Ú¸øFATFSÕ¼ÓÃ
#define FLASH_BLOCK_SIZE   	8     	//Ã¿¸öBLOCKÓÐ8¸öÉÈÇø

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	u8 res=0;	    
	switch(pdrv)
	{
		case SD_CARD://SD¿¨
			res=SD_Init();//SD¿¨³õÊ¼»¯ 
  			break;
		
		default:
			res=1; 
	}		 
	if(res)return  STA_NOINIT;
	else return 0; //³õÊ¼»¯³É¹¦
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	

	u8 res=0; 
    if (!count)return RES_PARERR;//count²»ÄÜµÈÓÚ0£¬·ñÔò·µ»Ø²ÎÊý´íÎó		 	 
	switch(pdrv)
	{
		case SD_CARD://SD¿¨
			res=SD_ReadDisk(buff,sector,count);	 
			while(res)//¶Á³ö´í
			{
				SD_Init();	//ÖØÐÂ³õÊ¼»¯SD¿¨
				res=SD_ReadDisk(buff,sector,count);	
				//printf("sd rd error:%d\r\n",res);
			}
			break;
//		case EX_FLASH://Íâ²¿flash
//			for(;count>0;count--)
//			{
//				W25Q128_Read(buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);  
//				sector++;
//				buff+=FLASH_SECTOR_SIZE; //ÓÉÓÚW25QXX_Read()ÖÐÊ¹ÓÃµÄÊÇpbuffer[i],ËùÒÔÕâÀïÒª+FLASH_SECTOR_SIZE
//			}
//			res=0;
//			break;
		default:
			res=1; 
	}
   //´¦Àí·µ»ØÖµ£¬½«SPI_SD_driver.cµÄ·µ»ØÖµ×ª³Éff.cµÄ·µ»ØÖµ
    if(res==0x00)return RES_OK;	 
    else return RES_ERROR;	   
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	
	u8 res=0;  
    if (!count)return RES_PARERR;//count²»ÄÜµÈÓÚ0£¬·ñÔò·µ»Ø²ÎÊý´íÎó		 	 
	switch(pdrv)
	{
		case SD_CARD://SD¿¨
			res=SD_WriteDisk((u8*)buff,sector,count);
			while(res)//Ð´³ö´í
			{
				SD_Init();	//ÖØÐÂ³õÊ¼»¯SD¿¨
				res=SD_WriteDisk((u8*)buff,sector,count);	
				//printf("sd wr error:%d\r\n",res);
			}
			break;
//		case EX_FLASH://Íâ²¿flash
//			for(;count>0;count--)
//			{										    
//				W25Q128_Write((u8*)buff,sector*FLASH_SECTOR_SIZE,FLASH_SECTOR_SIZE);
//				sector++;
//				buff+=FLASH_SECTOR_SIZE;
//			}
//			res=0;
//			break;
		default:
			res=1; 
	}
    //´¦Àí·µ»ØÖµ£¬½«SPI_SD_driver.cµÄ·µ»ØÖµ×ª³Éff.cµÄ·µ»ØÖµ
    if(res == 0x00)return RES_OK;	 
    else return RES_ERROR;	
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;						  			     
	if(pdrv==SD_CARD)//SD¿¨
	{
	    switch(cmd)
	    {
		    case CTRL_SYNC:
				res = RES_OK; 
		        break;	 
		    case GET_SECTOR_SIZE:
				*(DWORD*)buff = 512; 
		        res = RES_OK;
		        break;	 
		    case GET_BLOCK_SIZE:
				*(WORD*)buff = SDCardInfo.CardBlockSize; //¿é´óÐ¡¹Ì¶¨Îª512×Ö½Ú
		        res = RES_OK;
		        break;	 
		    case GET_SECTOR_COUNT:
		        *(DWORD*)buff = SDCardInfo.CardCapacity/512;
		        res = RES_OK;
		        break;
		    default:
		        res = RES_PARERR;
		        break;
	    }
	}
//	else if(pdrv==EX_FLASH)	//Íâ²¿FLASH  
//	{
//	    switch(cmd)
//	    {
//		    case CTRL_SYNC:
//				res = RES_OK; 
//		        break;	 
//		    case GET_SECTOR_SIZE:
//		        *(WORD*)buff = FLASH_SECTOR_SIZE;
//		        res = RES_OK;
//		        break;	 
//		    case GET_BLOCK_SIZE:
//		        *(WORD*)buff = FLASH_BLOCK_SIZE;
//		        res = RES_OK;
//		        break;	 
//		    case GET_SECTOR_COUNT:
//		        *(DWORD*)buff = FLASH_SECTOR_COUNT;
//		        res = RES_OK;
//		        break;
//		    default:
//		        res = RES_PARERR;
//		        break;
//	    }
//	}
	else res=RES_ERROR;//ÆäËûµÄ²»Ö§³Ö
    return res;
}

DWORD get_fattime(void)
{
	return 0;
}
