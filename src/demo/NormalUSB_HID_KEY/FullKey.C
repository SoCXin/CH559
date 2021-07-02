/********************************** (C) COPYRIGHT *******************************
* File Name          :Mouse.C
* Author             : WCH
* Version            : V1.0
* Date               : 2015/05/20
* Description        : CH559模拟键盘，支持类命令
*******************************************************************************/
#include "DEBUG.C"                                                       //调试信息打印
#include "DEBUG.H"
#include <string.h>
#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE
UINT8X  Ep0Buffer[THIS_ENDP0_SIZE] _at_ 0x0000;                                //端点0 OUT&IN缓冲区，必须是偶地址
UINT8X  Ep1Buffer[MAX_PACKET_SIZE] _at_ 0x0008;                                //端点1 IN缓冲区,必须是偶地址
UINT8X  Ep2Buffer[2*MAX_PACKET_SIZE] _at_ 0x0050;                                //端点2 OUT缓冲区,必须是偶地址
UINT8   SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
PUINT8  pDescr;                                                                //USB配置标志
USB_SETUP_REQ   SetupReqBuf;                                                     //暂存Setup包
#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0
#define FLASHOFFECT  0

sbit ComboKey1 = P3^4;
#define ComboKey2  (P4_IN&0x40)
#define ComboKey3  (P4_IN&0x80)
/*设备描述符*/
UINT8C DevDesc[18] = {0x12, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08,
                      0x89, 0x11, 0x90, 0x88,                           /* 厂商ID和产品ID */
                      0x00, 0x01, 0x01, 0x02, 0x03, 0x01
                     };

