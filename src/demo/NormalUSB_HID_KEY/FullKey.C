/********************************** (C) COPYRIGHT *******************************
* File Name          :Mouse.C
* Author             : WCH
* Version            : V1.0
* Date               : 2015/05/20
* Description        : CH559ģ����̣�֧��������
*******************************************************************************/
#include "DEBUG.C"                                                       //������Ϣ��ӡ
#include "DEBUG.H"
#include <string.h>
#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE
UINT8X  Ep0Buffer[THIS_ENDP0_SIZE] _at_ 0x0000;                                //�˵�0 OUT&IN��������������ż��ַ
UINT8X  Ep1Buffer[MAX_PACKET_SIZE] _at_ 0x0008;                                //�˵�1 IN������,������ż��ַ
UINT8X  Ep2Buffer[2*MAX_PACKET_SIZE] _at_ 0x0050;                                //�˵�2 OUT������,������ż��ַ
UINT8   SetupReq,SetupLen,Ready,Count,FLAG,UsbConfig;
PUINT8  pDescr;                                                                //USB���ñ�־
USB_SETUP_REQ   SetupReqBuf;                                                     //�ݴ�Setup��
#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0
#define FLASHOFFECT  0

sbit ComboKey1 = P3^4;
#define ComboKey2  (P4_IN&0x40)
#define ComboKey3  (P4_IN&0x80)
/*�豸������*/
UINT8C DevDesc[18] = {0x12, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08,
                      0x89, 0x11, 0x90, 0x88,                           /* ����ID�Ͳ�ƷID */
                      0x00, 0x01, 0x01, 0x02, 0x03, 0x01
                     };

