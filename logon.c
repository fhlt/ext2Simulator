/*************************************************************************
	> File Name: logon.c
	> Author: liangTao
	> Mail: 824573233@qq.com 
	> Created Time: 2016年12月27日 星期二 11时30分03秒
 ************************************************************************/

#include<stdio.h>

int main1()
{
	char name[20] = {0};
	char password[20] = {0};
	printf("please input logon name:");
	scanf("%s",name);
	printf("password->");
	scanf("%s",password);
	printf("welcome %s\n",name);
	return 0;
}