/*字符串描述符*/
/*HID类报表描述符*/
UINT8C KeyRepDesc[] =
{
//每行开始的第一字节为该条目的前缀，前缀的格式为：
 //D7~D4：bTag。D3~D2：bType；D1~D0：bSize。以下分别对每个条目注释。

/************************USB键盘部分报告描述符**********************/
/*******************************************************************/
 //这是一个全局（bType为1）条目，将用途页选择为普通桌面Generic Desktop Page(0x01)
 //后面跟一字节数据（bSize为1），后面的字节数就不注释了，
 //自己根据bSize来判断。
 0x05, 0x01, // USAGE_PAGE (Generic Desktop)

 //这是一个局部（bType为2）条目，说明接下来的集合用途用于键盘
 0x09, 0x06, // USAGE (Keyboard)

 //这是一个主条目（bType为0）条目，开集合，后面跟的数据0x01表示
 //该集合是一个应用集合。它的性质在前面由用途页和用途定义为
 //普通桌面用的键盘。
 0xa1, 0x01, // COLLECTION (Application)

 //这是一个全局条目，选择用途页为键盘（Keyboard/Keypad(0x07)）
 0x05, 0x07, //     USAGE_PAGE (Keyboard/Keypad)

 //这是一个局部条目，说明用途的最小值为0xe0。实际上是键盘左Ctrl键。
 //具体的用途值可在HID用途表中查看。
 0x19, 0xe0, //     USAGE_MINIMUM (Keyboard LeftControl)

 //这是一个局部条目，说明用途的最大值为0xe7。实际上是键盘右GUI键。
 0x29, 0xe7, //     USAGE_MAXIMUM (Keyboard Right GUI)

 //这是一个全局条目，说明返回的数据的逻辑值（就是我们返回的数据域的值）
 //最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
 0x15, 0x00, //     LOGICAL_MINIMUM (0)

 //这是一个全局条目，说明逻辑值最大为1。
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)

 //这是一个全局条目，说明数据域的数量为八个。
 0x95, 0x08, //     REPORT_COUNT (8)

 //这是一个全局条目，说明每个数据域的长度为1个bit。
 0x75, 0x01, //     REPORT_SIZE (1)

 //这是一个主条目，说明有8个长度为1bit的数据域（数量和长度
 //由前面的两个全局条目所定义）用来做为输入，
 //属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
 //这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
 //这样定义的结果就是，当某个域的值为1时，就表示对应的键按下。
 //bit0就对应着用途最小值0xe0，bit7对应着用途最大值0xe7。
 0x81, 0x02, //     INPUT (Data,Var,Abs)

 //这是一个全局条目，说明数据域数量为1个
 0x95, 0x01, //     REPORT_COUNT (1)

 //这是一个全局条目，说明每个数据域的长度为8bit。
 0x75, 0x08, //     REPORT_SIZE (8)

 //这是一个主条目，输入用，由前面两个全局条目可知，长度为8bit，
 //数量为1个。它的属性为常量（即返回的数据一直是0）。
 //该字节是保留字节（保留给OEM使用）。
 0x81, 0x03, //     INPUT (Cnst,Var,Abs)

 //这是一个全局条目。定义位域数量为6个。
 0x95, 0x06, //   REPORT_COUNT (6)

 //这是一个全局条目。定义每个位域长度为8bit。
 //其实这里这个条目不要也是可以的，因为在前面已经有一个定义
 //长度为8bit的全局条目了。
 0x75, 0x08, //   REPORT_SIZE (8)

 //这是一个全局条目，定义逻辑最小值为0。
 //同上，这里这个全局条目也是可以不要的，因为前面已经有一个
 //定义逻辑最小值为0的全局条目了。
 0x15, 0x00, //   LOGICAL_MINIMUM (0)

 //这是一个全局条目，定义逻辑最大值为255。
 0x25, 0xFF, //   LOGICAL_MAXIMUM (255)

 //这是一个全局条目，选择用途页为键盘。
 //前面已经选择过用途页为键盘了，所以该条目不要也可以。
 0x05, 0x07, //   USAGE_PAGE (Keyboard/Keypad)

 //这是一个局部条目，定义用途最小值为0（0表示没有键按下）
 0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))

 //这是一个局部条目，定义用途最大值为0x65
 0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)

 //这是一个主条目。它说明这六个8bit的数据域是输入用的，
 //属性为：Data,Ary,Abs。Data说明数据是可以变的，Ary说明
 //这些数据域是一个数组，即每个8bit都可以表示某个键值，
 //如果按下的键太多（例如超过这里定义的长度或者键盘本身无法
 //扫描出按键情况时），则这些数据返回全1（二进制），表示按键无效。
 //Abs表示这些值是绝对值。
 0x81, 0x00, //     INPUT (Data,Ary,Abs)

 //以下为输出报告的描述
 //逻辑最小值前面已经有定义为0了，这里可以省略。
 //这是一个全局条目，说明逻辑值最大为1。
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)

 //这是一个全局条目，说明数据域数量为5个。
 0x95, 0x05, //   REPORT_COUNT (5)

 //这是一个全局条目，说明数据域的长度为1bit。
 0x75, 0x01, //   REPORT_SIZE (1)

 //这是一个全局条目，说明使用的用途页为指示灯（LED）
 0x05, 0x08, //   USAGE_PAGE (LEDs)

 //这是一个局部条目，说明用途最小值为数字键盘灯。
 0x19, 0x01, //   USAGE_MINIMUM (Num Lock)

 //这是一个局部条目，说明用途最大值为Kana灯。
 0x29, 0x05, //   USAGE_MAXIMUM (Kana)

 //这是一个主条目。定义输出数据，即前面定义的5个LED。
 0x91, 0x02, //   OUTPUT (Data,Var,Abs)

 //这是一个全局条目。定义位域数量为1个。
 0x95, 0x01, //   REPORT_COUNT (1)

 //这是一个全局条目。定义位域长度为3bit。
 0x75, 0x03, //   REPORT_SIZE (3)

 //这是一个主条目，定义输出常量，前面用了5bit，所以这里需要
 //3个bit来凑成一字节。
 0x91, 0x03, //   OUTPUT (Cnst,Var,Abs)

 //下面这个主条目用来关闭前面的集合。bSize为0，所以后面没数据。
 0xc0,        // END_COLLECTION
};

UINT8C  HIDRepDesc[]={
	                        0x05,0x01,                                               //报表描述符，每个条目占一行
												  0x09,0x00,
												  0xa1,0x01,
												  0x15,0x00,
												  0x25,0xff,
												  0x19,0x01,
												  0x29,0x08,
												  0x95,0x08,
												  0x75,0x08,
											 	  0x81,0x02,
												  0x09,0x02,
												  0x15,0x00,
												  0x25,0xff,
                          0x75,0x08,
                          0x95,0x40,
                          0x91,0x06,	
												  0xc0
                          };

