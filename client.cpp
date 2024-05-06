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
#include <boost/filesystem.hpp>
#include "Table.h"

using namespace std;

void handleClientInput(int connfd);
int connectToServer(char* argv[],int sockfd, struct hostent *host, struct sockaddr_in *servAddr);
void upload(string arg);

string CWD,USER,SERVER,file;
int flag;
int main(int argc, char *argv[]){
	if(argc !=3){
		cout << "Invalid Args" << endl;
		return 0;
	}
	USER = Table::cmdOutput("/bin/whoami",true);
	CWD = boost::filesystem::current_path().string();
	SERVER = Table::charAToStr(argv[1],15);
	cout << CWD << "\t" << USER << "\t" << SERVER << endl;
	int sockfd,n,connfd;
	char rbuf[1000];
	struct sockaddr_in servAddr;
	struct hostent *host;
	
	cout << "Server " << argv[1] << endl;	
	cout << "Port " << argv[2] << endl;

	sockfd=connectToServer(argv,sockfd,host,&servAddr);
	
	while(1){
		bzero(rbuf,sizeof(rbuf));
		handleClientInput(sockfd);

		bzero(rbuf,sizeof(rbuf));
		n = recv(sockfd, rbuf, sizeof(rbuf),0);
		if(flag){
			flag = 0;
			system(("cat ~/Downloads/" + file).c_str());
		}
			
		printf("%s\n",rbuf);
	
		if(n<=0){
			printf("Error reading or disconnect\n");
			return 0;
		}
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
//send cmd and argument to server
void execute(int cmd,string arg,int sockfd){
	if(cmd==2)
		upload(arg);
	char buf[1000];
	sprintf(buf, "%d", cmd);
	send(sockfd, &buf, sizeof(buf),0);
	bzero(&buf,sizeof(buf));
	for(int i=0;i<arg.length();i++)buf[i]=arg[i];
	send(sockfd, &buf, sizeof(buf),0);
	if(cmd==0){
		flag=1;
		file=Table::splitBy(arg,"/")[1];
	}
}
int argType(string arg){
	vector<string> groupObjPair = Table::splitBy(arg,"/");
	return groupObjPair.size();
}
void handleClientInput(int connfd){
	cout << "Waiting for cmd..." <<endl;	
	string input;
	getline(cin, input);
	vector<string> cmdAndArg =Table::splitBy(input," ");
	string cmd =cmdAndArg[0]; 
	string arg;
	if(cmdAndArg.size()>1)
		arg=cmdAndArg[1];
	if(cmd=="download" && argType(arg)==2)
		execute(0,arg,connfd);
	else if(cmd=="list" && arg.size())
		execute(1,arg,connfd);
	else if(cmd=="upload" && argType(arg)==2)
		execute(2,arg,connfd);
	else if(cmd=="delete" && argType(arg)==2)
		execute(3,arg,connfd);
	else if(cmd=="add" && arg.size())
		execute(4,arg,connfd);
	else if(cmd=="remove" && arg.size())
		execute(5,arg,connfd);
	else if(cmd=="clean" && arg.size()==0)
		execute(6,"hello",connfd);
	else{
		cout << HELP << endl;
		handleClientInput(connfd);
	}
}
void upload(string arg){
	vector<string> groupObjPair = Table::splitBy(arg,"/");
	cout << ("scp " + CWD +"/" + groupObjPair[1] +" "+ USER+"@"+SERVER+ ":/tmp/achoudhury2Server") << endl;
	system(("scp " + CWD +"/" + groupObjPair[1] +" "+ USER+"@"+SERVER+ ":/tmp/achoudhury2Server").c_str());
}
