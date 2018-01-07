#include <iostream>
#include <string>
#include <string.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include "cacheline.h"
#include "cache.h"


void print_config(ll*, ll*, ll*, ll*, ll*, string*, string*, char*);
void print_result(Cache *);

int main(int argc, char *argv[])
{
	ll Blocksize, L1_Size, L1_Assoc, L2_Size, L2_Assoc, Address;
	char op;
	string Replacement_Policy, Inclusion;
	char *fname = (char *)malloc(100);

	Blocksize = atol(argv[1]);
	L1_Size = atol(argv[2]);
	L1_Assoc = atol(argv[3]);
	L2_Size = atol(argv[4]);
	L2_Assoc = atol(argv[5]);

	switch(atoi(argv[6]))
	{
		case 0: Replacement_Policy = "LRU"; break;
		case 1: Replacement_Policy = "FIFO"; break;
		case 2: Replacement_Policy ="Pseudo"; break;
		default: cout << "Wrong Policy Selected" << endl;
	}
	
	switch(atoi(argv[7]))
	{
		case 0: Inclusion = "non-inclusive"; break;
		case 1: Inclusion = "inclusive"; break;
		case 2: Inclusion = "exclusive"; break;
		default: cout << "Wrong Policy Selected" << endl;
	}
	strcpy(fname, argv[8]);

	Cache *L1, *L2;
	L1 = new Cache(Blocksize, L1_Size, L1_Assoc, Replacement_Policy, Inclusion);
	if(L2_Size!=0)
	{
		L2 = new Cache(Blocksize, L2_Size, L2_Assoc, Replacement_Policy, Inclusion);
		L1->set_L2Cache(L2);
	}

	ifstream fin;
	fin.open(fname);
	while(fin >> op >> hex >> Address)
	{
		L1->blockAccess(Address, op);
		if(L2_Size != 0)
		{
			if (Inclusion == "non-inclusive")							// NON-INCLUSIVE NON_EXCLUSIVE (NINE) POLICY
			{
				if(L1->getWriteBack() == true)							// If Access to L1 led to a 'MISS' & eviction of a Cacheline & evicted Cacheline is "Dirty". i.e. needed to be written to Lower Level(L2) Cache
					L2->blockAccess(L1->getEvictedAddress(), 'w');				// Access L2 Cache with evicted Address & It will be written to L2 Cache
				if(L1->getIsHit() == false)						
					L2->blockAccess(Address, 'r');						// If Access to L1 was a 'MISS' only, need to read 'Address' from L2 Cache
			}		

			if(Inclusion == "inclusive")								// INCLUSIVE POLICY
			{
				if(L1->getWriteBack() == true)							// If Access to L1 led to a'MISS' & eviction of a Cacheline & evicted Cacheline is "Dirty". i.e. needed to be written to Lower Level(L2) Cache
				{
					L2->blockAccess(L1->getEvictedAddress(), 'w');				// Access L2 Cache with evicted Address & It will be written to L2 Cache
					if(L2->getEvicted() == true)	
						L1->invalidateCacheline(L2->getEvictedAddress());		// If Access to L2 led to an eviction of a Cacheline, the evicted AAddresss needs to be 'Back Invalidated' from L1 Cache(since Inclusive Policy)
				}			
				if(L1->getIsHit() == false)							// If Access to L1 was a 'MISS' only, need to read 'Address' from L2 Cache
				{
					L2->blockAccess(Address, 'r');
					if(L2->getEvicted() == true)
						L1->invalidateCacheline(L2->getEvictedAddress());		// If Access to L2 led to an eviction of a Cacheline, the evicted AAddresss needs to be 'Back Invalidated' from L1 Cache(since Inclusive Policy)	
				}	
			}

			if(Inclusion == "exclusive")								// EXCLUSIVE POLICY
			{
				if(L1->getIsHit() == true)
					L2->invalidateCacheline(Address);					// If Access is 'HIT' in L1 Cache, need to make sure L2 doesn't have same Cacheline. Hence 'Invalidate' L2 Cache
				if(L1->getEvicted() == true)							// If Access to L1 led to a'MISS' & eviction of a Cacheline & evicted Cacheline is "Dirty". i.e. needed to be written to Lower Level(L2) Cache
				{
					L2->blockAccess(L1->getEvictedAddress(), 'w');				// Access L2 Cache with evicted Address & It will be written to L2 Cache	
					if(L1->getWriteBack() == false)						// Flag of Cacheline in L2 accessed with 'Evicted Address' from L1, should be same as 'Flag' of 'Evicted Address' that was in L1 
						L2->findBlock(L1->getEvictedAddress())->setFlag("Valid");	// Hence, if 'Flag' of 'Evicted Address' was not 'DIRTY' i.e. 'getWriteBack' was set to 'FALSE', the 'Flag' in L2 must be updated to 'VALID' instead of 'DIRTY' as would be set in L2 since 'Op' used is 'w'
				}
				if(L1->getIsHit() == false)							// If Access to L1 was a 'MISS' only, need to read 'Address' from L2 Cache
				{
					non_exclusive_cache_access = false;					// Set as false because, don't want to replace any Cacheline in L2 Cache in case of a 'MISS' with 'Address'
					L2->blockAccess(Address, 'r');
			
					non_exclusive_cache_access = true;
					if(L2->getIsHit() == true)						// If L2 Cache access was a'HIT' after access to L1 Cache was 'MISS', The 'FLAG' in L1 Cache should be same as was in L2 Cache
					{
						if(L2->findBlock(Address)->getFlag() == "Dirty")		// Since L2 was accessed with 'OP = r', the 'FLAG' set would be 'VALID'. 
							L1->findBlock(Address)->setFlag("Dirty");		// L1 Cache need to be set as 'DIRTY' if the 'FLAG' in L2 Cache was 'DIRTY'
						L2->invalidateCacheline(Address);				// Address can be present in either L1 or L2 Cache. So  need to 'Invalidate' Cacheline from L2
					}
				}
				
			}
		}
	}

	fin.close();
	
	print_config(&Blocksize, &L1_Size, &L1_Assoc, &L2_Size, &L2_Assoc, &Replacement_Policy, &Inclusion, fname);
	print_result(L1);

	return 1;
}