UINT8C CfgDesc[] =
{
    0x09, 0x02, 0x42, 0x00, 0x02, 0x01, 0x00, 0xA0, 0x32,             //配置描述符
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,             //接口描述符,键盘
    0x09, 0x21, 0x00, 0x01, 0x21, 0x01, 0x22, sizeof(KeyRepDesc), 0x00,             //HID类描述符
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A,                         //端点描述符，中断上传
	  0x09,0x04,0x01,0x00,0x02,0x03,0x00,0x00,0x00,             //接口描述符，无协议码
	  0x09,0x21,0x00,0x01,0x00,0x01,0x22,sizeof(HIDRepDesc),0x00,             //HID类描述符    
	  0x07,0x05,0x82,0x03,0x08,0x00,0x0a,                       //端点描述符    
	  0x07,0x05,0x02,0x03,0x08,0x00,0x0a                       //端点描述符 	
};

UINT8C MyProductIDInfo[] = {0x0E,0x03,'k',0,'e',0,'y',0,'5',0,'5',0,'9',0};

/* 语言描述符 */
UINT8C MyLangDescr[ ] = { 0x04, 0x03, 0x09, 0x04 };

/* 厂家信息 */
UINT8C MyManuInfo[ ] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };

/* 产品信息 */
UINT8C MyProdInfo[ ] = { 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '5', 0, '9', 0 };

