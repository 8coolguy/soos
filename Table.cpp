/*
 * File:		Table.cpp
 *
 * Description: This file contains the class definitions for types in
 *				Simple C.  A type is either a scalar type, an array type,
 *				or a function type.	 Types include a specifier and the
 *				number of levels of indirection.  Array types also have a
 *				length, and function types also have a parameter list.	An
 *				error type is also supported for use in undeclared
 *				identifiers and the results of type checking.
 *
 *				Extra functionality:
 *				- equality and inequality operators
 *				- predicate functions
 *				- stream operator
 *				- error type
 */
# include <algorithm>
# include <iostream>
# include <cassert>
# include <string>
# include <unistd.h>
# include <vector>
# include "Table.h"

using namespace std;


Table::Table(){
	_pp = 0;
}

Table::Table(string user,int pp){
	_pp = pp;
	_user = user;
}
int Table::loadDisk(string address){
	auto it  = find(_ipMap.begin(),_ipMap.end(),"");
	if(it==_ipMap.end()){
		_ipMap.push_back(address);
		return _ipMap.size()-1;
	}else{
		int index = it - _ipMap.begin(); 
        	_ipMap[index] = address;
		return index;
	}	
}
void Table::deloadDisk(int disk){
	_ipMap[disk] = "";
}

int Table::getPartitionNumber(string name){
	return stoi(cmdOutput(("echo '" + name + "' | md5sum").c_str(),true).substr(28,4),0,16) %  Table::myPow(2,_pp);
}
void Table::allocateDisks(){
	assert(_diskTable.size() == 0);
	int p = Table::myPow(2,_pp);
	int n = getDiskCount();
	int d = p/n;
	int diskPtr=0;
	for(int i=0;i<_ipMap.size();i++){
		if(_ipMap[i]=="")continue;
		DiskDiv div;
		div.start = diskPtr;
		div.end =diskPtr + d;
		diskPtr+=d;
		div.disk = i;
		_diskTable.push_back(div);	
	}
	for(auto d: _diskTable) cout << d.start << " " << d.end << " " << d.disk << endl;
}
int Table::getDisk(int partition){
	for(DiskDiv d: _diskTable){
		if(d.start <= partition && partition <= d.end){//!bug double counted somewhere
			return d.disk;
		}
	}
	return -1;
}
void Table::scp(int src,int dst,string name,string srcLoc){
	vector<string> group = splitBy(name,"/");

	string cmd = " \"mkdir /tmp/achoudhury2/"+group[0]+"\"";
	string ip =  _user + "@"+_ipMap[dst];
	//case if we deload disk and we don't have it in the ip map.
	string srcIp;
	cout << src << srcLoc << endl;
	if(src==-1)
		srcIp = _user + "@"+srcLoc+":/tmp/achoudhury2/"+name;
	else
		srcIp = _user + "@"+_ipMap[src]+":/tmp/achoudhury2/"+name;
	string dstIp= ip+":/tmp/achoudhury2/"+group[0];
	cout << ("ssh " +ip+" " + cmd) << endl;
	cout << ("scp "+srcIp+" "+dstIp) << endl;
	system(("ssh " +ip+" " + cmd).c_str());
	system(("scp "+srcIp+" "+dstIp).c_str());

}
string Table::addDisk(string address){
	system(("ssh "+ _user + "@" + address + " \"mkdir /tmp/achoudhury2\"").c_str());
	int newDisk = loadDisk(address);
	int n = getDiskCount();
	vector<DiskDiv> buffer;
	for(int i =0;i<_diskTable.size();i++){
		DiskDiv d = _diskTable[i];
		DiskDiv newDiv;
		newDiv.disk = newDisk;
		newDiv.start = d.start;
		int range = d.end - d.start;
		newDiv.end = d.start + range/n;
		d.start = newDiv.end;
		_diskTable[i] = d;
		buffer.push_back(newDiv);
	}
	for(auto d: buffer) _diskTable.push_back(d);
	string res = "Changes Made:\n";
	map<int,int> bufferMap;
	for(auto namePart: _nameMap){
		string name = namePart.first;
		int partition = getPartitionNumber(name);
		int newDiskLocation = getDisk(partition);
		if(newDiskLocation==-1) return "Something went wrong"; 
		int oldDiskLocation = _partitionTable.at(partition).first;
		int oldBackupLocation = _partitionTable.at(partition).second;//add code to delete with backups 
		if(oldDiskLocation!=newDiskLocation){
			scp(oldDiskLocation,newDiskLocation,name,"");
			bufferMap.insert({partition,newDiskLocation});
			res+=name+ " moved from "+to_string(oldDiskLocation)+" to " + to_string(newDiskLocation)+"\n";
		}	
	}
	for(auto p: bufferMap) _partitionTable.at(p.first).first = p.second;
	for(auto d: _diskTable) cout << d.start << " " << d.end << " " << d.disk << endl;
	return res;
}

