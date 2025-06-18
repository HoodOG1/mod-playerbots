// ShardedLRUCache.hpp
#pragma once
#include "LRUCache11.hpp"
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <shared_mutex>

template<class Key, class Value, size_t NumShards = 8>
class ShardedLRUCache
{
    using CacheType = lru11::Cache<Key, Value, std::shared_mutex>;
    std::vector<std::unique_ptr<CacheType>> shards;

    size_t getShardIndex(const Key& key) const {
        return std::hash<Key>{}(key) % NumShards;
    }

public:
    ShardedLRUCache(size_t maxSizePerShard, size_t elasticityPerShard)
    {
        for (size_t i = 0; i < NumShards; ++i)
            shards.emplace_back(std::make_unique<CacheType>(maxSizePerShard, elasticityPerShard));
    }

    bool tryGet(const Key& key, Value& value) {
        return shards[getShardIndex(key)]->tryGet(key, value);
    }

    void insert(const Key& key, Value value) {
        shards[getShardIndex(key)]->insert(key, value);
    }

    void clear() {
        for (auto& shard : shards)
            shard->clear();
    }

    template<typename F>
    void cwalk(F& f) {
        for (auto& shard : shards)
            shard->cwalk(f);
    }
};