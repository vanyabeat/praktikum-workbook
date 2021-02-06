#pragma once
#include <map>
#include <mutex>
#include <vector>
using namespace std;	/// не допустимо в h-файле так подлючать пространство имен, уже делал такое заемчание
template <typename Key, typename Value> class ConcurrentMap
{
  public:
	using LockBucket = std::pair<std::mutex, std::map<Key, Value>>;
	using BundleBuckets = std::vector<LockBucket>;

	static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

	struct Access
	{
		Value& ref_to_value;	/// соблюдайте правило именования полей либо а подчеркиванием либо нет
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
/// этот трюк несколько лишний, при доступе в мапу по оператору[], то объект создается самостоятельно
		if (buckets_[index].second.count(key) == 0)
		{
			buckets_[index].second.insert({key, Value()});
		}
		return Access{buckets_[index].second[key], buckets_[index].first};
	}

	std::map<Key, Value> BuildOrdinaryMap()
	{

		std::map<Key, Value> result;

		for (size_t i = 0; i < buckets_.size(); ++i)	/// индекс не используется отдельно от доступа к элементам, лучше использовать вариант for(auto &backet:backes)
		{
/// посмотрите std::lock_guard, лучше его использовать, так как если между локировкаой и разлокировкаой будет исключение, то мьютекс останется заблокированным
			buckets_[i].first.lock();

			result.insert(buckets_[i].second.begin(), buckets_[i].second.end());

			buckets_[i].first.unlock();
		}

		return result;
	}

	void erase(const Key& key)
	{
		LockBucket& bucket = GetBucket(key);
		bucket.first.lock();		/// std::lock_guard
		bucket.second.erase(key);
		bucket.first.unlock();
	}

  private:
	BundleBuckets buckets_;
	LockBucket& GetBucket(const Key& key)
	{
		return buckets_[static_cast<uint64_t>(key) % buckets_.size()];
	}
};