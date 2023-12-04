#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <set>

const int ARR_SIZE = 1 << 25;
const ulong REPETITIONS = 1 << 24;

const int MIN_STRIDE = 512;
const int MAX_STRIDE = 32768;
const ulong MIN_CACHE_SIZE = 1 << 14;
const ulong MAX_CACHE_SIZE = 1 << 16;
const int MAX_ASSOCIATIVITY = 64;
const double ASSOCIATIVITY_THRESHOLD = 1.25;

const int MIN_CACHE_LINE_POW = 3;
const int MAX_CACHE_LINE_POW = 9;
const double CACHE_LINE_THRESHOLD = 1.15;

uint trash = 0;

uint arr[ARR_SIZE] alignas(8192);
std::mt19937 generator = std::mt19937(std::random_device()());

void generateArray(uint stride, int spots) {
    uint actualStride = stride / sizeof(uint);
    std::vector <uint> reverse(spots, 0);

    arr[0] = 0;

    for (int cur = 1; cur < spots; cur++) {
        uint next = generator() % cur;
        uint prev = reverse[next];

        arr[cur * actualStride] = next * actualStride;
        arr[prev * actualStride] = cur * actualStride;
        reverse[next] = cur;
        reverse[cur] = prev;
    }
}

double measureAvgArrayTraversalTime(int spots) {
    long long sum = 0;
    trash = 0;

    uint idx = 0;
    for (int i = 0; i < spots; i++) {
        trash += idx;
        idx = arr[idx];
    }

    idx = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (ulong i = 0; i < REPETITIONS; i++) {
        trash += idx;
        idx = arr[idx];
    }
    auto end = std::chrono::high_resolution_clock::now();
    sum += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    return double(sum) / REPETITIONS;
}

std::pair <ulong, int> getAssociativityAndCacheSize() {
    std::unordered_map <ulong, std::set<int>> sizeJumps;
    std::unordered_map <ulong, int> sizeAssociativity;

    for (uint stride = MIN_STRIDE; stride <= MAX_STRIDE; stride *= 2) {
        double prevTime = -1;
        for (int spots = std::max(MIN_CACHE_SIZE / stride / 2, 2ul); spots <= MAX_ASSOCIATIVITY; spots *= 2) {
            if (spots * (stride / sizeof(uint)) >= ARR_SIZE || spots / 2 * stride > MAX_CACHE_SIZE) break;

            generateArray(stride, spots);

            double curTime = measureAvgArrayTraversalTime(spots);
            double ratio = curTime / prevTime;
            int associativity = spots / 2;
            ulong size = stride * associativity;
            prevTime = curTime;

            if (ratio > ASSOCIATIVITY_THRESHOLD && size >= MIN_CACHE_SIZE && size <= MAX_CACHE_SIZE) {
                sizeJumps[size].insert(associativity);

                if (sizeJumps[size / 2].find(associativity) != sizeJumps[size / 2].end()) {
                    sizeAssociativity[size / 2] = associativity;
                }
            }
        }
    }

    int maxCnt = -1;
    ulong size = -1;
    int associativity = -1;
    for (auto sj : sizeJumps) {
        ulong currentSize = sj.first;
        int currentJumpNum = sj.second.size();

        if (maxCnt < currentJumpNum || maxCnt == currentJumpNum && currentSize < size) {
            maxCnt = currentJumpNum;
            size = currentSize;
        }
    }

    if (sizeAssociativity[size] != 0) {
        associativity = sizeAssociativity[size];
    } else {
        associativity = *sizeJumps[size].begin();
    }

    return {size, associativity };
}

uint getCacheLineArrIdx(uint id, int assoc, uint cacheLineSize, ulong setElStride, ulong setStride) {
    uint lineElId = id % cacheLineSize;
    uint left = (id - lineElId) / cacheLineSize;
    uint setElId = left % assoc;
    uint setId = left / assoc;

    return lineElId + (setElId + setId * assoc) * setElStride + setId * setStride;
}

void generateArrayCacheLine(int assoc, uint setNum, uint cacheLineSize, ulong cacheSetStride, ulong cacheSetElStride) {
    uint lineSize = cacheLineSize / sizeof(uint);
    ulong setElStride = cacheSetElStride / sizeof(uint);
    ulong setStride = cacheSetStride / sizeof(uint);

    std::vector <uint> reverse(setNum * assoc * lineSize, 0);

    arr[0] = 0;

    for (int setId = 0; setId < setNum; setId++) {
        for (int setElId = 0; setElId < assoc; setElId++) {
            for (int lineElId = 0; lineElId < lineSize; lineElId++) {
                if (setId == 0 && setElId == 0 && lineElId == 0) continue;

                uint cur = lineElId + setElId * lineSize + setId * lineSize * assoc;
                uint next = generator() % cur;
                uint prev = reverse[next];

                uint curIdx = getCacheLineArrIdx(cur, assoc, lineSize, setElStride, setStride);
                uint nextIdx = getCacheLineArrIdx(next, assoc, lineSize, setElStride, setStride);
                uint prevIdx = getCacheLineArrIdx(prev, assoc, lineSize, setElStride, setStride);

                arr[curIdx] = nextIdx;
                arr[prevIdx] = curIdx;
                reverse[next] = cur;
                reverse[cur] = prev;
            }
        }
    }
}

ulong getCacheLineSize(int assoc, ulong stride) {
    double prevTime = 1e10;
    for (int cacheLinePow = MIN_CACHE_LINE_POW; cacheLinePow <= MAX_CACHE_LINE_POW; cacheLinePow++) {
        uint cacheLine = 1 << cacheLinePow;
        uint setNum = stride / cacheLine;
        int spots = setNum * assoc * cacheLine / sizeof(uint);

        generateArrayCacheLine(assoc, setNum, cacheLine, cacheLine, stride);

        double curTime = measureAvgArrayTraversalTime(spots);

        double ratio = prevTime / curTime;
        prevTime = curTime;

        if (ratio <= CACHE_LINE_THRESHOLD) {
            return cacheLine / 2;
        }
    }

    return 1 << MAX_CACHE_LINE_POW;
}

int main() {
    auto [size, assoc] = getAssociativityAndCacheSize();

    std::cout << "L1 cache size: " << size << std::endl;
    std::cout << "L1 cache associativity: " << assoc << std::endl;
    std::cout << "Cache line size: " << getCacheLineSize(assoc, size / assoc) << std::endl;

    return 0;
}
