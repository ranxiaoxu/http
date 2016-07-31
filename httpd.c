#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>

#define _SIZE_ 1024

static void usage(const char *proc)
{
	printf("usage:%s [ip] [port]\n",proc);
}

int startup(char *ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		perror("socket\n");
		return 2;
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

    if(bind(sock,(struct sockaddr *)&local,sizeof(local)) < 0){
   		perror("bind\n");
   		return 3;
   	}
    
   	if(listen(sock,5) < 0){
		perror("listen");
		return 4;
	}

	return sock;
}
static int get_line(int sock,char buf[],int len)
{
	if(buf == NULL || len < 0)
		return -1;
	
	int i = 0;
	char ch = 'a';
	
	while(i < len && ch != '\n'){
		recv(sock,&ch,1,0);
		if(ch == '\r'){
			recv(sock,&ch,1,MSG_PEEK);
			if(ch == '\n')  // \r\n
				recv(sock,&ch,1,0);
			else             //\r
				ch = '\n';
		}
		buf[i++] = ch;
	}
	buf[i] = '\0';
	return i;
}
static echo_errno(char *str)
{}

static void *accept_request(void *arg)
{
	int cgi = 0;
	char *query_string = 0;

	int sock = (int)arg;
	int ret;
#ifdef _DEBUG_
	char buf[_SIZE_];
	do{
		ret = get_line(sock,buf,_SIZE_);
		printf("%s",buf);
	}while(ret > 0 && strcmp(buf,"\n"));

#endif

	char method[_SIZE_/10];
	char buf[_SIZE_];
	char url[_SIZE_];
	char path[_SIZE_];
	memset(method,'\0',sizeof(method));
	memset(buf,'\0',sizeof(buf));
	memset(url,'\0',sizeof(url));
	memset(path,'\0',sizeof(path));

	ret = get_line(sock,buf,sizeof(buf));
	if(ret < 0){
		echo_errno("sock");
		return (void *)-1;
	}

	int i = 0,j = 0;
	while((i<sizeof(method)-1) && (j<sizeof(method)) && !isspace(buf[j])){
		method[i] = buf[j];
		++i;++j;
	}
	method[i] = '\0';

	//check method
	if(strcasecmp(method,"GET") != 0 && strcasecmp(method,"POST") != 0){
		echo_errno("method");
		return (void *) -2;
	}

	//get url
	while(isspace(buf[j]))
		++j;

	i = 0;
	while((i<sizeof(url)-1) && (j<sizeof(buf)) && !isspace(buf[j])){
		url[i] = buf[j];
		++i;++j;
	}

	printf("method:%s,url_path:%s\n",method,url);

	char *start = url;
	while(*start != '\0'){
		if(*start == '?'){
			cgi = 1;
			*start = '\0';
			query_string = start+1;
			break;
		}
		start++;
	}
	sprintf(path,"htdoc%s",url);
 
	if(path[strlen(path)-1] == '/')
		strcat(path,"index.html");
	printf("path:%s\n",path);
	
	return (void *) 0;
}
int main(int argc,char *argv[])
{
	if(argc != 3){
		usage(argv[0]);
		exit(1);
	}
	
	int sock = startup(argv[1],atoi(argv[2]));

	struct sockaddr_in peer;
	socklen_t len = sizeof(peer);

	int done = 0;
	while(!done){
		int new_sock = accept(sock,(struct sockaddr *)&peer,&len);
		if(new_sock < 0){
			perror("accept");
			continue;
		}

		pthread_t id;
		pthread_create(&id,NULL,accept_request,(void *)new_sock);
		pthread_detach(id);
	}

	return 0;
}
