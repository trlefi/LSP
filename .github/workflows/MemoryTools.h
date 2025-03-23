#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>

struct MAPS
{
	unsigned long long int addr;
	unsigned long long int taddr;
	struct MAPS *next;
};

struct RESULT
{
	unsigned long long int addr;
	struct RESULT *next;
};

struct FREEZE
{
	unsigned long long int addr;       //alamat
	char *value;         //nilai
	unsigned long long int type;            //Jenis dari
	struct FREEZE *next; //Pointer ke node berikutnya
};

#define LEN sizeof(struct MAPS)
#define FRE sizeof(struct FREEZE)
typedef struct MAPS *PMAPS;    //Daftar tertaut dari peta yang disimpan
typedef struct RESULT *PRES;   //Tertaut daftar hasil yang disimpan
typedef struct FREEZE *PFREEZE;//Daftar tertaut untuk menyimpan data beku

typedef int TYPE;
typedef int RANGE;
typedef int COUNT;
typedef int COLOR;
typedef long int OFFSET;
typedef long int ADDRESS;
typedef char PACKAGENAME;

enum type
{
	DWORD,
	FLOAT
};

enum Range
{
	ALL,        //Semua memori
	B_BAD,      //Memori B
	V,	  	//Memori video
	C_ALLOC,    //Memori Ca
	C_BSS,      //Memori Cb
	C_DATA,     //Memori Cd
	C_HEAP,     //Memori ch
	JAVA_HEAP,  //Memori Jh
	A_ANONMYOUS,//Memori A
	CODE_SYSTEM,//Memori xs
	STACK,      //S memory
	ASHMEM,    //Sebagai memori
	CODE_APP, // Memori Xa
	OTHER
};

enum Color
{
	COLOR_SILVERY,    //Perak
	COLOR_RED,        //merah
	COLOR_GREEN,      //hijau
	COLOR_YELLOW,     //kuning
	COLOR_DARK_BLUE,  //biru
	COLOR_PINK,       //Merah Jambu
	COLOR_SKY_BLUE,   //Langit biru
	COLOR_WHITE       //putih
};

PMAPS Res=NULL;//Penggemar global (tempat data disimpan)

PFREEZE Pfreeze=NULL;//Digunakan untuk menyimpan data beku
PFREEZE pEnd=NULL;
PFREEZE pNew=NULL;
int FreezeCount=0;//Jumlah data beku
int Freeze=0;//beralih
pthread_t pth;//pth是线程类型的数据类型
char Fbm[64];//Nama paket
unsigned long long int delay=30000;//Tunda pembekuan, default 30000us

int ResCount=0;//Jumlah hasil
int MemorySearchRange=0;//0 untuk semua

int SetTextColor(int color);
int getRoot(char *argv[]);//Dapatkan izin root
int getPID(char bm[64]);//Dapatkan pid

int SetSearchRange(int type);//Tetapkan lingkup pencarian
PMAPS readmaps(char *bm, int type);
PMAPS readmaps_all(char *bm);
PMAPS readmaps_bad(char *bm);
PMAPS readmaps_v(char *bm);
PMAPS readmaps_c_alloc(char *bm);
PMAPS readmaps_c_bss(char *bm);
PMAPS readmaps_c_data(char *bm);
PMAPS readmaps_c_heap(char *bm);
PMAPS readmaps_java_heap(char *bm);
PMAPS readmaps_a_anonmyous(char *bm);
PMAPS readmaps_code_system(char *bm);
PMAPS readmaps_stack(char *bm);
PMAPS readmaps_ashmem(char *bm);
PMAPS readmaps_code_app(char *bm);
PMAPS readmaps_other(char *bm);
//PMAPS readmaps_il2cpp(char *bm);

void BaseAddressSearch(char *bm,char *value,int *gs,int type,long int BaseAddr);//Pencarian basis
PMAPS BaseAddressSearch_DWORD(char *bm,int value,int *gs,long int BaseAddr,PMAPS pMap);//DWORD
PMAPS BaseAddressSearch_FLOAT(char *bm,float value,int *gs,long int BaseAddr,PMAPS pMap);//FLOAT

void RangeMemorySearch(char *bm,char *from_value,char *to_value,int *gs,int type);//Rentang pencarian
PMAPS RangeMemorySearch_DWORD(char *bm,int from_value,int to_value,int *gs,PMAPS pMap);//DWORD
PMAPS RangeMemorySearch_FLOAT(char *bm,float from_value,float to_value,int *gs,PMAPS pMap);//FLOAT

void MemorySearch(char *bm,char *value,int *gs,int TYPE);//类型搜索,这里value需要传入一个地址
PMAPS MemorySearch_DWORD(char *bm,int value,int *gs,PMAPS pMap);  //内存搜索DWORD
PMAPS MemorySearch_FLOAT(char *bm,float value,int *gs,PMAPS pMap);  //内存搜索FLOAT

void MemoryOffset(char *bm,char *value,long int offset,int *gs,int type);//搜索偏移
PMAPS MemoryOffset_DWORD(char *bm,int value,long int offset,PMAPS pBuff,int *gs);//搜索偏移DWORD
PMAPS MemoryOffset_FLOAT(char *bm,float value,long int offset,PMAPS pBuff,int *gs);//搜索偏移FLOAT

void RangeMemoryOffset(char *bm,char *from_value,char *to_value,long int offset,int *gs,int type);//范围偏移
PMAPS RangeMemoryOffset_DWORD(char *bm,int from_value,int to_value,long int offset,PMAPS pBuff,int *gs);//搜索偏移DWORD
PMAPS RangeMemoryOffset_FLOAT(char *bm,float from_value,float to_value,long int offset,PMAPS pBuff,int *gs);//搜索偏移FLOAT

void MemoryWrite(char *bm,char *value,long int offset,int type);	//内存写入
int MemoryWrite_DWORD(char *bm,int value,PMAPS pBuff,long int offset);	//内存写入DWORD
int MemoryWrite_FLOAT(char *bm,float value,PMAPS pBuff,long int offset);	//内存写入FLOAT

void *SearchAddress(char *bm,long int addr);//搜索地址中的值,返回一个指针
int WriteAddress(char *bm,long int addr,void *value,int type);//修改地址中的值
void BypassGameSafe();//绕过游戏保护
//void RecBypassGameSafe(char *bm);//解除(停止使用)
void Print();//打印Res里面的内容
void ClearResults();//清除链表,释放空间
void ClearMaps(PMAPS pMap);//清空maps

int isapkinstalled(char *bm);//检测应用是否安装
int isapkrunning(char *bm);//检测应用是否运行
int killprocess(char *bm);//杀掉进程
char GetProcessState(char *bm);//获取进程状态
int killGG();//杀掉gg修改器
int killXs();//杀xs
int uninstallapk(char *bm);//静默删除软件
int installapk(char *lj);//静默卸载软件
int rebootsystem();//重启系统(手机)
int PutDate();//输出系统日期
int GetDate(char *date);//获取系统时间

PMAPS GetResults();//获取结果,返回头指针
int AddFreezeItem_All(char *bm,char *Value,int type,long int offset);//冻结所有结果
int AddFreezeItem(char *bm,long int addr,char *value,int type,long int offset);//增加冻结数据
int AddFreezeItem_DWORD(char *bm,long int addr,char *value);//DWORD
int AddFreezeItem_FLOAT(char *bm,long int addr,char *value);//FLOAT
int RemoveFreezeItem(long int addr);//清除固定冻结数据
int RemoveFreezeItem_All();//清空所有冻结数据
int StartFreeze(char *bm);//开始冻结
int StopFreeze();//停止冻结
int SetFreezeDelay(long int De);//设置冻结延迟
int PrintFreezeItems();//打印冻结表

int SetTextColor(COLOR color)
{
	switch (color)
	{
		case COLOR_SILVERY:
		    printf("\033[30;1m");
		    break;
		case COLOR_RED:
		    printf("\033[31;1m");
		    break;
		case COLOR_GREEN:
		    printf("\033[32;1m");
		    break;
		case COLOR_YELLOW:
		    printf("\033[33;1m");
		    break;
		case COLOR_DARK_BLUE:
		    printf("\033[34;1m");
		    break;
		case COLOR_PINK:
		    printf("\033[35;1m");
		    break;
		case COLOR_SKY_BLUE:
		    printf("\033[36;1m");
		    break;
		case COLOR_WHITE:
		    printf("\033[37;1m");
		    break;
		default:
		    printf("\033[37;1m");
		    break;
	}
	return 0;
}

int getRoot(char *argv[])
{
	char ml[64];
	sprintf(ml,"su -c %s",*argv);
	if (getuid() != 0)
	{
		system(ml);
		exit(1);//退出没有root的进程
	}
}

/*int getPID(PACKAGENAME bm[64])
{
	FILE *fp;//文件指针
	pid_t pid;//pid
	char log[64];//命令
	sprintf(log,"pidof %s > log.txt",bm);
	system(log);//执行
	if ((fp = fopen("log.txt", "r")) == NULL)
	{
		return -1;
	}
	fscanf(fp, "%d", &pid);//读取pid
	remove("log.txt");
	return pid;
}*/

int getPID(PACKAGENAME *PackageName)
{
	DIR *dir=NULL;
	struct dirent *ptr=NULL;
	FILE *fp=NULL;
	char filepath[256];			// 大小随意，能装下cmdline文件的路径即可
	char filetext[128];			// 大小随意，能装下要识别的命令行文本即可
	dir = opendir("/proc");		/*打开路径，获取proc子目录下所有的文件和目录列表*/
	
/*DIR 结构体的原型为：struct_dirstream
struct __dirstream   
   {   
    void *__fd;    
    char *__data;    
    int __entry_data;    
    char *__ptr;    
    int __entry_ptr;    
    size_t __allocation;    
    size_t __size;    
    __libc_lock_define (, __lock)    
   };   
typedef struct __dirstream DIR

readdir
功能
读取opendir 返回值的那个列表
返回值
返回dirent结构体指针，dirent结构体成员如下，（文件和目录都行）

struct dirent   
{   
　　long d_ino; //inode number 索引节点号 
　　   
    off_t d_off; /* offset to this dirent 在目录文件中的偏移 
　　   
    unsigned short d_reclen; /* length of this d_name 文件名长 
　　   
    unsigned char d_type; /* the type of d_name 文件类型 
　　   
    char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 
}
*/
	if (NULL != dir)   //如果dir不是目录尾或者打开失败，并且没有发生错误。
	{
		while ((ptr = readdir(dir)) != NULL)	// 循环读取路径下的每一个文件/文件夹  //循环读取dir,目录和文件都去读。
		{
			// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;//跳过
			if (ptr->d_type != DT_DIR)
			/*//d_type值：
		DT_BLK：块设备文件
		DT_CHR：字符设备文件
		DT_DIR：目录文件
		DT_FIFO ：管道文件
		DT_LNK：软连接文件
		DT_REG ：普通文件
		DT_SOCK：本地套接字文件
		DT_UNKNOWN：无法识别的文件类型*/
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);	// 生成要读取的文件的路径  //将ptr读到的文件名称写入filepath
			fp = fopen(filepath, "r");	// 打开文件
			if (NULL != fp)
			{
				fgets(filetext,sizeof(filetext),fp);	// 读取文件
				if (strcmp(filetext,PackageName)==0)
				{
					//puts(filepath);
					//printf("packagename:%s\n",filetext);
					break;
				}
				fclose(fp);
			}
		}
	}
	if (readdir(dir) == NULL)  //假如没有读取到的话
	{
		//puts("Get pid fail");
		return 0;
	}
	closedir(dir);	// 关闭路径
	return atoi(ptr->d_name);  //将char转化成整型数
}

