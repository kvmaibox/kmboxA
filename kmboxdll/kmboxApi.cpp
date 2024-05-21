// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// km-dll.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
 #include <stdlib.h>
// TODO: �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������
#include "hidapi.h"
#include "HidTable.h"
#include "kmbox.h"
#include "windows.h"
#define Head_Sync 0xbb	//Э��ͬ��ͷ
hid_device * fd_kmbox;	//�豸���
HANDLE m_hMutex_lock=NULL;  //���̻߳�����
unsigned int ROM_SCRIPT[64*1024]={0};
int String2Hex(char *str, char *hex);
int String2Hex1(char *str, char *hex);
/*
�������ȵ��ô˺������������ͨ��
���룺   VID��PID ���ӵ�Ӳ��IDֵ��ֱ����ʾ���Ͽ��Կ���
����ֵ�� 
		-1���Ҳ���ָ����VID��PID
		0:����
*/
int KM_init(unsigned short vid,unsigned short pid)
{
	hid_device_info *hid_info;
	if(m_hMutex_lock==NULL)
	{
		m_hMutex_lock= CreateMutex(NULL,TRUE,"busy");  
	}
	ReleaseMutex(m_hMutex_lock);
	hid_info=hid_enumerate(vid,pid);

	do{
		if(hid_info==NULL) return -1;
		if(hid_info->usage_page==0xff00)
		{
			break;
		}else
			hid_info =hid_info->next;
	
	
	}while(1);

	fd_kmbox = hid_open_path(hid_info->path);//
	if (!fd_kmbox) {
		fd_kmbox=NULL;
		return -1;
	}
	return 0;
}



static struct keyboard_t
{	unsigned char head[5];	//0x00
	unsigned char ctrButton;//KEY_Z
	unsigned char data[59];	//0X00000
}data_keyboard={0x00,Head_Sync,0x02,0x0C,0x01,0x00};
/*
���̷��ͺ���
ֱ�ӷ��ͼ��̱��棺
ctrButton:Ϊ���Ƽ�����������alt,ctr,shift,window
key�������̵İ���ֵ�����HID��ֵ��Ӧ��key���10�ֽڣ�֧��10���޳�
����ֵ��
		-1������ʧ��
		 0�����ͳɹ�
*/
int KM_keyboard(unsigned char ctrButton,unsigned char *key)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	data_keyboard.ctrButton=ctrButton;//���Ƽ�
	for( i=0;i<10;i++)
	{
		data_keyboard.data[i]=key[i];
	}
	i=hid_write(fd_kmbox,(const unsigned char *)&data_keyboard,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}

static struct mouse_t
{	unsigned char head[5];//0x00
	unsigned char button;
	short x;
	short y;
	unsigned char wheel;
	unsigned char reserv[54];//0X00000
}data_mouse={0x00,Head_Sync,0x03,0x07,0X02};

/*
��귢�ͺ���
ֱ�ӷ�����걨�棺
lmr_side:Ϊ���Ƽ����������������м����Ҽ�
x		:���X�᷽���ƶ�����
y		:���Y�᷽���ƶ�����
wheel	:�����ƶ�����
����ֵ��
		-1������ʧ��
		 0�����ͳɹ�
*/
int KM_mouse(unsigned char lmr_side,short x,short y,unsigned char wheel)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	data_mouse.button=lmr_side;
	data_mouse.x=x;
	data_mouse.y=y;
	data_mouse.wheel=wheel;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}