/*�ַ���������*/
/*HID�౨��������*/
UINT8C KeyRepDesc[] =
{
//ÿ�п�ʼ�ĵ�һ�ֽ�Ϊ����Ŀ��ǰ׺��ǰ׺�ĸ�ʽΪ��
 //D7~D4��bTag��D3~D2��bType��D1~D0��bSize�����·ֱ��ÿ����Ŀע�͡�

/************************USB���̲��ֱ���������**********************/
/*******************************************************************/
 //����һ��ȫ�֣�bTypeΪ1����Ŀ������;ҳѡ��Ϊ��ͨ����Generic Desktop Page(0x01)
 //�����һ�ֽ����ݣ�bSizeΪ1����������ֽ����Ͳ�ע���ˣ�
 //�Լ�����bSize���жϡ�
 0x05, 0x01, // USAGE_PAGE (Generic Desktop)

 //����һ���ֲ���bTypeΪ2����Ŀ��˵���������ļ�����;���ڼ���
 0x09, 0x06, // USAGE (Keyboard)

 //����һ������Ŀ��bTypeΪ0����Ŀ�������ϣ������������0x01��ʾ
 //�ü�����һ��Ӧ�ü��ϡ�����������ǰ������;ҳ����;����Ϊ
 //��ͨ�����õļ��̡�
 0xa1, 0x01, // COLLECTION (Application)

 //����һ��ȫ����Ŀ��ѡ����;ҳΪ���̣�Keyboard/Keypad(0x07)��
 0x05, 0x07, //     USAGE_PAGE (Keyboard/Keypad)

 //����һ���ֲ���Ŀ��˵����;����СֵΪ0xe0��ʵ�����Ǽ�����Ctrl����
 //�������;ֵ����HID��;���в鿴��
 0x19, 0xe0, //     USAGE_MINIMUM (Keyboard LeftControl)

 //����һ���ֲ���Ŀ��˵����;�����ֵΪ0xe7��ʵ�����Ǽ�����GUI����
 0x29, 0xe7, //     USAGE_MAXIMUM (Keyboard Right GUI)

 //����һ��ȫ����Ŀ��˵�����ص����ݵ��߼�ֵ���������Ƿ��ص��������ֵ��
 //��СΪ0����Ϊ����������Bit����ʾһ�������������СΪ0�����Ϊ1��
 0x15, 0x00, //     LOGICAL_MINIMUM (0)

 //����һ��ȫ����Ŀ��˵���߼�ֵ���Ϊ1��
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)

 //����һ��ȫ����Ŀ��˵�������������Ϊ�˸���
 0x95, 0x08, //     REPORT_COUNT (8)

 //����һ��ȫ����Ŀ��˵��ÿ��������ĳ���Ϊ1��bit��
 0x75, 0x01, //     REPORT_SIZE (1)

 //����һ������Ŀ��˵����8������Ϊ1bit�������������ͳ���
 //��ǰ�������ȫ����Ŀ�����壩������Ϊ���룬
 //����Ϊ��Data,Var,Abs��Data��ʾ��Щ���ݿ��Ա䶯��Var��ʾ
 //��Щ�������Ƕ����ģ�ÿ�����ʾһ����˼��Abs��ʾ����ֵ��
 //��������Ľ�����ǣ���ĳ�����ֵΪ1ʱ���ͱ�ʾ��Ӧ�ļ����¡�
 //bit0�Ͷ�Ӧ����;��Сֵ0xe0��bit7��Ӧ����;���ֵ0xe7��
 0x81, 0x02, //     INPUT (Data,Var,Abs)

 //����һ��ȫ����Ŀ��˵������������Ϊ1��
 0x95, 0x01, //     REPORT_COUNT (1)

 //����һ��ȫ����Ŀ��˵��ÿ��������ĳ���Ϊ8bit��
 0x75, 0x08, //     REPORT_SIZE (8)

 //����һ������Ŀ�������ã���ǰ������ȫ����Ŀ��֪������Ϊ8bit��
 //����Ϊ1������������Ϊ�����������ص�����һֱ��0����
 //���ֽ��Ǳ����ֽڣ�������OEMʹ�ã���
 0x81, 0x03, //     INPUT (Cnst,Var,Abs)

 //����һ��ȫ����Ŀ������λ������Ϊ6����
 0x95, 0x06, //   REPORT_COUNT (6)

 //����һ��ȫ����Ŀ������ÿ��λ�򳤶�Ϊ8bit��
 //��ʵ���������Ŀ��ҪҲ�ǿ��Եģ���Ϊ��ǰ���Ѿ���һ������
 //����Ϊ8bit��ȫ����Ŀ�ˡ�
 0x75, 0x08, //   REPORT_SIZE (8)

 //����һ��ȫ����Ŀ�������߼���СֵΪ0��
 //ͬ�ϣ��������ȫ����ĿҲ�ǿ��Բ�Ҫ�ģ���Ϊǰ���Ѿ���һ��
 //�����߼���СֵΪ0��ȫ����Ŀ�ˡ�
 0x15, 0x00, //   LOGICAL_MINIMUM (0)

 //����һ��ȫ����Ŀ�������߼����ֵΪ255��
 0x25, 0xFF, //   LOGICAL_MAXIMUM (255)

 //����һ��ȫ����Ŀ��ѡ����;ҳΪ���̡�
 //ǰ���Ѿ�ѡ�����;ҳΪ�����ˣ����Ը���Ŀ��ҪҲ���ԡ�
 0x05, 0x07, //   USAGE_PAGE (Keyboard/Keypad)

 //����һ���ֲ���Ŀ��������;��СֵΪ0��0��ʾû�м����£�
 0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))

 //����һ���ֲ���Ŀ��������;���ֵΪ0x65
 0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)

 //����һ������Ŀ����˵��������8bit���������������õģ�
 //����Ϊ��Data,Ary,Abs��Data˵�������ǿ��Ա�ģ�Ary˵��
 //��Щ��������һ�����飬��ÿ��8bit�����Ա�ʾĳ����ֵ��
 //������µļ�̫�ࣨ���糬�����ﶨ��ĳ��Ȼ��߼��̱����޷�
 //ɨ����������ʱ��������Щ���ݷ���ȫ1�������ƣ�����ʾ������Ч��
 //Abs��ʾ��Щֵ�Ǿ���ֵ��
 0x81, 0x00, //     INPUT (Data,Ary,Abs)

 //����Ϊ������������
 //�߼���Сֵǰ���Ѿ��ж���Ϊ0�ˣ��������ʡ�ԡ�
 //����һ��ȫ����Ŀ��˵���߼�ֵ���Ϊ1��
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)

 //����һ��ȫ����Ŀ��˵������������Ϊ5����
 0x95, 0x05, //   REPORT_COUNT (5)

 //����һ��ȫ����Ŀ��˵��������ĳ���Ϊ1bit��
 0x75, 0x01, //   REPORT_SIZE (1)

 //����һ��ȫ����Ŀ��˵��ʹ�õ���;ҳΪָʾ�ƣ�LED��
 0x05, 0x08, //   USAGE_PAGE (LEDs)

 //����һ���ֲ���Ŀ��˵����;��СֵΪ���ּ��̵ơ�
 0x19, 0x01, //   USAGE_MINIMUM (Num Lock)

 //����һ���ֲ���Ŀ��˵����;���ֵΪKana�ơ�
 0x29, 0x05, //   USAGE_MAXIMUM (Kana)

 //����һ������Ŀ������������ݣ���ǰ�涨���5��LED��
 0x91, 0x02, //   OUTPUT (Data,Var,Abs)

 //����һ��ȫ����Ŀ������λ������Ϊ1����
 0x95, 0x01, //   REPORT_COUNT (1)

 //����һ��ȫ����Ŀ������λ�򳤶�Ϊ3bit��
 0x75, 0x03, //   REPORT_SIZE (3)

 //����һ������Ŀ���������������ǰ������5bit������������Ҫ
 //3��bit���ճ�һ�ֽڡ�
 0x91, 0x03, //   OUTPUT (Cnst,Var,Abs)

 //�����������Ŀ�����ر�ǰ��ļ��ϡ�bSizeΪ0�����Ժ���û���ݡ�
 0xc0,        // END_COLLECTION
};