int SetSearchRange(TYPE type)
{
	switch (type)
	{
		case ALL:
		    MemorySearchRange=0;
		    break;
		    //前面默认int MemorySearchRange=0
		case B_BAD:
		    MemorySearchRange=1;
		    break;
		case V:
			MemorySearchRange=2;
			break;
		case C_ALLOC:
		    MemorySearchRange=3;
		    break;
		case C_BSS:
		    MemorySearchRange=4;
		    break;
		case C_DATA:
		    MemorySearchRange=5;
		    break;
		case C_HEAP:
		    MemorySearchRange=6;
		    break;
		case JAVA_HEAP:
		    MemorySearchRange=7;
		    break;
		case A_ANONMYOUS:
		    MemorySearchRange=8;
		    break;
		case CODE_SYSTEM:
		    MemorySearchRange=9;
		    break;
		case STACK:
		    MemorySearchRange=10;
		    break;
		case ASHMEM:
		    MemorySearchRange=11;
		    break;
		case CODE_APP:
		    MemorySearchRange=12;
		    break;
		case OTHER:
		    MemorySearchRange=13;
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	return 0;
}

PMAPS readmaps(char* bm,TYPE type)
/*/struct MAPS  //这个又重新命名称为*PMAPS，所以用到这个的都是返回指针的函数
{
	long int addr;
	long int taddr;
	struct MAPS *next;
};
*/
{
	PMAPS pMap=NULL;
	switch (type)
	{
		case ALL:
		    pMap=readmaps_all(bm);
		    break;
		case B_BAD:
		    pMap=readmaps_bad(bm);
		    break;
		case V:
			pMap = readmaps_v(bm);
			break;
		case C_ALLOC:
		    pMap=readmaps_c_alloc(bm);
		    break;
		case C_BSS:
		    pMap=readmaps_c_bss(bm);
		    break;
		case C_DATA:
		    pMap=readmaps_c_data(bm);
		    break;
		case C_HEAP:
		    pMap=readmaps_c_heap(bm);
		    break;
		case JAVA_HEAP:
		    pMap=readmaps_java_heap(bm);
		    break;
		case A_ANONMYOUS:
		    pMap=readmaps_a_anonmyous(bm);
		    break;
		case CODE_SYSTEM:
		    pMap=readmaps_code_system(bm);
		    break;
		case STACK:
		    pMap=readmaps_stack(bm);
		    break;
		case ASHMEM:
		    pMap=readmaps_ashmem(bm);
		    break;
		case CODE_APP:
            pMap=readmaps_code_app(bm);
            break;
        case OTHER:
            pMap=readmaps_other(bm);
            break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pMap == NULL)
	{
		return 0;
	}
	return pMap;//返回的是一块对应内存类型的链表头
}

PMAPS readmaps_all(PACKAGENAME *bm)
/*struct MAPS
{
	long int addr;  //防止忘记再写一遍。
	long int taddr;
	struct MAPS *next;
};*/
{
	PMAPS pHead=NULL;
	PMAPS pNew;
	PMAPS pEnd;
	pEnd=pNew=(PMAPS)malloc(LEN);// 这里的LEN是sizeof(struct MAPS)
	//说白了就是转化成这个类型，然后给他动态分配内存。pN与pE同时指向这块内存
	
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);//将报名转化成整型赋给pid
	sprintf(lj, "/proc/%d/maps", pid);
	//在proc伪文件系统上许多文件名都是以数字命名
	//proc是在内存上，而不是在硬件上。双引号中是查看进程的虚拟地址空间
	/*/ proc / [PID] / maps显示进程的映射内存的图表。当我们说映射内存时，我们指的是与文件具有一对一对应关系的虚拟内存段。此映射使应用程序可以通过直接读写内存来修改和访问文件。这意味着当程序访问文件时，该文件最终将被记录在其/ proc / [PID] / maps文件中。
/ proc / [PID] / maps还向我们显示了该过程对每个段具有哪些权限，这可以帮助我们确定流程编辑了哪些文件以及读取了哪些文件
详细见
https://m.sohu.com/coo/sg/433127468_675300*/
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{  //	char lj[64], buff[256];再写一遍，防止遗忘。
		fgets(buff,sizeof(buff),fp);//读取一行,sizeof(buff)=256，实际读入255
		if (strstr(buff, "rw") != NULL && !feof(fp))
		//从buff当中没有找到rw出现首地址并且没有读到文件末尾
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//x表示16进制，l长整型，无符号长整形，双精度型
			//这里使用lx是为了能成功读取特别长的地址，format 说明符形式为 [=%[*][width][modifiers]type=]，具体讲解如https://www.runoob.com/cprogramming/c-function-sscanf.html
			//把各地址分配给&后的变量名的结构体
			/*struct MAPS
            {
	            long int addr;  //防止忘记再写一遍。
	            long int taddr;
	            struct MAPS *next;
            };*/
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)  //判断是否为第一次。
	    	{
	    		pNew->next=NULL;  //pNew.next=NULL，这时链表尾链
	    		pEnd=pNew;  //pE=pN，把链表的尾链赋值pE
	    		pHead=pNew;  //把pN赋值给头链
	    		/*如果是第一次读到rw，pN.next指向null来方便指向下一个。
	    		pE是pN当前的位置，
	    		pH作为开头，是当前pN的地址位置*/
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    		
	    		/*因为接下来就不可能是第一次读到rw，我们来解读这三行
	    		pN.next指向一个null便于指向下一个。
	    		pE.next指向pN当前位置，这时pE不是链表末，当前末尾指向新元素
	    		pE地址为pN当前位置，重新将pE作为链表末，新元素的地址变成新末尾
	    		
	    		*/
	    	}
	    	/*
	    	
	    	*/
	    	pNew=(PMAPS)malloc(LEN);//分配内存
	    	//就这样不断的形成单链表储存内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;  //把这块儿内存的头地址返回。
}

