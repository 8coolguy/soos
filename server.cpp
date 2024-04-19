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

using namespace std;

void createDirectory(char *argv[], int argc);
void checkConnectionError(int *n);
void handleCmd(int cmd, string arg);
string charAToStr(char *arr,int n);

void handleDownload(string arg);
void handleList(string arg);
void handleUpload(string arg);
void hanleDelete(string arg);
void handleAdd(string arg);
void handleRemove(string arg);

string USER;
struct sockaddr_in servAddr, clientAddr;
int main(int argc, char *argv[]){
	createDirectory(argv,argc);

	char rbuf[1000];
	int numberOfAvailableDisks = argc - 2;
	int partitionPower = stoi(argv[1]);
	int n,sockfd,connfd;

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
			bzero(rbuf,sizeof(rbuf));
			cout << "Waiting for Command " <<endl;
			n = recv(connfd, rbuf, sizeof(rbuf),0);
			checkConnectionError(&n);
			int cmd = rbuf[0] - '0';
						
			bzero(rbuf,sizeof(rbuf));
			cout << "Waiting for Arg " <<endl;
			n = read(connfd, rbuf, sizeof(rbuf));
			checkConnectionError(&n);

			handleCmd(cmd,charAToStr(rbuf,1000));
			
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
string charAToStr(char *arr,int n){
	string res;
	for(int i =0;i<n;i++){
		if(arr[i]=='\0')break;
		res+=arr[i];	
	}
	return res;
}
void createDirectory(char *argv[], int argc){
	FILE * fp ;

	if((fp= popen("/bin/whoami","r")) == NULL) {
		cout << "There is an error" << endl;
		exit(0);
	}
	char buffer[1000];
	while (!feof(fp)) fread(buffer, sizeof(buffer), 1, fp); 
	USER = charAToStr(buffer,1000);
	USER.erase(USER.find_last_not_of(" \n\r\t")+1);

	system(("mkdir /tmp/"+ USER).c_str());
	system(("mkdir /tmp/" + USER + "/work").c_str());

	for(int i = 2; i<argc; i++){
		system(("ssh "+ USER + "@" + charAToStr(argv[i],12) + " \"mkdir /tmp/achoudhury2\"").c_str());
	}
	
	fclose(fp);
	
}
void handleCmd(int cmd, string arg){

	cout << "CMD " << cmd << " Arg " << arg <<endl;
	switch (cmd) {
		case 0:
	    		cout << "download";
			handleDownload(arg);
	    		break;
	  	case 1:
	    		cout << "list";
			handleList(arg);
	    		break;
	  	case 2:
	    		cout << "upload";
			handleUpload(arg);
	    		break;
	  	case 3:
	    		cout << "add";
			handleAdd(arg);
	    		break;
	  	case 4:
	    		cout << "remove";
			handleRemove(arg);
	    		break;
	}
}



void handleDownload(string arg){
}
void handleList(string arg){}
void handleUpload(string arg){
	string file = arg;
	system(("scp " + USER +"@"+charAToStr(inet_ntoa(clientAddr.sin_addr),12)+":"+file+" /tmp/" +USER+"/work").c_str());
}
void hanleDelete(string arg){}
void handleAdd(string arg){}
void handleRemove(string arg){}
