/*************************************************************************
	> File Name: simulator.c
	> Author: liangTao
	> Mail: 824573233@qq.com 
	> Created Time: 2016年12月27日 星期二 15时28分16秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include"simulator.h"
/*************************************************																					Interface
 *************************************************/
void help(void)
{
    printf("   ***************************************************************************\n");
    printf("   *                   An simulation of ext2 file system                     *\n");
    printf("   *                                                                         *\n");
    printf("   * The available commands are:                                             *\n");
    printf("   * 01.change dir   : cd+dir_name       02.create dir     : mkdir+dir_name  *\n");
    printf("   * 03.create file  : mkf+file_name     04.delete dir     : rmdir+dir_name  *\n");
    printf("   * 05.delete file  : rm+file_name      06.read   file    : read+file_name  *\n");
    printf("   * 07.open   file  : open+file_name    08.write  file    : write+file_name *\n");
    printf("   * 09.close  file  : close+file_name   10.logoff         : quit            *\n");
    printf("   * 11.list   items : ls                12.this   menu    : help or h       *\n");
    printf("   * 13.format disk  : format            14.check sim disk : ckdisk          *\n");
    printf("   * 15.chmod  type  : chmod+filename+type                                    *\n");	
    printf("   ***************************************************************************\n");
}

static void reload_super_block(void)
{
	fseek(fp,DISK_START,SEEK_SET);
	fread(sb_block,SB_SIZE,1,fp);
}

static void update_super_block(void)
{
	fseek(fp,DISK_START,SEEK_SET);
	fwrite(sb_block,SB_SIZE,1,fp);
	fflush(stdin);
}

static void sleep(int k)
{
	int i,j;
	for(j = 0;j < k*0xff;j++)
	{
		for(i = 0;i<0xffff;i++)
			;
	}
}

static void update_group_desc(void)
{
	fseek(fp,GDT_START,SEEK_SET);
	fwrite(gdt,GD_SIZE,1,fp);
	fflush(stdin);
	//while((c=getchar())!='\n'&& c!=EOF);
}
/*
 * 载入组描述符
 */
static void reload_group_desc(void)
{
	fseek(fp,GDT_START,SEEK_SET);
	fread(gdt,GD_SIZE,1,fp);
}
/*
 * 更新第i个i节点
 */
static void update_inode_entry(unsigned short i)
{
	fseek(fp,INODE_TABLE+(i-1)*INODE_SIZE,SEEK_SET);
	fwrite(inode_area,INODE_SIZE,1,fp);
	fflush(stdin);
	//	while((c=getchar())!='\n' && c!=EOF);
}
/*
 * 载入第i个i节点
 */
static void reload_inode_entry(unsigned short i)
{
	fseek(fp,INODE_TABLE+(i-1)*INODE_SIZE,SEEK_SET);
	fread(inode_area,INODE_SIZE,1,fp);
}
/*
 * 更新第i个 dir_entry
 */
static void update_dir(unsigned short i)
{
	fseek(fp,DATA_BLOCK+i*BLOCK_SIZE,SEEK_SET);
	fwrite(dir,BLOCK_SIZE,1,fp);
	fflush(stdin);
	//	while((c=getchar())!='\n' && c!=EOF);
}
/*
 * 载入第i个 dir_entry
 */
static void reload_dir(unsigned short i)
{
	fseek(fp,DATA_BLOCK+i*BLOCK_SIZE,SEEK_SET);
	fread(dir,BLOCK_SIZE,1,fp);
}
/*
 * 更新block位图
 */
static void update_block_bitmap(void)
{
	fseek(fp,BLOCK_BITMAP,SEEK_SET);
	fwrite(bitbuf,BLOCK_SIZE,1,fp);
	fflush(stdin);
	//while((c=getchar())!='\n' && c!=EOF);
}
/*
 * 载入block位图
 */
static void reload_block_bitmap(void)
{
	fseek(fp,BLOCK_BITMAP,SEEK_SET);
	fread(bitbuf,BLOCK_SIZE,1,fp);
}
/*
 * 更新inode位图
 */
