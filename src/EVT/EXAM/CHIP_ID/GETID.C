
/********************************** (C) COPYRIGHT *******************************
* File Name          :GETID.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/06/24
* Description        :��ȡоƬΨһID�ź�ID��У�鹦��
                      ROM_CHIP_ID_ADDR��ʼ4�ֽ�ID�ţ�������2�ֽ���ID��У��
*******************************************************************************/

#include "..\DEBUG.C"                                                       //������Ϣ��ӡ
#include "..\DEBUG.H"
#include <stdio.h>
#include <string.h>

#pragma NOAREGS

#define ROM_CHIP_ID_ADDR 0x20

/*******************************************************************************
* Function Name  : GetChipID(void)
* Description    : ��ȡID�ź�ID�ź�У��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
UINT32 GetChipID( void )
{
	UINT8	d0, d1;
	UINT16	xl, xh;
	E_DIS = 1;                                                                  //��������ж�
	d0 = *(PUINT8C)( ROM_CHIP_ID_ADDR + 0 );
	d1 = *(PUINT8C)( ROM_CHIP_ID_ADDR + 1 );                                    //ID�ŵ���
	xl = ( d1 << 8 ) | d0;
	d0 = *(PUINT8C)( ROM_CHIP_ID_ADDR + 2 );
	d1 = *(PUINT8C)( ROM_CHIP_ID_ADDR + 3 );                                    //ID�Ÿ���
	xh = ( d1 << 8 ) | d0;
	d0 = *(PUINT8C)( ROM_CHIP_ID_ADDR + 6 );
	d1 = *(PUINT8C)( ROM_CHIP_ID_ADDR + 7 );                                    //IDУ���
	E_DIS = 0;
	if ( (UINT16)( xl + xh ) != (UINT16)( ( d1 << 8 ) | d0 ) ) return( 0xFFFFFFFF );//У��ID��
	return( ( (UINT32)xh << 16 ) | xl );
}

/*******************************************************************************
* Function Name  : CopyChipID(void)
* Description    : ��ȡID�ţ���ΪFlash˫�ֽڷ��ʣ����ֽ���ǰ��ʹ��ʱҪע��
* Input          : PUINT32X buf
* Output         : None
* Return         : None
*******************************************************************************/
void CopyChipID( PUINT32X buf )
{
	E_DIS = 1;
	*( (PUINT16X)buf + 0 ) = *(const unsigned short code *)( ROM_CHIP_ID_ADDR + 0 );
	*( (PUINT16X)buf + 1 ) = *(const unsigned short code *)( ROM_CHIP_ID_ADDR + 2 );
	E_DIS = 0;
}

void main()
{
    UINT32 x;
//  CfgFsys( );    
//  mDelaymS(5);                                                               //�ȴ��ⲿ�����ȶ�
    
    mInitSTDIO( );                                                             //����0,�������ڵ���
    printf( "Start @ChipID=%02X\n", (UINT16)CHIP_ID );	
    printf("ID+CRC:%lx\n",GetChipID());
    CopyChipID(&x);
    printf("ID:%lx\n",x);	
    while(1);
}