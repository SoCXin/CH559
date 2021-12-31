/********************************** (C) COPYRIGHT *******************************
* File Name          : Timer3.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/06/24
* Description        : CH559 TIME3�ӿں���                				   
*******************************************************************************/
#include ".\DEBUG.C"                                                          //������Ϣ��ӡ
#include ".\DEBUG.H"

#pragma  NOAREGS

UINT8V FLAG;
UINT16 Cap[8];

#define TIMER3    0
#define T3PWM3    0
#define T3CAP3    1

#define mTimer3Stop( )     {T3_CTRL &= ~bT3_CNT_EN;}                               //�رն�ʱ��3
#define mTimer3Start( )    {T3_CTRL |= bT3_CNT_EN;}                                //��ʼ��ʱ��/������3
#define mTimer3Init( dat ) {T3_END_L = dat & 0xff;T3_END_H = (dat >> 8) & 0xff;}   //T3 ��ʱ������ֵ

/*******************************************************************************
* Function Name  : mSetTimer3Clk(UINT16 DIV)
* Description    : CH559 Timer3ʱ������
* Input          : UINT16 ��Ƶϵ��
* Output         : None
* Return         : None
*******************************************************************************/
void mSetTimer3Clk(UINT16 DIV)
{
    T3_SETUP |= bT3_EN_CK_SE; 
    T3_CK_SE_L = DIV & 0xff;
    T3_CK_SE_H = (DIV >> 8) & 0xff;
    T3_SETUP &= ~bT3_EN_CK_SE;  
}

/*******************************************************************************
* Function Name  : void mTimer3Setup( )
* Description    : CH559��ʱ������3��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer3Setup( )
{
    T3_CTRL |= bT3_CLR_ALL;                                                    //���FIFO�ͼ���
    T3_CTRL &= ~bT3_CLR_ALL;
    T3_SETUP |= bT3_IE_END;                                                    //ʹ�ܶ�ʱ��/������3FIFO����ж�
    T3_CTRL &= ~bT3_MOD_CAP;                                                   //����Ϊ�����ڶ�ʱ��ģʽ
    T3_STAT |= bT3_IF_DMA_END | bT3_IF_FIFO_OV | bT3_IF_FIFO_REQ | bT3_IF_ACT | bT3_IF_END;//��ռĴ��������ж�
}

/*******************************************************************************
* Function Name  : mTimer3PWMSetup( )
* Description    : CH559��ʱ������PWM3��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer3PWMSetup( )
{
    PORT_CFG &= ~bP1_DRV;
    P1_DIR |= bPWM3;                                                           //����PWM3����Ϊ����ģʽ
    T3_CTRL |= bT3_CLR_ALL;                                                    //���FIFO�ͼ���
    T3_CTRL &= ~bT3_CLR_ALL;
    T3_SETUP |= bT3_IE_END | bT3_IE_ACT;
    T3_CTRL |= bT3_OUT_EN;
}

/*******************************************************************************
* Function Name  : mTimer3PWMPolarSel(UINT8 mode)
* Description    : 
* Input          : UINT8 mode,PWM��Ч��ƽ���ѡ��
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer3PWMPolarSel(UINT8 mode)
{
    if(mode)
    {
        T3_CTRL |= bT3_PWM_POLAR;                                             //����PWM ������ԣ��͵�ƽ
    }
    else
    {
        T3_CTRL &= ~bT3_PWM_POLAR;                                            //����PWM ������ԣ��ߵ�ƽ
    }	
}
/*******************************************************************************
* Function Name  : mTimer3PWMDatInit(UINT16 dat0,UINT16 dat1)
* Description    : 
* Input          : UINT16 dat0,PWM����ʱ��
                   UINT16 dat1,PWM��Ч��ƽ���ʱ��
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer3PWMDatInit(UINT16 dat0,UINT16 dat1)
{
    T3_END_L = dat0 & 0xff;		                                                 //����ռ�ձ�
    T3_END_H = (dat0 >> 8) & 0xff;
    T3_FIFO_L = dat1 & 0xff;
    T3_FIFO_H = (dat1 >> 8) & 0xff;
}

/*******************************************************************************
* Function Name  : mTimer3CaptureSetup(UINT16 dat0,UINT8 mode)
* Description    : CH559��ʱ������3��׽���ܳ�ʼ��CAP3
* Input          : UINT16 dat0,����ʱʱ���趨
				           UINT8 mode,����ģʽѡ��
				           0���رղ���
				           1���������ص�������֮��Ĳ���
				           2�����½��ص��½���֮��Ĳ���
				           3���������ص�������֮��Ĳ���
* Output         : None
* Return         : None
*******************************************************************************/
void mTimer3CaptureSetup(UINT16 dat0,UINT8 mode)
{
    T3_CTRL = 0;
    T3_CTRL |= bT3_CLR_ALL;                                                   //���T3��ؼĴ���
    T3_CTRL &= ~bT3_CLR_ALL;

    T3_SETUP |=  bT3_IE_ACT;
    T3_CTRL |= bT3_MOD_CAP | bT3_CAP_WIDTH;
    T3_CTRL |= mode << 6;
    T3_END_L = dat0 & 0xff;                                                    //��������ʱʱ�䣬��ʱʱ��time = Pclk*max_time, PclkΪϵͳʱ��
    T3_END_H = (dat0 >> 8) & 0xff;
    T3_FIFO_L = 0;                                                             //���
    T3_FIFO_H = 0;
    T3_STAT = 0xF0;                                                            //���ж�
}