static void update_inode_bitmap(void)
{
	fseek(fp,INODE_BITMAP,SEEK_SET);
	fwrite(bitbuf,BLOCK_SIZE,1,fp);	
	fflush(stdin);
	//while((c=getchar())!='\n' && c!=EOF);
}
/*
 * 载入 inode 位图
 */
static void reload_inode_bitmap(void)
{
	fseek(fp,INODE_BITMAP,SEEK_SET);
	fread(bitbuf,BLOCK_SIZE,1,fp);
}
/*
 * 更新第i个数据块
 */
static void update_block(unsigned short i)
{
	fseek(fp,DATA_BLOCK+i*BLOCK_SIZE,SEEK_SET);
	fwrite(Buffer,BLOCK_SIZE,1,fp);
	fflush(stdin);
	//while((c=getchar())!='\n' && c!=EOF);
}
/*
 *载入第i个数据块
 */
static void reload_block(unsigned short i)
{
	fseek(fp,DATA_BLOCK+i*BLOCK_SIZE,SEEK_SET);
	fread(Buffer,BLOCK_SIZE,1,fp);
}
/*
 * 在打开文件表中查找是否已打开文件
 */
static unsigned short search_file(unsigned short Ino)
{
	unsigned short fopen_table_point = 0;
	while(fopen_table_point<16&&fopen_table[fopen_table_point++]!=Ino)
		;
	if(fopen_table_point == 16)
	{
		return 0;
	}
	return 1;
}
/*
 * 分配一个数据块，返回数据块号
 */
static int alloc_block(void)
{
	unsigned short cur=last_alloc_block;
	unsigned char con=128;
	int flag = 0;
	if(gdt[0].bg_free_blocks_count ==0)
	{
		printf("there is no block to be alloced!\n");
		return (0);
	}
	reload_block_bitmap();
	cur/=8;
	while(bitbuf[cur] == 255)
	{
		if(cur==511)
			cur =0;
		else
			cur++;
	}
    while(bitbuf[cur]&con)
	{
		con = con/2;
		flag++;
	}
	bitbuf[cur]=bitbuf[cur]+con;
	last_alloc_block=cur*8+flag;
	update_block_bitmap();
	gdt[0].bg_free_blocks_count--;
	update_group_desc();
	return last_alloc_block;
}
/*
 * 当前目录中查找文件并得到该文件的inode号
 * tmp的数据块号
 * bn数据块目录项号dn
 * 并加在bn到dir中
 */
static unsigned short reserch_file(char tmp[9],int file_type,unsigned short *inode_num,unsigned short *block_num,unsigned short *dir_num)
{
	unsigned short j,k;
	reload_inode_entry(current_dir);
	j=0;
	while(j<inode_area[0].i_blocks)
	{
		reload_dir(inode_area[0].i_block[j]);
		k = 0;
		while(k<32)
		{
			if(!dir[k].inode || dir[k].file_type!=file_type || strcmp(dir[k].name,tmp))
			{
				k++;
			}
			else
			{
				*inode_num=dir[k].inode;
				*block_num=j;
				*dir_num=k;
				return 1;
			}
		}
		j++;
	}
	return 0;
}
/*
 *新目录和文件初始化'.'and'..'
 */
static void dir_prepare(unsigned short tmp,unsigned short len,int type)
{
	reload_inode_entry(tmp);//得到新目录的inode
	if(type == 2)//目录
	{
		inode_area[0].i_size=32;
		inode_area[0].i_blocks=1;
		inode_area[0].i_block[0]=alloc_block();
		dir[0].inode=tmp;// '.'
		dir[1].inode=current_dir;//'..'
		dir[0].name_len=len;
		dir[1].name_len = current_dirlen;
		dir[0].file_type = dir[1].file_type = 2;//目录

		for(type=2;type<32;type++)
		{
			dir[type].inode=0;
		}
		strcpy(dir[0].name,".");
		strcpy(dir[1].name,"..");
		update_dir(inode_area[0].i_block[0]);
		inode_area[0].i_mode=01006;//  drwxrwxrwx 目录

	}
	else
	{
		inode_area[0].i_size=0;
		inode_area[0].i_blocks=0;
		inode_area[0].i_mode=0407;//drwxrwxrwx  文件
	}
	update_inode_entry(tmp);
}
/*
 * 分配一个inode,返回序号
 */