/*键盘数据*/
UINT8 HIDKey[9] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0};
UINT8 HIDKey1[9] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0};
UINT8 HIDKey2[9] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0};
UINT8 ChangeHIDKey[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
UINT8 DataBuffer[27];
/*******************************************************************************
* Function Name  : USBDeviceCfg()
* Description    : USB设备模式配置
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceCfg()
{
    USB_CTRL = 0x00;                                                           //清空USB控制寄存器
//     USB_CTRL &= ~bUC_HOST_MODE;                                                //该位为选择设备模式
    USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;                            //USB设备和内部上拉使能,在中断期间中断标志未清除前自动返回NAK
    USB_DEV_AD = 0x00;                                                         //设备地址初始化
    UDEV_CTRL &= ~bUD_RECV_DIS;                                                //使能接收器
    USB_CTRL |= bUC_LOW_SPEED;
    UDEV_CTRL |= bUD_LOW_SPEED;                                                //选择低速1.5M模式
//    USB_CTRL &= ~bUC_LOW_SPEED;
//    UDEV_CTRL &= ~bUD_LOW_SPEED;                                             //选择全速12M模式，默认方式
    UDEV_CTRL |= bUD_DP_PD_DIS | bUD_DM_PD_DIS;                                //禁止DM、DP下拉电阻
    UDEV_CTRL |= bUD_PORT_EN;                                                  //使能物理端口
}
/*******************************************************************************
* Function Name  : USBDeviceIntCfg()
* Description    : USB设备模式中断初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceIntCfg()
{
    USB_INT_EN |= bUIE_SUSPEND;                                               //使能设备挂起中断
    USB_INT_EN |= bUIE_TRANSFER;                                              //使能USB传输完成中断
    USB_INT_EN |= bUIE_BUS_RST;                                               //使能设备模式USB总线复位中断
    USB_INT_FG |= 0x1F;                                                       //清中断标志
    IE_USB = 1;                                                               //使能USB中断
    EA = 1;                                                                                                   //允许单片机中断
}
/*******************************************************************************
* Function Name  : USBDeviceEndPointCfg()
* Description    : USB设备模式端点配置，模拟键盘，除了端点0的控制传输，还包括端点1中断上传
                   端点2的批量下传
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceEndPointCfg()
{
    UEP1_DMA = Ep1Buffer;                                                      //端点1数据传输地址
    UEP4_1_MOD |= bUEP1_TX_EN;                                                 //端点1发送使能
    UEP4_1_MOD &= ~bUEP1_RX_EN;                                                 //端点1接收使能
    UEP4_1_MOD &= ~bUEP1_BUF_MOD;                                              //端点1发64字节缓冲区
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //端点1自动翻转同步标志位，IN事务返回NAK
	
    UEP2_DMA = Ep2Buffer;                                                      //端点1数据传输地址
    UEP2_3_MOD |= bUEP2_TX_EN;                                                 //端点1发送使能
    UEP2_3_MOD |= bUEP2_RX_EN;                                                 //端点1接收使能
    UEP2_3_MOD &= ~bUEP2_BUF_MOD;                                              //端点1发64字节缓冲区
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;								 //端点2自动翻转同步标志位，IN事务返回NAK，OUT返回ACK
	
    UEP0_DMA = Ep0Buffer;                                                      //端点0数据传输地址
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);                                //端点0单64字节收发缓冲区
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                                 //OUT事务返回ACK，IN事务返回NAK
}
/*******************************************************************************
* Function Name  : enp1IntIn()
* Description    : USB设备模式端点1的中断上传
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void enp1IntIn( PUINT8 HIDKeyBuf )
{
    memcpy( Ep1Buffer, HIDKeyBuf, 8);                           //加载上传数据
    UEP1_T_LEN = 8;                                             //上传数据长度
    UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                //有数据时上传数据并应答ACK
}
/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description    : CH559USB中断处理函数
*******************************************************************************/
void    DeviceInterrupt( void ) interrupt INT_NO_USB using 1                      //USB中断服务程序,使用寄存器组1
{
    UINT8 len;
    if(UIF_TRANSFER)                                                            //USB传输完成标志
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 1:                                                  //endpoint 1# 中断端点上传          
            UEP1_T_LEN = 0;                                                     //预使用发送长度一定要清空
//            UEP2_CTRL ^= bUEP_T_TOG;                                          //如果不设置自动翻转则需要手动翻转
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //默认应答NAK
            if(FLAG&1) FLAG++;							
            break;
         case UIS_TOKEN_IN | 2:                                                 //endpoint 2# 中断端点上传
              UEP2_T_LEN = 0;	                                                  //预使用发送长度一定要清空	
//            UEP1_CTRL ^= bUEP_T_TOG;                                          //如果不设置自动翻转则需要手动翻转
			        UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;         //默认应答NAK
			  break;
         case UIS_TOKEN_OUT | 2:                                                //endpoint 2# 中断端点下传
            if ( U_TOG_OK )                                                     // 不同步的数据包将丢弃
            {			
                  UEP2_CTRL &= ~UEP_R_RES_ACK;	
                  UEP2_CTRL |= UEP_R_RES_NAK;									
                len = USB_RX_LEN;                                               //接收数据长度，数据从Ep2Buffer首地址开始存放		
                if(Ep2Buffer[0] == 1){//配置K1
                   memcpy(HIDKey, Ep2Buffer, 8);		
                   memcpy(DataBuffer,HIDKey,8);		
#if DEBUG
for(len=0;len<27;len++){
printf("%02x  ",(UINT16)DataBuffer[len]);
}	
#endif									
                }
                else if(Ep2Buffer[0] == 2){//配置K2
                   memcpy(HIDKey1, Ep2Buffer, 8);		
                   memcpy(DataBuffer+9,HIDKey1,8);		
#if DEBUG
for(len=0;len<27;len++){
printf("%02x  ",(UINT16)DataBuffer[len]);
}	
#endif											
                }
                else if(Ep2Buffer[0] == 3){//配置K3
                   memcpy(HIDKey2, Ep2Buffer, 8);		
                   memcpy(DataBuffer+18,HIDKey2,8);		
#if DEBUG
for(len=0;len<27;len++){
printf("%02x  ",(UINT16)DataBuffer[len]);
}	
#endif										
                }
                else{}																								
            }		
            UEP2_CTRL &= ~UEP_R_RES_NAK;	
            UEP2_CTRL |= UEP_R_RES_ACK;	
        break;						
        case UIS_TOKEN_SETUP | 0:                                                //SETUP事务
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;    // 限制总长度
                }
                len = 0;                                                      // 默认为成功并且上传0长度
                SetupReq = UsbSetupBuf->bRequest;
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )/* 只支持标准请求 */
                {								
									switch( SetupReq ) 
									{
										case 0x01://GetReport
                         len = 1;	
                         Ep0Buffer[0] = 0xaa;										
												 break;
										case 0x02://GetIdle
												 break;	
										case 0x03://GetProtocol
												 break;				
										case 0x09://SetReport			
                        Ready = 1;                                       //如果有更多接口，该标准位应该在最后一个接口配置完成后有效											
												 break;
										case 0x0A://SetIdle
												 break;	
										case 0x0B://SetProtocol
												 break;
										default:
												 len = 0xFF;  								 					/*操作失败*/					
												 break;
								  }
                }
								else{
                switch(SetupReq)                                                  //请求码
                {
                case USB_GET_DESCRIPTOR:
                    switch(UsbSetupBuf->wValueH)
                    {
                    case 1:                                                 //设备描述符
                        pDescr = DevDesc;                               //把设备描述符送到要发送的缓冲区
                        len = sizeof(DevDesc);
                        break;
                    case 2:                                                                 //配置描述符
                        pDescr = CfgDesc;                               //把设备描述符送到要发送的缓冲区
                        len = sizeof(CfgDesc);
                        break;
                    case 3:                                            // 字符串描述符
                        switch( UsbSetupBuf->wValueL )
                        {
                          case 1:
                                pDescr = (PUINT8)( &MyManuInfo[0] );       
                                len = sizeof( MyManuInfo );
                                break;
                          case 2:
                                pDescr = (PUINT8)( &MyProdInfo[0] );        
                                len = sizeof( MyProdInfo );
                                break;
                          case 0:
                                pDescr = (PUINT8)( &MyLangDescr[0] );
                                len = sizeof( MyLangDescr );
                                break;
                          case 3:
                                pDescr = (PUINT8)( &MyProductIDInfo[0] );
                                len = sizeof( MyProductIDInfo );
                                break;													
                          default:
                                len = 0xFF;                                 // 不支持的字符串描述符
                                break;
						            }
                        break;										
                    case 0x22:                                            //报表描述符
											 if(UsbSetupBuf->wIndexL == 0)                   //接口0报表描述符
											 {																 
													 pDescr = KeyRepDesc;                        //数据准备上传
													 len = sizeof(KeyRepDesc);
											 }
											 else if(UsbSetupBuf->wIndexL == 1)              //接口1报表描述符
											 {																 
													 pDescr = HIDRepDesc;                        //数据准备上传
													 len = sizeof(HIDRepDesc);
													 Ready = 1; 																 
																																			 //如果有更多接口，该标准位应该在最后一个接口配置完成后有效
											 }	
											 else len = 0xff;                                //本程序只有2个接口，这句话正常不可能执行		
                    default:
                        len = 0xff;                                     //不支持的命令或者出错
                        break;
                    }
                    if ( SetupLen > len )
                    {
                        SetupLen = len;    //限制总长度
                    }
                    len = SetupLen >= 8 ? 8 : SetupLen;                            //本次传输长度
                    memcpy(Ep0Buffer,pDescr,len);                                  //加载上传数据
                    SetupLen -= len;
                    pDescr += len;
                    break;
                case USB_SET_ADDRESS:
                    SetupLen = UsbSetupBuf->wValueL;                              //暂存USB设备地址
                    break;
                case USB_GET_CONFIGURATION:
                    Ep0Buffer[0] = UsbConfig;
                    if ( SetupLen >= 1 )
                    {
                        len = 1;
                    }
                    break;
                case USB_SET_CONFIGURATION:
                    UsbConfig = UsbSetupBuf->wValueL;
                    break;
                case 0x0A:
                    break;
                case USB_CLEAR_FEATURE:                                            //Clear Feature
                    if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// 端点
                    {
                       switch( UsbSetupBuf->wIndexL )
                       {
                          case 0x82:
                               UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                               break;
                          case 0x81:
                               UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                               break;
                          case 0x01:
                               UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                               break;
                          default:
                               len = 0xFF;                                         // 不支持的端点
                               break;
                        }
                     }
                     else
                     {
                        len = 0xFF;                                                // 不是端点不支持
                     }
                    break;
                    case USB_SET_FEATURE:                                          /* Set Feature */
                    if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x00 )                  /* 设置设备 */
                    {
                        if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                        {
                            if( CfgDesc[ 7 ] & 0x20 )
                            {
                                 /* 设置唤醒使能标志 */
                            }
                            else
                            {
                                len = 0xFF;                                        /* 操作失败 */
                            }
                        }
                        else
                        {
                            len = 0xFF;                                            /* 操作失败 */
                        }
                    }
                    else if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x02 )             /* 设置端点 */
                    {
                        if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                        {
                            switch( ( ( UINT16 )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                            {
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点2 IN STALL */
                                    break;

                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* 设置端点2 OUT Stall */
                                    break;

                                case 0x81:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* 设置端点1 IN STALL */
                                    break;

                                default:
                                    len = 0xFF;                                    /* 操作失败 */
                                    break;
                            }
                        }
                        else
                        {
                            len = 0xFF;                                      /* 操作失败 */
                        }
                    }
                    else
                    {
                        len = 0xFF;                                          /* 操作失败 */
                    }
                    break;
                case USB_GET_STATUS:
                     Ep0Buffer[0] = 0x00;
                     Ep0Buffer[1] = 0x00;
                     if ( SetupLen >= 2 )
                     {
                        len = 2;
                     }
                     else
                     {
                        len = SetupLen;
                     }
                     break;								
                default:
                    len = 0xff;                                                    //操作失败
                    break;
                }
							}
            }
            else
            {
                len = 0xff;                                                         //包长度错误
            }
            if(len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
            }
            else if(len <= 8)                                                       //上传数据或者状态阶段返回0长度包
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA1，返回应答ACK
            }
            else
            {
                UEP0_T_LEN = 0;  //虽然尚未到状态阶段，但是提前预置上传0长度数据包以防主机提前进入状态阶段
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA1,返回应答ACK
            }
            break;
        case UIS_TOKEN_IN | 0:                                                      //endpoint0 IN
            switch(SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= 8 ? 8 : SetupLen;                                 //本次传输长度
                memcpy( Ep0Buffer, pDescr, len );                                   //加载上传数据
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;                                             //同步标志位翻转
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;                                                      //状态阶段完成中断或者是强制上传0长度数据包结束控制传输
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
            len = USB_RX_LEN;
            UEP0_T_LEN = 0;  //虽然尚未到状态阶段，但是提前预置上传0长度数据包以防主机提前进入状态阶段
            UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_ACK;//默认数据包是DATA0,返回应答ACK						
            if(SetupReq == 0x09)
            {
                if(Ep0Buffer[0])
                {
                    //printf("Light on Num Lock LED!\n");
                }
                else if(Ep0Buffer[0] == 0)
                {
                    //printf("Light off Num Lock LED!\n");
                }
            }				
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0;                                                           //写0清空中断
    }
    if(UIF_BUS_RST)                                                               //设备模式USB总线复位中断
    {
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;                                                              //清中断标志
    }
    if (UIF_SUSPEND)                                                                //USB总线挂起/唤醒完成
    {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND )                                              //挂起
        {
            while ( XBUS_AUX & bUART0_TX )
            {
                ;    //等待发送完成
            }
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO;                                     //USB或者RXD0有信号时可被唤醒
            PCON |= PD;                                                                 //睡眠
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = 0x00;
        }
    }
    else {                                                                         //意外的中断,不可能发生的情况
        USB_INT_FG = 0x00;                                                           //清中断标志
//      printf("UnknownInt  N");
    }
}