/*
���̺���
����ָ������һֱ���ְ���
*/
int KM_down(unsigned char vk_key)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
    if(vk_key>=KEY_LEFTCONTROL&&vk_key<=KEY_RIGHT_GUI)//���Ƽ�
	{
		 switch(vk_key)
	     {  case KEY_LEFTCONTROL: data_keyboard.ctrButton  |=BIT0;break;
            case KEY_LEFTSHIFT:   data_keyboard.ctrButton |=BIT1;break;
            case KEY_LEFTALT:     data_keyboard.ctrButton |=BIT2;break;
            case KEY_LEFT_GUI:    data_keyboard.ctrButton |=BIT3;break;
            case KEY_RIGHTCONTROL:data_keyboard.ctrButton |=BIT4;break;
            case KEY_RIGHTSHIFT:  data_keyboard.ctrButton |=BIT5;break;
            case KEY_RIGHTALT:    data_keyboard.ctrButton |=BIT6;break;
            case KEY_RIGHT_GUI:   data_keyboard.ctrButton |=BIT7;break;
        }
	}else
	{//�����  
		for(i=0;i<10;i++)//���ȼ��������Ƿ����vk_key
		{	
			if(data_keyboard.data[i]==vk_key) 
				goto KM_down_send;// ���������Ѿ���vk_key ֱ�ӷ��;���
		}
		//��������û��vk_key 
		for(i=0;i<10;i++)//�������е����ݣ���vk_key��ӵ�������
		{
			if(data_keyboard.data[i]==0)
			{// ���������Ѿ���vk_key ֱ�ӷ��;���
				data_keyboard.data[i]=vk_key;
				goto KM_down_send;
			}
		}
		//�����Ѿ����� ��ô���޳��ʼ���Ǹ�
		memcpy(&data_keyboard.data[0],&data_keyboard.data[1],10);
		data_keyboard.data[9]=vk_key;
	}
KM_down_send:
	i=hid_write(fd_kmbox,(const unsigned char *)&data_keyboard,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}


/*
���̺���
����ָ������һֱ���ְ��� 
*/
int KM_up(unsigned char vk_key)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	 if(vk_key>=KEY_LEFTCONTROL&&vk_key<=KEY_RIGHT_GUI)//���Ƽ�
	{
		 switch(vk_key)
	     {  case KEY_LEFTCONTROL: data_keyboard.ctrButton &=~BIT0;break;
            case KEY_LEFTSHIFT:   data_keyboard.ctrButton &=~BIT1;break;
            case KEY_LEFTALT:     data_keyboard.ctrButton &=~BIT2;break;
            case KEY_LEFT_GUI:    data_keyboard.ctrButton &=~BIT3;break;
            case KEY_RIGHTCONTROL:data_keyboard.ctrButton &=~BIT4;break;
            case KEY_RIGHTSHIFT:  data_keyboard.ctrButton &=~BIT5;break;
            case KEY_RIGHTALT:    data_keyboard.ctrButton &=~BIT6;break;
            case KEY_RIGHT_GUI:   data_keyboard.ctrButton &=~BIT7;break;
        }
	}else
	{//�����  
		for(i=0;i<10;i++)//���ȼ��������Ƿ����vk_key
		{	
			if(data_keyboard.data[i]==vk_key)// ���������Ѿ���vk_key 
			{
				memcpy(&data_keyboard.data[i],&data_keyboard.data[i+1],10-i);
				data_keyboard.data[9]=0;
				goto KM_up_send;
			}
		}
	}
KM_up_send:
	i=hid_write(fd_kmbox,(const unsigned char *)&data_keyboard,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}


int KM_press(unsigned char vk_key)
{	int ret;
	ret=KM_down(vk_key);
	ret=KM_up(vk_key);
	return ret;
}


/*
���������� 0�ɿ� 1����
*/
int KM_left(unsigned char vk_key)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	if(vk_key)	
		data_mouse.button |=BIT0;
	else 
		data_mouse.button &=~BIT0;
	data_mouse.x=0;
	data_mouse.y=0;
	data_mouse.wheel=0;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}

/*
����м����� 0�ɿ� 1����
*/
int KM_middle(unsigned char vk_key)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	if(vk_key)	
		data_mouse.button |=BIT1;
	else 
		data_mouse.button &=~BIT1;
	data_mouse.x=0;
	data_mouse.y=0;
	data_mouse.wheel=0;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}


/*
����Ҽ������� 0�ɿ� 1����
*/
int KM_right(unsigned char vk_key)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	if(vk_key)	
		data_mouse.button |=BIT2;
	else 
		data_mouse.button &=~BIT2;
	data_mouse.x=0;
	data_mouse.y=0;
	data_mouse.wheel=0;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}


int KM_move(short x,short y)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	data_mouse.x=x;
	data_mouse.y=y;
	data_mouse.wheel=0;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}



int KM_wheel(unsigned char  w)
{	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	data_mouse.x=0;
	data_mouse.y=0;
	data_mouse.wheel=w;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65)Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}