static int get_inode(void)
{
	unsigned short cur=last_alloc_inode;
	unsigned char con = 128;
	int flag = 0;
	if(gdt[0].bg_free_inodes_count == 0)
	{
		printf("there is no Inode to be alloced!\n");
		return 0;
	}
	reload_inode_bitmap();
	cur = (cur-1)/8;
	while(bitbuf[cur] == 255)
	{
		if(cur==511)
			cur = 0;
		else

			cur++;
	}
	while(bitbuf[cur]&con)
	{
		con = con /2;
		flag++;
	}
	bitbuf[cur]=bitbuf[cur]+con;
	last_alloc_inode = cur*8+flag+1;
	update_inode_bitmap();
	gdt[0].bg_free_inodes_count--;
	update_group_desc();
	return last_alloc_inode;
}
/*
 * 删除一个block
 */
static void remove_block(unsigned short del_num)
{
	unsigned short tmp;
	tmp=del_num/8;
	reload_block_bitmap();
	switch(del_num%8)//更新block位图
	{
		case 0:bitbuf[tmp]=bitbuf[tmp]&127;break;
		case 1:bitbuf[tmp]=bitbuf[tmp]&191;break;
		case 2:bitbuf[tmp]=bitbuf[tmp]&223;break;
		case 3:bitbuf[tmp]=bitbuf[tmp]&239;break;
		case 4:bitbuf[tmp]=bitbuf[tmp]&247;break;
		case 5:bitbuf[tmp]=bitbuf[tmp]&251;break;
		case 6:bitbuf[tmp]=bitbuf[tmp]&253;break;
		case 7:bitbuf[tmp]=bitbuf[tmp]&254;break;
	}
	update_block_bitmap();
	gdt[0].bg_free_blocks_count++;
	update_group_desc();
}
/*
 * 删除一个inode
 */
