/*************************************************************************
	> File Name: simulator.h
	> Author: liangTao
	> Mail: 824573233@qq.com 
	> Created Time: 2016年12月27日 星期二 15时36分17秒
 ************************************************************************/

#ifndef _SIMULATOR_H
#define _SIMULATOR_H
#include "ext2.h"
char current_user[9];//当前用户
char current_path[256];//当前路径

#define INODE_TABLE (1536+512)  //索引节点表起始地址
#define INODE_SIZE 64   //sizeof(struct inode)
#define SB_SIZE 32      //sizeof(struct super_block)
#define GD_SIZE 32      //sizeof(struct group_desc)
#define GDT_START (0+512) //磁盘gdt开始地址
#define DISK_START 0    //磁盘开始地址
#define DISK_SIZE   4612    //磁盘总大小
#define BLOCKS_PER_GROUP 4612  //每组中的块数
#define BLOCK_SIZE 512   //块大小
#define DATA_BLOCK (263680+512) //数据块起始地址
#define BLOCK_BITMAP (512+512) //块位图起始地址
#define INODE_BITMAP (1024+512) //inode位图起始地址
#define INODE_TABLE_COUNTS 4096 //inode entry 数量
#define DATA_BLOCK_COUNTS 4096 //数据块数
#define VOLUME_NAME "EXT2FS"   //卷名

struct inode /*64 bytes */
{
	unsigned short i_mode;//文件类型及访问权限
	unsigned short i_blocks;//文件的数据块个数0-7
	unsigned long i_size;//文件或目录的大小
	unsigned long i_atime;//访问时间
	unsigned long i_ctime;//创建时间
	unsigned long i_mtime;//修改时间
	unsigned long i_dtime;//删除时间
	unsigned short i_block[8];//指向数据块号
	char i_pad[24];//填充
};
struct super_block/* 32 bytes */
{
	char sb_volume_name[16];//卷名
	unsigned short sb_disk_size;//磁盘总大小
	unsigned short sb_blocks_per_group;//每组中的块数
	unsigned short sb_size_per_block;//块大小
	char sb_pad[10];
};
struct group_desc/* 32 bytes */
{
	char bg_volume_name[16];//卷名
	unsigned short bg_block_bitmap;//保存块位图的块号
	unsigned short bg_inode_bitmap;//保存索引节点位图的块号
	unsigned short bg_inode_table;//索引节点表的起始块号
	unsigned short bg_free_blocks_count;//本组空闲块的个数
	unsigned short bg_free_inodes_count;//本组空闲索引节点的个数
	unsigned short bg_use_dirs_count;//本组目录的个数
	char bg_pad[4];
};
struct dir_entry/* 16 bytes,每个block(512 bytes) 有32 个dir——entry*/
{
	//目录项结构
	unsigned short inode;//索引节点号
	unsigned short rec_len;//目录项长度
	unsigned short name_len;//文件名长度
	char file_type;//文件类型，1普通文件，2目录
	char name[9];
};

static unsigned short last_alloc_inode;//最近分配的节点号
static unsigned short last_alloc_block;//最近分配节点的数据块号
static unsigned short current_dir;     //当前目录的节点号
static unsigned short current_dirlen;  //当前路径长度

static short fopen_table[16]; //文件打开表
static struct super_block sb_block[1]; //超级块缓冲区
static struct group_desc gdt[1];  //组描述符缓冲区
static struct inode inode_area[1];  //节点缓冲区
static unsigned char bitbuf[512];  //位图缓冲区
static struct dir_entry dir[32];   //目录项缓冲区 32*16 = 512
static char Buffer[512];      //数据块缓冲区
static char tempbuf[4096];    //文件写入缓冲区
static FILE *fp;              //虚拟磁盘指针

static void update_super_block(void);
static void reload_super_block(void);
static void update_group_desc(void);
static void reload_group_desc(void);
static void update_inode_entry(unsigned short i);
static void reload_inode_entry(unsigned short i);
static void update_block_bitmap(void);
static void reload_block_bitmap(void);
static void update_inode_bitmap(void);
static void reload_inode_bitmap(void);
static void update_dir(unsigned short i);
static void reload_dir(unsigned short i);
static void update_block(unsigned short i);
static void reload_block(unsigned short i);
static int alloc_block(void);
static int get_inode(void);
static unsigned short reserch_file(char tmp[9],int file_type,unsigned short *inode_num,
											unsigned short *block_num,unsigned short *dir_num);
static void dir_prepare(unsigned short tmp,unsigned short len,int type);
static void remove_block(unsigned short del_num);
static void remove_inode(unsigned short del_num);
static unsigned short search_file(unsigned short ino);
static void sleep(int k);
static void initialize_disk(void);


#endif