int KM_side1(unsigned char  w)
{
	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	if(w)	
		data_mouse.button |=BIT3;
	else 
		data_mouse.button &=~BIT3;
	data_mouse.x=0;
	data_mouse.y=0;
	data_mouse.wheel=0;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}

int KM_side2(unsigned char  w)
{
	int i;
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	if(w)	
		data_mouse.button |=BIT4;
	else 
		data_mouse.button &=~BIT4;
	data_mouse.x=0;
	data_mouse.y=0;
	data_mouse.wheel=0;
	i=hid_write(fd_kmbox,(const unsigned char *)&data_mouse,65);
	if(i==65) Sleep(1);
	ReleaseMutex(m_hMutex_lock);
	return i==65?0:-1;
}


static struct cmd_getregcode_t
{	unsigned char head[5];	//0x00
	unsigned char data[60];	//0X00000
}data_getRegcode={0x00,Head_Sync,0x05,0x20,0x00,0x00};

//��ȡע���루�鿴�Ƿ���Ȩ��
int  KM_GetRegcode(unsigned char *outMac)
{	unsigned char buff[65]={0};
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	hid_write(fd_kmbox,(const unsigned char *)&data_getRegcode,65);
	hid_read_timeout(fd_kmbox,buff,65,-1);
	ReleaseMutex(m_hMutex_lock);
	memcpy(outMac,&buff[3],16);
	return buff[2];
}

void hex2string(char *pt,char *retstr)
{
	int tmp=*pt;
	int h=(tmp&0xf0)>>4;
	int l=tmp&0xf;
	if(h>=0&&h<=9)        *retstr=h+'0';
	else if(h>=10&&h<=15) *retstr=h+'a'-10;

	if(l>=0&&l<=9)        *(retstr+1)=l+'0';
	else if(l>=10&&l<=15) *(retstr+1)=l+'a'-10;

}

static struct cmd_make_key
{	unsigned char head[5];	//0x00
	char data[60];	//0X00000
}data_make_key={0x00,Head_Sync,0xfc,0xfc,0xcf};

int MakeKey(char *mac,char *key)
{//cd ab 5a 3d 09 43 30 2c 0f 41 00 20
	if(strlen(mac)!=24) return -1;
	char buff[65]={0};
	sprintf(buff,"%s",mac);
	String2Hex1(buff, data_make_key.data);
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	hid_write(fd_kmbox,(const unsigned char *)&data_make_key,65);
	hid_read_timeout(fd_kmbox,(unsigned char *)buff,65,-1);
	ReleaseMutex(m_hMutex_lock);
	for(int i=0;i<32;i++)
	{
		hex2string(&buff[i],key);
		key=key+2;
	}
	return 0;
}


static struct cmd_Setregcode_t
{	unsigned char head[4];	//0x00
	unsigned char data[61];	//0X00000
}data_SetRegcode={0x00,Head_Sync,0x06,0x20};

char Char2Hex(char ch)
{
  if((ch>='0')&&(ch<='9'))   
  return   ch-0x30;   
  else   if((ch>='A')&&(ch<='F'))   
  return   ch-'A'+10;   
  else   if((ch>='a')&&(ch<='f'))   
  return   ch-'a'+10;   
  else   return   (-1);   
}

int String2Hex(char *str, char *hex)
{		
	 int hexh,hexl,n;
	 n=0;
	 for(int   i=0;i<64;i++)   
	 {   hexh=Char2Hex( str[i]);  //���ֽ�
		 hexl=Char2Hex( str[i+1]);//���ֽ�
		 if(hexh!=-1&&hexl!=-1)
		 {
			hex[n]=hexh<<4|hexl;
			n++;
		  }else 
			  return -1;
		  i++;
	 } 
	for(int i=0;i<32;)
	{
		n=hex[i];
		hex[i]=hex[i+3];
		hex[i+3]=n;
		n=hex[i+1];
		hex[i+1]=hex[i+2];
		hex[i+2]=n;
		i+=4;
	}

	 return 0;
}


int String2Hex1(char *str, char *hex)
{		
	 int hexh,hexl,n;
	 n=0;
	 for(int   i=0;i<64;i++)   
	 {   hexh=Char2Hex( str[i]);  //���ֽ�
		 hexl=Char2Hex( str[i+1]);//���ֽ�
		 if(hexh!=-1&&hexl!=-1)
		 {
			hex[n]=hexh<<4|hexl;
			n++;
		  }else 
			  return -1;
		  i++;
	 } 
	 return 0;
}