/*******************************************************************************
* Function Name  : mTimer3Interrupt()
* Description    : CH559��ʱ������3��ʱ�������жϴ�������
*******************************************************************************/
void	mTimer3Interrupt( void ) interrupt INT_NO_TMR3 using 2                 //timer3�жϷ������,ʹ�üĴ�����1
{
  mTimer3Stop( );
#if TIMER3
    if(T3_STAT & bT3_IF_END)                                                   //�����ж�
    { 
        T3_STAT |= bT3_IF_END;
    }
#endif

#if T3CAP3	
    if(T3_STAT & bT3_IF_ACT)                                                   //��׽�ж�
    {
        printf("Cap:%04X  ",(UINT16)T3_FIFO);        
//         Cap[FLAG++] = T3_FIFO;
        T3_STAT |= bT3_IF_ACT;
    }
#endif

//	 if(T3_STAT & bT3_IF_DMA_END)
//   	{
//	    T3_STAT |= bT3_IF_DMA_END;
//	  }
mTimer3Start();
}

main( ) 
{
    UINT8 i,tmp;
//  CfgFsys( );   
    mDelaymS(5);                                                               //�ȴ��ⲿ�����ȶ�		
    
    mInitSTDIO( );                                                             //����0,�������ڵ���
    printf("start ...\n");
	
    mSetTimer3Clk(0x02);                                                       //T3ʱ������	
/* ������ʱ�� */
#if TIMER3          
    mTimer3Setup( );                                                           //��ʱ����������  
    mTimer3Init( 0x2000 );                                                     //T3 ��ʱ������ֵ
    mTimer3Start( );                                                           //��ʼ��ʱ��/������3
    IE_TMR3 = 1;                                              	
    EA = 1; 
    while(1);
#endif
	
/* PWM3��ʾ */
#if T3PWM3			
    PORT_CFG &= ~bP1_OC;                                                        //P1.2����PWM��Ϊ�������ģʽ
    P1_DIR |= bPWM3;
    P1_PU |= bPWM3;
    mTimer3PWMSetup( );                                                         //PWM3�������� 
    mTimer3PWMPolarSel(1);                                                      //PWM3����ߵͼ���ѡ��
    mTimer3PWMDatInit(12,3);                                                    //PWM���ռ�ձ�����
    mTimer3Start( );
    IE_TMR3 = 1;                                       
    EA = 1;
    while(1);
#endif
 
 /* CAP3��ʾ */
#if T3CAP3	
    PIN_FUNC |= bTMR3_PIN_X;
    PORT_CFG |= bP1_OC;                                                         //CAP3��Ϊ׼˫��ģʽģʽ
    P1_DIR |= bCAP3;
    P1_PU |= bCAP3;
	
    FLAG = 0;
    mTimer3CaptureSetup(0x4000,1);				                                      //CAP3������
    mTimer3Start( );                                                            //��ʼ��ʱ��/������3
    EA = 1;
    IE_TMR3 = 1;	
		while(1);
	
    for(i=0; i<6; i++)
    {
        mDelayuS(300);
        CAP3 = !CAP3;													                                   //ģ��CAP3���ŵ�ƽ�仯	
    }
	
    mTimer3Stop();
    IE_TMR3 = 0;
    EA = 1;
	
    printf("FLAG:%02X  \n",(UINT16)FLAG);
    for(i = 0;i < FLAG;i++)
    {
        printf("Cap[%02X]:%04X  ",(UINT16)i,(UINT16)Cap[i]);
    }
    printf("\n");
    tmp = T3_STAT&MASK_T3_FIFO_CNT;	
    for(i = 0;i < tmp;i++)
    {
        printf("%04X  ",T3_FIFO);
    }
    printf("T3_STAT:%02X  \n",(UINT16)T3_STAT);	
#endif
    while(1);
}

/* 
1.��ǰTIME3��ʱ/���������Ǵ�0������T3_END��ʱ�䣬Ȼ������жϣ�
2.PWMģʽ�£�ռ�ձ�=T3_FIFO/T3_END
3.��׽ģʽ����ǰFIFO�е���ֵ������Ҫ��׽����ʱ�䣻
*/