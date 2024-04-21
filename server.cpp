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

void createDirectory(char *argv[], int argc);
void checkConnectionError(int *n);
string handleCmd(int cmd, string arg);
string handleDownload(string arg);
void handleList(string arg);
string handleUpload(string arg);
string handleDelete(string arg);
void handleAdd(string arg);
void handleRemove(string arg);

string USER;
Table t;
struct sockaddr_in servAddr, clientAddr;
int main(int argc, char *argv[]){

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
	


	cout << "Port: " << PORT << endl;//!need to make the port auto find
	cout << "Partition Power " << partitionPower << endl;
	cout << "Number of Available Disks " << numberOfAvailableDisks << endl;
		

	if((sockfd = socket(AF_INET, SOCK_STREAM,0)) < 0){	
		perror("cannot create socket");
		return 0;
	}

	servAddr.sin_family = AF_INET;
	servAddr.sin_port =htons(PORT);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	
	if (bind(sockfd,(struct sockaddr*)&servAddr, sizeof(servAddr)) < 0){
		perror("Failure to bind");
		return 0;	
	}

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
			cout << "Waiting for Request..." << endl;
			bzero(rbuf,sizeof(rbuf));
			n = recv(connfd, rbuf, sizeof(rbuf),0);
			checkConnectionError(&n);
			int cmd = rbuf[0] - '0';
						
			bzero(rbuf,sizeof(rbuf));
			n = recv(connfd, rbuf, sizeof(rbuf),0);
			checkConnectionError(&n);

			string res = handleCmd(cmd,Table::charAToStr(rbuf,1000));
			
			bzero(rbuf,sizeof(rbuf));
			for(int i=0;i<res.length();i++)rbuf[i]=res[i];
			send(connfd, &rbuf, sizeof(rbuf),0);
			
		}
	}
	close(sockfd);		
	return 0;
}
void checkConnectionError(int *n){
	if(*n<=0){
		printf("Error reading\n");
		exit(0);
	}
	*n=0;
}
void createDirectory(char *argv[], int argc){

	system(("mkdir /tmp/"+ USER).c_str());
	system(("mkdir /tmp/" + USER + "/work").c_str());

	for(int i = 2; i<argc; i++){
		string ip = Table::charAToStr(argv[i],15);
		t.loadDisk(ip);
		system(("ssh "+ USER + "@" + ip + " \"mkdir /tmp/achoudhury2\"").c_str());
	}
	t.allocateDisks();
}
string handleCmd(int cmd, string arg){
	cout << "CMD " << cmd << " Arg " << arg <<endl;
	switch (cmd) {
		case 0:
			return handleDownload(arg);
	    		break;
	  	case 1:
	    		cout << "list";
			handleList(arg);
	    		break;
	  	case 2:
			return handleUpload(arg);
	    		break;
	  	case 4:
	    		cout << "add";
			handleAdd(arg);
	    		break;
	  	case 5:
	    		cout << "remove";
			handleRemove(arg);
	    		break;
	  	case 3:
			return handleDelete(arg);
	    		break;
	}
	return " ";
}



string handleDownload(string arg){
	return t.retrieve(arg,Table::charAToStr(inet_ntoa(clientAddr.sin_addr),15));
}
void handleList(string arg){}
string handleUpload(string arg){
	return t.insert(arg);
}
string handleDelete(string arg){
	return t.deleteFile(arg);
}
void handleAdd(string arg){}
void handleRemove(string arg){}
