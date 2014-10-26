/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2014 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#include <iostream>
#include <fstream>
#include "pin.H"
#include <stdlib.h>
#include <map>
#include <list>
ofstream OutFile;


static UINT64 staticCount = 0;


static UINT64 icount = 0;
static UINT64 intcount = 0;
static UINT64 fpcount = 0;
static UINT64 blockSizeSum = 0;
static UINT64 totalBlocks = 0;
static UINT64 tmpBlockSize = 0;
static UINT64 loadCount = 0;
static UINT64 storeCount = 0;
static UINT64 branchTaken = 0;
static UINT64 branchCount = 0;
static UINT64 forwardBranchCount = 0;
static UINT64 forwardBranchTaken = 0;
std::map<string, UINT64> readDist;
std::map<string, UINT64> writeDist;

static UINT32 RAW0_2 = 0;
static UINT32 RAW3_8 = 0;
static UINT32 RAW9_32 = 0;
static UINT32 RAW33on = 0;
static UINT32 WAW0_2 = 0;
static UINT32 WAW3_8 = 0;
static UINT32 WAW9_32 = 0;
static UINT32 WAW33on = 0;
static UINT32 WAR0_2 = 0;
static UINT32 WAR3_8 = 0;
static UINT32 WAR9_32 = 0;
static UINT32 WAR33on = 0;

static UINT32 byte_0_2 = 0;
static UINT32 byte_3_8 = 0;
static UINT32 byte_9_16 = 0;
static UINT32 byte_17_32 = 0;
static UINT32 byte_33on = 0;

static UINT32 cacheline_0_2 = 0;
static UINT32 cacheline_3_8 = 0;
static UINT32 cacheline_9_16 = 0;
static UINT32 cacheline_17_32 = 0;
static UINT32 cacheline_33on = 0;

static UINT32 page_0_2 = 0;
static UINT32 page_3_8 = 0;
static UINT32 page_9_16 = 0;
static UINT32 page_17_32 = 0;
static UINT32 page_33on = 0;

std::map<ADDRINT, std::list<ADDRINT> > temporal_bytes;
std::map<ADDRINT, std::list<ADDRINT> > temporal_cachelines;
std::map<ADDRINT, std::list<ADDRINT> > temporal_pages;


VOID handleTemporal(ADDRINT ip){
	//this code is basically repeated 3x to handle bytes, cachelines, and pages. 

	ADDRINT cacheline = ip >> 5;
	ADDRINT page = ip >> 12;

	std::map<ADDRINT, std::list<ADDRINT> >::iterator bit;
	std::map<ADDRINT, std::list<ADDRINT> >::iterator cit;
	std::map<ADDRINT, std::list<ADDRINT> >::iterator pit;

	bit = temporal_bytes.find(ip);
	cit = temporal_cachelines.find(cacheline);
	pit = temporal_pages.find(page);

	if (bit != temporal_bytes.end()) {
		//found it!
		//put in correct bucket
		int numReferences = temporal_bytes[ip].size();
		if (numReferences > 32) {
			byte_33on++;
		} else if (numReferences > 16) {
			byte_17_32++;
		} else if (numReferences > 8) {
			byte_9_16++;
		} else if (numReferences > 2) {
			byte_3_8++;
		} else { 
			byte_0_2++;		
		}
		//delete list contents for address
		(temporal_bytes[ip]).clear();
		//add to other lists
	} else {
		//new reference
		std::list<ADDRINT> tmpList;
		temporal_bytes[ip] = tmpList;
	}
	for (bit = temporal_bytes.begin(); bit != temporal_bytes.end(); bit++ ) {
		(bit->second).push_front(ip); //add new value
		bit->second.unique();   //make sure it's not duplicate
		if ((bit->second).size() == 32) {
			//need to remove from list once it has 32 other references
			byte_33on++;
			temporal_bytes.erase(bit);
		}		
	}
	//add to other lists
	
	if (cit != temporal_cachelines.end()) {
		//found it
		//put in correct bucket
		int numReferences = temporal_cachelines[cacheline].size();
		if (numReferences > 32) {
			cacheline_33on++;
		} else if (numReferences > 16) {
			cacheline_17_32++;
		} else if (numReferences > 8) {
			cacheline_9_16++;
		} else if (numReferences > 2) {
			cacheline_3_8++;
		} else { 
			cacheline_0_2++;		
		}
		//delete list contents for address
		(temporal_cachelines[cacheline]).clear();
		//add to other lists
	} else {
		//new reference
		//add to map
		std::list<ADDRINT> tmpList;
		temporal_cachelines[cacheline] = tmpList;
	}
	//add to other lists
	for (cit = temporal_cachelines.begin(); cit != temporal_cachelines.end(); cit++ ) {
		cit->second.push_front(cacheline); //add new value
		cit->second.unique();   //make sure it's not duplicate
		if ((cit->second).size() == 32) {
			//need to remove from list once it has 32 other references
			cacheline_33on++;
			temporal_cachelines.erase(cit);
		}		
	}
	
	if (pit != temporal_pages.end()) {
		//found it
		//put in correct bucket
		int numReferences = temporal_pages[page].size();
		if (numReferences > 32) {
			page_33on++;
		} else if (numReferences > 16) {
			page_17_32++;
		} else if (numReferences > 8) {
			page_9_16++;
		} else if (numReferences > 2) {
			page_3_8++;
		} else { 
			page_0_2++;		
		}
		//delete list contents for address
		(temporal_pages[page]).clear();
		//add to other lists
	} else {
		//new reference
		//add to map
		std::list<ADDRINT> tmpList;
		temporal_pages[page] = tmpList;
	}
	//add to other lists
	for (pit = temporal_pages.begin(); pit != temporal_pages.end(); pit++ ) {
		pit->second.push_front(page); //add new value
		pit->second.unique();   //make sure it's not duplicate
		if ((pit->second).size() == 32) {
			//need to remove from list once it has 32 other references
			page_33on++;
			temporal_pages.erase(pit);
		}		
	}
}

