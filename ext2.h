/*************************************************************************
	> File Name: ext2.h
	> Author: liangTao
	> Mail: 824573233@qq.com 
	> Created Time: 2016年12月27日 星期二 11时34分01秒
 ************************************************************************/
#ifndef _EXT2_H
#define _EXT2_H
extern char current_path[256];//current path
extern char current_user[9];//current user
//interface
extern void chmod_file(char tmp[9],int type);
extern void check_disk(void);
extern void ls(void);
extern void write_file(char tmp[9]);
extern void read_file(char tmp[9]);
extern void close_file(char tmp[9]);
extern void open_file(char tmp[9]);
extern void del(char tmp[9]);
extern void rmdir(char tmp[9]);
extern void mkdir(char tmp[9],int type);
extern void cd(char tmp[9]);
extern void format(void);
extern void initialize_memory(void);
extern void help(void);


#endif

