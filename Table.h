/*
 * File:		Table.h
 *
 * Description: This file contains the class definition for a hash table used for
 *				partion and disk managment.  At this point, a Table merely consists of a
 *				name and a type, neither of which you can change.
 */

# ifndef TABLE_H
# define TABLE_H 
# include <string>
# include <vector>
# include <map>

struct DiskDiv{
	int start;
	int end;
	int disk;
};

class Table{
	int _pp;
	std::string _user;
	std::vector<std::string> _ipMap;//vector where the index points to the Map location
	std::vector<DiskDiv> _diskTable;
	std::map<int,std::pair<int,int>> _partitionTable;
	
public:
	Table();
	Table(std::string user,int pp);
	int loadDisk(std::string address);
	int getPartitionNumber(std::string name);
	void allocateDisks();
	void addDisk(std::string address);
	void deloadDisk(int disk);
	std::string rmDisk(int diskNumber);
	int diskIpLookUp(std::string name,int*disk,std::string*ipLoc);
	std::string insert(std::string name);
	std::string retrieve(std::string name,std::string client);
	std::string deleteFile(std::string name);
	static std::string cmdOutput(std::string cmd,bool strip);
	static std::string charAToStr(char *arr,int n);
	int getDiskCount();
	static int myPow(int x, unsigned int p);
	std::vector<int> listDisk();
	static std::vector<std::string> splitBy(std::string s, std::string d);
	int nextDisk(int i);
};


# endif /* TABLE_H */