static void remove_inode(unsigned short del_num)
{
	unsigned short tmp;
	tmp = (del_num - 1)/8;
	reload_inode_bitmap();
	switch((del_num-1)%8)//更改block位图
	{	
		case 0:bitbuf[tmp]=bitbuf[tmp]&127;break;
		case 1:bitbuf[tmp]=bitbuf[tmp]&191;break;
		case 2:bitbuf[tmp]=bitbuf[tmp]&223;break;
		case 3:bitbuf[tmp]=bitbuf[tmp]&239;break;
		case 4:bitbuf[tmp]=bitbuf[tmp]&247;break;
		case 5:bitbuf[tmp]=bitbuf[tmp]&251;break;
		case 6:bitbuf[tmp]=bitbuf[tmp]&253;break;
		case 7:bitbuf[tmp]=bitbuf[tmp]&254;break;
	}
	update_inode_bitmap();
	gdt[0].bg_free_inodes_count++;
	update_group_desc();
}
/*初始化磁盘*/
void initialize_disk(void)
{
	int i = 0;
	printf("Creating the ext2 file system\n");
	printf("please wait ");
    while(i<1)
	{
		printf("...");
		sleep(1);
		i++;
	}
	printf("\n");
	last_alloc_inode = 1;
	last_alloc_block = 0;
	for(i = 0;i<16;i++)
	{
		fopen_table[i] = 0;//清空缓冲表
	}
	for(i = 0;i<BLOCK_SIZE;i++)
	{
		Buffer[i] = 0;//清空缓冲区，通过缓冲区清空文件，即清空磁盘
	}
	if(fp!=NULL)
	{
		fclose(fp);
	}
	fp = fopen("./sim_hd/ext2_FS","w+b");
	fseek(fp,DISK_START,SEEK_SET);
	for(i = 0;i<DISK_SIZE;i++)
	{
		fwrite(Buffer,BLOCK_SIZE,1,fp);//清空文件，即清空磁盘全部用0填充
	}
	//初始化超级块的内容
	reload_super_block();
	strcpy(sb_block[0].sb_volume_name,VOLUME_NAME);
	sb_block[0].sb_disk_size=DISK_SIZE;
	sb_block[0].sb_blocks_per_group=BLOCKS_PER_GROUP;
	sb_block[0].sb_size_per_block=BLOCK_SIZE;
	update_super_block();

	//加载根目录
	reload_inode_entry(1);
	reload_dir(0);
	//strcpy();
	//初始化组描述符
	reload_group_desc();

	gdt[0].bg_block_bitmap=BLOCK_BITMAP;
	gdt[0].bg_inode_bitmap=INODE_BITMAP;
	gdt[0].bg_inode_table=INODE_TABLE;
	gdt[0].bg_free_blocks_count=DATA_BLOCK_COUNTS;
	gdt[0].bg_free_inodes_count=INODE_TABLE_COUNTS;
	gdt[0].bg_use_dirs_count=0;//初始化组描述符内容
	update_group_desc();//更新组描述符内容

	reload_block_bitmap();
	reload_inode_bitmap();

	//初始化根目录
	inode_area[0].i_mode=6;
	inode_area[0].i_blocks=0;
	inode_area[0].i_size=32;
	inode_area[0].i_atime=0;
	inode_area[0].i_ctime=0;
	inode_area[0].i_mtime=0;
	inode_area[0].i_dtime=0;
	inode_area[0].i_block[0]=alloc_block();
	inode_area[0].i_blocks++;
	current_dir=get_inode();
	update_inode_entry(current_dir);

	//初始化根目录
	dir[0].inode=dir[1].inode=current_dir;
	dir[0].name_len=0;
	dir[1].name_len=0;
	dir[0].file_type=dir[1].file_type=2;
	strcpy(dir[0].name,".");
	strcpy(dir[1].name,"..");
	update_dir(inode_area[0].i_block[0]);
	sleep(2);
	printf("the ext2 file system has been installed!\n");
	check_disk();
	fclose(fp);
}

/*初始化内存*/
void initialize_memory(void)
{
	int i=0;
	last_alloc_inode=1;
	last_alloc_block = 0;
	for(i = 0;i<16;i++)
	{
		fopen_table[i]=0;
	}
//	strcpy(current_path,current_user);
	current_dir = 1;
	fp=fopen("./sim_hd/ext2_FS","r+b");
	if(fp==NULL)
	{
		printf("the file system does not exist!\n");
		initialize_disk();
		return ;
	}
	reload_super_block();
	if(strcmp(sb_block[0].sb_volume_name,VOLUME_NAME))
	{
		printf("the file system [%s] is not supported yet!\n ",sb_block[0].sb_volume_name);
		printf("the file system loaded error!\n");
		initialize_disk();
		return;
	}
    reload_group_desc();
}
/*格式化*/
void format(void)
{
	initialize_disk();
    initialize_memory();
}
/*
 * cd命令
 */
