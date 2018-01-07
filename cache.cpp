#include "cache.h"
#include "cacheline.h"
#include <math.h>

using namespace std;
typedef long long ll;

bool non_exclusive_cache_access = true;

Cache :: Cache(ll Blocksize, ll Cache_Size, ll Assoc, string Replacement_Policy, string Inclusion)
{
	this->Blocksize = Blocksize;
	this->Cache_Size = Cache_Size;
	this->Assoc = Assoc;
	this->Replacement_Policy = Replacement_Policy;
	this->Inclusion = Inclusion;
	
	this->Sets = (Cache_Size)/(Assoc * Blocksize);
	
	Reads = Writes = RdMiss = WtMiss = RdHits = WtHits = Writebacks = Mem_Traffic = 0;
	Trace_Num = 0;

	Offset_Width = (int)log2(Blocksize);
	Index_Width = (int)log2(Sets);
	Tag_Width = 64 - Offset_Width - Index_Width;

	offset_mask = mask(Offset_Width);
	index_mask = mask(Index_Width + Offset_Width);
	tag_mask = mask(Tag_Width + Index_Width + Offset_Width);

	block = new Cacheline*[Sets];
	for(long long i=0; i<Sets; i++)
		block[i] = new Cacheline[Assoc];

	L2_Cache = NULL;

	if(Replacement_Policy == "Pseudo")
	{
		PseudoLRU = new int*[Sets];
		for(int i=0; i<(int)Sets; i++)
			PseudoLRU[i] = new int[Assoc];
	}
}

ll Cache :: mask(int N)
{
	ll mask = 0;
	for(int i=0; i<N; i++)
	{
		mask <<=1;
		mask |=1;
	}
	return mask;
}

void Cache :: blockAccess(ll Address, char Op)
{
	setWay(-1);
	setIsHit(false);
	setEvicted(false);
	setWriteBack(false);
	
	Trace_Num++;

	if(Op == 'r')
		Reads++;
	else
		Writes++;

	Cacheline *Cell = findBlock(Address);
	if(Cell == NULL)					// Address Miss in Cache
	{
		setIsHit(false);		
		if(Op == 'r')
			RdMiss++;
		else
			WtMiss++;

		Mem_Traffic++;
		if(non_exclusive_cache_access == true)
			Cell = replaceBlock(Address);			// Function to replace Cacheline in case of Miss
	}
	else							// Address Hit in Cache
	{
		setIsHit(true);					// Function to set if Address was Hit or Miss
		if(Op == 'r')
			RdHits++;
		else
			WtHits++;

		updateRanking(Cell, Address);			// Functoin to Update the Sequence numbers for Cachelines
	}

	if(Op == 'w')
		Cell->setFlag("Dirty");
}

Cacheline * Cache :: findBlock(ll Address)
{
	ll Tag = calTag(Address);
	ll Index = calIndex(Address);
	for(int i=0; i<(int)Assoc; i++)
	{
		if((block[Index][i].getTag() == Tag) && (block[Index][i].getFlag() != "Invalid"))
		{
			setWay(i);
			return &(block[Index][i]);
		}
	}
	return NULL;
}

Cacheline *Cache :: replaceBlock(ll Address)
{
	ll Tag = calTag(Address);
	ll Index = calIndex(Address);
	Cacheline *evictedBlock = getBlocktobeReplaced(Index);
	
	if(evictedBlock->getFlag() == "Dirty")
	{
		setWriteBack(true);
		Writebacks++;
		Mem_Traffic++;
	}

	evictedBlock->setTag(Tag);
	evictedBlock->setAddress(Address);
	evictedBlock->setFlag("Valid");
	updateRanking(evictedBlock, Address);

	return evictedBlock;
}

Cacheline *Cache :: getBlocktobeReplaced(ll Index)
{
	Cacheline *block_toReplace = getInvalidBlock(Index);
	if(block_toReplace == NULL)
	{
		block_toReplace = getReplPolicy(Index);
		setEvicted(true);
		setEvictedAddress(block_toReplace->getAddress());	
	}	
	return block_toReplace;
}

Cacheline *Cache :: getInvalidBlock(ll Index)
{
	for(int i=0; i<(int)Assoc; i++)
	{
		if(block[Index][i].getFlag() == "Invalid")
		{
			setWay(i);
			return &(block[Index][i]);
		}
	}
	return NULL;
}

Cacheline *Cache :: getReplPolicy(ll Index)
{
	WaytoReplace(Index);				// Find which way is needed to be replaced
	ll Way = getWay();
	return &(block[Index][Way]);
}

void Cache :: WaytoReplace(ll Index)
{
	int Way;
	ll min = Trace_Num;
	if (Replacement_Policy == "LRU" || Replacement_Policy == "FIFO")
	{
		for(int i=0; i<(int)Assoc; i++)
		{
			if(block[Index][i].getSeq_num() < min)	
			{
				min = block[Index][i].getSeq_num();
				Way = i;
			}
		}
	}

	if(Replacement_Policy == "Pseudo")
	{	
		int current_node_num = 1;
		int i = 0, j = Assoc, count = 0;
		while(count < log2(Assoc))
		{
			if(PseudoLRU[Index][current_node_num] == 0)
			{
				current_node_num = (2*current_node_num) + 1;
				i = ((j-i)/2)+i;
			}
			else
			{
				current_node_num = 2*current_node_num;
				j = ((j-i)/2)+i;
			}
			count++;
		}
		Way  = i;
	}

	setWay(Way);
}

void Cache :: updateRanking(Cacheline* Cell, ll Address)
{
	ll Index = calIndex(Address);

	if(Replacement_Policy == "LRU")
		Cell->setSeq_num(Trace_Num);

	if(Replacement_Policy == "FIFO")
	{
		if(getIsHit() == true)
			Cell->setSeq_num(Cell->getSeq_num());
		else if (getIsHit() == false)
			Cell->setSeq_num(Trace_Num);
	}

	if(Replacement_Policy == "Pseudo")
	{
		int position = getWay();
		int current_node_num = 1;
		int j = Assoc;
		int i = 0;
		int count = 0;
		while(count < log2(Assoc))
		{
			if(position < (((j-i)/2)+i))
			{		
				PseudoLRU[Index][current_node_num] = 0;
				current_node_num = 2*current_node_num;
				j = ((j-i)/2)+i;
			}
			else
			{	
				PseudoLRU[Index][current_node_num] = 1;
				current_node_num = (2*current_node_num) + 1;
				i = (j/2)+i;
			}
			count++;
		}
	}
}

void Cache :: invalidateCacheline(ll Address)
{
	Cacheline *Cell = findBlock(Address);
	if (Cell != NULL)
	{
		if(Inclusion == "inclusive" && L2_Cache)
		{
			if(Cell->getFlag() == "Dirty")
			{
				setWriteBack(true);
				backInvalidation_WB++;
			} 
		}
		Cell->setFlag("Invalid");	
	}
}
