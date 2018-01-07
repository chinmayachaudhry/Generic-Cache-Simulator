#ifndef __CACHELINE_H__
#define __CACHELINE_H__
#include <string>
//#include <string.h>

using namespace std;
typedef long long ll;

class Cacheline
{
	private:
		ll Address, Tag, Seq_num;
		string Flag;
	public:
		Cacheline()
		{
			this->Address = -1;
			this->Tag = 0;
			this->Seq_num = 0;
			this->Flag = "Invalid";
		}
		
		ll getTag() { return this->Tag;}
		ll getAddress() { return this->Address;}
		ll getSeq_num() { return this->Seq_num;}
		string getFlag() { return this->Flag;}

		void setTag(ll Tag) { this->Tag = Tag;}
		void setAddress(ll Address) { this->Address = Address;}
		void setSeq_num(ll Seq_num) { this->Seq_num = Seq_num;}
		void setFlag(string Flag) { this->Flag = Flag;}
};
#endif