void cd(char tmp[9])
{
	unsigned short i,j,k,flag;
	if(!strcmp(tmp,"../"))//将../装化为..
	{
		tmp[2]='\0';
	}
	else if(!strcmp(tmp,"./"))//将./转化为.
	{
		tmp[1]='\0';
	}
	flag=reserch_file(tmp,2,&i,&j,&k);
	if(flag)
	{
		reload_inode_entry(dir[k].inode);
		if(!(inode_area[0].i_mode&4))
		{
			printf("您没有进入该目录的权限！\n");
			return;
		}

		current_dir=i;
	//	if(current_dir == 1)
	//	{
	//		return ;
	//	}
		if(!strcmp(tmp,"..")&&dir[k-1].name_len)//cd 到父目录，根目录的/的name_len是0
		{
			//printf("%s\n",current_path);
			//printf("current_dir = %d   dir[k-1].name_len %d\n",current_dir,dir[k-1].name_len);
			current_path[strlen(current_path)-dir[k-1].name_len-1]='\0';
			current_dirlen=dir[k].name_len;
		}
		else if(!strcmp(tmp,"."))
		{
			;
		}
		else if(strcmp(tmp,".."))
		{
			//printf("hello");
			current_dirlen=strlen(tmp);
			strcat(current_path,tmp);
			strcat(current_path,"/");
		}
	}
	else
	{
		printf("this directory %s not exists!\n",tmp);
	}
}
void mkdir(char tmp[9],int type)
{
	unsigned short tmpno,i,j,k,flag;
//	printf("创建文件：%s\n",tmp);
	//当前目录下新增目录或文件
	reload_inode_entry(current_dir);//加载当前目录的inode到inode_area[0]
	
	if(!(inode_area[0].i_mode&2))
	{
		printf("该文件夹下不可新建文件或目录！\n");
		return;
	}
	//查看当前目录项
	if(!reserch_file(tmp,type,&i,&j,&k))//未找到同名文件
	{
		//printf("目录项个数%d",inode_area[0].i_size);
		if(inode_area[0].i_size == 4096)//目录项已满 8(max i_block)*512 = 4096
		{
			printf("Directory has no room to be alloced!\n");
			return ;
		}
		flag = 1;
		//printf("inode_area[0].i_blocks:%d\n",inode_area[0].i_blocks);
		if(inode_area[0].i_size!=inode_area[0].i_blocks*512)//目录中有些块中32个dir_entry未满
		{
			i = 0;
			while(flag&&i<inode_area[0].i_blocks)
			{
				reload_dir(inode_area[0].i_block[i]);
				j = 0;
				while(j<32)
				{
					if(dir[j].inode == 0)
					{
						flag = 0;
						break;
					}
					j++;
				}
				i++;
			}
			tmpno=dir[j].inode=get_inode();
			dir[j].name_len=strlen(tmp);
			dir[j].file_type=type;
			strcpy(dir[j].name,tmp);
			update_dir(inode_area[0].i_block[i-1]);
		}
		else//全满，新增加块
		{
			inode_area[0].i_block[inode_area[0].i_blocks]=alloc_block();
			inode_area[0].i_blocks++;
			reload_dir(inode_area[0].i_block[inode_area[0].i_blocks-1]);
			tmpno=dir[0].inode = get_inode();
			dir[0].name_len=strlen(tmp);
			dir[0].file_type=type;
			strcpy(dir[0].name,tmp);
			//初始化新块
			for(flag = 1;flag<32;flag++)
			{
				dir[flag].inode=0;
			}
			update_dir(inode_area[0].i_block[inode_area[0].i_blocks-1]);
		}
		//printf("处理目录项\n");
		inode_area[0].i_size+=16;
		update_inode_entry(current_dir);
		//为新增目录或文件分配dir_entry
		dir_prepare(tmpno,strlen(tmp),type);
	}
	else//已经存在同名文件或目录
	{
		if(type == 1)
		{
			printf("file has already existed!\n");
		}
		else
		{
			printf("Directory has already existed!\n");
		}
	}
}
void rmdir(char tmp[9])
{
	unsigned short i,j,k,flag;
	unsigned short m,n;
	if(!strcmp(tmp,"..")||!strcmp(tmp,"."))
	{
		printf("this directory can not be deleted!\n");
		return ;
	}
	flag = reserch_file(tmp,2,&i,&j,&k);
	if(flag)
	{
		reload_inode_entry(dir[k].inode);//加载要删除的节点
		if(inode_area[0].i_size == 32)//只有.and..
		{
			inode_area[0].i_size=0;
			inode_area[0].i_blocks=0;

			remove_block(inode_area[0].i_block[0]);
			//更新tmp所在父目录
			reload_inode_entry(current_dir);//加载当前目录
			reload_dir(inode_area[0].i_block[j]);//加载将要删除的dir_entry
			remove_inode(dir[k].inode);//remove inode
			dir[k].inode=0;//删除dir_entry
			update_dir(inode_area[0].i_block[j]);
			inode_area[0].i_size-=16;
			flag=0;
			/*
			 * 删除32个dir_entry全为空的数据块
			 * 由于inode_area[0].i_block[0]中有目录 .和..
			 * 所以这个数据块的非空dir_entry不可能为0
			 */
			m = 1;
			while(flag<32 && m<inode_area[0].i_blocks)
			{
				flag = n = 0;
				reload_dir(inode_area[0].i_block[m]);
				while(n<32)
				{
					if(!dir[n].inode)
					{
						flag++;
					}
					n++;
				}
				if(flag == 32)
				{
					remove_block(inode_area[0].i_block[m]);
					inode_area[0].i_blocks--;
					while(m<inode_area[0].i_blocks)
					{
						inode_area[0].i_block[m]=inode_area[0].i_block[m+1];
						m++;
					}
				}
			}
			update_inode_entry(current_dir);
		}
		else
		{
			printf("directory is not null!\n");
		}
	}
	else
	{
		printf("directory to be deleted not exists!\n");
	}
}
void del(char tmp[9])
{
	unsigned short i,j,k,m,n,flag;
	m=0;
	flag=reserch_file(tmp,1,&i,&j,&k);
	if(flag)
	{
		flag = 0;
		//若文件tmp已经打开，则清零fopen_table项
		while(fopen_table[flag]!=dir[k].inode&&flag<16)
		{
			flag++;
		}
		if(flag<16)
		{
			fopen_table[flag]=0;
		}
		reload_inode_entry(i);//加载删除文件
		while(m<inode_area[0].i_blocks)
		{
			remove_block(inode_area[0].i_block[m++]);
		}
		inode_area[0].i_blocks=0;
		inode_area[0].i_size=0;
		remove_inode(i);
		//更新父目录
		reload_inode_entry(current_dir);//加载当前目录节点
		reload_dir(inode_area[0].i_block[j]);//加载要删除的节点
		dir[k].inode=0;

		update_dir(inode_area[0].i_block[j]);
		inode_area[0].i_size-=16;
		m=1;

		while(m<inode_area[i].i_blocks)
		{
			flag=n=0;
			reload_dir(inode_area[0].i_block[m]);
			while(n<32)
			{
				if(!dir[n].inode)
				{
					flag++;
				}
				n++;
			}
			if(flag ==32)
			{
				remove_block(inode_area[i].i_block[m]);
				inode_area[i].i_blocks--;
				while(m<inode_area[i].i_blocks)
				{
					inode_area[i].i_block[m]=inode_area[i].i_block[m+1];
					m++;
				}
			}
		}
		update_inode_entry(current_dir);
	}
	else
	{
		printf("the file %s not exists!\n",tmp);
	}
}
void open_file(char tmp[9])
{
	unsigned short int flag,i,j,k;
	flag=reserch_file(tmp,1,&i,&j,&k);
	if(flag)
	{
		if(search_file(dir[k].inode))
		{
			printf("the file %s has opend!\n",tmp);
		}
		else
		{
			flag = 0;
			while(fopen_table[flag])
			{
				flag++;
			}
			fopen_table[flag]=dir[k].inode;
			printf("file %s! opened\n",tmp);
		}
	}
	else
	{
		printf("the file %s does not exist!\n",tmp);
	}
}
void close_file(char tmp[9])
{
	unsigned short flag,i,j,k;
	flag=reserch_file(tmp,1,&i,&j,&k);
	if(flag)
	{
		if(search_file(dir[k].inode))
		{
			flag = 0;
			while(fopen_table[flag]!=dir[k].inode)
			{
				flag++;
			}
			fopen_table[flag]=0;
			printf("file %s !close\n",tmp);
		}
		else
		{
			printf("the file %s has not been opend!\n",tmp);
		}
	}
	else
	{
		printf("the file %s does not exist!\n",tmp);
	}
}
void read_file(char tmp[9])
{
	unsigned short flag,i,j,k,t;
	flag=reserch_file(tmp,1,&i,&j,&k);
	if(flag)
	{
		if(search_file(dir[k].inode))
		{
			reload_inode_entry(dir[k].inode);
			if(!(inode_area[0].i_mode&4))
			{
				printf("the file %s can not be read!\n",tmp);
				return ;
			}
			for(flag=0;flag<inode_area[0].i_blocks;flag++)
			{
				reload_block(inode_area[0].i_block[flag]);
				for(t = 0;t<inode_area[0].i_size-flag*512;t++)
				{
					printf("%c",Buffer[t]);
				}
			}
			if(flag == 0)
			{
				printf("the file %s is empty!\n",tmp);
			}
			else
			{
				printf("\n");
			}
		}
		else
		{
			printf("the file %s has not been opened!\n",tmp);
		}
	}	
	else
	{	
    	printf("The file %s does not exist!\n",tmp);
	}
}
/*
 * 文件以覆盖方式写入
 */
