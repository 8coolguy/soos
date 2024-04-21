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
# include <mutex>
# include "Table.h"

using namespace std;


Table::Table(){
	_pp = 0;
}

Table::Table(string user,int pp){
	_pp = pp;
	_user = user;
}
void Table::init(std::string user,int pp){
	_pp = pp;
	_user = user;
}
int Table::loadDisk(string address){
	lock_guard<recursive_mutex> lock (_mutex);
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
	lock_guard<recursive_mutex> lock (_mutex);
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
	lock_guard<recursive_mutex> lock (_mutex);
	for(DiskDiv d: _diskTable){
		if(d.start <= partition && partition <= d.end){//!bug double counted somewhere
			return d.disk;
		}
	}
	return -1;
}
void Table::scp(int src,int dst,string name,string srcLoc){
	lock_guard<recursive_mutex> lock (_mutex);
	vector<string> group = splitBy(name,"/");

	string cmd = " \"mkdir /tmp/achoudhury2/"+group[0]+"\"";
	string ip =  _user + "@"+_ipMap[dst];
	//case if we deload disk and we don't have it in the ip map.
	string srcIp;

	string dir = "/tmp/achoudhury2/"+name;
	
	if(src==-1)
		srcIp = _user + "@"+srcLoc;
	else
		srcIp = _user + "@"+_ipMap[src];
	string srcIpF = srcIp +":"+dir;
	string dstIp= ip+":/tmp/achoudhury2/"+group[0];
	system(("ssh " +ip+" " + cmd).c_str());
	system(("scp "+srcIpF+" "+dstIp).c_str());
	system(("ssh " + srcIp +" \"rm " + dir+" \"").c_str());
}
string Table::addDisk(string address){
	lock_guard<recursive_mutex> lock (_mutex);
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

string Table::rmDisk(string address){
	lock_guard<recursive_mutex> lock (_mutex);
	auto it = find(_ipMap.begin(),_ipMap.end(),address);
	if(it == _ipMap.end()) return "Disk not found";
	int diskNumber = it - _ipMap.begin(); 
		
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
	map<int,int> bbufferMap;
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
		if(oldBackupLocation ==diskNumber){
			int newBackUpLocation = nextDisk(newDiskLocation);
			scp(-1,newBackUpLocation,name,srcIp);
			bbufferMap.insert({partition,newBackUpLocation});
			res+="Backup "+name+ " moved from "+to_string(oldBackupLocation)+" to " + to_string(newBackUpLocation)+"\n";
		}	
	}
	for(auto p: bufferMap) _partitionTable.at(p.first).first = p.second;
	for(auto p: bbufferMap) _partitionTable.at(p.first).second= p.second;
	for(auto d: _diskTable) cout << d.start << " " << d.end << " " << d.disk << endl;
	return res;
}
int Table::diskIpLookUp(string name,int*disk,string*ipLoc,int *back,string *backLoc){
	lock_guard<recursive_mutex> lock (_mutex);
	int partition = getPartitionNumber(name);
	try{
		*disk = _partitionTable.at(partition).first;
		*back = _partitionTable.at(partition).second;
		*ipLoc = _ipMap[*disk];
		*backLoc = _ipMap[*back];
		return 0;
	} catch(...){
		return 1;
	}
}
string Table::insert(string name){
	lock_guard<recursive_mutex> lock (_mutex);
	int partition = getPartitionNumber(name);
	vector<string> groupObjPair = splitBy(name,"/");
	if(groupObjPair.size()!=2) return "Argument needs to be divided by slash";
	_nameMap.insert({name,partition });
	
	int disk = getDisk(partition);
	int backup = nextDisk(disk);
	_partitionTable.insert({partition, make_pair(disk,backup) });

	string ipLoc = _ipMap[disk];
	string backLoc = _ipMap[backup];

	string ip1 =_user + "@"+ ipLoc;
	string ip2 =_user + "@"+ backLoc;

	string cmd = " \"mkdir /tmp/achoudhury2/"+groupObjPair[0]+"\"";
	string src = "/tmp/achoudhury2Server/"+groupObjPair[1];
	string dst = ":/tmp/achoudhury2/"+groupObjPair[0];
	
	system(("ssh " + ip1 + cmd).c_str());
	system(("scp " + src + " " + ip1+ dst).c_str());	
	//save to backup	
	system(("ssh " + ip2 + cmd).c_str());
	system(("scp " + src + " " + ip2+ dst).c_str());	
	return "Your file was saved in Disk: "+to_string(disk)+" located at "+ ipLoc +" and your file was saved in Disk: "+to_string(backup)+" located at "+ backLoc;
}
string Table::retrieve(string name,string client){
	lock_guard<recursive_mutex> lock (_mutex);
	int partition = getPartitionNumber(name);
	vector<string> groupObjPair = splitBy(name,"/");
	if(groupObjPair.size()!=2) return "Argument needs to be divided by slash";

	int disk;
	int back;
	string ipLoc;	
	string backLoc;
	if(diskIpLookUp(name,&disk,&ipLoc,&back,&backLoc)) return "Group/File Likely does not exist";
	
	string ip1 =_user + "@"+ ipLoc;
	string backUpIp =_user + "@"+ backLoc;
	string src = ip1+ ":/tmp/achoudhury2/"+name;
	string backUpSrc = backUpIp+ ":/tmp/achoudhury2/"+name;
	string ip2 =_user + "@"+ client;
	string dst = ip2+ ":~/Downloads/";

	//cout << ("scp " + src +" "+ dst) << endl;		
	if(system(("scp " + src + " " + dst).c_str())==0)
		return "Your file was saved in Disk: "+to_string(disk)+" located at "+ ipLoc + " Saved to your ~/Downloads/ folder.";
	if(system(("scp " + backUpSrc + " " + dst).c_str())==0)
		return "Your Backup file was saved in Disk: "+to_string(disk)+" located at "+ ipLoc + " Saved to your ~/Downloads/ folder.";
	return "Both Disks were corrupted";
}
string Table::deleteFile(string name){
	lock_guard<recursive_mutex> lock (_mutex);
	int partition = _nameMap[name];
	vector<string> groupObjPair = splitBy(name,"/");
	_nameMap.erase(name);
	if(groupObjPair.size()!=2) return "Argument needs to be divided by slash";

	int disk;
	int back;
	string ipLoc;	
	string backLoc;
	if(diskIpLookUp(name,&disk,&ipLoc,&back,&backLoc)) return "Group/File Likely does not exist";

	//_partitionTable.erase(partition);	

	string ip1 =_user + "@"+ ipLoc;
	string cmd = "\"rm /tmp/achoudhury2/" +name +"\"";

	string ip2 =_user + "@"+ backLoc;
	
	system(("ssh " + ip1 + " " + cmd).c_str());	
	system(("ssh " + ip2 + " " + cmd).c_str());	
	return "Your file was deleted in Disk: "+to_string(disk)+" located at "+ ipLoc +" and your file was deleted in Disk: "+to_string(back)+" located at "+ backLoc;
}
string Table::lsCmd(int disk,string group){
	lock_guard<recursive_mutex> lock (_mutex);
	string res = "ssh "+ _user+"@"+_ipMap[disk] + " \"ls -lrt /tmp/achoudhury2/"+group+"\"";
	if(system(res.c_str())!=0) return "No group\n";
	return cmdOutput(res.c_str(),1);
}
string Table::listFiles(string name){
	lock_guard<recursive_mutex> lock (_mutex);
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
	lock_guard<recursive_mutex> lock (_mutex);
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
	lock_guard<recursive_mutex> lock (_mutex);
	//cout << "Hello " <<endl;
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
	lock_guard<recursive_mutex> lock (_mutex);
	vector<int> disks = listDisk();
	auto it	= find(disks.begin(),disks.end(),i);
	int index = it - disks.begin(); 
	return disks[(index+1) % disks.size()];
}
