/********************************** (C) COPYRIGHT *******************************
* File Name          : UART1_INT.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/06/24
* Description        : �ṩUART1�����ú������ṩ�жϷ�ʽ���������շ����Լ�����FIFO��ʹ��ע������    
                       �жϷ�ʽ�����շ�
*******************************************************************************/                                            
#include "..\DEBUG.C"                                                         //������Ϣ��ӡ
#include "..\DEBUG.H"

#pragma  NOAREGS

#define CH559UART1_BPS    4800                                               /*����CH559����1ͨѶ������*/
#define CH559UART1_FIFO_EN   1                                               //ʹ��CH559����1����FIFO(���շ��͸�8�ֽ�)

#if CH559UART1_FIFO_EN
UINT8 CH559UART1_FIFO_CNT = 0;
UINT8 UART1SendLen = 0;                                                       //���ݷ��ͼ���
#define  CH559UART1_FIFO_TRIG  7                                              //FIFO��7�ֽڴ����ж�(1��2��4��7��ѡ)
#endif

UINT8 Str[] = {"hello world!"};
UINT8 buffer[20];
UINT8 Num;                                                                    //���ռ���

/*******************************************************************************
* Function Name  : UART1RegCfgValue()
* Description    : CH559UART1�ɶ��Ĵ�����ȷ���ú��ֵ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  UART1RegCfgValue( )
{
    printf("SER1_IER %02X\n",(UINT16)SER1_IER);                              //0x27/0x17/0x37���ǿ��ܵ�
    printf("SER1_IIR %02X\n",(UINT16)SER1_IIR);                              //0xc1/0xC2���жϻ��߿��жϣ�"C"��ʾFIFO����
    printf("SER1_LCR %02X\n",(UINT16)SER1_LCR);                              //0x03���ݸ�ʽ�����ã���ʾ����·�������У�飬1λֹͣλ��8λ����λ
    printf("SER1_MCR %02X\n",(UINT16)SER1_MCR);                              //0x08�ж����ʹ�ܣ����������ؿ�������������
    printf("SER1_LSR %02X\n",(UINT16)SER1_LSR);                              //0x60��FIFO����·״̬
    printf("SER1_MSR %02X\n",(UINT16)SER1_MSR);
}

/*******************************************************************************
* Function Name  : ResetUART1( )
* Description    : CH559UART1�˿�����λ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ResetUART1( )
{
    SER1_IER |= bIER_RESET;                                                   //��λ�����Զ���գ���λ���ڼĴ���
}

/*******************************************************************************
* Function Name  : CH559UART1Init(UINT8 DIV,UINT8 mode,UINT8 pin)
* Description    : CH559 UART1��ʼ������
* Input          : 
                   UINT8 DIV���÷�Ƶϵ����ʱ��Ƶ��=Fsys/DIV,DIV����Ϊ0
                   UINT8 mode��ģʽѡ��1����ͨ����ģʽ��0:485ģʽ
                   UINT8 pin����������ѡ��
                   ��mode=1ʱ
                   0��RXD1=P4.0,TXD1�رգ�
                   1��RXD1&TXD1=P4.0&P4.4��
                   2��RXD1&TXD1=P2.6&P2.7��
                   3��RXD1&TXD1&TNOW=P2.6&P2.7&P2.5��
                   ��mode=0ʱ
                   0��������
                   1��P5.4&P5.5����485,TNOW=P4.4��
                   2��P5.4&P5.5����485��
                   3��P5.4&P5.5����485,TNOW=P2.5��
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART1Init(UINT8 DIV,UINT8 mode,UINT8 pin)
{
    UINT32 x;
    UINT8 x2; 

    SER1_LCR |= bLCR_DLAB;                                                    // DLABλ��1��дDLL��DLM��DIV�Ĵ���
    SER1_DIV = DIV;                                                           // Ԥ��Ƶ
    x = 10 * FREQ_SYS *2 / DIV / 16 / CH559UART1_BPS;                             
    x2 = x % 10;
    x /= 10;
    if ( x2 >= 5 ) x ++;                                                      //��������
    SER1_DLM = x>>8;
    SER1_DLL = x&0xff;
    SER1_LCR &= ~bLCR_DLAB;                                                   //DLABλ��0,��ֹ�޸�UART1�����ʺ�ʱ��
    if(mode == 1)                                                             //�ر�RS485ģʽ RS485_EN = 0,����ʡ��
    {
	      XBUS_AUX |=  bALE_CLK_EN;                                     
    }
    else if(mode == 0)                                                        //����RS485ģʽ RS485_EN = 1;
    {
		    UHUB1_CTRL |= bUH1_DISABLE;                                   
		    PIN_FUNC &= ~bXBUS_CS_OE;
		    PIN_FUNC |= bXBUS_AL_OE;
		    XBUS_AUX &= ~bALE_CLK_EN;	
		    SER1_MCR |= bMCR_HALF;                                                //485ģʽֻ��ʹ�ð�˫��ģʽ	    
    }
    SER1_LCR |= MASK_U1_WORD_SZ;                                              //��·����
    SER1_LCR &= ~(bLCR_PAR_EN | bLCR_STOP_BIT);                               //����·�������У�飬1λֹͣλ��8λ����λ

    SER1_MCR &= ~bMCR_TNOW;
    SER1_IER |= bIER_EN_MODEM_O;
    SER1_IER |= ((pin << 4) & MASK_U1_PIN_MOD);                                //����ģʽ����
    SER1_IER |= /*bIER_MODEM_CHG | */bIER_LINE_STAT | /*bIER_THR_EMPTY |*/ bIER_RECV_RDY;//�ж�ʹ������
 