void write_file(char tmp[9]) /* 写文件 */
{
    unsigned short flag,i,j,k,size=0,need_blocks,length;
    flag=reserch_file(tmp,1,&i,&j,&k);
    if(flag)
    {
        if(search_file(dir[k].inode))
        {
            reload_inode_entry(dir[k].inode);
            if(!(inode_area[0].i_mode&2)) /* i_mode:111b:读,写,执行 */
            {
                printf("The file %s can not be writed!\n",tmp);
                return;
            }
            fflush(stdin);
            while(1)
            {
                tempbuf[size]=getchar();
                if(tempbuf[size]=='#')
                {
                    tempbuf[size]='\0';
                    break;
                }
                if(size>=4095)
                {
                    printf("Sorry,the max size of a file is 4KB!\n");
                    break;
                }
                size++;
            }
            if(size>=4095)
            {
            	length=4096;
            }
            else
            {
            	length=strlen(tempbuf);
            }
            need_blocks=length/512;
            if(length%512)
            {
            	need_blocks++;
            }
            if(need_blocks<9) /* 文件最大 9 个 blocks(512 bytes) */
            {
            	  /* 分配文件所需块数目 */
            	  if(inode_area[0].i_blocks<=need_blocks)
            	  {
                	while(inode_area[0].i_blocks<need_blocks)
                	{
                    inode_area[0].i_block[inode_area[0].i_blocks]=alloc_block();
                    inode_area[0].i_blocks++;
                	}
                }
                else
                {
                	while(inode_area[0].i_blocks>need_blocks)
                	{
                		remove_block(inode_area[0].i_block[inode_area[0].i_blocks - 1]);
                		inode_area[0].i_blocks--;
                	}
                }
                j=0;
                while(j<need_blocks)
                {
                    if(j!=need_blocks-1)
                    {
                        reload_block(inode_area[0].i_block[j]);
                        memcpy(Buffer,tempbuf+j*BLOCK_SIZE,BLOCK_SIZE);
                        update_block(inode_area[0].i_block[j]);
                    }
                    else
                    {
                        reload_block(inode_area[0].i_block[j]);
                        memcpy(Buffer,tempbuf+j*BLOCK_SIZE,length-j*BLOCK_SIZE);
                        inode_area[0].i_size=length;
                        update_block(inode_area[0].i_block[j]);
                    }
                    j++;
                }
                update_inode_entry(dir[k].inode);
            }
            else
            {
            	printf("Sorry,the max size of a file is 4KB!\n");
            }
        }
        else
        {
        	printf("The file %s has not opened!\n",tmp);
        }
    }
    else
    {
    	printf("The file %s does not exist!\n",tmp);
    }
}
/*
 *改变权限
 */