PMAPS readmaps_bad(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"kgsl-3d0"))  //这一行当中读到rw，kgsl-3d0并且没有读到文章末尾
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}
PMAPS readmaps_v(PACKAGENAME * bm)
{
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/kgsl-3d0"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		}
		else
		{
			flag = 0;
		}
		if (flag == 1)
		{
			i++;
			if (i == 1)
			{
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			}
			else
			{
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}
PMAPS readmaps_c_alloc(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"[anon:libc_malloc]"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_c_bss(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"[anon:.bss]"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_c_data(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"/data/app/"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_c_heap(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"[heap]"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_java_heap(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"/dev/ashmem/"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_a_anonmyous(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && (strlen(buff) < 42))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_code_system(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"/system"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_stack(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"[stack]"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_other(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"[anon:thread signal stack]"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_ashmem(PACKAGENAME *bm)
{
	PMAPS pHead=NULL;
	PMAPS pNew=NULL;
	PMAPS pEnd=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	FILE *fp;
	int i = 0,flag=1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL)
	{
		puts("分析失败");
		return NULL;
	}
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff,"/dev/ashmem/") && !strstr(buff,"dalvik"))
		{
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			//这里使用lx是为了能成功读取特别长的地址
			flag=1;
		}
		else
		{
			flag=0;
		}
		if (flag==1)
		{
	    	i++;
	    	if (i==1)
	    	{
	    		pNew->next=NULL;
	    		pEnd=pNew;
	    		pHead=pNew;
	    	}
	    	else
	    	{
	    		pNew->next=NULL;
	    		pEnd->next=pNew;
	    		pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);//分配内存
		}
	}
	free(pNew);//将多余的空间释放
	fclose(fp);//关闭文件指针
	return pHead;
}

PMAPS readmaps_code_app(PACKAGENAME *bm)
{
PMAPS pHead=NULL;
PMAPS pNew=NULL;
PMAPS pEnd=NULL;
pEnd=pNew=(PMAPS)malloc(LEN);
FILE *fp;
int i = 0,flag=1;
char lj[64], buff[256];
int pid = getPID(bm);
sprintf(lj, "/proc/%d/maps", pid);
fp = fopen(lj, "r");
if (fp == NULL)
{
puts("分析失败");
return NULL;
}
while (!feof(fp))
{
fgets(buff,sizeof(buff),fp);//读取一行
if (strstr(buff, "r-xp") != NULL && !feof(fp) && strstr(buff,"/data/app/"))
{
sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
//这里使用lx是为了能成功读取特别长的地址
flag=1;
}
else
{
flag=0;
}
if (flag==1)
{
i++;
if (i==1)
{
pNew->next=NULL;
pEnd=pNew;
pHead=pNew;
}
else
{
pNew->next=NULL;
pEnd->next=pNew;
pEnd=pNew;
}
pNew=(PMAPS)malloc(LEN);//分配内存
}
}
free(pNew);//将多余的空间释放
fclose(fp);//关闭文件指针
return pHead;
}




void Print()
{
	PMAPS temp = Res; //79行，Res=NULL
	//R为当前搜索出来的值的头地址
	//这里看到后面构建函数就知道R的用处了
	
	/*struct MAPS
{
	long int addr;
	long int taddr;
	struct MAPS *next;  //再写一遍，防止遗忘
};*/
	int i;
	for (i=0;i<ResCount;i++)  //90行,ResCount=0，所以我猜测后面绝对会有改动这个变量的赋值表达式
	{
		printf("addr:0x%lX,taddr:0x%lX\n", temp->addr, temp->taddr);
		temp = temp->next;//指向下一个节点
	}
}

void ClearResults()//清空
{
	PMAPS pHead=Res; //Res是搜索值的那块内存区域的链表头
	PMAPS pTemp=pHead; //pT与pH同位置
	int i;
	for (i=0;i<ResCount;i++)
	{
		pTemp=pHead;  //pT指向pH
		pHead=pHead->next;//pH指向下一个Res占用的内存
		free(pTemp);//释放当前地址占用的内存
	}
}

/*
typedef int TYPE;
typedef int RANGE;
typedef int COUNT;
typedef int COLOR;
typedef long int OFFSET;
typedef long int ADDRESS;
typedef char PACKAGENAME;  接下来又要用到看一遍，防止遗忘。
*/
void BaseAddressSearch(PACKAGENAME *bm,char *value,COUNT *gs,TYPE type,ADDRESS BaseAddr)
//这里的BaseAddr书写格式为0x16进制数
{
	PMAPS pHead=NULL;
	PMAPS pMap=NULL;
	switch (MemorySearchRange)  //317行，SetSearchRange
	//所以我们这里知道在用这个函数之前需要用到。SetSearchRange函数来变动MemorySearchRange全局变量
	//SetSearchRange设置搜索的内存
	{
		case ALL:
		    pMap=readmaps(bm,ALL);
		    //readmaps返回相应内存类型区域的首地址(内存类型由第二个参数指定)，这里看到第一参数指的就是包名
		    break;
		case B_BAD:
		    pMap=readmaps(bm,B_BAD);//这里bm是包名，且是第一参数
		    break;
		case V:
			pMap = readmaps_v(bm);
			break;
		case C_ALLOC:
		    pMap=readmaps(bm,C_ALLOC);
		    break;
		case C_BSS:
		    pMap=readmaps(bm,C_BSS);
		    break;
		case C_DATA:
		    pMap=readmaps(bm,C_DATA);
		    break;
		case C_HEAP:
		    pMap=readmaps(bm,C_HEAP);
		    break;
		case JAVA_HEAP:
		    pMap=readmaps(bm,JAVA_HEAP);
		    break;
		case A_ANONMYOUS:
		    pMap=readmaps(bm,A_ANONMYOUS);
		    break;
		case CODE_SYSTEM:
		    pMap=readmaps(bm,CODE_SYSTEM);
		    break;
		case STACK:
		    pMap=readmaps(bm,STACK);
		    break;
		case ASHMEM:
		    pMap=readmaps(bm,ASHMEM);
		    break;
		case CODE_APP:
            pMap=readmaps_code_app(bm);
            break;
        case OTHER:
            pMap=readmaps_other(bm);
            break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pMap == NULL)
	{
		puts("map error");
		return (void)0;
	}
	switch (type) //第四参数
	{
		case DWORD:
		    pHead=BaseAddressSearch_DWORD(bm,atoi(value),gs,BaseAddr,pMap);//第一参数，第二参数，第三参数，第五参数
		    //这里第一参数我们知道是包名
		    //第二参数，它的函数用法是将char转化成整型数，
		    //gs是个数，有下面的构建函数可以看到这个gs不管你填多少，传过去之后，他都会变成0，然后进行一个搜索出相应值的统计
		    //第四个参数是一个偏移量
		    //第五个参数是一个只向一块儿相同内存区域的链表
		    //返回的时候搜索出来的value所链接的一个链表(内存类型相同)
		    break;
		case FLOAT:
		    pHead=BaseAddressSearch_FLOAT(bm,atof(value),gs,BaseAddr,pMap);
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pHead == NULL)
	{
		puts("search error");
		return (void)0;
	}
	ResCount=*gs;//这里是搜索出来的个数，赋值给全局RC
	Res=pHead;//Res指针指向链表，赋值给R，便于进行下一步操作
}
/* 
typedef int TYPE;
typedef int RANGE;
typedef int COUNT;
typedef int COLOR;
typedef long int OFFSET;
typedef long int ADDRESS;
typedef char PACKAGENAME;  防止忘记再看一遍。
*/
PMAPS BaseAddressSearch_DWORD(PACKAGENAME *bm,int value,COUNT *gs,ADDRESS BaseAddr,PMAPS pMap)//基址搜索
//第一个参数是包名，第二个参数是你要找的值，第三个是个数，第四个是偏移量，第五个是指向内存类型指针
//typedef long int ADDRESS;

{
	*gs=0;
	//printf("BaseAddress:%lX\n",BaseAddr);
	pid_t pid=getPID(bm);//用到了第一个参数。
	//他这里有写pid_t，我怀疑他是为了装逼，不让人看明白，实际就是整型类型
	/*关于pid_t详细见
	https://blog.csdn.net/mybelief321/article/details/8994424?locationNum=15&fps=1*/
	if (pid == 0)
	{
		puts("can not get pid");//这里是没获取到安装包名
		return 0;
	}
	PMAPS e,n;
	e=n=(PMAPS)malloc(LEN);//#define LEN sizeof(struct MAPS)
	//这里e和n都指向同一块内存，这里不好说具体分配多少，
	//在c4droid中，一个分配了12，两个分配了24，所以是说动态分配
	
	/*struct MAPS
{
	long int addr;
	long int taddr;
	struct MAPS *next;  //再写一遍，防止遗忘
};*/
	PMAPS pBuff=n;//pB与n指向同一块内存，有后面的代码可以看出pB是头链
	int iCount=0;
	unsigned long long int c,ADDR;
	int handle;
	char lj[64];
	sprintf(lj, "/proc/%d/mem", pid);//lj数组存储路径
	handle = open(lj, O_RDWR);//打开mem文件，(原型int)成功返回文件句柄，说白了就是文件的开头地址，是一个文件描述符，学过文件指针的应该知道，失败的返回-1
	/*O_RDONLY 以只读方式打开文件
    O_WRONLY 以只写方式打开文件
    O_RDWR 以可读写方式打开文件
    上述三种旗标是互斥的，也就是不可同时使用，但可与下列的旗标利用OR(|)运算符组合。*/
	lseek(handle, 0, SEEK_SET);//读写位置移到开头
	/*
	SEEK_SET 参数offset 即为新的读写位置.
    SEEK_CUR 以目前的读写位置往后增加offset 个位移量.
    SEEK_END 将读写位置指向文件尾后再增加offset 个位移量. 当whence 值为SEEK_CUR 或
    SEEK_END 时, 参数offet 允许负值的出现.
下列是教特别的使用方式:
1) 欲将读写位置移到文件开头时:lseek(int fildes, 0, SEEK_SET);
2) 欲将读写位置移到文件尾时:lseek(int fildes, 0, SEEK_END);
3) 想要取得目前文件位置时:lseek(int fildes, 0, SEEK_CUR);

返回值：当调用成功时则返回目前的读写位置, 也就是距离文件开头多少个字节. 若有错误则返回-1, errno 会存放错误代码.
	*/
	void *BUF[8];
	PMAPS pTemp=pMap;//将第五个参数位置赋给pT，pM是一块内存类型区域的头地址。
	while (pTemp != NULL)//pT不是末链就循环
	{
		c=(pTemp->taddr-pTemp->addr)/4096;
		//pT末地址减去首地址  再除以4096，4096在16进制是1000，每一个链表的首地址和末地址之间，相差都是1000(16进制)的倍数，所以这一行求的是相差为1000的一共有多少个数，这个数不会很大
		//他这里的意思是以1000(16进制)为一个节点，c为一个链条首地址与末地址之间有几个节点 
		for (int j=0;j<c;j++)
		{
			ADDR=pTemp->addr+j*4096+BaseAddr;//用到了第四参数
			//说白了就是偏移数值，末地址加上j乘4096加第四参数，BaseAddr我猜这个数值不能超过4096，否则最后一次循环时会有突破链表地址的可能
			pread64(handle,BUF,8,ADDR);
			//第一个参数文件描述符在形式上是一个非负整数。实际上，它是一个索引值，指向内核为每一个进程所维护的该进程打开文件的记录表。
			//第二个参数是用来存储读取出来的数据。
			//第三个是读取的字节
			//第四个就是偏移量，读取的起始地址的偏移量，读取地址=文件开始+ADDR。注意，执行后，文件偏移指针不变，比如填个6
			//这个函数的用法是在handle指定的文件中，从开头偏移ADDR的位置，从这个位置开始读取八个字节存储到BUF
			if (*(int*)&BUF[0] == value)//如果BUF数值和第二个参数相同
			{
				iCount++; //链表长度
				*gs+=1;
		    	ResCount+=1;
			    n->addr=ADDR;//相同了就把AD地址赋值给n的首地址
			    //printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
			    if (iCount==1)//链表为头
		        {
		            n->next=NULL;
		            e=n;//e为n当前地址位置，有end的意思
		            pBuff=n;//pB作为一个链表首链
	            }
	            else
	            {
	                n->next=NULL;
	            	e->next=n;//e的下一个位置指向n，即末尾的元素指向一个新的元素。
		            e=n;//e为n位置，新的地址变成新的末尾
	            }
	            n=(PMAPS)malloc(LEN);//为new或者说n分配内存
			}
		}
		pTemp=pTemp->next;//转向下一个相同内存类型地址。
	}
	close(handle);
	return pBuff;//返回value相同的同一内存类型的一个链表头地址
}

PMAPS BaseAddressSearch_FLOAT(PACKAGENAME *bm,float value,COUNT *gs,ADDRESS BaseAddr,PMAPS pMap)
//typedef long int ADDRESS;
{
	*gs=0;
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	PMAPS e,n;
	e=n=(PMAPS)malloc(LEN);
	PMAPS pBuff=n;
	unsigned long long int c,ADDR;
	int handle;
	int iCount=0;
	char lj[64];
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);//打开mem文件，以读写方式
	lseek(handle, 0, SEEK_SET);
	void *BUF[8];
	PMAPS pTemp=pMap;
	while (pTemp != NULL)
	{
		c=(pTemp->taddr-pTemp->addr)/4096;
		for (int j=0;j<c;j++)
		{
			ADDR=pTemp->addr+j*4096+BaseAddr;
			pread64(handle,BUF,8,ADDR);
			if (*(float*)&BUF[0] == value)//与上面那构造函数不同之处，将值转换为float类型进行比较
			{
				iCount++;
				*gs+=1;
		    	ResCount+=1;
			    n->addr=ADDR;
			    //printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
			    if (iCount==1)
		        {
		            n->next=NULL;
		            e=n;
		            pBuff=n;
	            }
	            else
	            {
	                n->next=NULL;
	            	e->next=n;
		            e=n;
	            }
	            n=(PMAPS)malloc(LEN);
			}
		}
		pTemp=pTemp->next;
	}
	close(handle);
	return pBuff;
}

void RangeMemorySearch(PACKAGENAME *bm,char *from_value,char *to_value,COUNT *gs,TYPE type)//范围搜索，这里value填写的是数值，不是地址 

{
	PMAPS pHead=NULL;
	PMAPS pMap=NULL;
	switch (MemorySearchRange)
	//这里我们依然看到，需要设置内存，其函数在317行
	{
		case ALL:
		    pMap=readmaps(bm,ALL);
		    //readmap返回的是对应内存类型的链表头，其对应内存类型为第二函数
		    //所以看到现在这个指针指向ALL内存链表头
		    //这里用到了第一参数
		    break;
		case B_BAD:
		    pMap=readmaps(bm,B_BAD);
		    break;
		case V:
			pMap = readmaps_v(bm);
			break;
		case C_ALLOC:
		    pMap=readmaps(bm,C_ALLOC);
		    break;
		case C_BSS:
		    pMap=readmaps(bm,C_BSS);
		    break;
		case C_DATA:
		    pMap=readmaps(bm,C_DATA);
		    break;
		case C_HEAP:
		    pMap=readmaps(bm,C_HEAP);
		    break;
		case JAVA_HEAP:
		    pMap=readmaps(bm,JAVA_HEAP);
		    break;
		case A_ANONMYOUS:
		    pMap=readmaps(bm,A_ANONMYOUS);
		    break;
		case CODE_SYSTEM:
		    pMap=readmaps(bm,CODE_SYSTEM);
		    break;
		case STACK:
		    pMap=readmaps(bm,STACK);
		    break;
		case ASHMEM:
		    pMap=readmaps(bm,ASHMEM);
		    break;
		case CODE_APP:
            pMap=readmaps_code_app(bm);
            break;
        case OTHER:
            pMap=readmaps_other(bm);
            break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pMap == NULL)
	{
		puts("map error");//写个判断，这里是给萌新看的你都看到这里了，肯定会好奇，为什么会有这些好像是无用的代码，c程序员都会去写一些类似于这样的判断来判断代码的错误，假如发生错误这样既可以查看错误，又可以在判断当中运行程序段
		return (void)0;
	}
	switch (type)//这里用到还第五个参数
	{
		case DWORD:
		    if (atoi(from_value) > atoi(to_value))
		    //这里主要是怕有人第一个填写的值比较大，第二个填写的值比较小，应该填写小的值在前面，大的值在后面，我还是习惯称为初值和末值
		    	pHead=RangeMemorySearch_DWORD(bm,atoi(to_value),atoi(from_value),gs,pMap);//用到一二三四参数
		    else
		        pHead=RangeMemorySearch_DWORD(bm,atoi(from_value),atoi(to_value),gs,pMap);
		    break;
		case FLOAT:
		    if (atof(from_value) > atof(to_value))
		    	pHead=RangeMemorySearch_FLOAT(bm,atof(to_value),atof(from_value),gs,pMap);
		    else
		        pHead=RangeMemorySearch_FLOAT(bm,atof(from_value),atof(to_value),gs,pMap);
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pHead == NULL)
	{
		puts("RangeSearch Error");
		return (void)0;
	}
	ResCount=*gs;//统计搜索到的值个数
	Res=pHead;//Res指针指向链表
}

PMAPS RangeMemorySearch_DWORD(PACKAGENAME *bm,int from_value,int to_value,COUNT *gs,PMAPS pMap)//DWORD
{
	pid_t pid=getPID(bm);//获取包名，并将包名转化成整型数
	if (pid == 0)
	{
		puts("can not get pid");
		return NULL;
	}
	*gs=0;//搜索出值个数初始化为0，如果说他也是链表长度也没有错
	PMAPS pTemp=NULL;
	pTemp=pMap;
	PMAPS n,e;
	e=n=(PMAPS)malloc(LEN);//#define LEN sizeof(struct MAPS)
	PMAPS pBuff;//和下面的那一步没合并到一起，这是可以简化的
	pBuff=n;
	int handle;//句柄，这个我在前面注释说过，
	int iCount=0;//链表长度
	int c;
	char lj[64];//路径
	int buff[1024]={0};//缓冲区
	memset(buff,0,4);//将buff所指的内存区前4个字节设置为0
	//差不多就是将buff[0]的元素设置为零
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);//打开mem文件
	/*采用O_RDWR 在使用pread64和pwrite64时要求buffer的起始地址是4K对齐的，否则会失败，另外offset的偏移地址要求是512的整数倍。
pread64和pwrite64的参数问题，主要是buffer的起始地址和offset，分别要求是4k对齐和512的整数倍。
	*/
	lseek(handle, 0, SEEK_SET);
	//	SEEK_SET 参数offset 即为新的读写位置.
	while (pTemp != NULL)//记住pT=pM，pM是一块内存类型链表
	{
		c=(pTemp->taddr-pTemp->addr)/4096;
		//为什么要除以4096呢？如果你上网上搜过那种Linux的地址图片，你会知道他的一行中地址是 首地址-末地址 这种形式
		//而在一行当中，首地址和末地址相差的都是16进制的1000的倍数，相当于十进制4096的倍数，说白了就是以1000作为一个节点，做完一个节点，进行下一个节点。不理解就不理解吧。
		for (int j=0;j<c;j++)
		{
		    pread64(handle,buff,0x1000,pTemp->addr+j*4096);
		    //0×1000为4kb，也就是从这个第三个参数的位置开始读取4KB的数据存储buff缓存区当中，实际也就是读取1024个整型数，因为16进制的1000等于十进制的4096，而数组buff他是一个整型数是一元素个四字节，所以是1024个元素
		    //说白了就是当j=0的时候，在原地址上读取1000个内容，然后进行到下一次循环之后再偏移1000，再读取1000
		    //这里我们可以理解读取的是地址上的内容到缓存区buff
		    for (int i=0;i<1024;i++)
	    	{
		    	if (buff[i] >= from_value && buff[i] <= to_value)//判断值是否在这两者之间
		    	{
		    		iCount++;  //链表长度+1
		    		*gs+=1;  //个数加1
		    		ResCount+=1;  //个数加1
			    	n->addr=(pTemp->addr)+(j*4096)+(i*4);//把当前的地址赋值给n.addr，这里在外循环j，因为他每次读取1024个元素，这里你就理解成读取到的第j个1024个元素，在此基础上，第i个地址的内容，因为每个内容占四个字节，所以要乘以4
			    	if (iCount==1)//如果是头链
		            {
		            	n->next=NULL;
		            	e=n;//e为尾链，目前和n为同一位置
		            	pBuff=n;//pB现在为头链
	            	}
	            	else//如果不是头链
	                {
	                	n->next=NULL;
	            		e->next=n;
		            	e=n;//e继承n，并且让n继续工作。
	                }
	            	n=(PMAPS)malloc(LEN);//给n重新分配内存
		    	}
	    	}
		}
		pTemp = pTemp->next;//pT进入链表下一条数据
	}
	free(n);
	close(handle);
	return pBuff;//在from_value与to_value范围之间的值，构成一个链表返回头链
	//上面的ResCount已经记录搜索出来的有多少个数据
}

PMAPS RangeMemorySearch_FLOAT(PACKAGENAME *bm,float from_value,float to_value,COUNT *gs,PMAPS pMap)//FLOAT
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return NULL;
	}
	*gs=0;
	PMAPS pTemp=NULL;
	pTemp=pMap;
	PMAPS n,e;
	e=n=(PMAPS)malloc(LEN);
	PMAPS pBuff;
	pBuff=n;
	int handle;//句柄
	int iCount=0;//链表长度
	int c;
	char lj[64];//路径
	float buff[1024]={0};//缓冲区
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);//打开mem文件
	lseek(handle, 0, SEEK_SET);
	while (pTemp->next != NULL)
	{
		c=(pTemp->taddr-pTemp->addr)/4096;
		for (int j=0;j<c;j+=1)
		{
		    pread64(handle,buff,0x1000,pTemp->addr+(j*4096));
		    //这里你要知道float类型也占了四字节
		    for (int i=0;i<1024;i+=1)
	    	{
		    	if (buff[i] >= from_value && buff[i] <= to_value)//判断。。。
		    	{
		    		iCount++;
		    		*gs+=1;
		    		ResCount+=1;
			    	n->addr=(pTemp->addr)+(j*4096)+(i*4);
			    	if (iCount==1)
		            {
		            	n->next=NULL;
		            	e=n;
		            	pBuff=n;
	            	}
	            	else
	                {
	                	n->next=NULL;
	            		e->next=n;
		            	e=n;
	                }
	            	n=(PMAPS)malloc(LEN);
		    	}
		    	//printf("buff[%d]=%f\n",l,buff[l]);
		    	//usleep(1);
	    	}
	    	//memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	close(handle);
	return pBuff;
}

void MemorySearch(PACKAGENAME *bm,char *value,int *gs,TYPE type)
//精确搜索
{
	PMAPS pHead=NULL;
	PMAPS pMap=NULL;
	switch (MemorySearchRange)//需要设置内存，我记得设置函数在317行
	{
		case ALL:
		    pMap=readmaps(bm,ALL);//pM是一块连接起来相同内存类型链表的头链。
		    break;
		case B_BAD:
		    pMap=readmaps(bm,B_BAD);//用到了第一参数
		    break;
		case V:
			pMap = readmaps_v(bm);
			break;
		case C_ALLOC:
		    pMap=readmaps(bm,C_ALLOC);
		    break;
		case C_BSS:
		    pMap=readmaps(bm,C_BSS);
		    break;
		case C_DATA:
		    pMap=readmaps(bm,C_DATA);
		    break;
		case C_HEAP:
		    pMap=readmaps(bm,C_HEAP);
		    break;
		case JAVA_HEAP:
		    pMap=readmaps(bm,JAVA_HEAP);
		    break;
		case A_ANONMYOUS:
		    pMap=readmaps(bm,A_ANONMYOUS);
		    break;
		case CODE_SYSTEM:
		    pMap=readmaps(bm,CODE_SYSTEM);
		    break;
		case STACK:
		    pMap=readmaps(bm,STACK);
		    break;
		case ASHMEM:
		    pMap=readmaps(bm,ASHMEM);
		    break;
		case CODE_APP:
            pMap=readmaps_code_app(bm);
            break;
        case OTHER:
            pMap=readmaps_other(bm);
            break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pMap == NULL)
	{
		puts("map error");
		return (void)0;
	}
	switch (type)//第四参数
	{
		case DWORD:
		    pHead=MemorySearch_DWORD(bm,atoi(value),gs,pMap);
		    //第一参数，第二参数，第三参数，pM是内存头链。
		    break;
		case FLOAT:
		    pHead=MemorySearch_FLOAT(bm,atof(value),gs,pMap);
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pHead == NULL)
	{
		puts("search error");
		return (void)0;
	}
	ResCount=*gs;//统计搜索出来的个数
	Res=pHead;//Res指针指向链表
}

PMAPS MemorySearch_DWORD(PACKAGENAME *bm,int value,COUNT *gs,PMAPS pMap)
{
	pid_t pid=getPID(bm);//pid获取到标识包名
	if (pid == 0)
	{
		puts("can not get pid");
		return NULL;
	}
	*gs=0;
	PMAPS pTemp=NULL;
	pTemp=pMap;//pT现在是一块内存类型区域链表头链
	PMAPS n,e;
	e=n=(PMAPS)malloc(LEN);//指向同一块内存
	PMAPS pBuff;
	pBuff=n;//pB指向n所指的内存。
	int handle;//句柄
	int iCount=0;//链表长度
	int c;
	char lj[64];//路径
	int buff[1024]={0};//缓冲区
	memset(buff,0,4);//缓冲区中前四个字节设置为零
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);//打开mem文件
	lseek(handle, 0, SEEK_SET);
	while (pTemp != NULL)//读取maps里面的地址
	{
		c=(pTemp->taddr-pTemp->addr)/4096;
		for (int j=0;j<c;j++)
		{
		    pread64(handle,buff,0x1000,pTemp->addr+j*4096);
		    //这里说白了意思就是，从pT头链.addr开始，读取存放1024元素，然后再偏移1024元素位置。这个读取是读取连续的内存，因为每一段链条的手地址和末地址相减都是十进制4096的倍数，所以说他是以次为节点，读取1000，存放起来，然后偏移1000，在新的位置上再读取1000，说的已经很明白了。
		    for (int i=0;i<1024;i++)
		    //到这里都跟前面所讲的操作一样，所以我会省略大部分
	    	{
		    	if (buff[i]==value)
		    	{
		    		iCount++;//记录是否为头链
		    		*gs+=1;//统计搜索到的个数
		    		ResCount+=1;//统计搜索到的个数
			    	n->addr=(pTemp->addr)+(j*4096)+(i*4);//同前面注视所讲的操作。
			    	//printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
			    	if (iCount==1)
		            {
		            	n->next=NULL;
		            	e=n;//e作尾链，但当前位置与pB为同一位置。
		            	pBuff=n;//pB作为搜索到内容的头链
	            	}
	            	else
	                {
	                	n->next=NULL;
	            		e->next=n;//e作为尾链指向一个新的元素。
		            	e=n;//n成为新的尾链，并赋值给e
	                }
	            	n=(PMAPS)malloc(LEN);//给n重新分配内存
		    	}
	    	}
		}
		pTemp = pTemp->next;//指向pt链条的下一条
	}
	free(n);//释放n的内存
	close(handle);//关闭句柄
	return pBuff;//返回与value相同的，且内存类型相同的链表的头链。
}

PMAPS MemorySearch_FLOAT(PACKAGENAME *bm,float value,COUNT *gs,PMAPS pMap)
//同上
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return NULL;
	}
	*gs=0;
	PMAPS pTemp=NULL;
	pTemp=pMap;
	PMAPS n,e;
	e=n=(PMAPS)malloc(LEN);
	PMAPS pBuff;
	pBuff=n;
	int handle;//句柄
	int iCount=0;//链表长度
	int c;
	char lj[64];//路径
	float buff[1024]={0};//缓冲区
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);//打开mem文件
	lseek(handle, 0, SEEK_SET);
	while (pTemp->next != NULL)
	{
		c=(pTemp->taddr-pTemp->addr)/4096;
		for (int j=0;j<c;j+=1)
		{
		    pread64(handle,buff,0x1000,pTemp->addr+(j*4096));
		    for (int i=0;i<1024;i+=1)
	    	{
		    	if (buff[i]==value)
		    	{
		    		iCount++;
		    		*gs+=1;
		    		ResCount+=1;
			    	n->addr=(pTemp->addr)+(j*4096)+(i*4);
			    	if (iCount==1)
		            {
		            	n->next=NULL;
		            	e=n;
		            	pBuff=n;
	            	}
	            	else
	                {
	                	n->next=NULL;
	            		e->next=n;
		            	e=n;
	                }
	            	n=(PMAPS)malloc(LEN);
		    	}
		    	//printf("buff[%d]=%f\n",l,buff[l]);
		    	//usleep(1);
	    	}
	    	//memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	close(handle);
	return pBuff;
}

void MemoryOffset(PACKAGENAME *bm,char *value,OFFSET offset,COUNT *gs,TYPE type)
{
	PMAPS pHead=NULL;
	switch (type)//第五个参数
	{
		case DWORD:
		    pHead=MemoryOffset_DWORD(bm,atoi(value),offset,Res,gs);
		    //用到第1234参数
		    //Res为当前搜索到的值链表得头链
		    break;
		case FLOAT:
		    pHead=MemoryOffset_FLOAT(bm,atof(value),offset,Res,gs);
		    //看了下面的构造函数之后，我知道Memoryoffset返回的是一个链表头，而链表中的每一个地址经过offset偏移后与value参数相同
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pHead == NULL)
	{
		puts("offset error");
		return (void)0;
	}
	ResCount=*gs;//全局个数
	ClearResults();//清空存储的数据(释放空间)
	Res=pHead;//指向新搜索到的空间
}

PMAPS MemoryOffset_DWORD(PACKAGENAME *bm,int value,OFFSET offset,PMAPS pBuff,COUNT *gs)//搜索偏移//typedef long int OFFSET;
{
	pid_t pid=getPID(bm);//获取pid
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	*gs=0;//初始个数为0
	PMAPS pEnd=NULL;
	PMAPS pNew=NULL;
	PMAPS pTemp=pBuff;//把当前的已搜索到的头链赋值给pT
	PMAPS BUFF=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);//分配pE,pN并共同指向同一块内存
	BUFF=pNew;//将这块内存给BUFF
	int iCount=0,handle;//个数与句柄
	char lj[64];//路径
	unsigned long long int all;//总和
	int *buf=(int *)malloc(sizeof(int));//缓冲区,为其分配四个字节
	int jg;
	sprintf(lj,"/proc/%d/mem",pid);//写出路径
	handle=open(lj,O_RDWR);//读写的方式打开
	lseek(handle,0,SEEK_SET);//读写位置光标放到句柄开头
	while (pTemp != NULL)//不是尾链就循环
	{
		all=pTemp->addr+offset;//链条中的首地址加上偏移量(说白了就是 一个数值)把它赋值给all
		pread64(handle,buf,4,all);//从all的位置读取，4个字节，也就是读取一个整型数到buf
		jg=*buf;//把这个整型数赋值给jg
		if (jg == value)//如果与第二个参数value相等的话
		{
			iCount++;//用来是否判断是头链
			*gs+=1;//搜索到的数量
			pNew->addr=pTemp->addr;//把pT当前链条中的首地址赋值给pN.addr
			if (iCount == 1)//如果是头链
	    	{
		    	pNew->next=NULL;
		    	pEnd=pNew;//pE为尾链，把pN的位置给pE
		    	BUFF=pNew;//BUFF为头链,位置同上
	    	}
	    	else//非头链
	    	{
		    	pNew->next=NULL;
		    	pEnd->next=pNew;//pE.next指向新元素pN，pN成为尾链
		    	pEnd=pNew;//pE再次成为尾链
	    	}
	    	pNew=(PMAPS)malloc(LEN);//重新分配内存
	    	if (ResCount==1)
	    	{
	    		free(pNew);
            	close(handle);
            	return BUFF;//返回经过offset偏移后，链表中与value相同的值的头链。
            	/*假如不理解，你就想象出有一张列表 
            	列表当中本来有很多个不同数值，但是有一些值经过相同的偏移，会偏移到相同的值。比如说设定offset为5，value为15，一个值偏移4之后，得到一个15，，而另一个值偏移4之后也得到一个15，也许这两个15的地址不同 ，但值相同，就把第一个偏移到15的地址，作为列表中的第一个值，第二个偏移到15的地址，作为列表的第二值
            	用了这个函数之后，链表中的地址是不加上offset的原地址，这点要注意，我怕打乱你们思路
            	
            	*/
	    	}
		}
		/*else
		{
			printf("jg:%d,value:%d\n",jg,value);
		}*/
		pTemp=pTemp->next;//指向下一个节点读取数据
	}
	free(pNew);//释放内存
	close(handle);
	return BUFF;
}

