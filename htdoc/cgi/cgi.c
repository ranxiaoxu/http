#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	int content_len = -1;
	char content_length[1024];
	char method[1024];
	char arg[1024];
	int i = 0;

	if(getenv("REQUEST_METHOD")){
		strcpy(method,getenv("REQUEST_METHOD"));
	}
	if(strcasecmp(method,"GET") == 0){
		if(getenv("QUERY_STRING")){
			strcpy(arg,getenv("QUERY_STRING"));
		}
	}else if(strcasecmp(method,"POST") == 0){
		if(getenv("CONTENT_LENGTH")){
			strcpy(content_length,getenv("CONTENT_LENGTH"));
			content_len = atoi(content_length);
			int i = 0;
			for(;i < content_len;++i)
				read(0,&arg[i],1);
			arg[i] = '\0';
		}
	}
	
	printf("hehe%s",arg);
	return 0;
}