UINT8C  HIDRepDesc[]={
	                        0x05,0x01,                                               //������������ÿ����Ŀռһ��
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
    0x09, 0x02, 0x42, 0x00, 0x02, 0x01, 0x00, 0xA0, 0x32,             //����������
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x01, 0x00,             //�ӿ�������,����
    0x09, 0x21, 0x00, 0x01, 0x21, 0x01, 0x22, sizeof(KeyRepDesc), 0x00,             //HID��������
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A,                         //�˵����������ж��ϴ�
	  0x09,0x04,0x01,0x00,0x02,0x03,0x00,0x00,0x00,             //�ӿ�����������Э����
	  0x09,0x21,0x00,0x01,0x00,0x01,0x22,sizeof(HIDRepDesc),0x00,             //HID��������    
	  0x07,0x05,0x82,0x03,0x08,0x00,0x0a,                       //�˵�������    
	  0x07,0x05,0x02,0x03,0x08,0x00,0x0a                       //�˵������� 	
};

UINT8C MyProductIDInfo[] = {0x0E,0x03,'k',0,'e',0,'y',0,'5',0,'5',0,'9',0};

/* ���������� */
UINT8C MyLangDescr[ ] = { 0x04, 0x03, 0x09, 0x04 };

/* ������Ϣ */
UINT8C MyManuInfo[ ] = { 0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0 };

/* ��Ʒ��Ϣ */
UINT8C MyProdInfo[ ] = { 0x0C, 0x03, 'C', 0, 'H', 0, '5', 0, '5', 0, '9', 0 };

