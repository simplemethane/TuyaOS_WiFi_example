#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "8080"  //the port client will be connecting to
#define MAXDATASIZE 100  //max number of bytes we can get at once

//get sockaddr, IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	/*
	typedef struct addrinfo { 
    		int ai_flags;        //AI_PASSIVE,AI_CANONNAME,AI_NUMERICHOST 
    		int ai_family;        //AF_INET,AF_INET6 
    		int ai_socktype;    //SOCK_STREAM,SOCK_DGRAM 
    		int ai_protocol;    //IPPROTO_IP, IPPROTO_IPV4, IPPROTO_IPV6 etc. 
    		size_t ai_addrlen;            //must be zero or a null pointer 
    		char* ai_canonname;            //must be zero or a null pointer 
    		struct sockaddr* ai_addr;    //must be zero or a null pointer 
    		struct addrinfo* ai_next;    //must be zero or a null pointer 
	}
	其中ai_flags、ai_family、ai_socktype说明如下： 
	参数		取值			值	说明 
	ai_family	AF_INET			2	IPv4 
				AF_INET6		23	IPv6 
				AF_UNSPEC		0	协议无关 
	ai_protocol	IPPROTO_IP		0	IP协议 
				IPPROTO_IPV4    4   IPv4 
		        IPPROTO_IPV6    41  IPv6 
		        IPPROTO_UDP     17  UDP 
		        IPPROTO_TCP     6   TCP 
	ai_socktype SOCK_STREAM     1   流 
                SOCK_DGRAM      2   数据报 
	ai_flags    AI_PASSIVE      1   被动的，用于bind，通常用于server socket 
                AI_CANONNAME    2 
                AI_NUMERICHOST  4   地址为数字串 
	*/
	int rv;
	char s[INET6_ADDRSTRLEN];
	//如果命令行参数不等于 2 ，则执行下面的语句
	if(argc != 2)
	{
		fprintf(stderr, "usage:client hostname\n");  //打印错误消息
		exit(1);  //退出
	}
	//将hints内存的内容置 0
	memset(&hints, 0, sizeof hints);
	//设置协议无关
	hints.ai_family = AF_UNSPEC;
	//设置套接为流
	hints.ai_socktype = SOCK_STREAM;
	/*
	int getaddrinfo( const char *hostname, const char *service, 
		const struct addrinfo *hints, struct addrinfo **result );
	参数说明
	hostname:一个主机名或者地址串(IPv4的点分十进制串或者IPv6的16进制串)
	service：服务名可以是十进制的端口号，也可以是已定义的服务名称，如ftp、http等
	hints：可以是一个空指针，也可以是一个指向某个addrinfo结构体的指针，
		调用者在这个结构中填入关于期望返回的信息类型的暗示。
		举例来说：指定的服务既可支持TCP也可支持UDP，
		所以调用者可以把hints结构中的ai_socktype成员设置成SOCK_DGRAM，
		使得返回的仅仅是适用于数据报套接口的信息。
	result：本函数通过result指针参数返回一个指向addrinfo结构体链表的指针。
	返回值：0——成功，非0——出错

	getaddrinfo 函数能够处理名字到地址以及服务到端口这两种转换，
		返回的是一个sockaddr结构的链表而不是一个地址清单。
		这些sockaddr结构随后可由套接口函数直接使用。
		如此一来，getaddrinfo函数把协议相关性安全隐藏在这个库函数内部。
		应用程序只要处理由getaddrinfo函数填写的套接口地址结构。
	*/
	if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo:%s\n",gai_strerror(rv));
		return 1;
	}
	//遍历所有返回结果并链接到第一个成功连接的套接
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		//创建一个套接字
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client:socket");
			continue;
		}
		//连接状态判断
		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			perror("client:connect");
			continue;
		}
		//如果创建套接字成功且连接成功，则退出循环
		break;
	}
	//如果套接口地址为空，则打印结果
	if(p == NULL)
	{
		fprintf(stderr, "client:failed to connect\n");
		return 2;
	}
	//inet_ntop 函数可以将 IP 地址在“点分十进制”和“整数”之间转换
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
	printf("client:connecting to %s\n",s);
	//freeaddrinfo 函数释放 getaddriinfo 函数返回的存储空间
	freeaddrinfo(servinfo);
	//recv 函数用于判断缓冲区数据传输的状态，传输异常则打印消息比并退出
	if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1)
	{
		perror("recv");
		exit(1);
	}
	//将字符数组的最后一位置 \0 ，用于后面一次性输出
	buf[numbytes] = '\0';
	printf("client:received %s\n",buf);
	close(sockfd);
	return 0;
}