PMAPS MemoryOffset_FLOAT(PACKAGENAME *bm,float value,OFFSET offset,PMAPS pBuff,COUNT *gs)//搜索偏移
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	*gs=0;//初始个数为0
	PMAPS pEnd=NULL;
	PMAPS pNew=NULL;
	PMAPS pTemp=pBuff;
	PMAPS BUFF=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	BUFF=pNew;
	int iCount=0,handle;//个数与句柄
	char lj[64];//路径
	unsigned long long int all;//总和
	float *buf=(float *)malloc(sizeof(float));//缓冲区
	//int buf[16];  //出现异常使用
	float jg;
	sprintf(lj,"/proc/%d/mem",pid);
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);
	while (pTemp != NULL)
	{
		all=pTemp->addr+offset;//偏移后的地址
		pread64(handle,buf,4,all);
		jg=*buf;
		if (jg == value)
		{
			iCount++;
			*gs+=1;
			//printf("偏移成功,addr:%lx\n",all);
			pNew->addr=pTemp->addr;
			if (iCount == 1)
	    	{
		    	pNew->next=NULL;
		    	pEnd=pNew;
		    	BUFF=pNew;
	    	}
	    	else
	    	{
		    	pNew->next=NULL;
		    	pEnd->next=pNew;
		    	pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);
	    	if (ResCount==1)
	    	{
	    		free(pNew);
            	close(handle);
            	return BUFF;
	    	}
		}
		/*else
		{
			printf("jg:%e,value:%e\n",jg,value);
		}*/
		pTemp=pTemp->next;//指向下一个节点读取数据
	}
	free(pNew);
	close(handle);
	return BUFF;
}