void print_config(ll *Blocksize, ll *L1_Size, ll *L1_Assoc, ll *L2_Size, ll *L2_Assoc, string *Replacement_Policy, string *Inclusion, char *fname)
{
	cout << "===== Simulator configuration =====" << endl;
	cout << "BLOCKSIZE:              " << *Blocksize << endl;
	cout << "L1_Size:                " << *L1_Size << endl;
	cout << "L1_Assoc:               " << *L1_Assoc << endl;
	cout << "L2_Size:                " << *L2_Size << endl;
	cout << "L2_Assoc:               " << *L2_Assoc << endl;
	cout << "REPLACEMENT POLICY:     " << *Replacement_Policy << endl;
	cout << "INCLUSION PROPERTY:     " << *Inclusion << endl;
	cout << "trace_file:             " << fname << endl;
}

void print_result(Cache *L1)
{
	cout << "===== Simulation Results (raw) =====" << endl;
	cout << "a. number of L1 reads:             " << L1->getReads() << endl;
	cout << "b. number of L1 read misses:       " << L1->getRdMiss() << endl;
	cout << "c. number of L1 writes:            " << L1->getWrites() << endl;
	cout << "d. number of L1 write misses:      " << L1->getWtMiss() << endl;
	cout << "e. L1 miss rate:                   " << fixed << setprecision(6) << (float)(((float)(L1->getRdMiss() + L1->getWtMiss()))/(L1->getReads() + L1->getWrites())) << endl;
	cout << "f. number of L1 writebacks:        " << L1->getWtBacks() << endl;
	
	Cache *L2 = L1->get_L2Cache();
	
	if(L2 == NULL)
	{	
		cout << "g. number of L2 reads:             " << 0 << endl;
		cout << "h. number of L2 read misses:       " << 0 << endl;
		cout << "i. number of L2 writes:            " << 0 << endl;
		cout << "j. number of L2 write misses:      " << 0 << endl;
		cout << "k. L2 miss rate:                   " << 0 << endl;
		cout << "l. number of L2 writebacks:        " << 0 << endl;
		cout << "m. total memory traffic:           " << L1->getMem_Traffic() << endl;
	}
	else
	{	
		cout << "g. number of L2 reads:             " << L2->getReads() << endl;                   	
		cout << "h. number of L2 read misses:       " << L2->getRdMiss() << endl;                   
		cout << "i. number of L2 writes:            " << L2->getWrites() << endl;
		cout << "j. number of L2 write misses:      " << L2->getWtMiss() << endl;
		cout << "k. L2 miss rate:                   " << fixed << setprecision(6) << (float)(((float)(L2->getRdMiss() + L2->getWtMiss()))/(L2->getReads() + L2->getWrites())) << endl;
		cout << "l. number of L2 writebacks:        " << L2->getWtBacks() << endl;
		cout << "m. total memory traffic:           " << L1->getbackInvalidation() + L2->getMem_Traffic() << endl;
	}
}


