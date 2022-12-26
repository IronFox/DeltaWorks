#ifndef observableH
#define observableH

#include "../container/vectorList.h"
#include "../container/hashtable.h"
#include <functional>


namespace DeltaWorks
{
	/**
	General-purpose observer registration to be triggered on specific events.
	The class is generally not thread-safe.
	Usage: Observable &lt; int &gt; if the event requires an int parameter, or Observable &lt; &gt; if no parameters are required.
	*/
	template <typename...T>
		class Observable
		{
		public:
			typedef std::function<void(T...)>	F;
			typedef Observable<T...>	Self;


			/**/	Observable() {}
			/**/	Observable(const Self&other) = delete;//these aren't inherently bad or impossible but typically imply that something isn't as it should be
			void	operator=(const Self &other) = delete;

			/**
			Drops all observers from the local observable.
			*/
			void	Clear()
			{
				functions.Clear();
				keyMap.Clear();
				dropBuffer.Clear();	//under normal circumstances this would be empty. If this method is invoked during a trigger, any previous keys would be invalid. Better clear
			}
			/**
			Triggers the local observable.
			All registered observers are executed in order of their registration.
			Executed observers may safely alter the local registration during execution, and/or trigger other/this observable recursively.
			Newly appended observers are executed after all previously existing observers have been executed.
			Any observers unregistered during invocation will be removed after invocation has finished.

			If Clear() is called during invocation, then iteration usually stops, even if new observers are subsequently registered.
			Execution or non-execution are not guaranteed in this case, however.
			*/
			void	operator()(T...p)
			{
				iterating ++;
				for (index_t i = 0; i < functions.Count(); i++)
					functions[i].function(p...);
				iterating --;
				if (iterating == 0)
				{
					foreach(dropBuffer, key)
						Unregister(*key);
					dropBuffer.Clear();
				}
			}
			/**
			Appends the specified observer function to the local registration
			@param f Observer to append. The same observer may be appended multiple times, causing it to be invoked multiple times on trigger
			*/
			void	operator+=(const F&f)
			{
				functions.Append().function =f;
			}
			/**
			Appends the specified observer function to the local registration, 
			and returns a reference to this for further appends.
			@param f Observer to append. The same observer may be appended multiple times, causing it to be invoked multiple times on trigger
			@return *this
			*/
			Observable&	operator<<(const F&f)
			{
				functions.Append().function =f;
				return *this;
			}

			/**
			Checks if an observer using the specified key is currently registered
			@param key Observer key to look for. See Register() for details
			*/
			bool		IsRegistered(index_t key) const
			{
				return keyMap.IsSet(key);
			}

			/**
			Appends the specified observer function to  the local registration,
			and returns a unique key to it, so that it can be removed individually later.
			Keys iterate across the local register size (32bit, 64bit), and are not reused
			until the iteration value space wraps back to 0.
			The method never returns InvalidIndex or a key to a currently registered observer.
			@param f Observer to append. The same observer may be appended multiple times, causing it to be invoked multiple times on trigger
			@return Unique key to the registered observer. 
					Never InvalidIndex, or the key of a currently registered observer.
			*/
			index_t		Register(const F&f)
			{
				index_t at = functions.Count();
				TFunction&func = functions.Append();
				func.function = f;
				keyCounter++;
				while (keyCounter == InvalidIndex || keyMap.IsSet(keyCounter))
					keyCounter++;
				func.key = keyCounter;
				keyMap.Set(keyCounter,at);
				return keyCounter;
			}

			/**
			Simple but common shortcut for unsetting a key and directly setting it to InvalidIndex.
			Has no effect if the key already is invalid
			*/
			void		UnregisterAndSetInvalid(index_t&key)
			{
				Unregister(key);
				key = InvalidIndex;
			}

			/**
			Unregisters the observer associated with the specified key from the local observable.
			If the specified key is not currently registered, the method will fail silently.
			@param key Unique identifier of the observer to unregister. See Register() for details
			*/
			void		Unregister(index_t key)
			{
				if (key == InvalidIndex)
					return;
				if (iterating)
				{
					dropBuffer << key;
					return;
				}
				index_t at;
				if (!keyMap.QueryAndUnset(key,at))
					return;
				keyMap.VisitAllValues([at](index_t&v)
				{
					DBG_ASSERT__(v != at);
					if (v > at)
						v--;
				});
				DBG_ASSERT_EQUAL__(functions[at].key,key);
				functions.Erase(at);
			}

			/**
			Checks if the local observable is empty (has no observers registered)
			*/
			bool		IsEmpty() const { return functions.IsEmpty(); }
			/**
			Checks if the local observable has any observers registered
			*/
			bool		IsNotEmpty() const { return functions.IsNotEmpty(); }

			void		swap(Self&other)
			{
				dropBuffer.swap(other.dropBuffer);
				swp(iterating, other.iterating);
				functions.swap(other.functions);
				keyMap.swap(other.keyMap);
			}

		private:
			struct TFunction
			{
				F		function;
				index_t	key = InvalidIndex;

				void	swap(TFunction&other)
				{
					function.swap(other.function);
					swp(key,other.key);
				}
			};

			Ctr::Vector<index_t>		dropBuffer;
			index_t						iterating = 0;
			Ctr::Vector<TFunction,Swap>functions;
			Ctr::IndexTable<index_t>	keyMap;
			index_t						keyCounter = 0;
		};
}

#endif