typedef union
{
	unsigned int buff[32];
	 char u8data[32*4];
}t_key_int;
//				efab60354ca14e73e78d7aa69f375432cc997769cd26bd9bba7487de92841160     
//				efab60354ca14e73e78d7aa69f375432cc997769cd26bd9bba7487de92841160
//����ע����  1905e6d71ac78eeb071e3d32cb406598eec804fafb30dcda67887cb6ea84b490     efab60354ca1
int  KM_SetRegcode(char * skey)
{	unsigned char buff[65]={0};
	t_key_int hexkey={0};
	if(String2Hex1(skey, hexkey.u8data)==0)
	{	
		memcpy(data_SetRegcode.data,hexkey.buff,32);
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		hid_write(fd_kmbox,(const unsigned char *)&data_SetRegcode,65);
		hid_read_timeout(fd_kmbox,buff,65,-1);
		ReleaseMutex(m_hMutex_lock);
		return buff[3];
	}
	return -1;
}


static struct lcdstr_t
{	unsigned char head[4];//0x00
	unsigned char mode;
	char x;
	char y;
	unsigned char data[58];//0X00000
}cmd07_lcdstr={0x00,Head_Sync,0x07,0x70};
//
int KM_LCDstr(int mode,char *str,int x,int y)
{		unsigned char buff[65]={0};
		cmd07_lcdstr.mode=mode;
		cmd07_lcdstr.x=x;
		cmd07_lcdstr.y=y;
		memset(cmd07_lcdstr.data,0,58);
		memcpy(cmd07_lcdstr.data,str,strlen(str));
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		hid_write(fd_kmbox,(const unsigned char *)&cmd07_lcdstr,65);
		hid_read_timeout(fd_kmbox,buff,65,-1);
		ReleaseMutex(m_hMutex_lock);
		return 0;
}


//LCDˢ����ͼƬ
static struct lcdpic_t
{	unsigned char head[6];//0x00
	unsigned char data[65];//0X00000
}cmd04_lcdpic={0x00,Head_Sync,0x04,0x40,0x40,0x40};
//
int KM_LCDpic(unsigned char *bmp)
{		unsigned char buff[65]={0};
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		//��������
		hid_write(fd_kmbox,(const unsigned char *)&cmd04_lcdpic,65);
		hid_read_timeout(fd_kmbox,buff,65,-1);
		memset(cmd04_lcdpic.data,0,65);
		for(int i=0;i<16;i++)
		{
			memcpy(&cmd04_lcdpic.data[1],bmp,64);
			hid_write(fd_kmbox,(const unsigned char *)&cmd04_lcdpic.data,65);
			hid_read_timeout(fd_kmbox,buff,65,-1);
			bmp+=64;
		}
		ReleaseMutex(m_hMutex_lock);
		return 0;
}


/*******************************����*****************************************/
static struct cmd08_eraflash_t
{	unsigned char head[3];//0x00
	unsigned char data[62];//0X00000
}cmd08_eraflash={0x00,Head_Sync,0x08};

int KM_ERASE(void)
{		unsigned char buff[65]={0};
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		hid_write(fd_kmbox,(const unsigned char *)&cmd08_eraflash,65);
		hid_read_timeout(fd_kmbox,buff,65,-1);
		ReleaseMutex(m_hMutex_lock);
		return 0;
}

static struct cmd01_setVIDPID_t
{	unsigned char head[4];//0x00
	int vid;
	int pid;
	unsigned char data[60];//0X00000
}cmd01_setvid={0x00,Head_Sync,0x01,0xaa};

int KM_SetVIDPID(int VID,int PID)
{		unsigned char buff[65]={0};
		cmd01_setvid.vid=VID;
		cmd01_setvid.pid=PID;
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		hid_write(fd_kmbox,(const unsigned char *)&cmd01_setvid,65);
		hid_read_timeout(fd_kmbox,buff,65,-1);
		ReleaseMutex(m_hMutex_lock);
		return 0;
}

/*���ؽű��ְ�����*/