// This function is called before every instruction 
VOID docount(ADDRINT ip, UINT32 k, VOID* readRegs, UINT32 k2, VOID* writeRegs) { 
	//handle the temporal analysis separately
	handleTemporal(ip);	
	
	bool writeFP = false;
	bool readFP = false;	
	bool writeInt = false;
	bool readInt = false;	
	
	REG* readRegisters = (REG*) readRegs;		
	REG* writeRegisters = (REG*) writeRegs;		
	
	std::map<string, UINT64>::iterator rit;
	std::map<string, UINT64>::iterator wit;
	
	for (unsigned i = 0; i < k; ++i ) {
		REG r = readRegisters[i];
		
		
		readFP = (readFP || REG_is_fr(r)); //checks if any of the registers are FP;
		readInt = (readInt || REG_is_gr(r) || REG_is_gr64(r) || REG_is_gr32(r) || REG_is_gr16(r) || REG_is_gr8(r));
		
		string reg = REG_StringShort(readRegisters[i]);
        wit = writeDist.find(reg);
		if (wit != writeDist.end()) {
			//find distance since last write
			UINT64 lastWritten = icount - writeDist[reg];
		    if (lastWritten > 32) {
			    RAW33on++;
		    } else if (lastWritten > 8) {
			    RAW9_32++;
		    } else if (lastWritten > 2) {
			    RAW3_8++;
		    } else {
			    RAW0_2++;
		    }
			writeDist.erase(wit); //erase if it was a write so we don't have to check both lists for the most recent access
		}
		//update most recent access
		readDist[reg] = icount;
	}

	for (unsigned i = 0; i < k2; ++i ) {
		REG r = writeRegisters[i];
		writeFP = (writeFP || REG_is_fr(r)); //checks if any of the registers are FP;
		writeInt = (writeInt || REG_is_gr(r) || REG_is_gr64(r) || REG_is_gr32(r) || REG_is_gr16(r) || REG_is_gr8(r));
		string reg = REG_StringShort(r);
        
		wit = writeDist.find(reg);
		rit = readDist.find(reg);
		if (rit != readDist.end()) {
			//find distance since last read
			UINT64 lastRead = icount - readDist[reg];
		    if (lastRead > 32) {
			    WAR33on++;
		    } else if (lastRead > 8) {
			    WAR9_32++;
		    } else if (lastRead > 2) {
			    WAR3_8++;
		    } else {
			    WAR0_2++;
		    }
			//erase read so that we don't have to check both places for the most recent access
			readDist.erase(rit);
		} else {
			if (wit != writeDist.end()) {
				//find distance since last write 
				UINT64 lastWritten = icount - writeDist[reg];
		    	if (lastWritten > 32) {
			    	WAW33on++;
		        } else if (lastWritten > 8) {
			        WAW9_32++;
		        } else if (lastWritten > 2) {
			        WAW3_8++;
		        } else {
			        WAW0_2++;
		        }

			}

		}
		writeDist[reg] = icount; //update the most recent access
	}
	
	if (writeFP || readFP ) { //if there were any FP registers used
		fpcount++;
	} 
	if (writeInt || readInt ) { //if there were any INT registers used 
		intcount++;
	} 
	icount++; 

}

//called on any memory loads
VOID read(ADDRINT ip, ADDRINT addr, UINT32 k, UINT32 k2)
{

      ++loadCount;
}

//called on any memory stores
VOID write(ADDRINT ip, ADDRINT addr)
{
      ++storeCount;
}

