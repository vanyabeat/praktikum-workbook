#pragma once

#include <map>
#include <mutex>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value> class ConcurrentMap
{
  public:
	using LockBucket = std::pair<std::mutex, std::map<Key, Value>>;
	using BundleBuckets = std::vector<LockBucket>;
	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

	struct Access
	{
		std::lock_guard<std::mutex> guard_;
		Value& ref_to_value_;

		Access(const Key& key, LockBucket& bucket) : guard_(bucket.first), ref_to_value_(bucket.second[key])
		{
		}
	};

	explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count)
	{
	}

	Access operator[](const Key& key)
	{
		return {key, GetBucket(key)};
	}

	void erase(const Key& key)
	{
		LockBucket& bucket = GetBucket(key);
		std::lock_guard guard(bucket.first);
		bucket.second.erase(key);
	}

	std::map<Key, Value> BuildOrdinaryMap()
	{
		std::map<Key, Value> result;
		for (auto& [mutex, map] : buckets_)
		{
			std::lock_guard g(mutex);
			result.insert(map.begin(), map.end());
		}
		return result;
	}

  private:
	BundleBuckets buckets_;

	LockBucket& GetBucket(const Key& key)
	{
		return buckets_[static_cast<uint64_t>(key) % buckets_.size()];
	}
};