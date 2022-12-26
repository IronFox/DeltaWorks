#pragma once

#include "hashtable.h"
#include "vectorList.h"

namespace DeltaWorks
{
	namespace Container
	{

		template <typename K, typename T, typename KStrategy=StrategySelector<K>::Default, typename TStrategy=StrategySelector<T>::Default>
			class HashMappedList
			{
			public:
				typedef HashMappedList<K,T,KStrategy,TStrategy>	Self;

				struct TEntry
				{
					K	key;
					T	value;

					void	swap(TEntry&other)
					{
						KStrategy::swap(key,other.key);
						TStrategy::swap(value,other.value);
					}
				};
				typedef Vector<TEntry,Swap>			List;

				typedef typename List::const_iterator	const_iterator;

				const_iterator		begin() const {return list.begin();}
				const_iterator		end() const {return list.end();}
				size_t				size() const {return list.size();}
				count_t				Count() const {return list.Count();}

				void				Set(const K&key, const T&value)
				{
					index_t at = list.Count();
					TEntry& p = list.Append();
					map.Set(key,at);
					p.key = key;
					p.value = value;
				}


				bool				Query(const K&key, T&rsOut)	const
				{
					index_t at;
					if (!map.Query(key,at))
						return false;
					DBG_ASSERT_EQUAL__(list[at].key,key);
					rsOut = list[at].value;
					return true;
				}
				T*					QueryPointer(const K&key)
				{
					index_t at;
					if (!map.Query(key,at))
						return nullptr;
					DBG_ASSERT_EQUAL__(list[at].key,key);
					return &list[at].value;
				}
				const T*					QueryPointer(const K&key) const
				{
					index_t at;
					if (!map.Query(key,at))
						return nullptr;
					DBG_ASSERT_EQUAL__(list[at].key,key);
					return &list[at].value;
				}

				bool				Unset(const K&key)
				{
					index_t at;
					if (!map.QueryAndUnset(key,at))
						return false;
					map.VisitAllValues([at](index_t&entry)
					{
						if (entry > at)
							entry--;
					});
					list.Erase(at);
					return true;
				}

				friend void			swap(Self&a,Self&b)
				{
					a.swap(b);
				}
				void				swap(Self&other)
				{
					map.swap(other.map);
					list.swap(other.list);
				}
				void				Clear()
				{
					map.Clear();
					list.Clear();
				}

	
			private:
				List									list;
				HashTable<K,index_t,StdHash, KStrategy>	map;
			};
	
	}	
	
}
