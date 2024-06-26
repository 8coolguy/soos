#define PORT 6000

#include<iostream>
#include<string>
#include<sys/socket.h>
#include<strings.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<netinet/in.h>
#include<unistd.h>
#include<cstdlib>
#include<cstdio>
#include<fstream>
#include"Table.h"

using namespace std;

string USER;
Table t;
struct sockaddr_in servAddr, clientAddr;
int port;

void createDirectory(char *argv[], int argc);
void checkConnectionError(int n);
string handleCmd(int cmd, string arg);
string handleDownload(string arg){return t.retrieve(arg,Table::charAToStr(inet_ntoa(clientAddr.sin_addr),15));}
string handleList(string arg){return t.listFiles(arg);}
string handleUpload(string arg){return t.insert(arg);}
string handleDelete(string arg){return t.deleteFile(arg);}
string handleAdd(string arg){return t.addDisk(arg);}
string handleRemove(string arg){return t.rmDisk(stoi(arg));}
int main(int argc, char *argv[]){
	port = PORT;
	if(argc < 3){
		cout << "invlaid args" << endl;
		exit(0);
	}	

	char rbuf[1000];
	int numberOfAvailableDisks = argc - 2;
	int partitionPower = stoi(argv[1]);
	int n,sockfd,connfd;
	USER = Table::cmdOutput("/bin/whoami",true);	
	t = Table(USER,partitionPower);
	createDirectory(argv,argc);
	
	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0){	
		perror("cannot create socket");
		return 0;
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port =htons(PORT);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	
	while(bind(sockfd,(struct sockaddr*)&servAddr, sizeof(servAddr)) != 0){
		port = rand() % 2000 +4000;
		servAddr.sin_port =htons(port);
	}
	cout << "Port: " << port << endl;//!need to make the port auto find
	listen(sockfd,8);
	int sin_size = sizeof(struct sockaddr_in);
	while(1){
		printf("Waiting for connection\n");
		if((connfd = accept(sockfd, (struct sockaddr*)&clientAddr, (socklen_t *)&sin_size)) < 0){
			perror("Failure to accept the client connection\n");
			return 0;		
		}
	
		printf("Connection Established with client: IP %s and Port %d\n",inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		
	while(1){
			//cmd	
			cout << "Waiting for Request..." << endl;
			bzero(rbuf,sizeof(rbuf));
			n = recv(connfd, rbuf, sizeof(rbuf),0);
			checkConnectionError(n);
			if(n==0) break;			
			int cmd = rbuf[0] - '0';
			
			//arg		
			bzero(rbuf,sizeof(rbuf));
			n = recv(connfd, rbuf, sizeof(rbuf),0);
			checkConnectionError(n);
			if(n==0) break;			

			//response
			string res = handleCmd(cmd,Table::charAToStr(rbuf,1000));	
			bzero(rbuf,sizeof(rbuf));
			for(int i=0;i<res.length();i++)rbuf[i]=res[i];
			n = send(connfd, &rbuf, sizeof(rbuf),0);
			if(n==0) break;			
		}
	}
}
void checkConnectionError(int n){
	if(n<0){
		printf("Error reading\n");
		exit(0);
	}
}
void createDirectory(char *argv[], int argc){
	system("rm -rf /tmp/achoudhury2Server");
	system("mkdir /tmp/achoudhury2Server/");

	for(int i = 2; i<argc; i++){
		string ip = Table::charAToStr(argv[i],15);
		t.loadDisk(ip);
		system(("ssh "+ USER + "@" + ip + " \"rm -rf /tmp/achoudhury2\"").c_str());
		system(("ssh "+ USER + "@" + ip + " \"mkdir /tmp/achoudhury2\"").c_str());
	}
	t.allocateDisks();
}
string handleCmd(int cmd, string arg){
	//cout << "CMD " << cmd << " Arg " << arg <<endl;
	switch (cmd) {
		case 0:
			return handleDownload(arg);
	    		break;
	  	case 1:
			return handleList(arg);
	    		break;
	  	case 2:
			return handleUpload(arg);
	    		break;
	  	case 4:
			return handleAdd(arg);
	    		break;
	  	case 5:
			return handleRemove(arg);
	    		break;
	  	case 3:
			return handleDelete(arg);
	    		break;
	}
	return " ";
}