void RangeMemoryOffset(PACKAGENAME *bm,char *from_value,char *to_value,OFFSET offset,COUNT *gs,TYPE type)//范围偏移，共6参数
{
	PMAPS pHead=NULL;
	switch (type)//到了第六个参数
	{
		case DWORD:
		    if (atoi(from_value) > atoi(to_value))//这里是防止有人第二个参数写大了，第三个参数写小了，记住这里填的是值，不是地址
		    	pHead=RangeMemoryOffset_DWORD(bm,atoi(to_value),atoi(from_value),offset,Res,gs);//如果填写的第一个地址大于填写的第二个地址，那把填写的第二个地址作为范围开始到写的第一个地址作为范围结束 
		    else
		        pHead=RangeMemoryOffset_DWORD(bm,atoi(from_value),atoi(to_value),offset,Res,gs);//按照上面来推理 
		    break;
		case FLOAT://也是如此推理
		    if (atof(from_value) > atof(to_value))
		    	pHead=RangeMemoryOffset_FLOAT(bm,atof(to_value),atof(from_value),offset,Res,gs);
		    else
		        pHead=RangeMemoryOffset_FLOAT(bm,atof(from_value),atof(to_value),offset,Res,gs);
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	if (pHead == NULL)
	{
		puts("RangeOffset error");
		return (void)0;
	}
	ResCount=*gs;//全局个数
	ClearResults();//清空存储的数据(释放空间)
	Res=pHead;//指向新搜索到的空间
}

PMAPS RangeMemoryOffset_DWORD(PACKAGENAME *bm,int from_value,int to_value,OFFSET offset,PMAPS pBuff,COUNT *gs)//搜索偏移DWORD，共六个参数 
{
	pid_t pid=getPID(bm);//获取标识包名
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	*gs=0;//初始个数为0
	PMAPS pEnd=NULL;
	PMAPS pNew=NULL;
	PMAPS pTemp=pBuff;//pB是当前搜索到的内存区域的头链，pT指向此处
	PMAPS BUFF=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);//分配内存给pE,pN
	BUFF=pNew;//BUFF指向pN所在的内存位置
	int iCount=0,handle;//个数与句柄
	char lj[64];//路径
	unsigned long long int all;//总和
	int *buf=(int *)malloc(sizeof(int));//缓冲区，4字节
	int jg;
	sprintf(lj,"/proc/%d/mem",pid);//将路径写入lj
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);//句柄开头
	while (pTemp != NULL)
	{
		all=pTemp->addr+offset;//pT.addr加上第四参数offset的地址作为值赋给all
		pread64(handle,buf,4,all);//从开头偏移all，读取4字节，说白了，读取一个整型数给buf
		jg=*buf;//把这个整形数赋给jg
		if (jg >= from_value && jg <= to_value)
		//如果这个数比
		{
			iCount++;//链表长度
			*gs+=1;//搜索到值的个数
			pNew->addr=pTemp->addr;//pN.addr现在等于pT所指向内存区域的地址，这块内存区域里面有已经搜索到的值
			if (iCount == 1)//判断是否制成头链
	    	{
		    	pNew->next=NULL;
		    	pEnd=pNew;//尾链，现在都指向同一个位置 
		    	BUFF=pNew;//头链
	    	}
	    	else
	    	{
		    	pNew->next=NULL;
		    	pEnd->next=pNew;//pE像一个新的元素，pN成为尾链
		    	pEnd=pNew;//pE代替pN成为尾链
	    	}
	    	pNew=(PMAPS)malloc(LEN);//重新分配内存 
	    	if (ResCount==1)//判断链表是否只有头链，这里ResCount指的是内存空间当中有多少条地址，判断依据为pBuff，因为我看到上面填写构造函数时填的是Res，调用过这个的函数就会调用到ResCount，这里判断在上面链表形成之前具有的地址数量，
	    	{
	    		free(pNew);
            	close(handle);
            	return BUFF;//返回BUFF的地址，说白了就是原来的pTemp.addr，其地址经过offset偏移后等于value范围内
	    	}
		}
		/*else
		{
			printf("jg:%d,value:%d\n",jg,value);
		}*/
		pTemp=pTemp->next;//指向下一个节点读取数据
	}
	free(pNew);
	close(handle);
	return BUFF;//返回链表头链，表中的地址经过相同offset偏移后，值在fvalue与tvalue范围内
}

