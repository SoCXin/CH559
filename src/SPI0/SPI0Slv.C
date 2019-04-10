
/********************************** (C) COPYRIGHT *******************************
* File Name          : SPI0Slv.C
* Author             : WCH
* Version            : V1.3
* Date               : 2016/06/24
* Description        : CH559提供SPI0从机模式操作接口函数 
注：片选有效时，从机会自动加载SPI0_S_PRE的预置值到发送移位缓冲区，所以最好可以在片选
有效前向SPI0_S_PRE寄存器写入预发值，或者在主机端丢弃首个接收字节，发送时注意主机会先
取走SPI0_S_PRE里面的值产生一个S0_IF_BYTE中断。
如果片选从无效到有效，从机首先进行发送的话，最好把输出的首字节放到SPI0_S_PRE寄存器中；
如果已经处于片选有效的话，数据数据使用SPI0_DATA就可以了。
*******************************************************************************/
#include "..\DEBUG.C"                                                          //调试信息打印
#include "..\DEBUG.H"

#pragma  NOAREGS

#define SPI0Interrupt   0                                                      //设定SPI0数据收发中断方式或者查询方式
#define CH559SPI0SlvWrite(dat)  {SPI0_DATA = dat;while(S0_IF_BYTE == 0);S0_IF_BYTE = 0;}
#define CH559SPI0SlvRead(dat)    {while(S0_IF_BYTE == 0);dat = SPI0_DATA;S0_IF_BYTE = 0;}
UINT8 Num;
UINT8X buf[200];

/*硬件接口定义*/
/******************************************************************************
使用CH559 硬件SPI接口 
         CH559        DIR       
         P1.4        <==>       SCS
         P1.5        <==>       MOSI
         P1.6        <==>       MISO
         P1.7        <==>       SCK
*******************************************************************************/

/*******************************************************************************
* Function Name  : CH559SPI0InterruptInit()
* Description    : CH559SPI0中断初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH559SPI0InterruptInit()
{
    //IP_EX |= bIP_SPI0;                                                       //SPI0中断优先级设置
    //SPI0_SETUP |= bS0_IE_FIRST;                                              //使能首字节接收中断	
    //一般用于首字节是命令码
    SPI0_SETUP |=  bS0_IE_BYTE;                                                //使能接收1字节中断，使能FIFO溢出中断
    SPI0_CTRL |= bS0_AUTO_IF;                                                  //自动清S0_IF_BYTE中断标志
    SPI0_STAT |= 0xff;                                                         //清空SPI0中断标志
    IE_SPI0 = 1;                                                               //使能SPI0中断
}

/*******************************************************************************
* Function Name  : CH559SPI0Init()
* Description    : CH559SPI0初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CH559SPI0Init(void)
{
    SPI0_S_PRE = 0x59;                                                        //预置数据，用于SPI主机发出首字节命令码后得到的交换数据
    PORT_CFG &= ~bP1_OC;                                                 
    P1_DIR &= 0x0F;
    P1_PU &=  0x0F;                                                           //SCS,MISO,MOSI,SCK 引脚置为高阻输入,片选有效MISO自动使能输出
    SPI0_SETUP |= bS0_MODE_SLV;                                               //设置为设备模式，
    SPI0_SETUP &= ~	(bS0_BIT_ORDER);                                          //字节顺序为大端模式	
    SPI0_CTRL &= ~(bS0_MOSI_OE | bS0_SCK_OE);                                  //关闭SCK和MOSI输出
    SPI0_CTRL |=  bS0_MISO_OE;                                              
    SPI0_CTRL &= ~(bS0_MST_CLK | bS0_DATA_DIR | bS0_2_WIRE);                                
    SPI0_CTRL &= ~bS0_CLR_ALL;                                                //清空SPI0的FIFO,默认是1，必须置零才能发送数据
}

#if SPI0Interrupt
/*******************************************************************************S
* Function Name  : SPI0HostInterrupt(void)
* Description    : SPI0 从机模式中断服务程序
* Input          : None
* Output         : None
* Return         : UINT8 ret   
*******************************************************************************/
void	SPI0HostInterrupt( void ) interrupt INT_NO_SPI0 using 1                //* SPI0中断服务程序,使用寄存器组1
{
	  //if(S0_IF_FIRST)                                                        //如果首字节是命令码则进行相应处理
		//{}
    buf[Num] = SPI0_DATA;	
    S0_IF_BYTE = 0;                                                          // 清中断标志	
    Num++;
//  printf(" %02X\n",(UINT16)Num);
}
#endif

main( ) 
{
    UINT8 j;
    mDelaymS(30);                                                              //上电延时,等待内部晶振稳定
//  CfgFsys( );  
    
    P4_DIR |= bLED2;
    P3_DIR |= bTXD;
    mInitSTDIO( );                                                      //串口0,可以用于调试
    printf("start ...\n");  
    CH559SPI0Init();
    CH559SPI0InterruptInit(); 
#if SPI0Interrupt 
    EA  = 1;                                                            //使能全局中断
    j = 0;
    while(1)                                                            //中断方式接收
    {
	      if(Num == 10)
	      {
		        for(j = 0;j < 10;j++)
			      {
		            printf("  %02X",(UINT16)buf[j]);
			      }
	          Num = 0;
	      }
    } 
#endif 
    j = 0;
//     SPI0_S_PRE = 0;                                                    //从机数据发送
//     while(1)                                                             
//     {
//         CH559SPI0SlvWrite(j);
//         j++;
//     }	
    while(1)                                                              //从机接收数据
    {                                                                     //因为有打印，所以注意主机时钟速率
        CH559SPI0SlvRead(j);
        j++;
        printf("%02X  ",(UINT16)j);
    }			
    while(1);
}