static struct cmd0a_t
{	unsigned char head[3];	//0x00
	unsigned char index;
	unsigned int address;
	unsigned char data[57];	//0X00000
}cmd0a_downflash_t={0x00,Head_Sync,0x0a};

int KM_download(unsigned int address,unsigned int *buff,int length)
{	unsigned char ret[65];
	WaitForSingleObject(m_hMutex_lock, INFINITE);
	for(int i=0;i<length/32;i++) //1��length��128�ֽ�
	{
			cmd0a_downflash_t.index=0x00;
			cmd0a_downflash_t.address=address+i*128;
			memset(cmd0a_downflash_t.data,0,57);
			memcpy(cmd0a_downflash_t.data,buff+i*32,56);
			hid_write(fd_kmbox,(const unsigned char *)&cmd0a_downflash_t,65);
			hid_read_timeout(fd_kmbox,ret,65,-1);

			cmd0a_downflash_t.index=0x01;
			memset(cmd0a_downflash_t.data,0,57);
			memcpy(cmd0a_downflash_t.data,buff+i*32+14,56);
			hid_write(fd_kmbox,(const unsigned char *)&cmd0a_downflash_t,65);
			hid_read_timeout(fd_kmbox,ret,65,-1);

			cmd0a_downflash_t.index=0x02;
			memset(cmd0a_downflash_t.data,0,57);
			memcpy(cmd0a_downflash_t.data,buff+28+i*32,16);
			hid_write(fd_kmbox,(const unsigned char *)&cmd0a_downflash_t,65);
			hid_read_timeout(fd_kmbox,ret,65,-1);
	}
	ReleaseMutex(m_hMutex_lock);
	return 0;
}


/*******************************���ű���Ϣ(��������)****************************************/
static struct cmd09_readflash_t
{	unsigned char head[4];//0x00
	unsigned char index;
	unsigned char data[60];//0X00000
}cmd09_readflash={0x00,Head_Sync,0x09,0x00};


typedef struct
{
	unsigned int  NewBoardFlag;// �°��־        1
	unsigned int  defaultVID;	//�忨Ĭ�ϵ�VID     2
	unsigned int  defaultPID;	//�忨Ĭ�ϵ�PID	2
	unsigned int  TotalSize;	//�ܹ��Ĵ洢�ռ�    4
	unsigned int  UsedSize;	//�ܹ��Ĵ洢�ռ�    4	
	unsigned int  NowIndex;	//��ǰ���еĽű���� 1

	unsigned int hostVIDPID;//������VID��PID
	unsigned int hostHIDDID;//������HID��DID
	unsigned int hostmType_scanTime;
}config_summary_t;

typedef union
{
	unsigned char	 buf[128];//128�ֽ�
	config_summary_t ROM;    //
}t_config_param;


typedef union
{
	unsigned char	 buf[128];//128�ֽ�
	script_detail_t  ROM;    //
}t_script_param;	


kmbox_t g_script_info;
int KM_Readscript(kmbox_t *kmbox)
{		t_config_param config;
		t_script_param script;	
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		memset(kmbox,0,sizeof(kmbox_t));
		memset(&g_script_info,0,sizeof(g_script_info));
		cmd09_readflash.index=0;//��ȡVID��PID,�ű���Ҫ
		hid_write(fd_kmbox,(const unsigned char *)&cmd09_readflash,65);
		hid_read_timeout(fd_kmbox,config.buf,65,-1);
		memcpy(kmbox,config.buf,sizeof(config_summary_t));
		memcpy(&g_script_info,config.buf,sizeof(config_summary_t));
		for(int i=1;i<=5;i++)
		{
			cmd09_readflash.index=i;//��ȡVID��PID,�ű���Ҫ
			hid_write(fd_kmbox,(const unsigned char *)&cmd09_readflash,65);
			hid_read_timeout(fd_kmbox,script.buf,65,-1);
			memcpy(&(kmbox->script[i-1]),script.buf,sizeof(script_detail_t));
			memcpy(&(g_script_info.script[i-1]),script.buf,sizeof(script_detail_t));
		}
		ReleaseMutex(m_hMutex_lock);
		return 0;
}



