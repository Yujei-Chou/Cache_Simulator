#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
#include<cstdlib>
#include<math.h>
#include<vector>
#include<algorithm>
#include<time.h>
using namespace std;

ofstream outFile; 
//int set;
char bitstr[33];
//for direted map 
vector<int> quotient;
vector<int> reminder;

//for 4-way associative
vector< vector<int> > four_way_quotient(10000000);

//for fully associative
vector<int> fully_cache;


int StringToInt(string);
unsigned int BinaryToDec(int);
void HexToBinary(string);
void direct_mapped(unsigned int,int);
void four_way_set(unsigned int,int,int);
void fully_associate(unsigned int,int,int);

int main(int argc,char* argv[])
{
	string cache_size_str;
	string block_size_str;
	string associativity_str;
	string algorithm_str;
	string block_addr;

	int cache_size;
	int block_size;
	int asso;
	int algorithm;
	int tag_len;
	int block_num;
	int offset_len;
	int ignore;

	ifstream inFile(argv[1],ios::in);
	outFile.open(argv[2],ios::out);


	if(!inFile){
		cerr<<"Failed opening"<<endl;
		exit(1);
	}
        if(!outFile){
                cerr<<"Failed opening"<<endl;
                exit(1);
        }

	getline(inFile,cache_size_str);
	getline(inFile,block_size_str);
	getline(inFile,associativity_str);
	getline(inFile,algorithm_str);
	
	cache_size=StringToInt(cache_size_str);
	block_size=StringToInt(block_size_str);
	asso=StringToInt(associativity_str);
	algorithm=StringToInt(algorithm_str);

	block_num=cache_size*1024/block_size;
	offset_len=log2(block_size);

	
	while(getline(inFile,block_addr)!=NULL){
		HexToBinary(block_addr);
		if(asso == 0){
			direct_mapped(BinaryToDec(offset_len),block_num);
		}
		else if(asso == 1){
			four_way_set(BinaryToDec(offset_len),block_num,algorithm);
		}
		else if(asso == 2){
			fully_associate(BinaryToDec(offset_len),block_num,algorithm);
		}
	}
			
	return 0;
}

int StringToInt(string str)
{
	int rst;
	istringstream is(str);
	is>>rst;
	return rst;
}	

void HexToBinary(string str){
	unsigned int rst;
	stringstream ss;
	ss<<std::hex<<str;
	ss>>rst;
	bitstr[32]=0;
	for(int i=31;i>=0;i--){
		bitstr[i]='0'+rst%2;
		rst=rst/2;
	}
	
}
unsigned int BinaryToDec(int offset){
	unsigned int rst=0;
	for(int i=0;i<(32-offset);i++){
		if(bitstr[i]=='1'){
			rst=rst+pow(2,31-offset-i);
		}
	}
	return rst;

}




void direct_mapped(unsigned int block_addr,int block_num){ //(block address) % (number of blocks in cache)

	if(reminder.empty()){
		reminder.push_back(block_addr % block_num);	//index
		quotient.push_back(block_addr / block_num);	//tag
		outFile<<-1<<endl;							//first one must be Miss
	}
	else{
		int i=0;
		bool Find=false;
		bool Hit=false;
		while(i<reminder.size()){
			if((block_addr % block_num) == reminder.at(i) && (block_addr / block_num) == quotient.at(i)){	//Hit
				outFile<<-1<<endl;
				Hit=true;
				break;
			}
			else{		//Miss
				if((block_addr % block_num) == reminder.at(i)){	//same index but different tag
					outFile<<quotient.at(i)<<endl;
					quotient.at(i)=block_addr / block_num;
					Find=true;
					break;
				}
				else{											//different block
					i++;
				}
			}
		}
		if(!Find && !Hit){	//Miss
			reminder.push_back(block_addr % block_num);
			quotient.push_back(block_addr / block_num);
			outFile<<-1<<endl;
		}
	}

}

void four_way_set(unsigned int block_addr,int block_num,int algorithm){ //(block address) % (number of sets in the cache) 

	int set=block_num / 4;
	int set_addr=block_addr % set;	
	if(four_way_quotient[set_addr].empty()){	
		four_way_quotient[set_addr].push_back(block_addr / set); 	//tag
		outFile<<-1<<endl;					      	//First one must be Miss
	}
	else{
		int i=0;
		bool Hit=false;
		while(i<four_way_quotient[set_addr].size()){
			if((block_addr / set) == four_way_quotient[set_addr].at(i)){	//Hit
				if(algorithm == 1 || algorithm == 2){
					four_way_quotient[set_addr].erase(four_way_quotient[set_addr].begin()+i);
					four_way_quotient[set_addr].push_back(block_addr / set);
				}
				outFile<<-1<<endl;
				Hit=true;
				break;
			}
			else{
				i++;
			}
		}
		if(!Hit){							//Miss
			if(four_way_quotient[set_addr].size()<4){
				four_way_quotient[set_addr].push_back(block_addr / set);
				outFile<<-1<<endl;
			}
			else{							//block in set is full
				if(algorithm == 0 || algorithm == 1){           //FIFO  or LRU
                                	outFile<<four_way_quotient[set_addr].at(0)<<endl;                     //pop first-in tag
                                	four_way_quotient[set_addr].erase(four_way_quotient[set_addr].begin());
					four_way_quotient[set_addr].push_back(block_addr/set);	
                        	 }
                         	 else if(algorithm == 2){      //my policy
                                	int random_one=rand()%2;
					outFile<<four_way_quotient[set_addr].at(random_one)<<endl;
					four_way_quotient[set_addr].erase(four_way_quotient[set_addr].begin()+random_one);
					four_way_quotient[set_addr].push_back(block_addr / set);
                         	 }
			}
		}
	}
	
}
void fully_associate(unsigned int block_addr,int block_num,int algorithm){ 
	if(fully_cache.empty()){
		fully_cache.push_back(block_addr);			//block address=tag
		outFile<<-1<<endl;					//First one must be Miss
	}
	else{
		int i=0;
		bool Hit=false;
		while(i<fully_cache.size()){
			if(block_addr == fully_cache.at(i)){		//Hit
			
				if(algorithm == 1 || algorithm == 2){
					fully_cache.erase(fully_cache.begin()+i);
					fully_cache.push_back(block_addr);
				}
                                outFile<<-1<<endl;
                                Hit=true;
				break;
			}
			else{
				i++;
			}
		}
		if(!Hit){						//Miss
			if(fully_cache.size()<block_num){					     
				fully_cache.push_back(block_addr);
				outFile<<-1<<endl;
			}
			else{	//All blocks is full
				if(algorithm == 0 || algorithm == 1){
					outFile<<fully_cache.at(0)<<endl;
					fully_cache.erase(fully_cache.begin());
					fully_cache.push_back(block_addr);
				}
				else if(algorithm == 2){
					int random_one=rand()%(block_num/2);
					outFile<<fully_cache.at(random_one)<<endl;
					fully_cache.erase(fully_cache.begin()+random_one);
					fully_cache.push_back(block_addr);
				}
			}
		}
	}
} 