//taken on branch instructions, counts branches and forward branches
VOID branch(ADDRINT ip, ADDRINT target) {
	branchCount++;
	if (target > ip) {
		forwardBranchCount++;
	}
}    
//taken on branches taken,  counts branches taken and forward branches taken
VOID branchTakenFunc(ADDRINT ip, ADDRINT target) {
	branchTaken++;
	if (target > ip) {
		forwardBranchTaken++;
	}
}    

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
	staticCount++; //counts static instructions. Just for curiosity
	//cout << INS_Mnemonic(ins) << endl;	//prints debug file of instructions

	//The next statements are to keep track of the basic block size,
	//as well as calling branch counting functions.
	//It counts the instructions between branch or calls, so that each 
	//basic block is counted. This means that each basic block is only
	//counted once, statically. 
	if (INS_IsBranchOrCall(ins)) {
	 	INS_InsertPredicatedCall(
			ins, IPOINT_BEFORE, (AFUNPTR)branch,
			IARG_INST_PTR,
			IARG_BRANCH_TARGET_ADDR,
			IARG_END);
	 	INS_InsertPredicatedCall(
			ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)branchTakenFunc,
			IARG_INST_PTR,
			IARG_BRANCH_TARGET_ADDR,
			IARG_END);
		tmpBlockSize++;
		blockSizeSum += tmpBlockSize;
		totalBlocks++;
		tmpBlockSize = 0; //because the current instruction is the last instruction of its block

	} else {
		tmpBlockSize++;	
	}

	//Counts reads and writes
	if (INS_IsMemoryRead(ins))
	    {
	 	INS_InsertPredicatedCall(
			ins, IPOINT_BEFORE, (AFUNPTR)read,
			IARG_INST_PTR,
			IARG_MEMORYREAD_EA,
			IARG_END);
	    }

	    if (INS_IsMemoryWrite(ins))
	    {
	 	INS_InsertPredicatedCall(
			ins, IPOINT_BEFORE, (AFUNPTR)write,
			IARG_INST_PTR,
			IARG_MEMORYWRITE_EA, 
			IARG_END);
	    }
	UINT32 k = INS_MaxNumRRegs(ins);
	UINT32 k2 = INS_MaxNumWRegs(ins);
    REG* readRegs =(REG*) malloc(k * sizeof(REG)/sizeof(char));
	for (unsigned i = 0; i < k; ++i ) {
		readRegs[i] = INS_RegR(ins, i);
	}
    REG* writeRegs =(REG*) malloc(k2 * sizeof(REG)/sizeof(char));
	for (unsigned i = 0; i < k2; ++i ) {
		writeRegs[i] = INS_RegW(ins, i);
	}
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_INST_PTR, IARG_UINT32, k, IARG_PTR, (VOID*) readRegs, IARG_UINT32, k2, IARG_PTR, (VOID*) writeRegs, IARG_END);
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "mine.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Dynamic Count " << icount << endl;
//    OutFile << "Static Count " << staticCount << endl;
    OutFile << "Integer Count " << intcount << endl;
    OutFile << "Floating Point Count " << fpcount << endl;
    OutFile << "Loads " << loadCount << endl;
    OutFile << "Stores " << storeCount << endl;
    OutFile << "Branches " << branchCount << endl;
    OutFile << "Percent Branches Taken " << (double) branchTaken/branchCount <<"%"<< endl;
//    OutFile << "Forward Branches " << forwardBranchCount << endl;
    OutFile << "Percent Forward Branches Taken " << (double) forwardBranchTaken/forwardBranchCount<<"%" << endl;
    OutFile << "Basic Block Size " << (double) blockSizeSum/totalBlocks<< endl;
	
	OutFile << "RAW 0-2 " << RAW0_2 << endl;
	OutFile << "RAW 3-8 " << RAW3_8 << endl;
	OutFile << "RAW 9-32 " << RAW9_32 << endl;
	OutFile << "RAW 33+ " << RAW33on << endl;
	
	OutFile << "WAW 0-2 " << WAW0_2 << endl;
	OutFile << "WAW 3-8 " << WAW3_8 << endl;
	OutFile << "WAW 9-32 " << WAW9_32 << endl;
	OutFile << "WAW 33+ " << WAW33on << endl;
	
	OutFile << "WAR 0-2 " << WAR0_2 << endl;
	OutFile << "WAR 3-8 " << WAR3_8 << endl;
	OutFile << "WAR 9-32 " << WAR9_32 << endl;
	OutFile << "WAR 33+ " << WAR33on << endl;
    

	OutFile << "Temporal Density Byte 0-2 " << byte_0_2 << endl;
	OutFile << "Temporal Density Byte 3-8 " << byte_3_8 << endl;
	OutFile << "Temporal Density Byte 9-16 " << byte_9_16 << endl;
	OutFile << "Temporal Density Byte 17-32 " << byte_17_32 << endl;
	OutFile << "Temporal Density Byte 33+ " << byte_33on << endl;
	
	OutFile << "Temporal Density Cacheline 0-2 " << cacheline_0_2 << endl;
	OutFile << "Temporal Density Cacheline 3-8 " << cacheline_3_8 << endl;
	OutFile << "Temporal Density Cacheline 9-16 " << cacheline_9_16 << endl;
	OutFile << "Temporal Density Cacheline 17-32 " << cacheline_17_32 << endl;
	OutFile << "Temporal Density Cacheline 33+ " << cacheline_33on << endl;
	
	OutFile << "Temporal Density Page 0-2 " << page_0_2 << endl;
	OutFile << "Temporal Density Page 3-8 " << page_3_8 << endl;
	OutFile << "Temporal Density Page 9-16 " << page_9_16 << endl;
	OutFile << "Temporal Density Page 17-32 " << page_17_32 << endl;
	OutFile << "Temporal Density Page 33+ " << page_33on << endl;
	OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts lots of things :) " << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