/*��������*/
UINT8 HIDKey[9] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0};
UINT8 HIDKey1[9] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0};
UINT8 HIDKey2[9] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0};
UINT8 ChangeHIDKey[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
UINT8 DataBuffer[27];
/*******************************************************************************
* Function Name  : USBDeviceCfg()
* Description    : USB�豸ģʽ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceCfg()
{
    USB_CTRL = 0x00;                                                           //���USB���ƼĴ���
//     USB_CTRL &= ~bUC_HOST_MODE;                                                //��λΪѡ���豸ģʽ
    USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;                            //USB�豸���ڲ�����ʹ��,���ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
    USB_DEV_AD = 0x00;                                                         //�豸��ַ��ʼ��
    UDEV_CTRL &= ~bUD_RECV_DIS;                                                //ʹ�ܽ�����
    USB_CTRL |= bUC_LOW_SPEED;
    UDEV_CTRL |= bUD_LOW_SPEED;                                                //ѡ�����1.5Mģʽ
//    USB_CTRL &= ~bUC_LOW_SPEED;
//    UDEV_CTRL &= ~bUD_LOW_SPEED;                                             //ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
    UDEV_CTRL |= bUD_DP_PD_DIS | bUD_DM_PD_DIS;                                //��ֹDM��DP��������
    UDEV_CTRL |= bUD_PORT_EN;                                                  //ʹ�������˿�
}
/*******************************************************************************
* Function Name  : USBDeviceIntCfg()
* Description    : USB�豸ģʽ�жϳ�ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceIntCfg()
{
    USB_INT_EN |= bUIE_SUSPEND;                                               //ʹ���豸�����ж�
    USB_INT_EN |= bUIE_TRANSFER;                                              //ʹ��USB��������ж�
    USB_INT_EN |= bUIE_BUS_RST;                                               //ʹ���豸ģʽUSB���߸�λ�ж�
    USB_INT_FG |= 0x1F;                                                       //���жϱ�־
    IE_USB = 1;                                                               //ʹ��USB�ж�
    EA = 1;                                                                                                   //������Ƭ���ж�
}
/*******************************************************************************
* Function Name  : USBDeviceEndPointCfg()
* Description    : USB�豸ģʽ�˵����ã�ģ����̣����˶˵�0�Ŀ��ƴ��䣬�������˵�1�ж��ϴ�
                   �˵�2�������´�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceEndPointCfg()
{
    UEP1_DMA = Ep1Buffer;                                                      //�˵�1���ݴ����ַ
    UEP4_1_MOD |= bUEP1_TX_EN;                                                 //�˵�1����ʹ��
    UEP4_1_MOD &= ~bUEP1_RX_EN;                                                 //�˵�1����ʹ��
    UEP4_1_MOD &= ~bUEP1_BUF_MOD;                                              //�˵�1��64�ֽڻ�����
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //�˵�1�Զ���תͬ����־λ��IN���񷵻�NAK
	
    UEP2_DMA = Ep2Buffer;                                                      //�˵�1���ݴ����ַ
    UEP2_3_MOD |= bUEP2_TX_EN;                                                 //�˵�1����ʹ��
    UEP2_3_MOD |= bUEP2_RX_EN;                                                 //�˵�1����ʹ��
    UEP2_3_MOD &= ~bUEP2_BUF_MOD;                                              //�˵�1��64�ֽڻ�����
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;								 //�˵�2�Զ���תͬ����־λ��IN���񷵻�NAK��OUT����ACK
	
    UEP0_DMA = Ep0Buffer;                                                      //�˵�0���ݴ����ַ
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN);                                //�˵�0��64�ֽ��շ�������
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                                 //OUT���񷵻�ACK��IN���񷵻�NAK
}
/*******************************************************************************
* Function Name  : enp1IntIn()
* Description    : USB�豸ģʽ�˵�1���ж��ϴ�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void enp1IntIn( PUINT8 HIDKeyBuf )
{
    memcpy( Ep1Buffer, HIDKeyBuf, 8);                           //�����ϴ�����
    UEP1_T_LEN = 8;                                             //�ϴ����ݳ���
    UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;                //������ʱ�ϴ����ݲ�Ӧ��ACK
}
/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description    : CH559USB�жϴ�������
*******************************************************************************/
void    DeviceInterrupt( void ) interrupt INT_NO_USB using 1                      //USB�жϷ������,ʹ�üĴ�����1
{
    UINT8 len;
    if(UIF_TRANSFER)                                                            //USB������ɱ�־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 1:                                                  //endpoint 1# �ж϶˵��ϴ�          
            UEP1_T_LEN = 0;                                                     //Ԥʹ�÷��ͳ���һ��Ҫ���
//            UEP2_CTRL ^= bUEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
            UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ��Ӧ��NAK
            if(FLAG&1) FLAG++;							
            break;
         case UIS_TOKEN_IN | 2:                                                 //endpoint 2# �ж϶˵��ϴ�
              UEP2_T_LEN = 0;	                                                  //Ԥʹ�÷��ͳ���һ��Ҫ���	
//            UEP1_CTRL ^= bUEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
			        UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;         //Ĭ��Ӧ��NAK
			  break;
         case UIS_TOKEN_OUT | 2:                                                //endpoint 2# �ж϶˵��´�
            if ( U_TOG_OK )                                                     // ��ͬ�������ݰ�������
            {			
                  UEP2_CTRL &= ~UEP_R_RES_ACK;	
                  UEP2_CTRL |= UEP_R_RES_NAK;									
                len = USB_RX_LEN;                                               //�������ݳ��ȣ����ݴ�Ep2Buffer�׵�ַ��ʼ���		
                if(Ep2Buffer[0] == 1){//����K1
                   memcpy(HIDKey, Ep2Buffer, 8);		
                   memcpy(DataBuffer,HIDKey,8);		
#if DEBUG
for(len=0;len<27;len++){
printf("%02x  ",(UINT16)DataBuffer[len]);
}	
#endif									
                }
                else if(Ep2Buffer[0] == 2){//����K2
                   memcpy(HIDKey1, Ep2Buffer, 8);		
                   memcpy(DataBuffer+9,HIDKey1,8);		
#if DEBUG
for(len=0;len<27;len++){
printf("%02x  ",(UINT16)DataBuffer[len]);
}	
#endif											
                }
                else if(Ep2Buffer[0] == 3){//����K3
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
        case UIS_TOKEN_SETUP | 0:                                                //SETUP����
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;    // �����ܳ���
                }
                len = 0;                                                      // Ĭ��Ϊ�ɹ������ϴ�0����
                SetupReq = UsbSetupBuf->bRequest;
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD )/* ֻ֧�ֱ�׼���� */
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
                        Ready = 1;                                       //����и���ӿڣ��ñ�׼λӦ�������һ���ӿ�������ɺ���Ч											
												 break;
										case 0x0A://SetIdle
												 break;	
										case 0x0B://SetProtocol
												 break;
										default:
												 len = 0xFF;  								 					/*����ʧ��*/					
												 break;
								  }
                }
								else{
                switch(SetupReq)                                                  //������
                {
                case USB_GET_DESCRIPTOR:
                    switch(UsbSetupBuf->wValueH)
                    {
                    case 1:                                                 //�豸������
                        pDescr = DevDesc;                               //���豸�������͵�Ҫ���͵Ļ�����
                        len = sizeof(DevDesc);
                        break;
                    case 2:                                                                 //����������
                        pDescr = CfgDesc;                               //���豸�������͵�Ҫ���͵Ļ�����
                        len = sizeof(CfgDesc);
                        break;
                    case 3:                                            // �ַ���������
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
                                len = 0xFF;                                 // ��֧�ֵ��ַ���������
                                break;
						            }
                        break;										
                    case 0x22:                                            //����������
											 if(UsbSetupBuf->wIndexL == 0)                   //�ӿ�0����������
											 {																 
													 pDescr = KeyRepDesc;                        //����׼���ϴ�
													 len = sizeof(KeyRepDesc);
											 }
											 else if(UsbSetupBuf->wIndexL == 1)              //�ӿ�1����������
											 {																 
													 pDescr = HIDRepDesc;                        //����׼���ϴ�
													 len = sizeof(HIDRepDesc);
													 Ready = 1; 																 
																																			 //����и���ӿڣ��ñ�׼λӦ�������һ���ӿ�������ɺ���Ч
											 }	
											 else len = 0xff;                                //������ֻ��2���ӿڣ���仰����������ִ��		
                    default:
                        len = 0xff;                                     //��֧�ֵ�������߳���
                        break;
                    }
                    if ( SetupLen > len )
                    {
                        SetupLen = len;    //�����ܳ���
                    }
                    len = SetupLen >= 8 ? 8 : SetupLen;                            //���δ��䳤��
                    memcpy(Ep0Buffer,pDescr,len);                                  //�����ϴ�����
                    SetupLen -= len;
                    pDescr += len;
                    break;
                case USB_SET_ADDRESS:
                    SetupLen = UsbSetupBuf->wValueL;                              //�ݴ�USB�豸��ַ
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
                    if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// �˵�
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
                               len = 0xFF;                                         // ��֧�ֵĶ˵�
                               break;
                        }
                     }
                     else
                     {
                        len = 0xFF;                                                // ���Ƕ˵㲻֧��
                     }
                    break;
                    case USB_SET_FEATURE:                                          /* Set Feature */
                    if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x00 )                  /* �����豸 */
                    {
                        if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                        {
                            if( CfgDesc[ 7 ] & 0x20 )
                            {
                                 /* ���û���ʹ�ܱ�־ */
                            }
                            else
                            {
                                len = 0xFF;                                        /* ����ʧ�� */
                            }
                        }
                        else
                        {
                            len = 0xFF;                                            /* ����ʧ�� */
                        }
                    }
                    else if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x02 )             /* ���ö˵� */
                    {
                        if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                        {
                            switch( ( ( UINT16 )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                            {
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�2 IN STALL */
                                    break;

                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* ���ö˵�2 OUT Stall */
                                    break;

                                case 0x81:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�1 IN STALL */
                                    break;

                                default:
                                    len = 0xFF;                                    /* ����ʧ�� */
                                    break;
                            }
                        }
                        else
                        {
                            len = 0xFF;                                      /* ����ʧ�� */
                        }
                    }
                    else
                    {
                        len = 0xFF;                                          /* ����ʧ�� */
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
                    len = 0xff;                                                    //����ʧ��
                    break;
                }
							}
            }
            else
            {
                len = 0xff;                                                         //�����ȴ���
            }
            if(len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
            }
            else if(len <= 8)                                                       //�ϴ����ݻ���״̬�׶η���0���Ȱ�
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1������Ӧ��ACK
            }
            else
            {
                UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1,����Ӧ��ACK
            }
            break;
        case UIS_TOKEN_IN | 0:                                                      //endpoint0 IN
            switch(SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= 8 ? 8 : SetupLen;                                 //���δ��䳤��
                memcpy( Ep0Buffer, pDescr, len );                                   //�����ϴ�����
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;                                             //ͬ����־λ��ת
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;                                                      //״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
            len = USB_RX_LEN;
            UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
            UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA0,����Ӧ��ACK						
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
        UIF_TRANSFER = 0;                                                           //д0����ж�
    }
    if(UIF_BUS_RST)                                                               //�豸ģʽUSB���߸�λ�ж�
    {
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;                                                              //���жϱ�־
    }
    if (UIF_SUSPEND)                                                                //USB���߹���/�������
    {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND )                                              //����
        {
            while ( XBUS_AUX & bUART0_TX )
            {
                ;    //�ȴ��������
            }
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO;                                     //USB����RXD0���ź�ʱ�ɱ�����
            PCON |= PD;                                                                 //˯��
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = 0x00;
        }
    }
    else {                                                                         //������ж�,�����ܷ��������
        USB_INT_FG = 0x00;                                                           //���жϱ�־
//      printf("UnknownInt  N");
    }
}