PMAPS RangeMemoryOffset_FLOAT(PACKAGENAME *bm,float from_value,float to_value,OFFSET offset,PMAPS pBuff,COUNT *gs)//搜索偏移FLOAT
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	*gs=0;//初始个数为0
	PMAPS pEnd=NULL;
	PMAPS pNew=NULL;
	PMAPS pTemp=pBuff;
	PMAPS BUFF=NULL;
	pEnd=pNew=(PMAPS)malloc(LEN);
	BUFF=pNew;
	int iCount=0,handle;//个数与句柄
	char lj[64];//路径
	unsigned long long int all;//总和
	float *buf=(float *)malloc(sizeof(float));//缓冲区
	//int buf[16];  //出现异常使用
	float jg;
	sprintf(lj,"/proc/%d/mem",pid);
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);
	while (pTemp != NULL)
	{
		all=pTemp->addr+offset;//偏移后的地址
		pread64(handle,buf,4,all);
		jg=*buf;
		if (jg >= from_value && jg <= to_value)
		{
			iCount++;
			*gs+=1;
			//printf("偏移成功,addr:%lx\n",all);
			pNew->addr=pTemp->addr;
			if (iCount == 1)
	    	{
		    	pNew->next=NULL;
		    	pEnd=pNew;
		    	BUFF=pNew;
	    	}
	    	else
	    	{
		    	pNew->next=NULL;
		    	pEnd->next=pNew;
		    	pEnd=pNew;
	    	}
	    	pNew=(PMAPS)malloc(LEN);
	    	if (ResCount==1)
	    	{
	    		free(pNew);
            	close(handle);
            	return BUFF;
	    	}
		}
		/*else
		{
			printf("jg:%e,value:%e\n",jg,value);
		}*/
		pTemp=pTemp->next;//指向下一个节点读取数据
	}
	free(pNew);
	close(handle);
	return BUFF;
}

void MemoryWrite(PACKAGENAME *bm,char *value,OFFSET offset,TYPE type)
{
	switch (type)//用到了第四参数 
	{
		case DWORD://值类型
		    MemoryWrite_DWORD(bm,atoi(value),Res,offset);
		    break;
		case FLOAT:
		    MemoryWrite_FLOAT(bm,atof(value),Res,offset);
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	//ClearResults();//清空list
}

int MemoryWrite_DWORD(PACKAGENAME *bm,int value,PMAPS pBuff,OFFSET offset)//内存写入，内存改写
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	PMAPS pTemp=NULL;
	char lj[64];
	int handle;
	pTemp=pBuff;//pB为当前搜索出来的内存区域的链表头
	sprintf(lj,"/proc/%d/mem",pid);
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);
	int i;
	for (i=0;i<ResCount;i++)//当前列表当中一共有多少条地址 
	//这个说白了就是把空间列表中所有的地址内容改写成value值
	{
		pwrite64(handle,&value,4,pTemp->addr+offset);//从文件开始偏移pTemp.addr+offset的位置，写入4个字节的value值,或者说改成也可以
		if (pTemp->next != NULL)
	        pTemp=pTemp->next;//pT成为链表中下一段地址
	}
	close(handle);
	return 0;
}

int MemoryWrite_FLOAT(PACKAGENAME *bm,float value,PMAPS pBuff,OFFSET offset)//同上
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	PMAPS pTemp=NULL;
	char lj[64];
	int handle;
	pTemp=pBuff;
	sprintf(lj,"/proc/%d/mem",pid);
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);
	int i;
	for (i=0;i<ResCount;i++)
	{
		pwrite64(handle,&value,4,pTemp->addr+offset);
	    if (pTemp->next != NULL)
	        pTemp=pTemp->next;
	}
	close(handle);
	return 0;
}

void *SearchAddress(PACKAGENAME *bm,ADDRESS addr)//返回一个void指针,可以自行转换类型，addr书写格式一般应该为0x16进制，你要不想写0x   就把这个十六进制数值转成十进制然后写进去
//typedef long int ADDRESS; 
{
	pid_t pid=getPID(bm);
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	char lj[64];//存储路径
	int handle;//句柄
	void *buf=malloc(8);//分配8个字节内存
	sprintf(lj,"/proc/%d/mem",pid);//将路径写入lj
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);//还不懂的话就理解成设置文件光标在文件的开头
	pread64(handle,buf,8,addr);//从文件开头偏移addr位置处，读取一个8字节的数，将其存储到buf当中
	close(handle);
	return buf;//这里因为返回的是一个void，在赋值引用时应对其进行显式的类型转化
	//返回的是搜索addr地址所对应的值，或者说内容
}

int WriteAddress(PACKAGENAME *bm,ADDRESS addr,void *value,TYPE type)
//这里的addr书写也应该为0x16进制，非要写十进制也可以
//这个函数大体意思就是addr地址上对应的内容改写为TYPE类型的value值
{
	pid_t pid=getPID(bm);//标识包名
	if (pid == 0)
	{
		puts("can not get pid");
		return 0;
	}
	char lj[64];//存储路径
	int handle;//句柄
	sprintf(lj,"/proc/%d/mem",pid);//将路径写入数组lj
	handle=open(lj,O_RDWR);
	lseek(handle,0,SEEK_SET);//设置光标为开头
	switch (type)//用到了第三参数
	{
		case DWORD:
		    pwrite64(handle,(int*)value,4,addr);//从addr位置写入四字节的value值
		    break;
		case FLOAT:
		    pwrite64(handle,(float*)value,4,addr);//同上
		    break;
		default:
		    printf("\033[32;1mYou Select A NULL Type!\n");
		    break;
	}
	close(handle);
	return 0;
}

int isapkinstalled(PACKAGENAME *bm)//判断apk是否安装
{
	char LJ[128];
	sprintf(LJ,"/data/data/%s/",bm);//打开这个路径，需要root权限
	DIR *dir;
	dir=opendir(LJ);//见258行
	if (dir == NULL)//判断这个路径上是否有这个软件或者安装包
	{
		return 0;//没有就返回0
	}
	else
	{
		return 1;//成功打开则返回1
	}
}

int isapkrunning(PACKAGENAME *bm)//判断apk是否运行
//个人认为不是很好
{
	DIR *dir=NULL;
	struct dirent *ptr=NULL;
/*struct dirent   
{   
　　long d_ino; //inode number 索引节点号 
　　   
    off_t d_off; /* offset to this dirent 在目录文件中的偏移 
　　   
    unsigned short d_reclen; /* length of this d_name 文件名长 
　　   
    unsigned char d_type; /* the type of d_name 文件类型 
　　   
    char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 
}
*/
	FILE *fp=NULL;
	char filepath[50];			// 大小随意，能装下cmdline文件的路径即可
	char filetext[128];			// 大小随意，能装下要识别的命令行文本即可
	dir = opendir("/proc/");		// 打开路径，获得PID进程
	if (dir != NULL)//如果不是读到目录尾，或者打开失败
	{
		while ((ptr = readdir(dir)) != NULL)	// 循环读取路径下的每一个文件/文件夹
		{
			// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)//如果获取到的文件类型不是目录文件(文件夹)
		/*d_type值：
		DT_BLK：块设备文件
		DT_CHR：字符设备文件
		DT_DIR：目录文件
		DT_FIFO ：管道文件
		DT_LNK：软连接文件
		DT_REG ：普通文件
		DT_SOCK：本地套接字文件
		DT_UNKNOWN：无法识别的文件类型*/
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);	// 生成要读取的文件的路径，写入filepath数组
			/*cmdline文件:该文件包含的是该进程的命令行参数，包括进程的启动路径(argv[0])。
			
			*/
			fp = fopen(filepath, "r");	// 打开文件
			if (NULL != fp)//不到文件末尾就
			{
				fgets(filetext,sizeof(filetext),fp);	// 读取文件
				//从fp所指文件中读取sizeof(filetext)-1个字符存储到filetext数组中
				if (strcmp(filetext,bm)==0)
				///cmdline命令后面就是运行的程序，如果他与包名相同，执行下面
				{
					closedir(dir);
					return 1;//结束函数，确定apk正在运行
				}
				fclose(fp);
			}
		}//while结束
	}
	closedir(dir);	// 关闭路径
	return 0;//结束函数，确定apk未运行
}

