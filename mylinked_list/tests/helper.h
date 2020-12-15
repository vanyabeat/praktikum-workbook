#pragma once
#include <iostream>
#include <set>
#include <vector>
#include <map>
template <typename T, typename S> std::ostream& operator<<(std::ostream& os, const std::pair<T, S>& v)
{
	os << v.first << ": " << v.second;
	return os;
}

template <typename C> void container_print(std::ostream& os, const C& cont)
{
	size_t counter = 0;
	size_t size = cont.size();
	for (const auto& item : cont)
	{
		if (counter == (size - 1))
		{
			os << item;
			continue;
		}
		os << item << ", ";
		++counter;
	}
}

template <typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& obj)
{
	os << "[";
	container_print(os, obj);
	os << "]";
	return os;
}

template <typename T> std::ostream& operator<<(std::ostream& os, const std::set<T>& obj)
{
	os << "{";
	container_print(os, obj);
	os << "}";
	return os;
}

template <typename T, typename V> std::ostream& operator<<(std::ostream& os, const std::map<T, V>& obj)
{
	os << "{";
	container_print(os, obj);
	os << "}";
	return os;
}