#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace std;

struct Block {
    unsigned long tag;
    bool dirty;
    bool valid;
    int lru_count;

    Block() : tag(0), dirty(false), lru_count(0), valid(false) {}
};

class Cache {
public:
    Cache(int blockSize, int size, int assoc)
        : blockSize(blockSize), size(size), assoc(assoc) {
        numSets = size / (blockSize * assoc);
        sets.resize(numSets, vector<Block>(assoc));
    }

    bool access(unsigned long addr) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize);

        for (int i = 0; i < assoc; ++i) {
            if (sets[index][i].tag == tag && sets[index][i].valid) {
                updateLRU(index, i);
                return true;
            }
        }

        return false;
    }

    bool accessWithoutLRU(unsigned long addr) {
    unsigned long index = (addr / blockSize) % numSets;
    unsigned long tag = addr / (blockSize);

    for (int i = 0; i < assoc; ++i) {
        if (sets[index][i].tag == tag && sets[index][i].valid) {
            // updateLRU(index, i);
            return true;
        }
    }

    return false;
    }

    void setDirty(unsigned long addr) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize);

        for (int i = 0; i < assoc; ++i) {
            if (sets[index][i].tag == tag && sets[index][i].valid) {
                sets[index][i].dirty = true;
                // cout << "addr: " << addr << " is set dirty" << endl;
                return;
            }
        }
    }

    unsigned long getOld(unsigned long addr) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize);  

        int lru = getLRU(index);
        return sets[index][lru].tag * (blockSize);

    }

    void invalidate(unsigned long addr) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize);

        for (int i = 0; i < assoc; ++i) {
            if (sets[index][i].tag == tag && sets[index][i].valid) {
                sets[index][i].valid = false;
                // cout << "addr: " << addr << " is set dirty" << endl;
                return;
            }
        }
    }

    // unsigned long getOld(unsigned long addr, bool isWrite) {
    //     unsigned long index = (addr / blockSize) % numSets;
    //     unsigned long tag = addr / (blockSize * numSets);

    //     if (isWrite){
    //         // if write, get the lru to where we want to write and return its old address
    //         int lru = getLRU(index);
    //         return sets[index][lru].tag * (blockSize * numSets) + index * blockSize;
    //     }else{
    //         // if read, check all blocks in the set for the tag and return its old address
    //         for (int i = 0; i < assoc; ++i) {
    //             if (sets[index][i].tag == tag && sets[index][i].valid) {
    //                 return sets[index][i].tag * (blockSize * numSets) + index * blockSize;
    //             }
    //         }
    //     }
    // }

    bool checkDirty(unsigned long addr) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize);  

        int lru = getLRU(index);
        return sets[index][lru].dirty && sets[index][lru].valid;

    }

    bool insert(unsigned long addr, bool isWrite, bool &evicted, bool &dirty, unsigned long &oldAddr) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize);
        int lru = getLRU(index);
        evicted = sets[index][lru].valid;
        dirty = sets[index][lru].dirty;
        oldAddr = sets[index][lru].tag * (blockSize);
        sets[index][lru].tag = tag;
        sets[index][lru].dirty = isWrite;
        sets[index][lru].valid = true;
        // cout << "addr: " << addr << "index: " << index  << "oldAddr: " << oldAddr<< endl;

        updateLRU(index, lru);

        return evicted;
    }