/*卸载APK：
pm uninstall 包名。
例如：
pm uninstall com.TDiJoy.fane*/
int uninstallapk(PACKAGENAME *bm)//卸载apk
{
	char ml[128];
	sprintf(ml,"pm uninstall %s",bm);//同上文卸载APK例子
	system(ml);//调用系统函数
	return 0;
}
/* 安装APK：
pm install [-l] [-r] [-t] [-i INSTALLER_PACKAGE_NAME] [-s] [-f] PATH

PATH 指 APK文件绝对路径和文件名。

例如：
pm install /data/3dijoy_fane.apk
这几个参数很有用：
-l:安装带有FORWARD_LOCK的包。
-r:重新安装现有的应用程序，保留其数据。
-t:允许测试。要安装的apks。
-i:指定安装程序包名称。
-s:在sdcard上安装软件包。
-f:在内部闪存上安装软件包。

-k:保留数据和缓存目录。*/
int installapk(char *lj)//安装apk，填入apk绝对路径和文件名 
//举例pm install /sdcard/*.apk
{
	char ml[128];
	sprintf(ml,"pm install %s",lj);
	system(ml);
	return 0;
}

int killprocess(PACKAGENAME *bm)//杀进程，填包名
{
	int pid=getPID(bm);
	if (pid == 0)
	{
		return -1;
	}
	char ml[32];
	sprintf(ml,"kill %d",pid);
	/*kill pid
kill pid和kill -s 15 pid含义一样，表示发送一个SIGTERM的信号给对应的程序。程序收到该信号后，将会发生以下事情，
1 程序立刻停止
2 程序释放相应资源后立刻停止
3 程序可能仍然继续运行
大部分程序在接收到SIGTERM信号后，会先释放自己的资源，然后再停止。但也有一些程序在收到信号后，做一些其他事情，并且这些事情是可以配置的。也就是说，SIGTERM多半是会被阻塞，忽略的。*/
	system(ml);//杀掉进程
	return 0;
}

char GetProcessState(PACKAGENAME *bm)//获取进程状态
{
	/*
	    D. Kondisi tidur yang tidak dapat tidur (biasanya proses IO);
	    D磁盘休眠状态（Disk sleep）:有时候也叫不可中断睡眠状态（uninterruptible sleep），在这个状态的进程通常会等待IO的结束。
        R sedang berjalan, dalam antrian interruptible;
        R运行状态（running）:并不意味着进程一定在运行中，它表明进程要么是在运行中要么在运行队列
        S adalah dalam keadaan tidak aktif, keadaan statis;
        S睡眠状态（sleeping):意味着进程在等待事件完成（这里的睡眠有时候也叫做可中断睡眠（interruptible sleep））。
        T Berhenti atau dilacak, tunda eksekusi;
        T停止状态（stopped）:可以通过发送 SIGSTOP 信号给进程来停止（T）进程。这个被暂停的进程可以通过发送 SIGCONT 信号让进程继续运行。
        X Proses mati X;
        X死亡状态（dead）:这个状态只是一个返回状态，你不会在任务列表里看到这个状态。当父进程读取子进程的返回结果时，子进程立刻释放资源。死亡状态是非常短暂的，几乎不可能通过ps命令捕捉到。
        Z Proses zombie tidak ada tetapi tidak bisa dihilangkan sementara waktu;
        Z僵尸状态（Zombies）
        W: Tidak ada cukup memori untuk paging
        W memasuki swap memori (tidak valid sejak kernel 2.6);
        w在2.6内核以后就无了
        具体见网址
        https://blog.csdn.net/qq_49613557/article/details/120294908?utm_medium=distribute.wap_aggpage_search_result.none-task-blog-2~aggregatepage~first_rank_ecpm_v1~rank_v31_ecpm-1-120294908-null-null.wap_agg_so&utm_term=linux%E7%B3%BB%E7%BB%9F%E4%B8%AD%E8%BF%9B%E7%A8%8B%E7%9A%84%E5%85%AD%E7%A7%8D%E7%8A%B6%E6%80%81%E6%98%AF%E4%BB%80%E4%B9%88
	*/
	int pid=getPID(bm);//Dapatkan pid
	if (pid == 0)
	{
		return 0;//Tidak ada proses
	}
	FILE *fp;//文件句柄
	char lj[64];//存放路径
	char buff[64];
	char zt;
	char zt1[16];
	sprintf(lj,"/proc/%d/status",pid);
	fp = fopen(lj,"r");//Buka file status
	if (fp == NULL)
	{
		return 0;//Tidak ada proses
	}
	//puts("loop");
	while (!feof(fp))
	{
		fgets(buff,sizeof(buff),fp);//从fp开始不断获取buff-1容量打字符串储存到buff
		if (strstr(buff,"State"))//如果获取到的字符串当中 具有这个英文单词
		{
			sscanf(buff,"State: %c",&zt);//从这段字符串当中匹配格式，并将一个字符传递给zt，zt表示状态
			//printf("state:%c\n",zt);
			//sleep(1);
			//puts("emmmm");
			break;//Keluar dari lingkaran
		}
	}
	//putchar(zt);
	//puts(zt2);
	fclose(fp);
	//puts("loopopp");
	return zt;//返回本函数开头写的那些状态，切记为char类型，格式打印用%c
}

int rebootsystem()//重启系统/手机
{
	return system("su -c 'reboot'");//重启命令
}

int PutDate()//输出时间
{
	return system("date +%F-%T");//获取当前系统时间
	 // %F   full date; same as %Y-%m-%d
	 // %T   time; same as %H:%M:%S
}

int GetDate(char *date)//获取时间是否能成功
{
	FILE *fp;//Penunjuk file
	system("date +%F-%T > log.txt");//我去当前系统时间存入到txt文档中 
	if ((fp = fopen("log.txt", "r")) == NULL)
	{
		return 0;//打开失败就返回零 
	}
	fscanf(fp,"%s",date);//从文件当中获取时间，存储到date
	remove("log.txt");//移除txt
	return 1;
}

void BypassGameSafe()
{
	system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
	//向/proc/sys/fs/inotify/max_user_watches的这个路径上的文件max_user_watches输入文本0
	//--以下为原文件中 没写进去的
	//char ml[80];
	//sprintf(ml,"chmod 444 /data/data/%s/files",bm);
	//chmod (u g o a) (+ - =) (r w x) (文件名)，用法就是这个样子，可以去网上查具体，他没用上，我也就不多写了 
	//444是r(读取)，为ml赋予读取这个路径上的文件权利
	//system(ml);//用系统函数启动他
}

