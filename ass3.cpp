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
    int lru_count;

    Block() : tag(0), dirty(false), lru_count(0) {}
};

class Cache {
public:
    Cache(int blockSize, int size, int assoc)
        : blockSize(blockSize), size(size), assoc(assoc) {
        numSets = size / (blockSize * assoc);
        sets.resize(numSets, vector<Block>(assoc));
    }

    bool access(unsigned long addr, bool isWrite) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize * numSets);

        for (int i = 0; i < assoc; ++i) {
            if (sets[index][i].tag == tag) {
                if (isWrite) {
                    sets[index][i].dirty = true;
                }
                updateLRU(index, i);
                return true;
            }
        }

        return false;
    }

    bool insert(unsigned long addr, bool isWrite, bool &evicted, bool &dirty) {
        unsigned long index = (addr / blockSize) % numSets;
        unsigned long tag = addr / (blockSize * numSets);
        int lru = getLRU(index);

        evicted = (sets[index][lru].tag != 0);
        dirty = sets[index][lru].dirty;
        sets[index][lru].tag = tag;
        sets[index][lru].dirty = isWrite;

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
                max_lru = sets[index][i].lru_count;
                lru_index = i;
            }
        }

        return lru_index;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 6) {
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
        bool l1_hit = l1.access(addr, isWrite);

        if (l1_hit) {
            total_time += 1; // L1 hit
        } else {
            bool l2_hit = l2.access(addr, isWrite);

            if (l2_hit) {
                total_time += 20; // L2 hit
            } else {
                total_time += 200; // DRAM access
                l2_read_misses++;
            }

            if (isWrite) {
                l1_write_misses++;
            } else {
                l1_read_misses++;
            }

            bool evicted, dirty;
            l1.insert(addr, isWrite, evicted, dirty);

            if (evicted && dirty) {
                l1_writebacks++;
            }

            if (!l2_hit) {
                l2.insert(addr, isWrite, evicted, dirty);

                if (evicted && dirty) {
                    l2_writebacks++;
                }
            }
        }

        if (isWrite) {
            l1_writes++;
            l2_writes++;
        } else {
            l1_reads++;
            l2_reads++;
        }
    }

    trace.close();

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