void chmod_file(char tmp[9],int type )
{
	//printf("%s------------%d",tmp,type);
	unsigned short int flag,i,j,k,op;
	op = 0;
	flag=reserch_file(tmp,1,&i,&j,&k);
	if(flag)
	{	
		   op = 1;
	 	   reload_inode_entry(dir[k].inode);
		   //printf("name = %s\n",dir[k].name);
		   if(dir[k].file_type == 1)
		   {
			//   printf("i_mode = %d\n",inode_area[0].i_mode&7);
			   inode_area[0].i_mode=type; /* i_mode:111b:读,写,执行 */

			//   printf("i_mode = %d\n",inode_area[0].i_mode&7);
			   update_inode_entry(dir[k].inode);
		   }
		   else
		   {
			   printf("the %s is directory and you can not change its permission\n",tmp);
		   }
	}	

	flag=reserch_file(tmp,2,&i,&j,&k);
	if(flag)
	{
		op = 1;
		reload_inode_entry(dir[k].inode);
		if(dir[k].file_type == 2)
		{
			if(type&1)
			{
				printf("您不能为文件夹赋予执行权限!\n");
			}
			else
			{
				inode_area[0].i_mode=type;
				update_inode_entry(dir[k].inode);
			}
		}
		else
		{
			   printf("the %s is directory and you can not change its permission\n",tmp);
		}
	}
	if(op == 0)
	{
		printf("您要修改的目录或文件不存在！\n");
	}
}
void ls(void)
{
	//printf("curret_dir = %d\n",current_dir);
	printf("items		type		mode		size\n");
	unsigned short i,j,k,flag;
	i = 0;
	reload_inode_entry(current_dir);
	while(i<inode_area[0].i_blocks)
	{
		k = 0;
		reload_dir(inode_area[0].i_block[i]);
		while(k<32)
		{
			if(dir[k].inode)
			{
				printf("%s",dir[k].name);
				if(dir[k].file_type == 2)
				{
					j = 0;
					reload_inode_entry(dir[k].inode);
					if(!strcmp(dir[k].name,".."))
					{
						while(j++<13)
						{
							printf(" ");
						}
						flag = 1;
					}
					else if(!strcmp(dir[k].name,"."))
					{
						while(j++<14)
						{
							printf(" ");
						}
						flag = 0;
					}
					else //其它目录
					{
						while(j++<15-dir[k].name_len)
						{
							printf(" ");
						}
						flag = 2;
					}
					printf(" <DIR>		");
					switch(inode_area[0].i_mode&7)
					{
						case 1:printf("__x");break;
						case 2:printf("_w_");break;
						case 3:printf("_wx");break;
						case 4:printf("r__");break;
						case 5:printf("r_x");break;
						case 6:printf("rw_");break;
						case 7:printf("rwx");break;
					}
					if(flag!=2)
					{
						printf("		----");
					}
					else
					{
						printf("	      ");
						printf("%4ld bytes",inode_area[0].i_size);
					}
				}
				else if(dir[k].file_type == 1)
				{
					j = 0;
					reload_inode_entry(dir[k].inode);
					while(j++<15-dir[k].name_len)
						printf(" ");
					printf("<FILE>		");
					switch(inode_area[0].i_mode&7)
					{
						case 1:printf("__x");break;
						case 2:printf("_w_");break;
						case 3:printf("_wx");break;
						case 4:printf("r__");break;
						case 5:printf("r_x");break;
						case 6:printf("rw_");break;
						case 7:printf("rwx");break;
					}
					printf("	      ");
					printf("%4ld bytes",inode_area[0].i_size);
				}
				printf("\n");
			}
			k++;
			reload_inode_entry(current_dir);
		}
		i++;
	}
}
/*
 * 超级块信息
 */
void check_disk(void)
{
	reload_super_block();
	printf("volume name		 :%s\n",sb_block[0].sb_volume_name);
	printf("disk size		 :%d(blocks)\n",sb_block[0].sb_disk_size);
	printf("blocks per group	 :%d(blocks)\n",sb_block[0].sb_blocks_per_group);
	printf("sim file size		 :%d(kb)\n",sb_block[0].sb_disk_size*sb_block[0].sb_size_per_block/1024);
	printf("block size		 :%d(kb)\n",sb_block[0].sb_size_per_block);
}

