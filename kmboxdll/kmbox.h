#ifndef __kmbox_h__
#define __kmbox_h__

typedef struct
{	unsigned int  Onoff;		//�ű�������ʽ  
	unsigned int  StartAddr;	//�ű��洢λ��  
	unsigned int  Length;		//�ű�����      
	unsigned int  RunCnt;		//���д���
	int		 	  Exist;		//�ű��Ƿ����
	char 		  Name[12];		//�ű�����    
}script_detail_t;

typedef struct 
{
	unsigned int  NewBoardFlag;// �°��־          
	unsigned int  defaultVID;	//�忨Ĭ�ϵ�VID     
	unsigned int  defaultPID;	//�忨Ĭ�ϵ�PID	    
	unsigned int  TotalSize;	//�ܹ��Ĵ洢�ռ�    
	unsigned int  UsedSize;		//�Ѿ��洢�ռ�   	
	unsigned int  NowIndex;		//��ǰ���еĽű���� ���л��ű���
	script_detail_t script[5];	//����ű���ϸ��Ϣ
}kmbox_t;


/**********************************************************
�������ȵ��ô˺������������ͨ��
���룺   VID��PID ���ӵ�Ӳ��IDֵ��ֱ����ʾ���Ͽ��Կ���
����ֵ�� 
		-1���Ҳ���ָ����VID��PID
		0:����
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
*************************************************************/
int KM_init(unsigned short vid,unsigned short pid);



/**********************************************************
����ָ��������������
���룺    HID��ֵ��
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
ע�⣺���ڵ����λ��ʱ�����Խ�������λ��������ʼ��̵�����
�޷�ֱ�ӿ�����
*************************************************************/
int KM_press(unsigned char vk_key);

/***********************************************************
���̺���
����ָ������һֱ���ְ��� 
���룺    HID��ֵ��
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
***********************************************************/
int KM_down(unsigned char vk_key);

/***********************************************************
���̺���
����ָ������̧�����down����ʹ�ã� 
���룺    HID��ֵ��
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
***********************************************************/
int KM_up(unsigned char vk_key);


/***********************************************************
���̷��ͺ���
ֱ�ӷ��ͼ��̱��棺
ctrButton:Ϊ���Ƽ�����������alt,ctr,shift,window
key�������̵İ���ֵ�����HID��ֵ��Ӧ��key���10�ֽڣ�֧��10���޳�
����ֵ��
		-1������ʧ��
		 0�����ͳɹ�
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
***********************************************************/
int KM_keyboard(unsigned char ctrButton,unsigned char *key);


/***********************************************************
��귢�ͺ���
ֱ�ӷ�����걨�棺
lmr_side:Ϊ���Ƽ����������������м����Ҽ�
x		:���X�᷽���ƶ�����
y		:���Y�᷽���ƶ�����
wheel	:�����ƶ�����
����ֵ��
		-1������ʧ��
		 0�����ͳɹ�
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
***********************************************************/
int KM_mouse(unsigned char lmr_side,short x,short y,unsigned char wheel);


/***********************************************************
���������� 0�ɿ� 1����
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
***********************************************************/
int KM_left(unsigned char vk_key);

/*
����м����� 0�ɿ� 1����
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
*/
int KM_middle(unsigned char vk_key);


/*
����Ҽ������� 0�ɿ� 1����
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
*/
int KM_right(unsigned char vk_key);


/*
�������ƶ�
x		:���X�᷽���ƶ�����
y		:���Y�᷽���ƶ�����
����ֵ��
		-1������ʧ��
		 0�����ͳɹ�
�Ƿ���ԣ���
����ʱ�䣺20211001 by hw
*/
int KM_move(short x,short y);



int KM_wheel(unsigned char  w);
int KM_side1(unsigned char  w);
int KM_side2(unsigned char  w);

//��ȡע���루Ҳ�ɵ�������ʹ�ã�ÿ���忨Ψһ�� outMacΪ16�ֽ�����
int KM_GetRegcode(unsigned char *outMac);
/*
����ע����skeyΪ64�ֽ�˽Կ�ַ�������忨һһ��Ӧ�������ҳ��ԡ�����10��ֱ�Ӹ�ʽ���������ݡ�
����ֵ��
	0������ʧ�ܣ�����ʧ�ܻ�ͳ�ƴ����������������10��ֱ�Ӳ�����Ƭ���������ݣ�����Ƶ�����ԣ�
	1������ɹ����ɹ���������Ч������ˢ�̼������������ҹ̼��ṩ�ߣ�
	-1����Կ����
*/
int  KM_SetRegcode(char * skey);

/*
LCD��ʾ�ַ���
mode ����ʾģʽ
	0 ��ȫ�������ر���ʾ����������str
	-1��������ʾ����str��ʾ�����һ�У�ԭ������˳���Ϲ�
	1����ģʽΪָ��str��ʾ����ʾ����x,yΪ����λ�á�
x,y:
	��mode=1ʱ��Ч�����������Ч
	xȡֵ0-8
	yȡֵ0-128
*/
int KM_LCDstr(int mode,char *str,int x,int y);



/*
LCD��ʾͼƬ��
bmp��Ϊһ��12864��ͼƬ�����ܹ�1KB(ȡģ������������)
*/
int KM_LCDpic(unsigned char *bmp);


/*
�������а��ؽű�
�������û��������� 
�û������д�뿴 KM_UserData����
*/
int KM_ERASE(void);

//��д�û�����
/*
���������û����������ܹ�64�ֽڡ����Դ���û��Զ������ݡ�
������������λ�������У�顣���Լ����ܹ����Ӳ���豸���Ծܾ�����
*/
int KM_UserData(int rw,unsigned char *buff);

//��ȡ�忨�ű���Ϣ
int KM_Readscript(kmbox_t *km);

//���ð忨VID��PID ���籣��
int KM_SetVIDPID(int VID,int PID);


//�ѻ��ű�����
/*
*name:�ű����� ���11���ַ�
index:�ű���� ȡֵ��Χ1-4
trigger����ǰ�ű���������  ȡֵ��Χ0-255. ��Ӧ����ϵİ����� ֧�ְ�λ�������
		������������1���Ҽ���2���м���4�����1��8�������2��16.
		�������������������������trigger=1.������Ҽ�ͬʱ���´���Ϊ��1+2=3
doneNext:�ű��������֮���״̬��0����ͣ�ȴ���ǰ������������ 1��ѭ��ִ�е�ǰ�ű�
switch:������л������ж����λʱ����������Զ��л�����һ����λ��ȡֵͬtrigger
str   ��������Ľű��ַ�����


����ֵ��

*/	
int KM_WriteScript(char *name,int index,int trigger,int doneNext,int Switch,char *str);


int KM_HostVidpid(int rw,unsigned int *vidpid,unsigned int *hiddid,unsigned int *mtype);

//ע����Ȩ����
int MakeKey(char *mac,char *key);
#endif