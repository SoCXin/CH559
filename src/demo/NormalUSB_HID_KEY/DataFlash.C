
/********************************** (C) COPYRIGHT *******************************
* File Name          : DataFlash.C
* Author             : WCH
* Version            : V2.0
* Date               : 2016/6/24
* Description        : 读写CH559的DataFlash,CH55X的内部Flash是双字节写（大端），单字节读
*******************************************************************************/
#define NO_XSFR_DEFINE
#include "CH559.H"
#include <intrins.h>

#pragma  NOAREGS

/*******************************************************************************
* Function Name  : EraseBlock(UINT16 Addr)
* Description    : CodeFlash块擦除(1KB)，全部数据位写1
* Input          : UINT16 Addr
* Output         : None
* Return         : None
*******************************************************************************/
UINT8	EraseBlock( UINT16 Addr )
{
	ROM_ADDR = Addr;
	if ( ROM_STATUS & bROM_ADDR_OK ) {                                          // 操作地址有效
		ROM_CTRL = ROM_CMD_ERASE;
		return( ( ROM_STATUS ^ bROM_ADDR_OK ) & 0x7F );                           // 返回状态,0x00=success, 0x01=time out(bROM_CMD_TOUT), 0x02=unknown command(bROM_CMD_ERR)
	}
	else return( 0x40 );
}

/*******************************************************************************
* Function Name  : ProgWord( UINT16 Addr, UINT16 Data )
* Description    : 写EEPROM，双字节写
* Input          : UNIT16 Addr,写地址
                   UINT16 Data,数据
* Output         : None
* Return         : SUCESS 
*******************************************************************************/
UINT8	ProgWord( UINT16 Addr, UINT16 Data )
{
	ROM_ADDR = Addr;
	ROM_DATA = Data;
	if ( ROM_STATUS & bROM_ADDR_OK ) {                                           // 操作地址有效
		ROM_CTRL = ROM_CMD_PROG;
		return( ( ROM_STATUS ^ bROM_ADDR_OK ) & 0x7F );                            // 返回状态,0x00=success, 0x01=time out(bROM_CMD_TOUT), 0x02=unknown command(bROM_CMD_ERR)
	}
	else return( 0x40 );
}

/*******************************************************************************
* Function Name  : EraseDataFlash(UINT16 Addr)
* Description    : DataFlash块擦除(1KB)，全部数据位写1
* Input          : UINT16 Addr
* Output         : None
* Return         : UINT8 status
*******************************************************************************/
UINT8 EraseDataFlash(UINT16 Addr)
{
    UINT8 status;

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //进入安全模式
    GLOBAL_CFG |= bDATA_WE;                                                    //使能DataFlash写
    status = EraseBlock(Addr);	
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //进入安全模式
    GLOBAL_CFG &= ~bDATA_WE;                                                   //开启DataFlash写保护
    return status;
}

/*******************************************************************************
* Function Name  : WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
* Description    : DataFlash写
* Input          : UINT16 Addr，PUINT16 buf,UINT8 len
* Output         : None
* Return         : 
*******************************************************************************/
void WriteDataFlash(UINT16 Addr,PUINT8 buf,UINT8 len)
{
    UINT8 j;
    UINT16 Tmp;

    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //进入安全模式
    GLOBAL_CFG |= bDATA_WE;                                                    //使能DataFlash写
    for(j=0;j<len;j=j+2)
    {
        Tmp = buf[j+1];Tmp <<= 8;
        Tmp |= buf[j];
        ProgWord(Addr,Tmp);
        Addr = Addr + 2;
    }
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;                                                           //进入安全模式
    GLOBAL_CFG &= ~bDATA_WE;                                                   //开启DataFlash写保护
}