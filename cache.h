#ifndef __CACHE_H__
#define __CACHE_H__
#include "cacheline.h"
#include <string>
//#include <string.h>

using namespace std;
typedef long long ll;

extern bool non_exclusive_cache_access;

class Cache
{
	private:
		Cacheline ** block;
		int **PseudoLRU;
		Cache *L2_Cache;
		ll Blocksize, Cache_Size, Assoc, Sets, Tag, Offset, Index;
		string Replacement_Policy, Inclusion;
		int Index_Width, Tag_Width, Offset_Width;
		ll offset_mask, tag_mask, index_mask;
		ll Reads, Writes, RdMiss, WtMiss, RdHits, WtHits, Writebacks, Mem_Traffic, backInvalidation_WB;
		ll Trace_Num;
		ll mask(int);
		
		bool Hit, Evicted, WB;
		int Way;
		ll EvictedAddress;
	public:
		Cache(ll, ll, ll, string, string);
		ll calTag(ll Address) {return ((Address & tag_mask)>>(Index_Width + Offset_Width));}
		ll calIndex(ll Address) {return ((Address & index_mask)>>(Offset_Width));}
		ll calOffset(ll Address) {return (Address & offset_mask);}

		void blockAccess(ll, char);
		Cacheline *findBlock(ll);
		Cacheline *replaceBlock(ll);
		Cacheline *getBlocktobeReplaced(ll);
		Cacheline *getInvalidBlock(ll);
		Cacheline *getReplPolicy(ll);
		void WaytoReplace(ll);
		void updateRanking(Cacheline *, ll);
		void invalidateCacheline(ll);
	
		bool getIsHit() {return this->Hit;}
		bool getEvicted() {return this->Evicted;}
		int getWay() {return this->Way;}
		ll getEvictedAddress() {return this->EvictedAddress;}
		bool getWriteBack() {return this->WB;}

		void setIsHit(bool Hit) {this->Hit = Hit;}
		void setEvicted(bool Evicted) {this->Evicted = Evicted;}
		void setWay(int Way) {this->Way = Way;}
		void setEvictedAddress(ll EvictedAddress) {this->EvictedAddress = EvictedAddress;}
		void setWriteBack(bool WB) {this->WB = WB;}
	
		ll getReads() { return this->Reads;}
		ll getWrites() {return this->Writes;}
		ll getRdMiss() {return this->RdMiss;}
		ll getWtMiss() {return this->WtMiss;}
		ll getRdHits() {return this->RdHits;}
		ll getWtHits() {return this->WtHits;}
		ll getWtBacks() {return this->Writebacks;}
		ll getMem_Traffic() {return this->Mem_Traffic;}
		ll getbackInvalidation() {return this-> backInvalidation_WB;}

		void set_L2Cache(Cache *L2_Cache) {this->L2_Cache = L2_Cache;}
		Cache *get_L2Cache(){ return this->L2_Cache;}
};
#endif
