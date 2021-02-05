#pragma once
#include <map>
#include <mutex>
#include <vector>
using namespace std;
template <typename Key, typename Value> class ConcurrentMap
{
  public:
	using LockBucket = std::pair<std::mutex, std::map<Key, Value>>;
	using BundleBuckets = std::vector<LockBucket>;

	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

	struct Access
	{
		Value& ref_to_value;
		std::mutex& access_;

		~Access()
		{
			access_.unlock();
		}
		// ...
	};

	explicit ConcurrentMap(size_t bucket_count)
	{
		buckets_ = BundleBuckets(bucket_count);
	}

	Access operator[](const Key& key)
	{
		// нашли индекс бакета!
		size_t index = key % buckets_.size();
		buckets_[index].first.lock();
		// делаем такой трюк как в прошлых заданиях типо изи быстренько создаем по ключу дефолтный обхекст
		if (buckets_[index].second.count(key) == 0)
		{
			buckets_[index].second.insert({key, Value()});
		}
		return Access{buckets_[index].second[key], buckets_[index].first};
	}

	std::map<Key, Value> BuildOrdinaryMap()
	{

		std::map<Key, Value> result;

		for (size_t i = 0; i < buckets_.size(); ++i)
		{
			buckets_[i].first.lock();

			result.insert(buckets_[i].second.begin(), buckets_[i].second.end());

			buckets_[i].first.unlock();
		}

		return result;
	}

	void erase(Key key)
	{
		size_t index = key % buckets_.size();
		buckets_[index].first.lock();
		if (buckets_[index].second.count(key) != 0)
		{
			auto it = buckets_[index].second.find(key);
			buckets_[index].second.erase(it);
		}
		buckets_[index].first.unlock();
	}

  private:
	BundleBuckets buckets_;
};