int killGG()//杀掉GG修改器，刚才去测试了一把，没杀掉，本函数看完之后请参考2503行函数注释
{
	// Dalam / data / data / [nama paket pengubah GG] / file / ada nama folder adalah GG - ****
// Jika ada folder ini, dapatkan nama paket yang disebutkan di atas dan bunuh pengubah GG
	DIR *dir=NULL;
	DIR *dirGG=NULL;
	struct dirent *ptr=NULL;
	struct dirent *ptrGG=NULL;
/*
struct dirent   
{   
　　long d_ino; //inode number 索引节点号 
    off_t d_off; /* offset to this dirent 在目录文件中的偏移 
    unsigned short d_reclen; /* length of this d_name 文件名长 
    unsigned char d_type; /* the type of d_name 文件类型 
    char d_name [NAME_MAX+1]; /* file name (null-terminated) 文件名，最长255字符 
}
*/
	char filepath[256];			//路径
	char filetext[128];			// 没用上
	dir = opendir("/data/data");		// Jalur terbuka
	//puts("Membunuh GG");
	int flag=1;
	if (dir != NULL)
	{
		while (flag && (ptr = readdir(dir)) != NULL)	//循环读取路径下每一个文件/文件夹
		{
		//如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				/*d_type值：
		DT_BLK：块设备文件
		DT_CHR：字符设备文件
		DT_DIR：目录文件
		DT_FIFO ：管道文件
		DT_LNK：软连接文件
		DT_REG ：普通文件
		DT_SOCK：本地套接字文件
		DT_UNKNOWN：无法识别的文件类型*/
				continue;
			sprintf(filepath, "/data/data/%s/files", ptr->d_name);	// 将ptr获取路径存储到filepath中
			dirGG = opendir(filepath);	//打开文件
			if (dirGG != NULL)
			{
				while ((ptrGG = readdir(dirGG)) != NULL)//循环读取目录文件
				{
					if ((strcmp(ptrGG->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				        continue;
			        if (ptrGG->d_type != DT_DIR)
			        	continue;
			        if (strstr(ptrGG->d_name,"GG"))//Tentukan nama folder
			        //找到GG在ptrGG.d_name每次出现的位置
			        //我的文件名叫GG_HO8S
			        {
			        	int pid;//pid
			        	pid = getPID(ptr->d_name);//Dapatkan nama paket GG
			        	//ptr->d_name  Simpan nama file (yaitu, nama paket)
			        	if (pid == 0)//Jika pid adalah 0, GG tidak berjalan
			        	    continue;
			        	else//Jika pid berhasil diperoleh
			        		killprocess(ptr->d_name);
			        		//详细请看2503行，并不一定能杀掉 
			        }
				}
			}
			/*else
			{
				puts(filepath);//debugging
			}*/
		}
	}
	closedir(dir);	// mematikan
	closedir(dirGG);
	return 0;
}

int killXs()//Bunuh Xs
{
	DIR *dir=NULL;
	struct dirent *ptr=NULL;
	char filepath[256];			//路径
	char filetext[128];			//没用上
	dir = opendir("/data/data");		// Jalur terbuka
	FILE *fp=NULL;
	if (NULL != dir)
	{
		while ((ptr = readdir(dir)) != NULL)	// Putar setiap file / folder di jalur
		{
			// Jika berbunyi"."atau".."Melewatkan,Lewati jika itu bukan nama folder
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
				///data/data/%s/lib/libxscript.so
			sprintf(filepath, "/data/data/%s/lib/libxscript.so", ptr->d_name);	// 生成要读取的文件的路径
			fp=fopen(filepath,"r");
			if (fp == NULL)
			    continue;
			else//Jika bacaan berhasil (xs sedang berjalan)
			{
				killprocess(ptr->d_name);
				//Bunuh prosesnya
			}
			    //killprocess(ptr->d_name);
		}
	}
	closedir(dir);	// mematikan
	return 0;
}

/*void RecBypassGameSafe(char *bm)
{
	char ml[80];
	sprintf(ml,"chmod 771 /data/data/%s/files",bm);
	//用法chmod (u g o a) (+ - =) (r w x) (文件名)
	//三个数字分别代表User、Group、及Other的权限
	//771等价于chmod ug=rwx,o=x file  
	//r=4，w=2，x=1 
//若要rwx属性则4+2+1=7； 
//若要rw-属性则4+2=6； 
//若要r-x属性则4+1=7。 这里这个函数没用上，我也就不多写了 
/*u 表示该文件的拥有者，g 表示与该文件的拥有者属于同一个群体(group)者，o 表示其他以外的人，a 表示这三者皆是。

表示增加权限、- 表示取消权限、= 表示唯一设定权限。
r 表示可读取，w 表示可写入，x 表示可执行，X 表示只有当该文件是个子目录或者该文件已经被设定过为可执行。
	system(ml);
}*/
//xdrk
void *FreezeThread(void* a)//见StartFreeze函数
{
	int handle;
	int pid;
	pid=getPID(Fbm);//详细见2976行，此可见此函数必须在StartFreeze后使用，Fbm说白了就是包名
	if (pid == 0)
	{
		puts("Error -1");
		return (void*)0;
	}
	char lj[64];//路径
	int buf_i;
	float buf_f;
	sprintf(lj,"/proc/%d/mem",pid);//路径写入lj
	handle=open(lj,O_RDWR);//读写方式打开文件
	if (handle == -1)//打开失败
	{
		puts("Error -2");
		return (void*)0;
	}
	lseek(handle,0,SEEK_SET);//文件的光标/指针设置在开头
	PFREEZE pTemp=Pfreeze;//指向Pf结构体位置
	//typedef struct FREEZE *PFREEZE;
	/*struct FREEZE
{
	long int addr;       //地址
	char *value;         //值
	int type;            //类型
	struct FREEZE *next; //链条，以方便进入下一条链表 
};*/
	while (Freeze == 1)
	{
		for (int i=0;i<FreezeCount;i++)
		{
			switch (pTemp->type)
			{
				case DWORD:
				    buf_i=atoi(pTemp->value);
				    //atoi将字符转换为整型
				    pwrite64(handle,&buf_i,4,pTemp->addr);
				    //将buf_i值写入handle中开头偏移pT.a位置
				    break;
				case FLOAT:
				    buf_f=atof(pTemp->value);
				    pwrite64(handle,&buf_f,4,pTemp->addr);
				    break;
				default:
				    break;
			}
			pTemp=pTemp->next;//下一段链表
			usleep(delay);//long int delay=30000;
			//usleep是与线程挂起一段时间，里面的单位是微秒
		}
		pTemp=Pfreeze;//Pfreeze是所有冻结值链表的头链
	}
	return NULL;
}

PMAPS GetResults()//获取当前内存区域链表
{
	if (Res == NULL)
	{
		return NULL;
	}
	else
	{
		return Res;//返回当前搜索出的内存区域的头链
	}
}

int AddFreezeItem_All(PACKAGENAME *bm,char *Value,TYPE type,OFFSET offset)//
{
	if (ResCount == 0)//当前搜索出空间内没地址
	{
		return -1;
	}
	PMAPS pTemp=Res;
	for (int i=0;i<ResCount;i++)//ResCount为空间内所有地址
	{
		switch (type)
		{
			case DWORD:
			    AddFreezeItem(bm,pTemp->addr,Value,DWORD,offset);
			    //用到AddFreezeItem_AII  1234参数
			    break;
			case FLOAT:
			    AddFreezeItem(bm,pTemp->addr,Value,FLOAT,offset);
			    break;
			default:
			    SetTextColor(COLOR_SKY_BLUE);
		        puts("You Choose a NULL type");
			    break;
		}
		pTemp=pTemp->next;
	}
	return 0;
}

int AddFreezeItem(PACKAGENAME *bm,ADDRESS addr,char *value,TYPE type,OFFSET offset)//addr是地址，offset是偏移
//typedef long int ADDRESS; 
//typedef long int OFFSET; 
//了解到书写格式应该为 0x16进制，不想写0x就直接转成十进制 
{
	switch (type)//用到第4参数
	{
		case DWORD:
		    AddFreezeItem_DWORD(bm,addr+offset,value);
		    //用到了AddFreezeItem 1235参数
		    break;
		case FLOAT:
		    AddFreezeItem_FLOAT(bm,addr+offset,value);
		    break;
		default:
		    SetTextColor(COLOR_SKY_BLUE);
		    puts("You Choose a NULL type");
		    break;
	}
	return 0;
}

int AddFreezeItem_DWORD(PACKAGENAME *bm,ADDRESS addr,char *value)//填写包名，地址，要冻结的值
{
	if (FreezeCount == 0)//这个是已经冻结数值的数量，这个判断表明的是，如果你是第一次冻结就执行这里，否则就执行else条件
	//初始化在84行
	{  //#define FRE sizeof(struct FREEZE)
		Pfreeze=pEnd=pNew=(PFREEZE)malloc(FRE);//分配内存，见81行
	//typedef struct FREEZE *PFREEZE;
	/*struct FREEZE
{
	long int addr;       //地址
	char *value;         //值
	int type;            //类型
	struct FREEZE *next; //链条，以方便进入下一条链表 
};*/
		pNew->next=NULL;
		pEnd=pNew;
		Pfreeze=pNew;//了解到Pf,pN,pE现在都在同一位置，由下面的操作可以知道 Pfreeze为头链
		pNew->addr=addr;//把第二参数赋值给pN.addr
		pNew->type=DWORD;
		pNew->value=value;//将第三参数赋给pN.value
		FreezeCount+=1;//冻结数量加+1
		//现在继续运行就直接进入 else条件了，除非对FreezeCount进行相减或直接赋值为0
	}
	else
	{
		pNew=(PFREEZE)malloc(FRE);//再为pN分配内存，知道为什么不给pE分配内存吗，如果这样做了的话，pE永远不会是尾链，这个需要自己悟 
		pNew->next=NULL;
		pEnd->next=pNew;//pE指向一个新的元素pN[现在pN为尾链]
		pEnd=pNew;//pE重新变成尾链
		pNew->addr=addr;//第二参数赋值给pN.addr
		pNew->type=DWORD;
		pNew->value=value;//第三参数赋给pN.value
		//这里要知道，pE，pN现在指向同一个地址，pN被赋予的这些地址和value，pE那里也会得到变化，因为他们在同一个地址，pN变化了地址上的内容，pE同样与pN变化
		FreezeCount+=1;//冻结值的数量+1
	}
	return 0;
}

int AddFreezeItem_FLOAT(PACKAGENAME *bm,ADDRESS addr,char *value)
{
	if (FreezeCount == 0)//Jika tidak ada data
	{
		Pfreeze=pEnd=pNew=(PFREEZE)malloc(FRE);//Alokasikan ruang baru
		pNew->next=NULL;
		pEnd=pNew;
		Pfreeze=pNew;
		pNew->addr=addr;//Alamat toko
		pNew->type=FLOAT;
		pNew->value=value;
		FreezeCount+=1;
	}
	else
	{
		pNew=(PFREEZE)malloc(FRE);//Alokasikan ruang
		pNew->next=NULL;
		pEnd->next=pNew;
		pEnd=pNew;
		pNew->addr=addr;
		pNew->type=FLOAT;
		pNew->value=value;
		FreezeCount+=1;
	}
	return 0;
}

int RemoveFreezeItem(ADDRESS addr)//移除冻结链表中的一个节点
//你只需要填
{
	PFREEZE pTemp=Pfreeze;//Pfreeze是所有冻结值链接的链表头
	PFREEZE p1=NULL;
	PFREEZE p2=NULL;
	for (int i=0;i<FreezeCount;i++)//FreezeCount是所有被冻结值的数量
	{
		p1=pTemp;//p1当前位置为pT当前位置
		p2=pTemp->next;//p2为p1下一个位置
		if (pTemp->addr == addr)//如果
		{
			p1->next=p2;//pT下个位置为p2
			free(pTemp);//释放当前链表pT占用内存
			FreezeCount-=1;//总数-1
			//printf("Jumlah pembekuan:%d\n",FreezeCount);
			//break;//Untuk mencegah pembekuan berulang dari alamat, jadi jangan menambahkan, tentu saja, Anda juga bisa menambahkan
		}
		pTemp=p2;//pT位置成为p2的位置
	}
	return 0;
}

int RemoveFreezeItem_All()//移除所有冻结值
{
	PFREEZE pHead=Pfreeze;
	PFREEZE pTemp=pHead;
	int i;
	for (i=0;i<FreezeCount;i++)
	{
		pTemp=pHead;
		pHead=pHead->next;
		free(pTemp);
		FreezeCount-=1;
	}
	free(Pfreeze);
	FreezeCount-=1;
	return 0;
}

int StartFreeze(PACKAGENAME *bm)//第一次冻结可使用此函数
{
	if (Freeze == 1)//如果不是第一次冻结，或者要不要进行冻结，
	//这里就是假如你是第一次冻结或者用了StopFreeze停止了冻结，需要重新将判断是否冻结的变量Freeze重新变为1，其他的冻结函数能运行
	{
		return -1;//结束函数
	}
	int a;
	strcpy(Fbm,bm);//将包名bm拷贝到Fbn所在地址
	Freeze=1;
	pthread_create(&pth,NULL,FreezeThread,NULL);//线程pth需执行FreezeThread指针所指向的函数
	/*原型int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine) (void *),
                   void *arg);*/
     //参数详细介绍见http://m.biancheng.net/view/8607.html
	return 0;
}

int StopFreeze()//虽然他这里说的是停止冻结，
{
	Freeze=0;//把判断是否冻结的变量改为0(为1时，调用上下的冻结函数，会进行冻结 )
	return 0;
}

int SetFreezeDelay(long int De)//这里填的是时间，以微秒做单位 
{
	delay=De;//冻结时间，以微秒做单位
	return 0;
}

int PrintFreezeItems()//打印被冻结的地址，类型，值
{
	PFREEZE pTemp=Pfreeze;//pT=要被冻结内容的头链
	for (int i=0;i<FreezeCount;i++)//遍历冻结链表
	{
		printf("FreezeAddr:%lx,type:%s,value:%s\n",pTemp->addr,pTemp->type == DWORD ? "DWORD":"FLOAT",pTemp->value);
		//表达式？a:b是一个三目表达式，我表达式是true，就执行a，如果为false就执行b
		pTemp=pTemp->next;//进入下一个节点
	}
	return 0;
}
