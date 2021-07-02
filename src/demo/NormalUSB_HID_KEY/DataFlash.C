
/********************************** (C) COPYRIGHT *******************************
* File Name          : DataFlash.C
* Author             : WCH
* Version            : V2.0
* Date               : 2016/6/24
* Description        : ��дCH559��DataFlash,CH55X���ڲ�Flash��˫�ֽ�д����ˣ������ֽڶ�
*******************************************************************************/
#define NO_XSFR_DEFINE
#include "CH559.H"
#include <intrins.h>

#pragma  NOAREGS

/*******************************************************************************
* Function Name  : EraseBlock(UINT16 Addr)
* Description    : CodeFlash�����(1KB)��ȫ������λд1
* Input          : UINT16 Addr
* Output         : None
* Return         : None
*******************************************************************************/
UINT8	EraseBlock( UINT16 Addr )
{
	ROM_ADDR = Addr;
	if ( ROM_STATUS & bROM_ADDR_OK ) {                                          // ������ַ��Ч
		ROM_CTRL = ROM_CMD_ERASE;
		return( ( ROM_STATUS ^ bROM_ADDR_OK ) & 0x7F );                           // ����״̬,0x00=success, 0x01=time out(bROM_CMD_TOUT), 0x02=unknown command(bROM_CMD_ERR)
	}
	else return( 0x40 );
}

/*******************************************************************************
* Function Name  : ProgWord( UINT16 Addr, UINT16 Data )
* Description    : дEEPROM��˫�ֽ�д
* Input          : UNIT16 Addr,д��ַ
                   UINT16 Data,����
* Output         : None
* Return         : SUCESS 
*******************************************************************************/
UINT8	ProgWord( UINT16 Addr, UINT16 Data )
{
	ROM_ADDR = Addr;
	ROM_DATA = Data;
	if ( ROM_STATUS & bROM_ADDR_OK ) {                                           // ������ַ��Ч
		ROM_CTRL = ROM_CMD_PROG;
		return( ( ROM_STATUS ^ bROM_ADDR_OK ) & 0x7F );                            // ����״̬,0x00=success, 0x01=time out(bROM_CMD_TOUT), 0x02=unknown command(bROM_CMD_ERR)
	}
	else return( 0x40 );
}

/*******************************************************************************
* Function Name  : EraseDataFlash(UINT16 Addr)
* Description    : DataFlash�����(1KB)��ȫ������λд1
* Input          : UINT16 Addr
* Output         : None
* Return         : UINT8 status
*******************************************************************************/
UINT8 EraseDataFlash(UINT16 Addr)
{
    UINT8 status;

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG |= bDATA_WE;                                                    //ʹ��DataFlashд
    status = EraseBlock(Addr);	
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG &= ~bDATA_WE;                                                   //����DataFlashд����
    return status;
}

/*******************************************************************************
* Function Name  : WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
* Description    : DataFlashд
* Input          : UINT16 Addr��PUINT16 buf,UINT8 len
* Output         : None
* Return         : 
*******************************************************************************/
void WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
{
    UINT8 j;
    UINT16 Tmp;

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG |= bDATA_WE;                                                    //ʹ��DataFlashд
    for(j=0;j<len;j=j+2)
    {
        Tmp = buf[j+1];Tmp <<= 8;
        Tmp |= buf[j];
        ProgWord(Addr,Tmp);
        Addr = Addr + 2;
    }
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //���밲ȫģʽ
    GLOBAL_CFG &= ~bDATA_WE;                                                   //����DataFlashд����
}