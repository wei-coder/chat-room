#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <pwd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <mcheck.h>

#define SERVER_PORT 3456

typedef struct loginfo
{
    char* user_name;
    char* passwd;
}LOGINFO;

typedef struct chatinfo
{
    int namelen;
    char* user_name;
    time_t  time;
    int conlen;
    char* content;
}CHATINFO;

char g_usrname[10]={0};

void packetinfo(void* package, const CHATINFO*  CI)
{
     strcat(package,  CI->user_name);
     strcat(package,"*");
     strncat(package, (char*)(&(CI->time)),sizeof(time_t));
     strcat(package,"*");
     strcat(package,  CI->content);

     return;
};

void showinfo(void* buff, CHATINFO* tmpinfo)
{
      char* tmpbuff=NULL;
      tmpbuff=buff;

       tmpinfo->user_name=strsep(&tmpbuff,"*");
	tmpinfo->namelen=strlen(tmpinfo->user_name);
	tmpinfo->time=(time_t)*(strsep(&tmpbuff,"*"));
	tmpinfo->content=strsep(&tmpbuff,"*");
	tmpinfo->conlen=strlen(tmpinfo->content);

	return;
};

static int  set_socket_non_blocking (int sfd)  
{  
  int flags, s;  
  
  //得到文件状态标志  
  flags = fcntl (sfd, F_GETFL, 0);  
  if (flags == -1)  
    {  
      perror ("fcntl");  
      return -1;  
    }  
  
  //设置文件状态标志  
  flags |= O_NONBLOCK;  
  s = fcntl (sfd, F_SETFL, flags);  
  if (s == -1)  
    {  
      perror ("fcntl");  
      return -1;  
    }  
  
  return 0;  
}  



#ifdef MY_SHELL
#define HEAD_LEN  8

struct prompt_info
{
	char shell_head[HEAD_LEN];
	char user_name[];
	char host_name[];
	char path[];
};
#endif