#if  CH559UART1_FIFO_EN
    SER1_FCR |= MASK_U1_FIFO_TRIG | bFCR_T_FIFO_CLR | bFCR_R_FIFO_CLR | bFCR_FIFO_EN;//FIFO������
                                                                               //��ս��ա�����FIFO��7�ֽڽ��մ�����FIFOʹ��
#endif
    SER1_MCR |= bMCR_OUT2;                                                     //MODEM���ƼĴ���
    x2 = SER1_IIR;                                                             //��IIR���ж�
                                                                               //�ж����������������ʵ���ж�
    SER1_ADDR |= 0xff;                                                         //�رն��ͨ��
}

/*******************************************************************************
* Function Name  : CH559UART1RcvByte()
* Description    : CH559UART1����һ���ֽ�
* Input          : None
* Output         : None
* Return         : ��ȷ��UINT8 Rcvdat;��������
*******************************************************************************/
UINT8  CH559UART1RcvByte( )
{
    while((SER1_LSR & bLSR_DATA_RDY) == 0);                                   //�ȴ�����׼����
    return SER1_RBR;
}

/*******************************************************************************
* Function Name  : CH559UART1SendByte(UINT8 SendDat)
* Description    : CH559UART1����һ���ֽ�
* Input          : UINT8 SendDat��Ҫ���͵�����
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART1SendByte(UINT8 SendDat)
{
#if CH559UART1_FIFO_EN
    while(1)
    {
        if(SER1_LSR & bLSR_T_FIFO_EMP) CH559UART1_FIFO_CNT=8;//FIFO�տ����������8�ֽ�
        if ( CH559UART1_FIFO_CNT!=0 ) 
        {
            SER1_THR = SendDat;
            CH559UART1_FIFO_CNT--;//FIFO����
            break;
        }
        while ( (SER1_LSR & bLSR_T_FIFO_EMP) == 0 );//����FIFO����ֻ�ܵȴ�ǰ��1�ֽڷ������
		}         
#else
    while ( (SER1_LSR & bLSR_T_ALL_EMP) == 0 );//û��FIFO���ȴ�1�ֽڷ������
    SER1_THR = SendDat;
#endif
}

/*******************************************************************************
* Function Name  : CH559UART1INTSendStr(PUINT8 SendStr)
* Description    : CH559UART1���Ϳ��жϷ��Ͷ���ֽ�
* Input          : UINT8 SendStr ��Ҫ���͵����ݵ��׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART1INTSendStr(PUINT8 SendStr)
{
    PUINT8  tmp;
    tmp = SendStr+UART1SendLen;
	
#if CH559UART1_FIFO_EN
	
    CH559UART1_FIFO_CNT=CH559UART1_FIFO_TRIG;//FIFO�տ����������7�ֽ�
    while( CH559UART1_FIFO_CNT!=0 ) 
    {
        if( *tmp == '\0' )//�������
        {
            UART1SendLen = 0;
            break;
        }
        SER1_THR = *tmp++;
        UART1SendLen++;
        CH559UART1_FIFO_CNT--;//FIFO����
    }
		
#else
		
    if( *tmp != '\0' )//��������
    {
        SER1_THR = *tmp++;
        UART1SendLen += 1;  
    }
    else//�������
 		{
        UART1SendLen = 0;        
		}
		
#endif	
}

/*******************************************************************************
* Function Name  : UART1Interrupt(void)
* Description    : UART1�жϷ������ 
*******************************************************************************/
void	UART1Interrupt( void ) interrupt INT_NO_UART1 using 1                 //UART1�жϷ������,ʹ�üĴ�����1
{
    UINT8 InterruptStatus,i; 
    InterruptStatus = SER1_IIR & 0x0f;                                        //��ȡ�ж�״̬ 
//     printf("InterruptStatus %02X\n",(UINT16)InterruptStatus);
    switch(InterruptStatus)
    {
        case U1_INT_RECV_RDY:                                                 //�������ݿ����жϣ������ȶ�ȡָ���ֽ��������жϵ����ݸ���
#if CH559UART1_FIFO_EN
			       for(i = 0;i < CH559UART1_FIFO_TRIG;i++)                          //ֱ�Ӷ�ȡ��������ֽ���
			       {
                  if(Num >= 19)
                  {
                      Num = 19;
                  }								 
				          buffer[Num] = SER1_RBR;										
				          Num++;
			       }
#else
             if(Num >= 19)
             {
                 Num = 19;
             }						 
             buffer[Num] = SER1_RBR;                                          //FIFOû����ȡ1���ֽ�
				     Num++;
#endif
			   break;				
        case U1_INT_RECV_TOUT:                                                //���ճ�ʱ�ж�
             if(Num >= 19)
             {
                 Num = 19;
             }					
             buffer[Num] = CH559UART1RcvByte( );
				     Num++;
			    break;		
        case U1_INT_LINE_STAT:                                                //��·״̬�ж�
			   break;		
        case U1_INT_SLV_ADDR:                                                 //�豸��ַmatch�ж�
			   break;			
        case U1_INT_NO_INTER:                                                 //���ж�
          break;		
        case U1_INT_MODEM_CHG:                                                //MODEM�ж�
		         i = SER1_MSR;
         break;	
        case U1_INT_THR_EMPTY:                                                //���Ϳ��жϣ����������´η��ͻ��ߵȴ�����
             CH559UART1INTSendStr(Str);
             if(UART1SendLen == 0)//�������
             {
                 SER1_IER &= ~bIER_THR_EMPTY;                                     //�����������ֶ�ʹ�ܷ��Ϳ��жϣ����Խ������ݷ���	
             }
			   break;
        default:
         break;
	}                                                                    
}

main( ) 
{
    UINT8 i;
//  CfgFsys( );                                                                //ʱ������   
//  mDelaymS(5);                                                              //�ȴ��ⲿ�����ȶ� 	 		
    
    mInitSTDIO( );                                                             //����0,�������ڵ���
    printf("start ...\n");

/*�жϷ�ʽ*/
    IE_UART1 = 1;                                                              //UART1�ж�ʹ��
    EA = 1;                                                                    //��ȫ���ж�
    Num = 0;
	
    CH559UART1Init(13,1,2);
    UART1RegCfgValue( );                                                       //UART1�Ĵ�������
    P4_DIR |= 0x10;                                                            //ʹ��P4��ʱ��һ��Ҫ���÷���,TXD1��Ϊ���

    while(1)
    {
        if(Num >= 19)
        {
            for(i = 0;i < 20;i++)
            {
                printf("%02X  ",(UINT16)buffer[i]);				 
            }
            SER1_IER |= bIER_THR_EMPTY;                                       //�����������ֶ�ʹ�ܷ��Ϳ��жϣ����Խ������ݷ���	
            Num = 0;
				}
    }
}