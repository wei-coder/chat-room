#include "chat.h"


bool getinfo(CHATINFO* ptrCI)
{
	char chcont[100]={0};
	scanf("%99[^\n]",chcont);
	setbuf(stdin,NULL);
	ptrCI->content = malloc(strlen(chcont)+1);
	if(ptrCI->content == NULL)
	{
		return false;
	};
	strcpy(ptrCI->content, chcont);
	ptrCI->conlen=strlen(chcont);
	ptrCI->namelen = strlen(g_usrname);
	ptrCI->user_name = g_usrname;
	ptrCI->time = time(NULL);
	return true;
};

#define CHAT_CLIENT

#ifdef CHAT_CLIENT



void* showinfo_routine(void* ptr_fd)
{
       char  info_buff[100] = {0};
	int ret = 0;
	CHATINFO tmpCI;
	char strtime[9] = {0};
	struct tm * tm=NULL;
	int socket_fd = *(int*)ptr_fd;
	   
	while(true)
    	{
    	       memset(info_buff, 0, 100);
    		ret = recv(socket_fd, info_buff, 100, 0);
		if( 0 < ret)
		{
			showinfo(info_buff, &tmpCI);
			printf("%s ", tmpCI.user_name);
			tm=localtime(&(tmpCI.time));
			printf("%4d-%02d-%02d %02d:%02d:%02d\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
			printf("$$$$:%s \n", tmpCI.content);
		}
		else
		{
			perror("recv");
			printf("socket closed!\n");
			break;
		};
    	};
	pthread_detach(pthread_self());
	return NULL;
};

void main()
{
	CHATINFO tmpCI;
	pthread_t  thread;
	struct sockaddr_in dest_addr;
	int socket_fd=0;
	char* ptrpack;
	char order;
	int ret = 0;
	pthread_t tid = 0;
/*
	setenv("MALLOC_TRACE", "output", 1);  
	mtrace();  
*/
	printf("please input your nick-name to login: \n");
	scanf("%10[A-Za-z0-9]",g_usrname);
	setbuf(stdin,NULL);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(0>socket_fd)
	{
		perror("socket");
		return;
	};

	dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(SERVER_PORT);
       ret = connect(socket_fd, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in));
	if(-1 == ret)
	{
	    perror("connect");
	    return;
	};
	
    if((tid = pthread_create(&thread,NULL, showinfo_routine, &socket_fd) != 0))
    {
       printf("create thread failed!%s\n",strerror(errno));
	return;
    };

	printf("start chat,please input your message:\n");
    while (true)
    {
       memset((void*)(&tmpCI),0,sizeof(CHATINFO));
       if( true == getinfo(&tmpCI))
       {
	       if(0 == strcmp(tmpCI.content, "quit"))
	       {
	           printf("you want to quit? y/n \n");
		    scanf("%c",&order);
		    setbuf(stdin,NULL);
		    if(('y' == order) ||('Y' == order))
		    {
			printf("welcom to chatroom next time\n");
			pthread_cancel(thread);
			int result = close(socket_fd);
			if(-1==result)
			{
				printf("close socket failed! %s\n",strerror(errno));
			};			
			free(tmpCI.content);
			pthread_join(thread,NULL);
			return;
		    };
	       };
	       ptrpack = malloc(tmpCI.conlen+tmpCI.namelen+sizeof(time_t)+2);
		memset(ptrpack,0,(tmpCI.conlen+tmpCI.namelen+sizeof(time_t)+2));
	       packetinfo(ptrpack, &tmpCI);
		ret = send(socket_fd, ptrpack, tmpCI.conlen+tmpCI.namelen+sizeof(time_t)+2,0);
		if(-1 == ret)
		{
		    printf("message send failed!\n");
		};
		free(ptrpack);
		free(tmpCI.content);
	};
    };

	return;
    
};


#endif
