#ifndef eventH
#define eventH

#include "../global_string.h"
#include "../container/vectorList.h"
#include <functional>

namespace DeltaWorks
{
	/**
	@brief Generalized event object

	An event executes all registered listeners if triggered. A listener can be either a simple function or method, with or without pointer parameter pointing to the domain object that caused the  event.
	*/
	template <class DomainClass>
		class Event
		{
		public:
			class Object
			{};

			class Callback	//! Listener container
			{
			public:
				typedef std::function<void()>	FBasic;
				typedef std::function<void(DomainClass*)> FExtended;

				/**/				Callback() {}
				/**/				Callback(const FBasic&basic):basicFunction(basic)	{}
				/**/				Callback(const FExtended&ext):extendedFunction(ext)	{}
				void				Execute(DomainClass*cl)	const
				{
					if (basicFunction)
						basicFunction();
					if (extendedFunction)
						extendedFunction(cl);
				}
				bool				IsSet()	const
									{
										return basicFunction || extendedFunction;
									}
				void				swap(Callback&other)
				{
					basicFunction.swap(other,basicFunction);
					extendedFunction.swap(other.extendedFunction);
				}
				friend void			swap(Callback&a, Callback&b)
				{
					a.swap(b);
				}
			private:
				FBasic				basicFunction;
				FExtended			extendedFunction;
			};
				
	
		protected:
			Ctr::Vector<Callback>	callbacks;
		public:
			void					operator+=(const Callback&callback)
			{
				if (!callback.isset())
					return;
				callbacks << callback;
			}
			void					operator()(DomainClass*cl)	const
			{
				for (index_t i = 0; i < callbacks.Count(); i++)
					callbacks[i].Execute(cl);
			}
			void					Clear()
			{
				callbacks.Clear();
			}
		};

	class FunctionalEvent
	{
	public:
		typedef std::function<void()>	Callback;
		void			operator+=(const Callback&callback)
		{
			callbacks << callback;
		}
		void			operator()()	const
		{
			for (index_t i = 0; i < callbacks.Count(); i++)
				callbacks[i]();
		}
		void			Clear()		{callbacks.Clear();}

	private:
		Ctr::Vector<Callback,Swap>		callbacks;
	};
}

#endif