void HIDValueHandle()
{
    mDelaymS(2);                                                     //��ʱ���ⰴ����
    if ((ComboKey1 == 0)&&(HIDKey[0] != 0xFF)){
        FLAG = 0x11;					
//���������ϴ�ʾ�� A
//         HIDKey[0] = 0x00;                                                
//         HIDKey[2] = 0x3a;                                                //F1
        enp1IntIn(HIDKey+1);
    }
    else if ((ComboKey2 == 0)&&(HIDKey1[0] != 0xFF)){
        FLAG = 0x21;					
//���������ϴ�ʾ�� A
//         HIDKey[0] = 0x00;                                                
//         HIDKey[2] = 0x3b;                                                //F2
        enp1IntIn(HIDKey1+1);
    }
    else if ((ComboKey3 == 0)&&(HIDKey2[0] != 0xFF)){
        FLAG = 0x31;					
//���������ϴ�ʾ�� A
//         HIDKey[0] = 0x00;                                                
//         HIDKey[2] = 0x3c;                                                //F3
        enp1IntIn(HIDKey2+1);
    }		
    else{
        UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;        //Ĭ��Ӧ��NAK
    }
}
main()
{
    UINT8 i,status;
    WDOGInit(0,1);	                                                      //�������Ź���λ
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
//  CfgFsys( );                                                           //CH559ʱ��ѡ������
    P4_PU |= 0xC0;
    mInitSTDIO( );                                                        //����0,�������ڵ���
    printf("start ...\n");
    USBDeviceCfg();                                                       //ģ�����
    USBDeviceEndPointCfg();                                               //�˵�����
    USBDeviceIntCfg();                                                    //�жϳ�ʼ��
    UEP1_T_LEN = 0;                                                       //Ԥʹ�÷��ͳ���һ��Ҫ���
    UEP2_T_LEN = 0;                                                       //Ԥʹ�÷��ͳ���һ��Ҫ���
    FLAG = 0;
    Ready = 0;
    while(1)
    {
        if((Ready == 1)&&(FLAG == 0)&&((ComboKey1==0)||(ComboKey2==0)||(ComboKey3 == 0))){
            HIDValueHandle( );                                           //F1
// 					  printf("ComboKey1&ComboKey2= %02x, ComboKey3= %02x\n",(UINT16)(ComboKey1&ComboKey2),(UINT16)ComboKey3);
        }
        if(((FLAG==0x12)&&(ComboKey1 == 1))||((FLAG==0x22)&&ComboKey2)||((FLAG==0x32)&&ComboKey3)){//�����Ѿ��ͷ�
          FLAG = 0;					
          mDelaymS( 1 );                                                  //����Ƶ���ϴ���Ҳ�ɸ���������
          if(ComboKey1 || ComboKey2 || ComboKey3){
//             HIDKey[0] = 0;
//             HIDKey[2] = 0;                                                //��������
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
        WDOG_COUNT = 0;                                                    //ι��						
    }
}