static struct cmd0b_Setflash_t
{	unsigned char head[4];//0x00
	unsigned char index;
	unsigned char data[60];//0X00000
}cmd0b_Setflash={0x00,Head_Sync,0x0b,0x00};


//�ű����                     �ܴ�С   ��ǰ�ű��ֽ���
int KM_Setscript(int index,int addr,int currlength)
{		t_config_param config;
		t_script_param script;	
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		cmd0b_Setflash.index=0;//����
		memcpy(cmd0b_Setflash.data,&g_script_info.NewBoardFlag,sizeof(config_summary_t));
		hid_write(fd_kmbox,(const unsigned char *)&cmd0b_Setflash,65);
		hid_read_timeout(fd_kmbox,config.buf,65,-1);


		g_script_info.script[index-1].StartAddr=g_script_info.script[index-1].StartAddr+addr-currlength;
		memcpy(cmd0b_Setflash.data,&g_script_info.script[index-1],sizeof(script_detail_t));
		cmd0b_Setflash.index=index;//��ȡVID��PID,�ű���Ҫ
		hid_write(fd_kmbox,(const unsigned char *)&cmd0b_Setflash,65);
		hid_read_timeout(fd_kmbox,script.buf,65,-1);

		ReleaseMutex(m_hMutex_lock);
		return 0;
}

static struct cmd0c_Userdata_t
{	unsigned char head[4];//0x00
	unsigned char rw;
	unsigned char data[64];//0X00000
}cmd0c_wr_userdata={0x00,Head_Sync,0x0c,0x00};


//��д�û����ݣ������Լ�д����߶�һ��64�ֽڣ�
int KM_UserData(int rw,unsigned char *buff)
{		t_config_param config;
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		cmd0c_wr_userdata.rw=rw;//����
		memcpy(cmd0c_wr_userdata.data,buff,64);
		hid_write(fd_kmbox,(const unsigned char *)&cmd0c_wr_userdata,65);
		hid_read_timeout(fd_kmbox,config.buf,65,-1);
		if(rw==0)
			memcpy(buff,config.buf,64);
		ReleaseMutex(m_hMutex_lock);
		return 0;
}




static struct cmd0d_HostVIDPID_t
{	unsigned char head[4];//0x00
	unsigned char rw;
	unsigned char data[64];//0X00000
}cmd0d_wr_hostvidpid={0x00,Head_Sync,0x0d,0x00};


//�ⲿ����������
int KM_HostVidpid(int rw,unsigned int *vidpid,unsigned int *hiddid,unsigned int *mtype)
{		t_config_param config;
		WaitForSingleObject(m_hMutex_lock, INFINITE);
		cmd0d_wr_hostvidpid.rw=0;
		hid_write(fd_kmbox,(const unsigned char *)&cmd0d_wr_hostvidpid,65);
		hid_read_timeout(fd_kmbox,config.buf,65,-1);

		if(rw==0)
		{
			*vidpid=config.ROM.hostVIDPID;
			*hiddid=config.ROM.hostHIDDID;
			*mtype =config.ROM.hostmType_scanTime;
			ReleaseMutex(m_hMutex_lock);
			return 0;
		}else
		{
			config.ROM.hostVIDPID=*vidpid;
			config.ROM.hostHIDDID=*hiddid;
			config.ROM.hostmType_scanTime=*mtype;
			cmd0d_wr_hostvidpid.rw=1;
			memcpy(cmd0d_wr_hostvidpid.data,config.buf,64);
			hid_write(fd_kmbox,(const unsigned char *)&cmd0d_wr_hostvidpid,65);
			hid_read_timeout(fd_kmbox,config.buf,65,-1);
		}
		ReleaseMutex(m_hMutex_lock);
		return 0;
}






typedef struct
{
	unsigned char function;//���� ���̶̹�Ϊ1
	unsigned char  downorup;//
	unsigned char  vkey;	 //����
	unsigned char  reserved; //����
}_keyboard_t;
typedef struct
{
	unsigned char  function; //���� ���̶̹�Ϊ3
	unsigned char  x; //
	unsigned char  xy;//����
	unsigned char  y; //����
}_mouse_t;

typedef union
{	unsigned int   data;
	_keyboard_t    keyboard;
	_mouse_t   	   mouse;
}Script_t;


