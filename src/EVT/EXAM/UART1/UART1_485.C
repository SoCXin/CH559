/********************************** (C) COPYRIGHT *******************************
* File Name          : UART1_485.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/06/24
* Description        : �ṩUART1�����ú������ṩ��ѯ��ʽ���������շ����Լ�����FIFO��ʹ��ע������ 
                       UART1��485ģʽ�շ����ݣ��������ð�˫��
											 ��ʾʱ��Ҫ����������CH559�Ĵ����������ݣ�
*******************************************************************************/                                            
#include "..\DEBUG.C"                                                         //������Ϣ��ӡ
#include "..\DEBUG.H"

#pragma  NOAREGS

#define CH559UART1_BPS    57600                                               /*����CH559����1ͨѶ������*/
#define CH559UART1_FIFO_EN   1                                                //ʹ��CH559����1����FIFO(���շ��͸�8�ֽ�)

#if CH559UART1_FIFO_EN
UINT8 CH559UART1_FIFO_CNT;
#define  CH559UART1_FIFO_TRIG  7                                              //FIFO��7�ֽڴ����ж�(1��2��4��7��ѡ)
#endif

UINT8 Str[] = {"hello world1234!"};

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
    SER1_IER |= /*bIER_MODEM_CHG | */bIER_LINE_STAT | bIER_THR_EMPTY | bIER_RECV_RDY;//�ж�ʹ������
 
#if  CH559UART1_FIFO_EN
    SER1_FCR |= MASK_U1_FIFO_TRIG | bFCR_T_FIFO_CLR | bFCR_R_FIFO_CLR | bFCR_FIFO_EN;//FIFO������
                                                                               //��ս��ա�����FIFO��7�ֽڽ��մ�����FIFOʹ��
#endif
    SER1_MCR |= bMCR_OUT2;                                                     //MODEM���ƼĴ���
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


#if CH559UART1_FIFO_EN
/*******************************************************************************
* Function Name  : CH559UART1Rcv(PUINT8 buf,UINT8 len)
* Description    : CH559UART1���ն��ֽ�,���뿪FIFO
* Input          : PUINT8 buf  ���ݴ洢������
                   UINT8 len   ����Ԥ���ճ���
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART1Rcv(PUINT8 buf,UINT8 len)
{
    UINT8 i,j;
    j = 0;
    while(len)
    {
         if(len >= CH559UART1_FIFO_TRIG)//Ԥ���ճ��ȳ���FIFO���մ�����
         {
             while((SER1_IIR & U1_INT_RECV_RDY) == 0);//�ȴ����ݿ����ж�
             for(i=0;i<CH559UART1_FIFO_TRIG;i++)
             {
                 *(buf+j) = SER1_RBR; 
                 j++;
             }
             len -= CH559UART1_FIFO_TRIG;             
         }
         else
         {
             while((SER1_LSR & bLSR_DATA_RDY) == 0);//�ȴ�����׼����
             *(buf+j) = SER1_RBR; 
             j++;
             len--;              
         }    
    }

}
#endif

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
* Function Name  : CH559UART1SendStr(PUINT8 SendStr)
* Description    : CH559UART1���Ͷ���ֽ�
* Input          : UINT8 SendStr ��Ҫ���͵����ݵ��׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void CH559UART1SendStr(PUINT8 SendStr)
{
#if CH559UART1_FIFO_EN
    while(1)
    {
        if(SER1_LSR & bLSR_T_FIFO_EMP) CH559UART1_FIFO_CNT=8;//FIFO�տ����������8�ֽ�
        while( CH559UART1_FIFO_CNT!=0 ) 
        {
            if( *SendStr == '\0' ) break;//�������
            SER1_THR = *SendStr++;
            CH559UART1_FIFO_CNT--;//FIFO����
        }
        if( *SendStr == '\0' ) break;//�������
        while ( (SER1_LSR & bLSR_T_FIFO_EMP) == 0 );//����FIFO����ֻ�ܵȴ�ǰ��1�ֽڷ������
		}         
#else
    while( *SendStr != '\0' )
    {
        SER1_THR = *SendStr++;
        while ( (SER1_LSR & bLSR_T_ALL_EMP) == 0 );//û��FIFO���ȴ�1�ֽڷ������
    }
#endif	
}

main( ) 
{
   UINT8 i,j;
   UINT16 cnt;
   UINT8 buffer[20];
   UINT8 tmp[16]={0x13,0xab,0x5f,0x9d,0x32,0xde,0x56,0xaa,0x23,0x28,0x36,0x48,0x59,0x46,0x96,0xdd};
//  CfgFsys( );                                                                //ʱ������   
//  mDelaymS(5);                                                              //�ȴ��ⲿ�����ȶ� 	 	
    
    mInitSTDIO( );                                                             //����0,�������ڵ���
    printf("start ...\n");
	
    CH559UART1Init(13,0,2);
//  UART1RegCfgValue( );                                                       //UART1�Ĵ�������
    P4_DIR |= 0xff;                                                            //ʹ��P4��ʱ��һ��Ҫ���÷���,TXD1��Ϊ���
    P4_OUT = 0xff;
    cnt = 0;
/*��ѯ��ʽ*/
   while(1)
   {
#if 0
       CH559UART1SendByte(0x13);//���ݷ���
       CH559UART1SendByte(0xab);
       CH559UART1SendByte(0x5f);
       CH559UART1SendByte(0x9d);
       CH559UART1SendByte(0x32);
       CH559UART1SendByte(0xde);
       CH559UART1SendByte(0x56);
       CH559UART1SendByte(0xAA);
       CH559UART1SendByte(0x23);
       CH559UART1SendByte(0x28);
       CH559UART1SendByte(0x36);
       CH559UART1SendByte(0x48);
       CH559UART1SendByte(0x59);
       CH559UART1SendByte(0x46);
       CH559UART1SendByte(0x96);
       CH559UART1SendByte(0xDD);
//     CH559UART1SendStr(Str);
       mDelaymS(500);
       P4_OUT = ~P4_OUT;
#endif

#if 1
#if CH559UART1_FIFO_EN//���ݽ���
       CH559UART1Rcv(buffer,16);
       for(j=0;j<16;j++) 
       {
           if(tmp[j] != buffer[j]) 
           {
              cnt++;
              printf("Error CNT: %d     ",cnt);
              for(i = 0;i < 16;i++)
              {
                  printf("%02X  ",(UINT16)buffer[i]);
              }
              printf("\n");
							PWM1_ = 0;//���ݳ���
              break;
           }
           else
           {
              RXD1_ = ~RXD1_;//P4.0����������
           }
       }
       for(i = 0;i < 16;i++)
       {
           printf("%02X  ",(UINT16)buffer[i]);				 
       }
       printf("\n");
#else
       for(i=0;i<16;i++)
			 {
           buffer[i] = CH559UART1RcvByte();
       }
       for(i = 0;i < 16;i++)
       {
           printf("%02X  ",(UINT16)buffer[i]);
       }
       printf("\n");
#endif
#endif
   }
}