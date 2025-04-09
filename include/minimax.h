#ifndef MINIMAX_H
#define MINIMAX_H

#include "board.h"
#include "threadpool.h"
#include "move.h"  // Assumed header for Move and MoveUndo types.
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>
#include <atomic>

// Used to mark whether a transposition table entry is exact or a bound.
enum class BoundType { EXACT, LOWER, UPPER };

struct TTEntry {
    int score;
    int depth;
    bool isMaxNode;
    BoundType flag;
};

class ShardedTranspositionTable {
public:
    ShardedTranspositionTable(size_t numShards = 64) : shards(numShards), hits(0), misses(0) {}

    bool lookup(uint64_t hash, TTEntry& entry) {
        auto& shard = getShard(hash);
        std::lock_guard<std::mutex> lock(shard.mutex);
        auto it = shard.table.find(hash);
        if (it != shard.table.end()) {
            entry = it->second;
            ++hits;
            return true;
        }
        ++misses;
        return false;
    }

    void insert(uint64_t hash, const TTEntry& entry) {
        auto& shard = getShard(hash);
        std::lock_guard<std::mutex> lock(shard.mutex);
        auto& table = shard.table;
        auto it = table.find(hash);
        if (it == table.end() || entry.depth >= it->second.depth) {
            table[hash] = entry;
        }
    }

    void clear() {
        for (auto& shard : shards) {
            std::lock_guard<std::mutex> lock(shard.mutex);
            shard.table.clear();
        }
        hits = 0;
        misses = 0;
    }

    size_t getHits() const { return hits; }
    size_t getMisses() const { return misses; }

private:
    struct Shard {
        std::unordered_map<uint64_t, TTEntry> table;
        std::mutex mutex;
    };

    std::vector<Shard> shards;
    std::atomic<size_t> hits;
    std::atomic<size_t> misses;

    Shard& getShard(uint64_t hash) {
        return shards[hash % shards.size()];
    }
};

class Minimax {
public:
    // New constructor: besides maxDepth, now specify the move time limit (in seconds)
    // for player 2 and a buffer time (in seconds).
    Minimax(int maxDepth, int timeLimitSeconds, int bufferTimeSeconds);

    Move findBestMove(Board& board, int currentPlayer);
    int minimax(Board& board, int depth, int currentPlayer,
                bool isMaximizing, int alpha, int beta);

    // Optional: accessor for the node count.
    size_t getNodeCount() const { return nodeCount.load(); }

private:
    int maxDepth;
    ShardedTranspositionTable transpositionTable;
    std::atomic<size_t> nodeCount { 0 };

    // New member variables for time management.
    int timeLimitSeconds;    // Overall time limit for player 2’s move.
    int bufferTimeSeconds;   // Buffer time to leave before the time limit.

    // New member for cancellation: set to true once the time expires.
    std::atomic<bool> stopSearch { false };
};

#endif // MINIMAX_H