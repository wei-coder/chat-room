
 #include "chat.h"
 #include "my_list.h"
 #define CHAT_SERVER
 //#define MY_SHELL

#ifdef MY_SHELL
void main()
{
       char* command, parameters;
	while(1)
	{
		type_prompt();
		read_command(command,parameters);
		if(fork() != 0)
		{
			waitpid(-1, &status, 0);
		}
		else
		{
			execve(command,  parameters, 0);
		};
	};
};

void type_prompt()
{
       struct passwd* pmyinfo;
       char* prompt_info;
	getpwuid(getuid());
	gethostname();
	getcwd();
};

void read_command(char* command, char* parameters)
{

};
#endif

#ifdef CHAT_SERVER

#define EPOLL_MAX 100
#define OFFSET_ENTRY (sizeof(struct usr_node)-sizeof(struct list_head))
struct usr_node
{
//	char* usr_name;
	int client_fd;
	bool islogin;
//	struct list_head list;
};
/*
struct usr_node * addusr(const struct usr_node * usr_head)
{
	struct usr_node * new_node = NULL;
	struct list_head* new_head;
	struct list_head* tmp_head;
	new_node = malloc(sizeof(struct usr_node));
	if(NULL == new_node)
	{
		return NULL;
	};
	new_head = &(new_node->list);
	tmp_head = &(usr_head->list);
	list_add_tail(new_head,tmp_head);
	return new_node;
};
*/
int search_null(const struct usr_node* ptr)
{
	for(int i=0; i<100; i++)
	{
		if(ptr[i].islogin == false)
		{
//			printf("i=%d\n",i);
			return i;
		};
	};
};
/*
void release_list(struct usr_node * usr_head)
{
	struct usr_node* tmp_node = NULL;
	while(usr_head->list.prev != usr_head->list.next)
	{
		tmp_node = (struct usr_node *)(usr_head->list.next - OFFSET_ENTRY);
		list_del(&(usr_head->list));
		if(usr_head->islogin)
		{
//			free(usr_head->usr_name);
		};
		free(usr_head);
		usr_head = tmp_node;
	};
	return;
};
*/

void release_pip(struct usr_node *usr_pip)
{
	return;
};

int handle_message(struct epoll_event ev, struct usr_node* ptrhead)
{
       char  info_buff[100] = {0};
	int recv_len = 0;
	int send_len = 0;
	CHATINFO tmpCI;
	struct tm * tm=NULL;
	int fd = ev.data.fd;
	int over_fd = 0;
	time_t tmptime = 0;

	memset(info_buff, 0, 100);
	if((ev.events&EPOLLHUP)||(ev.events&EPOLLERR) ||(ev.events&EPOLLRDHUP))
	{
		over_fd = ev.data.fd;
		tmptime = time(NULL);
		sprintf(info_buff,"system*%s*user %d is down line\n", (char*)(&tmptime), over_fd);
		recv_len = strlen(info_buff);
	}
	else if(ev.events == EPOLLOUT)
	{
		tmptime = time(NULL);
		sprintf(info_buff,"system*%s*user %d is online\n", (char*)(&tmptime), fd);
		recv_len = strlen(info_buff);
	}
	else
	{
		recv_len = recv(fd, info_buff, 100, 0);
		if( -1 == recv_len)
		{
			perror("recv");
			return -1;
		}
		else if(0 == recv_len)
		{
			return 0;
		};
	};
	
	for(int i = 0; i < 100; i++)
	{
		if(ptrhead[i].islogin == false)
		{
			continue;
		};

		if(over_fd == ptrhead[i].client_fd)
		{
			close(over_fd);
			ptrhead[i].islogin=false;
		};
	
		if((i != 0) && (ptrhead[i].client_fd != fd) && (ptrhead[i].client_fd != 0))
		{
			send_len = send(ptrhead[i].client_fd, info_buff, recv_len,0);
			if(-1 == send_len)
			{
				perror("send");
				printf("send to %d failed!\n", ptrhead[i].client_fd);
			};
		};
	};
	showinfo(info_buff, &tmpCI);
	printf("%s ", tmpCI.user_name);
	tm=localtime(&(tmpCI.time));
	printf("%4d-%02d-%02d %02d:%02d:%02d\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	printf("$$$$:%s \n", tmpCI.content);
	return 1;
};

void main()
{
	CHATINFO tmpCI;
	struct sockaddr_in my_addr,client_addr;
	int listen_fd=0;
	int ret = 0;
	pthread_t tid = 0;
	struct usr_node usr_pip[100]={0};
	int epfd = 0;
	int epoll_count = 0;
	char server[]="system";
	int addrlen = 0;
	bool isquit =true;
/*
	setenv("MALLOC_TRACE", "output", 1);  
	mtrace();  
*/
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_fd < 0)
	{
		perror("socket");
		return;
	};
	
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVER_PORT);

       int on = 1;
	if((setsockopt(listen_fd, SOL_SOCKET,SO_REUSEADDR,&on, sizeof(on)))<0)  
	{  
		perror("setsockopt failed");  
		return;
	};

	ret = set_socket_non_blocking(listen_fd);
	if(-1 == ret)
	{
		printf("set socket non_blocking failed!\n");
	};
		
       ret = bind(listen_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr));
	if(-1 == ret)
	{
	    perror("bind");
	    return;
	};
	ret = listen(listen_fd, 10);
	if(-1 == ret)
	{
		perror("listen");
		return;
	};

	epfd = epoll_create(EPOLL_MAX);
	if(0 > epfd)
	{
		printf("create epoll failed! \n");
		return;
	};

	static struct epoll_event event[EPOLL_MAX];
	struct epoll_event ev;
	ev.data.fd = listen_fd;
	ev.events = EPOLLIN|EPOLLOUT |EPOLLPRI |EPOLLHUP |EPOLLERR  |EPOLLRDHUP | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

	ev.data.fd = 0;
	ev.events = EPOLLIN |EPOLLPRI | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &ev);

	usr_pip[0].client_fd = listen_fd;
	usr_pip[0].islogin = true;
	printf("chatroom server is starting!\n");

	while(isquit)
	{
		epoll_count = epoll_wait(epfd, event, EPOLL_MAX, -1);
		for(int i = 0; i<epoll_count; i++)
		{
			if(event[i].data.fd == listen_fd)
			{
				int token = search_null(usr_pip);
				addrlen = sizeof(struct sockaddr_in);
				usr_pip[token].client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
				if(-1 == usr_pip[token].client_fd)
				{
					perror("accept");
					continue;
				};
				ret = set_socket_non_blocking(usr_pip[token].client_fd);
				if(-1 == ret)
				{
					printf("set socket non_blocking failed!\n");
				};
				usr_pip[token].islogin = true;
				ev.data.fd = usr_pip[token].client_fd;
				ev.events = EPOLLIN|EPOLLOUT |EPOLLPRI  |EPOLLERR  |EPOLLRDHUP  |EPOLLHUP |EPOLLET;
				epoll_ctl(epfd,EPOLL_CTL_ADD,usr_pip[token].client_fd,&ev);
			}
			else if(event[i].data.fd == 0)
			{
				char order;
				scanf("%c",&order);
				setbuf(stdin,NULL);
				if('q' == order)
				{
					isquit = false;
					return;
				};
			}
			else
			{
				ret = handle_message(event[i], usr_pip);
				if(-1 == ret)
				{
					printf("handle messge faile!\n");
					continue;
				};
			};
		};
	}
	close(listen_fd);
	close(epfd);
	release_pip(usr_pip);
	return;
};


#endif