void HIDValueHandle()
{
    mDelaymS(2);                                                     //延时避免按键误按
    if ((ComboKey1 == 0)&&(HIDKey[0] != 0xFF)){
        FLAG = 0x11;					
//键盘数据上传示例 A
//         HIDKey[0] = 0x00;                                                
//         HIDKey[2] = 0x3a;                                                //F1
        enp1IntIn(HIDKey+1);
    }
    else if ((ComboKey2 == 0)&&(HIDKey1[0] != 0xFF)){
        FLAG = 0x21;					
//键盘数据上传示例 A
//         HIDKey[0] = 0x00;                                                
//         HIDKey[2] = 0x3b;                                                //F2
        enp1IntIn(HIDKey1+1);
    }
    else if ((ComboKey3 == 0)&&(HIDKey2[0] != 0xFF)){
        FLAG = 0x31;					
//键盘数据上传示例 A
//         HIDKey[0] = 0x00;                                                
//         HIDKey[2] = 0x3c;                                                //F3
        enp1IntIn(HIDKey2+1);
    }		
    else{
        UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;        //默认应答NAK
    }
}
main()
{
    UINT8 i,status;
    WDOGInit(0,1);	                                                      //开启看门狗复位
    memset(DataBuffer,0x00,27);
    if((*((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+2))==0)&&(*((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT))==0xAA))
	  {	
      for(i=0;i<9;i++){
				HIDKey[i] = *((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+0+i));
      }		
      memcpy(DataBuffer,HIDKey,9);				
    }
    if((*((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+2+9))==0)&&(*((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+9))==0xAA))
	  {	
      for(i=0;i<9;i++){
				HIDKey1[i] = *((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+9+i));
      }	
      memcpy(DataBuffer+9,HIDKey1,9);				
    }
    if((*((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+2+18))==0)&&(*((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+18))==0xAA))
	  {	
      for(i=0;i<9;i++){
				HIDKey2[i] = *((PUINT8C)(DATA_FLASH_ADDR+FLASHOFFECT+18+i));
      }	
      memcpy(DataBuffer+18,HIDKey2,9);				
    }
//  CfgFsys( );                                                           //CH559时钟选择配置
    P4_PU |= 0xC0;
    mInitSTDIO( );                                                        //串口0,可以用于调试
    printf("start ...\n");
    USBDeviceCfg();                                                       //模拟鼠标
    USBDeviceEndPointCfg();                                               //端点配置
    USBDeviceIntCfg();                                                    //中断初始化
    UEP1_T_LEN = 0;                                                       //预使用发送长度一定要清空
    UEP2_T_LEN = 0;                                                       //预使用发送长度一定要清空
    FLAG = 0;
    Ready = 0;
    while(1)
    {
        if((Ready == 1)&&(FLAG == 0)&&((ComboKey1==0)||(ComboKey2==0)||(ComboKey3 == 0))){
            HIDValueHandle( );                                           //F1
// 					  printf("ComboKey1&ComboKey2= %02x, ComboKey3= %02x\n",(UINT16)(ComboKey1&ComboKey2),(UINT16)ComboKey3);
        }
        if(((FLAG==0x12)&&(ComboKey1 == 1))||((FLAG==0x22)&&ComboKey2)||((FLAG==0x32)&&ComboKey3)){//按键已经释放
          FLAG = 0;					
          mDelaymS( 1 );                                                  //避免频繁上传，也可干其他事情
          if(ComboKey1 || ComboKey2 || ComboKey3){
//             HIDKey[0] = 0;
//             HIDKey[2] = 0;                                                //按键结束
            enp1IntIn(ChangeHIDKey);						
          }
				}
        if((HIDKey[0] == 1)||(HIDKey1[0] == 2)||(HIDKey2[0] == 3)){
					if(HIDKey[0] == 1) HIDKey[0] = 0xAA;DataBuffer[0] = 0xAA;
					if(HIDKey1[0] == 2) HIDKey1[0] = 0xAA;DataBuffer[9] = 0xAA;		
					if(HIDKey2[0] == 3) HIDKey2[0] = 0xAA;DataBuffer[18] = 0xAA;	
				  i = 3;							
				  while(i--){
					  status = EraseDataFlash(DATA_FLASH_ADDR);
					  if(status != 0) continue;
#if DEBUG					
					  printf("write ..\n");
#endif					
					  WriteDataFlash(DATA_FLASH_ADDR+FLASHOFFECT, DataBuffer, 27);
					  break;			  
				  }						
        }					
        WDOG_COUNT = 0;                                                    //喂狗						
    }
}
