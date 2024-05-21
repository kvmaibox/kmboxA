#ifndef __kmbox_h__
#define __kmbox_h__

typedef struct
{	unsigned int  Onoff;		//脚本触发方式  
	unsigned int  StartAddr;	//脚本存储位置  
	unsigned int  Length;		//脚本长度      
	unsigned int  RunCnt;		//运行次数
	int		 	  Exist;		//脚本是否存在
	char 		  Name[12];		//脚本名称    
}script_detail_t;

typedef struct 
{
	unsigned int  NewBoardFlag;// 新板标志          
	unsigned int  defaultVID;	//板卡默认的VID     
	unsigned int  defaultPID;	//板卡默认的PID	    
	unsigned int  TotalSize;	//总供的存储空间    
	unsigned int  UsedSize;		//已经存储空间   	
	unsigned int  NowIndex;		//当前运行的脚本编号 （切换脚本）
	script_detail_t script[5];	//五个脚本详细信息
}kmbox_t;


/**********************************************************
必须首先调用此函数才能与盒子通信
输入：   VID，PID 板子的硬件ID值，直接显示屏上可以看到
返回值： 
		-1：找不到指定的VID和PID
		0:正常
是否测试：是
测试时间：20211001 by hw
*************************************************************/
int KM_init(unsigned short vid,unsigned short pid);



/**********************************************************
键盘指定按键单击函数
输入：    HID键值表
是否测试：是
测试时间：20211001 by hw
注意：由于点击上位机时，电脑焦点在上位机程序里。故键盘的输入
无法直接看到。
*************************************************************/
int KM_press(unsigned char vk_key);

/***********************************************************
键盘函数
键盘指定按键一直保持按下 
输入：    HID键值表
是否测试：是
测试时间：20211001 by hw
***********************************************************/
int KM_down(unsigned char vk_key);

/***********************************************************
键盘函数
键盘指定按键抬起（配合down函数使用） 
输入：    HID键值表
是否测试：是
测试时间：20211001 by hw
***********************************************************/
int KM_up(unsigned char vk_key);


/***********************************************************
键盘发送函数
直接发送键盘报告：
ctrButton:为控制键，包括左右alt,ctr,shift,window
key：按键盘的按键值，详见HID键值对应表。key最多10字节，支持10键无冲
返回值：
		-1：发送失败
		 0：发送成功
是否测试：是
测试时间：20211001 by hw
***********************************************************/
int KM_keyboard(unsigned char ctrButton,unsigned char *key);


/***********************************************************
鼠标发送函数
直接发送鼠标报告：
lmr_side:为控制键，包括鼠标左键，中键，右键
x		:鼠标X轴方向移动距离
y		:鼠标Y轴方向移动距离
wheel	:滚轮移动距离
返回值：
		-1：发送失败
		 0：发送成功
是否测试：是
测试时间：20211001 by hw
***********************************************************/
int KM_mouse(unsigned char lmr_side,short x,short y,unsigned char wheel);


/***********************************************************
鼠标左键控制 0松开 1按下
是否测试：是
测试时间：20211001 by hw
***********************************************************/
int KM_left(unsigned char vk_key);

/*
鼠标中键控制 0松开 1按下
是否测试：是
测试时间：20211001 by hw
*/
int KM_middle(unsigned char vk_key);


/*
鼠标右键键控制 0松开 1按下
是否测试：是
测试时间：20211001 by hw
*/
int KM_right(unsigned char vk_key);


/*
鼠标相对移动
x		:鼠标X轴方向移动距离
y		:鼠标Y轴方向移动距离
返回值：
		-1：发送失败
		 0：发送成功
是否测试：是
测试时间：20211001 by hw
*/
int KM_move(short x,short y);



int KM_wheel(unsigned char  w);
int KM_side1(unsigned char  w);
int KM_side2(unsigned char  w);

//获取注册码（也可当机器码使用，每个板卡唯一） outMac为16字节数组
int KM_GetRegcode(unsigned char *outMac);
/*
设置注册码skey为64字节私钥字符串，与板卡一一对应，请勿乱尝试。错误10次直接格式化所有数据。
返回值：
	0：激活失败（激活失败会统计错误次数。连续错误10次直接擦除单片机所有数据，请勿频繁尝试）
	1：激活成功（成功后永久生效，除非刷固件，激活码请找固件提供者）
	-1：密钥错误
*/
int  KM_SetRegcode(char * skey);

/*
LCD显示字符。
mode 是显示模式
	0 ：全屏清理（关闭显示），不会理str
	-1：滚屏显示，即str显示到最后一行，原来内容顺序上滚
	1：此模式为指定str显示在显示屏的x,y为起点的位置。
x,y:
	当mode=1时有效，其他情况无效
	x取值0-8
	y取值0-128
*/
int KM_LCDstr(int mode,char *str,int x,int y);



/*
LCD显示图片。
bmp：为一张12864的图片数据总供1KB(取模软件输出的数组)
*/
int KM_LCDpic(unsigned char *bmp);


/*
擦除所有板载脚本
不包括用户数据区域。 
用户区域读写请看 KM_UserData函数
*/
int KM_ERASE(void);

//读写用户数据
/*
此区域是用户数据区，总共64字节。可以存放用户自定义数据。
可用来配套上位机做软件校验。非自己加密规则的硬件设备可以拒绝服务
*/
int KM_UserData(int rw,unsigned char *buff);

//读取板卡脚本信息
int KM_Readscript(kmbox_t *km);

//设置板卡VID和PID 掉电保存
int KM_SetVIDPID(int VID,int PID);


//脱机脚本编译
/*
*name:脚本名称 最多11个字符
index:脚本组号 取值范围1-4
trigger：当前脚本触发条件  取值范围0-255. 对应鼠标上的按键。 支持按位与操作。
		常规鼠标左键是1，右键是2，中键是4，侧键1是8，侧键按2是16.
		如果触发条件是鼠标左键按下则trigger=1.左键和右键同时按下触发为：1+2=3
doneNext:脚本运行完毕之后的状态。0：暂停等待当前触发条件结束 1：循环执行当前脚本
switch:是组件切换。当有多个挡位时满足此条件自动切换到下一个挡位。取值同trigger
str   ：待保存的脚本字符串。


返回值：

*/	
int KM_WriteScript(char *name,int index,int trigger,int doneNext,int Switch,char *str);


int KM_HostVidpid(int rw,unsigned int *vidpid,unsigned int *hiddid,unsigned int *mtype);

//注册授权函数
int MakeKey(char *mac,char *key);
#endif