int Compile(char *str)
{		Script_t script;
		script.data=0;
		if(strncmp("press(",str,6)==0) //ok
		{	script.keyboard.function=0x03;
			script.keyboard.vkey=atoi(str+6);
			//while((*str)!='\n') str++;
		}else if(strncmp("down(",str,5)==0) //ok
		{	script.keyboard.function=0x01;
			script.keyboard.vkey=atoi(str+5);
			//while((*str)!='\n') str++;
		}else if(strncmp("up(",str,3)==0) //ok
		{	script.keyboard.function=0x02;
			script.keyboard.vkey=atoi(str+3);
		}
		else if(strncmp("move(",str,5)==0)//ok
		{
			script.keyboard.function=0x10;
			int x=atoi(str+5);
			while((*str)!=',') str++;
			int y=atoi(str+1);
			script.mouse.x=x&0xff;
			script.mouse.xy=(x>>8)&0x0f|(((y>>8)&0x0f)<<4);
			script.mouse.y=y&0xff;
		}else if(strncmp("left(",str,5)==0)
		{	script.mouse.function=0x11;
			script.mouse.y=atoi(str+5);
		}else if(strncmp("right(",str,6)==0)
		{	script.mouse.function=0x13;
			script.mouse.y=atoi(str+6);
		}else if(strncmp("middle(",str,7)==0)
		{	script.mouse.function=0x12;
			script.mouse.y=atoi(str+7);
		}else if(strncmp("side1(",str,6)==0)
		{
			script.mouse.function=0x14;
			script.mouse.y=atoi(str+6);
		}else if(strncmp("side2(",str,6)==0)
		{	script.mouse.function=0x15;
			script.mouse.y=atoi(str+6);
		}else if(strncmp("wheel(",str,6)==0)
		{
			script.mouse.function=0x16;
			script.mouse.y=atoi(str+6);
		}else if(strncmp("delay(",str,6)==0)
		{	script.keyboard.function=0xde;
			int x=atoi(str+6);
			while((*str)!=',') str++;
			int y=atoi(str+1);
			script.mouse.x=x&0xff;
			script.mouse.xy=(x>>8)&0x0f|(((y>>8)&0x0f)<<4);
			script.mouse.y=y&0xff;
		}
		return script.data;
}

//�ѻ��ű����ر���-----------
int KM_WriteScript(char *name,int index,int trigger,int doneNext,int Switch,char *str)
{	kmbox_t km;
	KM_Readscript(&km);// �ȶ�ȡ������Ϣ
	memset(ROM_SCRIPT,0,64*1024);
	int reallen=strlen(str); //�ַ�������
	int done=0;
	int cmdlen=0; //�ű�ʵ�ʳ��� 128byte���� 32��int
	int ret;
	do
	{	ret=Compile(str);
		if(ret==0) 
			return -1;//�н������˵��ַ�
		else {
			ROM_SCRIPT[cmdlen]=ret;
			cmdlen++;
		}
		while(*str!='\n')
		{	str++;
			done++;
		}
		done++;
		str++;
	}while (done<reallen);

	if((g_script_info.UsedSize+cmdlen)>=g_script_info.TotalSize)
		return -2;//û�д洢�ռ���
	g_script_info.NowIndex=Switch;
	if(	g_script_info.script[index-1].Exist==0)
	{
		g_script_info.script[index-1].Exist=1;			//�ű��Ƿ����
		g_script_info.script[index-1].Onoff=trigger;	//�ű�������ʽ
		g_script_info.script[index-1].RunCnt=doneNext;	//�ű�������Ϻ��״̬
		memcpy(g_script_info.script[index-1].Name,name,strlen(name));//�ű�����
		g_script_info.script[index-1].Length=cmdlen%32?((cmdlen+32)/32*32):((cmdlen));//�ű�ռ�ÿռ� int���ֽ���
		KM_download(g_script_info.script[index-1].StartAddr+g_script_info.UsedSize,ROM_SCRIPT,g_script_info.script[index-1].Length);//����ű���flash
		g_script_info.UsedSize +=g_script_info.script[index-1].Length*4;//�ű���ʹ���ֽ��� 
		KM_Setscript(index, g_script_info.UsedSize,g_script_info.script[index-1].Length*4);

	}else
	{
		return -3;
	}

	return 0;
}

