/********************************** (C) COPYRIGHT ******************************
* File Name          : README.TXT
* Author             : WCH
* Version            : V1.4
* Date               : 2017/02/24
* Description        : 1. A demo for USB compound device created by CH559, support
					   keyboard and mouse, and HID compatible device.
					   2. The device will auto run the HID_RECV.exe, after plug on
						The path of the app locate at "E:\HID_RECV.EXE"
					   3. The device support interrupt transmit, interval set as 10ms
					   for keyboard and 50ms for comppsite device.
********************************************************************************/
调试日志
2017/02/22
1.	实现按键与鼠标功能
2.	理解远程唤醒？
3.	解决鼠标单击失效BUG
4.	串口调试，增加WIN+R系统热键

2017/02/23
1.	企图通过单片机操作键盘数据打开应用程序
2.	实现WIN+R热键打开某路径文件之功能
3. 	解决传数据丢包BUG(枚举完成标志 SET REPORT...)

2017/02/24
1.	HID兼容设???
2.	加深理解USB协议，HID设备描述符解决相关BUG

2017/02/28
1. 	解决HID兼容设备问题

2017/03/05
1. 联调通过

2017/03/07
1. 解决端点2乱码问题(开辟对应缓冲区[OUT]+[IN])
2. 解决SET_IDLE时等待问题(给STALL)

2017/03/09
1. 系统测试：支持WINXP/WIN7/WIN8/WIN10，不支持Linux系统，不支持虚拟机

2017/03/10
1. 修正中断处理(CLASS REQ)
2. 联调测试邵、陈软件�
3. 解决虚拟机测试无法识别问题(设置USB设备全部可见，如果还是不行，加入“usb.generic.autoconnect = "TRUE"”语句进*.vmx文件)