private:
    int blockSize;
    int size;
    int assoc;
    int numSets;
    vector<vector<Block>> sets;

    void updateLRU(int index, int accessed) {
        for (int i = 0; i < assoc; ++i) {
            if (i != accessed) {
                sets[index][i].lru_count++;
            } else {
                sets[index][i].lru_count = 0;
            }
        }
    }

    int getLRU(int index) {
        int max_lru = 0;
        int lru_index = 0;

        for (int i = 0; i < assoc; ++i) {
            if (sets[index][i].lru_count > max_lru) {
                //If empty block is found, return it
                // if (sets[index][i].valid == false){
                //     return i;
                // }
                max_lru = sets[index][i].lru_count;
                lru_index = i;
            }
        }

        return lru_index;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 7) {
        cout << "Usage: ./cache_simulate BLOCKSIZE L1_SIZE L1_ASSOC L2_SIZE L2_ASSOC trace_file" << endl;
        return 1;
    }

    int blockSize = stoi(argv[1]);
    int l1_size = stoi(argv[2]);
    int l1_assoc = stoi(argv[3]);
    int l2_size = stoi(argv[4]);
    int l2_assoc = stoi(argv[5]);
    string trace_file = argv[6];

    ifstream trace(trace_file);

    if (!trace.is_open()) {
        cout << "Error opening trace file " << trace_file << endl;
        return 1;
    }

    Cache l1(blockSize, l1_size, l1_assoc);
    Cache l2(blockSize, l2_size, l2_assoc);

    int l1_reads = 0, l1_read_misses = 0, l1_writes = 0, l1_write_misses = 0;
    int l1_writebacks = 0, l2_reads = 0, l2_read_misses = 0, l2_writes = 0;
    int l2_write_misses = 0, l2_writebacks = 0;
    long long total_time = 0;

    char op;
    unsigned long addr;

    while (trace >> op >> hex >> addr) {
        bool isWrite = (op == 'w');
        bool l1_hit = l1.access(addr);

        if (l1_hit) {
            // if write then update dirty bit, increment write and read counts regardless
            if (isWrite){
                l1.setDirty(addr);
                l1_writes++;
            }else{
                l1_reads++;
            }
            total_time += 1; // L1 hit
        } else {
            // update l1 misses count
            if (isWrite) {
                l1_write_misses++;
            } else {
                l1_read_misses++;
            }

            // if it's a dirty bit then write it before doing anything else
            if (l1.checkDirty(addr)) {
                l1_writebacks++;
                // do we add time for this?
                // get the address for which write back is due
                    // in L2, update its value
                l2_writes ++;
                total_time += 20; // L2 access
                l2.setDirty(l1.getOld(addr)); // set dirty bit since l1 writeback to it happens in l2
            }

            // check if the new block req is in l2
            bool l2_hit = l2.access(addr);

            // add time due to miss, update l2 misses count
            if (l2_hit) {
                l2_reads ++;
                total_time += 20; // L2 access
                // continue; //if found in l2 skip rest of the iteration
            } else {
                l2_reads ++;
                total_time += 20; // L2 access
                total_time += 200; // DRAM access
                l2_read_misses++; // since not in L2

                // block not in memory so get it from memory and insert in l2
                bool evicted, dirty;
                unsigned long oldAddr;
                l2.insert(addr, false, evicted, dirty, oldAddr);
                if (evicted && dirty) { // if some value already present in that location
                    bool inL1 = l1.accessWithoutLRU(oldAddr); // check if it's in l1
                    if (inL1){
                        if (l1.checkDirty(oldAddr)){ // if it's dirty then write it back to l2
                            l1_writebacks++;
                            l2_writes++;
                            total_time += 20; // L2 write
                        }
                        l1.invalidate(oldAddr); // invalidate it in l1
                    }
                    l2_writebacks++; // write it back to dram
                    total_time += 200; // write block currently in that location to DRAM
                }
            }
            bool evicted, dirty;
            unsigned long oldAddr;
            l1.insert(addr, isWrite, evicted, dirty, oldAddr);
            if (isWrite) {
                l1.setDirty(addr);
            }

            // increment writes and reads of both l1 and l2
            if (isWrite) {
                l1_writes++;
                total_time += 1; // L1 hit
            } else {
                l1_reads++;
                total_time += 1; // L1 hit
            }
        }
    }

    trace.close();
    cout << "\n\n===== Simulation Results =====" << endl;
    cout << "number of L1 reads: " << l1_reads << endl;
    cout << "number of L1 read misses: " << l1_read_misses << endl;
    cout << "number of L1 writes: " << l1_writes << endl;
    cout << "number of L1 write misses: " << l1_write_misses << endl;
    cout << "L1 miss rate: " << fixed << setprecision(2) << (double)(l1_read_misses + l1_write_misses) / (l1_reads + l1_writes) * 100 << "%" << endl;
    cout << "number of writebacks from L1 memory: " << l1_writebacks << endl;
    cout << "number of L2 reads: " << l2_reads << endl;
    cout << "number of L2 read misses: " << l2_read_misses << endl;
    cout << "number of L2 writes: " << l2_writes << endl;
    cout << "number of L2 write misses: " << l2_write_misses << endl;
    cout << "L2 miss rate: " << fixed << setprecision(2) << (double)(l2_read_misses + l2_write_misses) / (l2_reads + l2_writes) * 100 << "%" << endl;
    cout << "number of writebacks from L2 memory: " << l2_writebacks << endl;
    cout << "total access time: " << total_time << " ns" << endl;
    return 0;
}