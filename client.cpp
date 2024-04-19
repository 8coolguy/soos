#define HELP "Here are the commands you can use for the Open Stack System:\n\t1. download <user/object>\n\t2. list <user>\n\t3. upload <user/object>\n\t4. delete <user/object>\n\t5. add <disk>\n\t6. remove <disk>\n\t7. help\n\t"
#include<iostream>
#include<cctype>
#include<string>
#include<sys/socket.h>
#include <bits/stdc++.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/in.h>
#include <unistd.h>
#include<strings.h>
#include<stdio.h>
#include<stdlib.h>
#include<vector>

using namespace std;

void hanleClientInput(int connfd);
int connectToServer(char* argv[],int sockfd, struct hostent *host, struct sockaddr_in *servAddr);

int main(int argc, char *argv[]){
	int sockfd,n,connfd;
	char rbuf[1000];
	struct sockaddr_in servAddr;
	struct hostent *host;
	
	cout << "Server " << argv[1] << endl;	
	cout << "Port " << argv[2] << endl;

	sockfd=connectToServer(argv,sockfd,host,&servAddr);
	
	while(1){
		
		bzero(rbuf,1);
		cout << "Waiting for instruction" << endl;
		hanleClientInput(sockfd);
		//n = read(sockfd, rbuf, sizeof(rbuf));
		//printf("%s\n",rbuf);
	
		if(n<0){
			printf("Error reading\n");
			return 0;
		}
		//if(n==0) break;
	}
	close(sockfd);
	return 0;
}
int connectToServer(char *argv[], int sockfd, struct hostent *host, struct sockaddr_in *servAddr){
	int connfd;
	host = (struct hostent *)gethostbyname(argv[1]);	
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if(0 >sockfd){
		perror("Failed to setup socket");
		exit(0);
	}
	servAddr->sin_family = AF_INET;
	servAddr->sin_port =htons(atoi(argv[2]));
	servAddr->sin_addr = *((struct in_addr *) host->h_addr);
	connfd = connect(sockfd, (struct sockaddr *)servAddr,  sizeof(struct sockaddr));
	if(connfd){
		perror("Failed to Connect");
		exit(0);
	}
	return sockfd;
}
vector<string> split(string sentence){
	stringstream words(sentence);
	string word;
	vector<string> res;
	while (getline(words, word, ' ')) res.push_back(word);
	return res;
}
//send cmd and argument to server
void execute(int cmd,string arg,int sockfd){
	//cout << "CMD " << cmd << " ARG " <<arg <<endl;
	char buf[1000];
	sprintf(buf, "%d", cmd);
	send(sockfd, &buf, sizeof(buf),0);
	bzero(&buf,sizeof(buf));
	for(int i=0;i<arg.length();i++)buf[i]=arg[i];
	send(sockfd, &buf, sizeof(buf),0);
}
void hanleClientInput(int connfd){
	string input;
	getline(cin, input);
	vector<string> cmdAndArg =split(input);
	string cmd =cmdAndArg[0]; 
	string arg;
	if(cmdAndArg.size()>1)
		arg=cmdAndArg[1];
	if(cmd=="help")
		cout << HELP << endl;
	else if(cmd=="download")
		execute(0,arg,connfd);
	else if(cmd=="list")
		execute(1,arg,connfd);
	else if(cmd=="upload")
		execute(2,arg,connfd);
	else if(cmd=="delete")
		execute(3,arg,connfd);
	else if(cmd=="add")
		execute(4,arg,connfd);
	else if(cmd=="remove")
		execute(5,arg,connfd);
	else
		cout << HELP << endl;
}
