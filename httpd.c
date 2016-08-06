#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>
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
void clear_head(int sock)
{
	int ret = 0;
	char buf[1024];
	do{
		ret = get_line(sock,buf,_SIZE_);
	}while(ret > 0 && strcmp(buf,"\n"));
}
static int execut_cgi(int sock,const char *path,const char *method,const char *query_string)
{
	int content_len = -1;
	char buf[1024];
	memset(buf,'\0',1024);

	char method_env[1024];
	memset(method,'\0',1024);
	char content_length_env[100];
	memset(content_length_env,'\0',100);
	char query_string_env[1024];
	memset(query_string_env,'\0',1024);

	if(strcasecmp(method,"GET") == 0){
		clear_head(sock);
	}else{
		int ret = 0;
		do{
			ret = get_line(sock,buf,sizeof(buf));
            if(ret > 0 && strncasecmp(buf,"Content-Length:",16) == 0)
				content_len = atoi(&buf[16]);
		} while(ret > 0 && strcmp(buf,'\n'));
		if(content_len = -1){
			echo_errno("content");
			return -1;
		}
	}
	sprintf(buf,"HTTP1.0 200 OK\r\n\r\n");
	send(sock,buf,strlen(buf),0);

	int cgi_input[2];
	int cgi_output[2];

	if(pipe(cgi_input) < 0){
		echo_errno("pipe");
		return -2;
	}
	if(pipe(cgi_output) < 0){
		echo_errno("pipe");
		return -3;
	}

	pid_t id = fork();
	if(id == 0){     //child
		close(cgi_input[1]);
		close(cgi_output[0]);
		dup2(cgi_input[0],0);
		dup2(cgi_output[1],1);
		
		sprintf(method_env,"REQUEST_METHOD=%s",method);
		putenv(method_env);
       
        if(strcasecmp(method,"GET") == 0){
			sprintf(query_string_env,"QUERY_STRING=%s",query_string);
			putenv(query_string_env);
		}else if(strcasecmp(method,"POST") == 0){
			sprintf(content_length_env,"CONTENT_LENGTH=%d",content_len);
			putenv(content_length_env);
		}

		execl(path,path,NULL);
		exit(1);
	}else{        //father
		int i = 0;
		char c = '\0';

		if(strcasecmp(method,"POST") == 0){
			for(;i < content_len;++i){
				recv(sock,&c,1,0);
				write(cgi_input[1],&c,1);
			}
		}

		while(read(cgi_output[0],&c,1) > 0)
			send(sock,&c,1,0);

		waitpid(id,NULL,0);

		close(cgi_input[1]);
		close(cgi_output[0]);
	}
	return 0;
}

static int echo_www(int sock,const char *path,ssize_t size)
{
	int fd = open(path,O_RDONLY);
	if(fd < 0){
		echo_errno("open");
		return  -4;
	}

	char buf[_SIZE_];
	sprintf(buf,"HTTP1.0 200 OK\r\n\r\n");
	send(sock,buf,strlen(buf),0);
	
	if(sendfile(sock,fd,NULL,size) < 0){
		echo_errno("sendfile");
		return -5;
	}
}
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

	if(strcasecmp(method,"POST") == 0)
		cgi = 1;
	if(strcasecmp(method,"GET") == 0){
		query_string = url;
		while(*query_string != '\0' && *query_string != '?')
			query_string++;
		if(*query_string == '?'){
			cgi = 1;
			*query_string = '\0';
			query_string++;
		}
	}

	sprintf(path,"htdoc%s",url);
 
	if(path[strlen(path)-1] == '/')
		strcat(path,"index.html");
	printf("path:%s\n",path);
	
	struct stat st;
	if(stat(path,&st) < 0){
		echo_errno("stat");
		return  (void *)-3;
	}else{
		if(S_ISDIR(st.st_mode)){
			strcpy(path,"htdoc/index.html");
		}else if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP ) || (st.st_mode & S_IXOTH))
			cgi = 1;
	}

	if(cgi){
		execut_cgi(sock,path,method,query_string);
	}else{
		echo_www(sock,path,st.st_size);
	}

	close(sock);

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