string Table::rmDisk(int diskNumber){
	string srcIp = _ipMap[diskNumber];
	deloadDisk(diskNumber);
	int n = getDiskCount();
	vector<DiskDiv> buffer;
	vector<int> validDisks = listDisk();

	for(int i =0;i<_diskTable.size();i++){	
		DiskDiv d = _diskTable[i];
		if(d.disk!=diskNumber){
			buffer.push_back(d);
			continue;
		}
		int range = d.end - d.start;
		int diskPtr = d.start;
		for(auto disk: validDisks){
			DiskDiv newDiv;
			newDiv.start= diskPtr;
			newDiv.end = diskPtr + range/n;
			newDiv.disk = disk;
			diskPtr +=range/n;
			buffer.push_back(newDiv);
		}
	}
	_diskTable = buffer;

	string res = "Changes Made:\n";
	map<int,int> bufferMap;
	for(auto namePart: _nameMap){
		string name = namePart.first;
		int partition = getPartitionNumber(name);
		int newDiskLocation = getDisk(partition);
		if(newDiskLocation==-1) return "Something went wrong"; 
		int oldDiskLocation = _partitionTable.at(partition).first;
		int oldBackupLocation = _partitionTable.at(partition).second;//add code to delete with backups 
		if(oldDiskLocation==diskNumber){
			scp(-1,newDiskLocation,name,srcIp);
			bufferMap.insert({partition,newDiskLocation});
			res+=name+ " moved from "+to_string(oldDiskLocation)+" to " + to_string(newDiskLocation)+"\n";
		}	
	}
	for(auto p: bufferMap) _partitionTable.at(p.first).first = p.second;
	for(auto d: _diskTable) cout << d.start << " " << d.end << " " << d.disk << endl;
	return res;
}
int Table::diskIpLookUp(string name,int*disk,string*ipLoc){
	int partition = getPartitionNumber(name);
	try{
		*disk = _partitionTable.at(partition).first;
		*ipLoc = _ipMap[*disk];
		return 0;
	} catch(...){
		return 1;
	}
}
string Table::insert(string name){
	int partition = getPartitionNumber(name);
	vector<string> groupObjPair = splitBy(name,"/");
	if(groupObjPair.size()!=2) return "Argument needs to be divided by slash";
	_nameMap.insert({name,partition });
	//check if already in table
	for(DiskDiv d: _diskTable){
		if(d.start <= partition && partition <= d.end){//!bug double counted somewhere
			_partitionTable.insert({partition, make_pair(d.disk,nextDisk(d.disk)) });
			break;
		}
	}
	
	int disk;
	string ipLoc;	
	if(diskIpLookUp(name,&disk,&ipLoc)) return "Group/File Likely does not exist";

	string ip =_user + "@"+ ipLoc;//!implement code for backup if first drive fails
	string cmd = " \"mkdir /tmp/achoudhury2/"+groupObjPair[0]+"\"";
	string src = "/tmp/achoudhury2Server/"+groupObjPair[1];
	string dst = ":/tmp/achoudhury2/"+groupObjPair[0];
	cout << ("ssh " + ip + cmd) <<endl;
	cout << ("scp " + src + " " + ip+ dst) << endl;	
	system(("ssh " + ip + cmd).c_str());
	system(("scp " + src + " " + ip+ dst).c_str());	
	return "Your file was saved in Disk: "+to_string(disk)+" located at "+ ipLoc;
}
string Table::retrieve(string name,string client){
	int partition = getPartitionNumber(name);
	vector<string> groupObjPair = splitBy(name,"/");
	if(groupObjPair.size()!=2) return "Argument needs to be divided by slash";

	int disk;
	string ipLoc;	
	if(diskIpLookUp(name,&disk,&ipLoc)) return "Group/File Likely does not exist";
	
	string ip1 =_user + "@"+ ipLoc;
	string src = ip1+ ":/tmp/achoudhury2/"+name;
	string ip2 =_user + "@"+ client;
	string dst = ip2+ ":~/Downloads/";

	cout << ("scp " + src +" "+ dst) << endl;		
	system(("scp " + src + " " + dst).c_str());	
	return "Your file was saved in Disk: "+to_string(disk)+" located at "+ ipLoc + " Saved to your ~/Downloads/ folder.";
}
string Table::deleteFile(string name){
	int partition = getPartitionNumber(name);
	vector<string> groupObjPair = splitBy(name,"/");
	if(groupObjPair.size()!=2) return "Argument needs to be divided by slash";

	int disk;
	string ipLoc;	
	if(diskIpLookUp(name,&disk,&ipLoc)) return "Group/File Likely does not exist";

	_partitionTable.erase(partition);	

	string ip1 =_user + "@"+ ipLoc;
	string cmd = "\"rm /tmp/achoudhury2/" +name +"\"";
	
	cout << ("ssh " + ip1 + " " + cmd)<< endl;		
	system(("ssh " + ip1 + " " + cmd).c_str());	
	return "Your file was deleted in Disk: "+to_string(disk)+" located at "+ ipLoc;
}
string Table::lsCmd(int disk,string group){
	string res = "ssh "+ _user+"@"+_ipMap[disk] + " \"ls -lrt /tmp/achoudhury2/"+group+"\"";
	if(system(res.c_str())!=0) return "No files\n";
	return cmdOutput(res.c_str(),1);
}
string Table::listFiles(string name){
	vector<int> disks =listDisk();
	string res;
	for(int disk : disks){
		res+="*** Disk " +to_string(disk) +"@"+_ipMap[disk]+" ***\n";
		res+=lsCmd(disk,name);
		res+="\n";
	}
	return res;
}
string Table::cmdOutput(string cmd,bool strip){
	char buffer[1000];
	fill(buffer, buffer+1000, 0);
	FILE *fp;
	if((fp= popen(cmd.c_str(),"r")) == NULL) {
		cout << "There is an error" << endl;
		exit(0);
	}
	while (!feof(fp)) fread(buffer, sizeof(buffer), 1, fp); 
	string res = charAToStr(buffer,1000);
	if(strip) res.erase(res.find_last_not_of(" \n\r\t")+1);
	return res;
}
string Table::charAToStr(char *arr,int n){
	string res;
	for(int i =0;i<n;i++){
		if(arr[i]=='\0')break;
		res+=arr[i];	
	}
	return res;
}
int Table::getDiskCount(){
	int count = 0;
	for (auto s : _ipMap)
		if(s!="") count+=1;
	return count;	
}
int Table::myPow(int x, unsigned int p)
{
  if (p == 0) return 1;
  if (p == 1) return x;
  
  int tmp = myPow(x, p/2);
  if (p%2 == 0) return tmp * tmp;
  else return x * tmp * tmp;
}
vector<int> Table::listDisk(){
	vector<int> res;
	for(int i =0;i<_ipMap.size();i++)
		if(_ipMap[i]!="") res.push_back(i);
	return res;
}
vector<string> Table::splitBy(string s, string d){
	vector<string> res;
	for(int i = 0; i<s.size();i++){
		if(s[i]==d[0]){
			res.push_back(s.substr(0,i));
			res.push_back(s.substr(i+1));
			break;
		}
	}
	if(res.size()==0) res.push_back(s);
	return res;
}
int Table::nextDisk(int i){
	vector<int> disks = listDisk();
	auto it	= find(disks.begin(),disks.end(),i);
	int index = it - disks.begin(); 
	return disks[(index+1) % disks.size()];
}
