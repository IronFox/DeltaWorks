#ifndef container_strategyH
#define container_strategyH

#include <cstring>
#include <utility>
#include "../general/typedefs.h"

namespace DeltaWorks
{

	namespace Strategy
	{

		/**
			@brief Base strategy that defines range constructors and destructors
		*/
		class BaseStrategy
		{
		public:
	
			/**
				@brief Calls constructors for the given range
			*/
		template <typename T>
			static	inline	void	constructRange(T*begin, T*end)
			{
				while (begin != end)
				{
					new (begin) T;
					begin++;
				}
			}

			/**
				@brief Calls constructors for the given range
			*/
		template <typename T>
			static	inline	void	constructRange(T*begin, T*end, const T&pattern)
			{
				while (begin != end)
				{
					new (begin) T(pattern);
					begin++;
				}
			}

			/**
				@brief Calls destructors for the given range
			*/
		template <typename T>
			static	inline	void	destructRange(T*begin, T*end)
			{
				while (begin != end)
				{
					begin->~T();
					begin++;
				}
			}

		/**
			@brief Shifts elements in reverse order

			This method is useful to shift data up

			@param shift_from_rbegin Last element of the range, first element that should be shifted
			@param shift_from_rend Element one before the first element of the range, one past the last element that should be shifted
			@param shift_to_rbegin Last element of the target range, first element that should be shifted to
		*/
		template <typename T>
			static	inline	void	reverseShiftRange(T*shift_from_rbegin, T*shift_from_rend, T*shift_to_rbegin)
			{
				if (shift_from_rbegin != shift_from_rend)
					std::move_backward(shift_from_rbegin, shift_from_rend, shift_to_rbegin);
				//while (shift_from_rbegin != shift_from_rend)
				//	(*shift_to_rbegin--) = std::move(*shift_from_rend--);
			}


		/**
			@brief Shifts elements in forward order

			This method is useful to shift data down

			@param shift_from_begin First element of the range, first element that should be shifted
			@param shift_from_end Element one past the last element of the range
			@param shift_to_begin First element of the target range, first element that should be shifted to
		*/
		template <typename T>
			static	inline	void	shiftRange(T*shift_from_begin, T*shift_from_end, T*shift_to_begin)
			{
				if (shift_from_begin != shift_from_end)
					std::move(shift_from_begin, shift_from_end, shift_to_begin);
				//while (shift_from_begin != shift_from_end)
				//	(*shift_to_begin++) = std::move(*shift_from_end++);
			}


		template <typename T0, typename T1>
			inline static	void	copyRange(const T0*from_begin, const T0*from_end, T1*to_begin)
			{
				while (from_begin != from_end)
					(*to_begin++) = (*from_begin++);
			}
		};



		template <>
			inline	void	BaseStrategy::destructRange<int>(int*,int*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<unsigned>(unsigned*,unsigned*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<short>(short*,short*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<unsigned short>(unsigned short*,unsigned short*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<char>(char*,char*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<unsigned char>(unsigned char*,unsigned char*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<long long>(long long*,long long*)
			{}

		template <>
			inline	void	BaseStrategy::destructRange<unsigned long long>(unsigned long long*,unsigned long long*)
			{}


		template <>
			inline	void	BaseStrategy::constructRange<int>(int*,int*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<unsigned>(unsigned*,unsigned*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<short>(short*,short*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<unsigned short>(unsigned short*,unsigned short*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<char>(char*,char*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<unsigned char>(unsigned char*,unsigned char*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<long long>(long long*,long long*)
			{}

		template <>
			inline	void	BaseStrategy::constructRange<unsigned long long>(unsigned long long*,unsigned long long*)
			{}




		/**
			@brief Strategy class plugin (for CommonStrategy<>) to use for traditional and primitive objects, which are initialized and moved via constructor and assignment-operator
		*/
		class CopyStrategyPlugin:public BaseStrategy
		{
		public:
			static	const char*			name;
				/**
					@brief Constructs all elements starting with @a write_begin and ends one before @a write_end . Data to initialize with is taken from @a read_begin , leaving all elements from @a read_begin ready for destructor call

					Elements from @a write_begin have not been constructed and should be initialized using placement new before or during copy/move.

					@param write_begin	First (unconstructed) element to write to
					@param write_end	Element one past the last element of @a write_begin to construct and initialize. May be identical to @a write_begin which means no operation should be performed
					@param read_begin	First element to read from
				 */
			template <typename T>
				inline static	void	constructRangeFromFleetingData(T*write_begin, T*write_end, T*read_begin)
				{
					while (write_begin != write_end)
						new (write_begin++) T(std::move(*read_begin++));
				}

				/**
					@brief Constructs a single element from fleeting (temporary data) that will be destructed afterwards @param target Element to construct @param data Element to read data from. This element should be left in valid but possibly empty state, since it will be erased later
				*/
			template <typename T>
				inline static	void	constructSingleFromFleetingData(T*target, T&data)
				{
					new (target) T(std::move(data));
				}
			template <typename T>
				inline static	void	constructSingleFromFleetingData(T*target, T&&data)
				{
					new (target) T(std::move(data));
				}


				/**
					@brief Copies/moves one element to another

					Both objects have been constructed and should remain constructed

					@param from Element to copy/move from
					@param to Element to copy/move to
				*/
			template <typename T>
				inline static	void	move(T&from, T&to)
				{
					to = std::move(from);
				}
			template <typename T>
				inline static	void	move(T&&from, T&to)
				{
					to = std::move(from);
				}

				/**
					@brief Swaps data of @b u and @b v
				*/
			template <typename T>
				inline static	void	swap(T&u, T&v)
				{
					using std::swap;
					swap(u,v);
				}


			template <typename T0, typename T1>
				inline static	void	moveRange(T0*from_begin, T0*from_end, T1*to_begin)
				{
					if (from_begin != from_end)
						std::move(from_begin, from_end, to_begin);
					//while (from_begin != from_end)
					//	(*to_begin++) = std::move(*from_begin++);
				}

		};




		/**
			@brief Strategy class plugin (for CommonStrategy<>) to use for native/primitive types and POD structs only. The plugin assumes objects do not contain dynamic data and copies binary at times

			Moved fields are copied via memcpy if their type is the same, and assigned via 'a = T_a(b)' otherwise
		*/
		class PrimitiveStrategyPlugin
		{
		private:
				template <typename T0, typename T1>
					class Copy
					{
					public:
							static inline void	execute(const T0*from_begin, const T0*from_end, T1*to_begin)
							{
								while (from_begin != from_end)
									(*to_begin++) = T1(*from_begin++);
							}
					};
				template <typename T>
					class Copy<T,T>
					{
					public:
							static inline void	execute(const T*from_begin, const T*from_end, T*to_begin)
							{
								memcpy(to_begin,from_begin,(char*)(from_end)-(char*)(from_begin));
							}
					};

		public:
				/**
					@brief Calls constructors for the given range
				*/
			template <typename T>
				static	inline	void	constructRange(T*begin, T*end)
				{}

			template <typename T>
				static	inline	void	constructRange(T*begin, T*end, const T&pattern)
				{
					while (begin != end)
					{
						new (begin) T(pattern);
						begin++;
					}
				}

				/**
					@brief Calls destructors for the given range
				*/
			template <typename T>
				static	inline	void	destructRange(T*begin, T*end)
				{}

			static	const char*			name;

				/**
					@brief Constructs all elements starting with @a write_begin and ends one before @a write_end . Data to initialize with is taken from @a read_begin , leaving all elements from @a read_begin ready for destructor call

					Elements from @a write_begin have not been constructed and should be initialized using placement new before or during copy/move.

					@param write_begin	First (unconstructed) element to write to
					@param write_end	Element one past the last element of @a write_begin to construct and initialize. May be identical to @a write_begin which means no operation should be performed
					@param read_begin	First element to read from
				 */
			template <typename T>
				inline static	void	constructRangeFromFleetingData(T*write_begin, T*write_end, T*read_begin)
				{
					while (write_begin != write_end)
						new (write_begin++) T(std::move(*read_begin++));
				}

				/**
					@brief Constructs a single element from fleeting (temporary data) that will be destructed afterwards @param target Element to construct @param data Element to read data from. This element should be left in valid but possibly empty state, since it will be erased later
				*/
			template <typename T>
				inline static	void	constructSingleFromFleetingData(T*target, T&data)
				{
					(*target) = data;
				}


				/**
					@brief Copies/moves one element to another

					Both objects have been constructed and should remain constructed

					@param from Element to copy/move from
					@param to Element to copy/move to
				*/
			template <typename T>
				inline static	void	move(T&from, T&to)
				{
					to = std::move(from);
				}
				/**
					@brief Swaps data of @b u and @b v
				*/
			template <typename T>
				inline static	void	swap(T&u, T&v)
				{
					using std::swap;
					swap(u,v);
				}



				inline static	void	moveRange(void*from_begin, void*from_end, void*to_begin)
				{
					std::memcpy(to_begin,from_begin,(char*)(from_end)-(char*)(from_begin));
				}

			template <typename T0, typename T1>
				inline static	void	copyRange(const T0*from_begin, const T0*from_end, T1*to_begin)
				{
					Copy<T0,T1>::execute(from_begin,from_end,to_begin);
				}
		};



		/**
			@brief Strategy class plugin (for CommonStrategy<>) to use for objects that implement the adoptFrom() method, which are initialized and moved via that method
		*/
		class AdoptStrategyPlugin:public BaseStrategy
		{
		private:
			template <typename T0, typename T1>
				class Move
				{
				public:
						static inline void	execute(T0*from_begin, T0*from_end, T1*to_begin)
						{
							while (from_begin != from_end)
								(*to_begin++) = std::move(*from_begin++);
						}
				};
			template <typename T>
				class Move<T,T>
				{
				public:
						static inline void	execute(T*from_begin, T*from_end, T*to_begin)
						{
							while (from_begin != from_end)
								(*to_begin++).adoptData(*from_begin++);
						}
				};
		public:
			static	const char*			name;

				/**
					@brief Constructs all elements starting with @a write_begin and ends one before @a write_end . Data to initialize with is taken from @a read_begin , leaving all elements from @a read_begin ready for destructor call

					Elements from @a write_begin have not been constructed and should be initialized using placement new before or during copy/move.

					@param write_begin	First (unconstructed) element to write to
					@param write_end	Element one past the last element of @a write_begin to construct and initialize. May be identical to @a write_begin which means no operation should be performed
					@param read_begin	First element to read from
				 */
			template <typename T>
				inline static	void	constructRangeFromFleetingData(T*write_begin, T*write_end, T*read_begin)
				{
					while (write_begin != write_end)
					{
						new (write_begin) T;
						(*write_begin++).adoptData(*read_begin++);
					}
				}

				/**
					@brief Constructs a single element from fleeting (temporary data) that will be destructed afterwards @param target Element to construct @param data Element to read data from. This element should be left in valid but possibly empty state, since it will be erased later
				*/
			template <typename T>
				inline static	void	constructSingleFromFleetingData(T*target, T&data)
				{
					new (target) T;
					target->adoptData(data);
				}



				/**
					@brief Copies/moves one element to another

					Both objects have been constructed and should remain constructed

					@param from Element to copy/move from
					@param to Element to copy/move to
				*/
			template <typename T>
				inline static	void	move(T&from, T&to)
				{
					to.adoptData(from);
				}

				/**
					@brief Swaps data of @b u and @b v
				*/
			template <typename T>
				inline static	void	swap(T&u, T&v)
				{
					T tmp;
					tmp.adoptData(u);
					u.adoptData(v);
					v.adoptData(tmp);
				}


			template <typename T0, typename T1>
				inline static	void	moveRange(T0*from_begin, T0*from_end, T1*to_begin)
				{
					Move<T0,T1>::execute(from_begin,from_end,to_begin);
				}


			/**
				@brief Shifts elements in reverse order

				This method is useful to shift data up

				@param shift_from_rbegin Last element of the range, first element that should be shifted
				@param shift_from_rend Element one before the first element of the range, one past the last element that should be shifted
				@param shift_to_rbegin Last element of the target range, first element that should be shifted to
			*/
			template <typename T>
				static	inline	void	reverseShiftRange(T*shift_from_rbegin, T*shift_from_rend, T*shift_to_rbegin)
				{
					while (shift_from_rbegin != shift_from_rend)
						(*shift_to_rbegin--).adoptData(*shift_from_rend--);
				}


			/**
				@brief Shifts elements in forward order

				This method is useful to shift data down

				@param shift_from_begin First element of the range, first element that should be shifted
				@param shift_from_end Element one past the last element of the range
				@param shift_to_begin First element of the target range, first element that should be shifted to
			*/
			template <typename T>
				static	inline	void	shiftRange(T*shift_from_begin, T*shift_from_end, T*shift_to_begin)
				{
					while (shift_from_begin != shift_from_end)
						(*shift_to_begin++).adoptData(*shift_from_end++);
				}

			template <typename T0, typename T1>
				inline static	void	copyRange(const T0*from_begin, const T0*from_end, T1*to_begin)
				{
					while (from_begin != from_end)
						(*to_begin++) = (*from_begin++);
				}
		};


		/**
			@brief Strategy class plugin (for CommonStrategy<>) to use for objects that implement the swap() method, which swap data via that method
		*/
		class SwapStrategyPlugin:public BaseStrategy
		{
		private:
			template <typename T0, typename T1>
				class Move
				{
				public:
						static inline void	execute(T0*from_begin, T0*from_end, T1*to_begin)
						{
							while (from_begin != from_end)
								(*to_begin++) = std::move(*from_begin++);
						}
				};
			template <typename T>
				class Move<T,T>
				{
				public:
						static inline void	execute(T*from_begin, T*from_end, T*to_begin)
						{
							while (from_begin != from_end)
								(*to_begin++).swap(*from_begin++);
						}
				};
		public:
			static	const char*			name;

				/**
					@brief Constructs all elements starting with @a write_begin and ends one before @a write_end . Data to initialize with is taken from @a read_begin , leaving all elements from @a read_begin ready for destructor call

					Elements from @a write_begin have not been constructed and should be initialized using placement new before or during copy/move.

					@param write_begin	First (unconstructed) element to write to
					@param write_end	Element one past the last element of @a write_begin to construct and initialize. May be identical to @a write_begin which means no operation should be performed
					@param read_begin	First element to read from
				 */
			template <typename T>
				inline static	void	constructRangeFromFleetingData(T*write_begin, T*write_end, T*read_begin)
				{
					while (write_begin != write_end)
					{
						new (write_begin) T;
						(*write_begin++).swap(*read_begin++);
					}
				}

				/**
					@brief Constructs a single element from fleeting (temporary data) that will be destructed afterwards @param target Element to construct @param data Element to read data from. This element should be left in valid but possibly empty state, since it will be erased later
				*/
			template <typename T>
				inline static	void	constructSingleFromFleetingData(T*target, T&data)
				{
					new (target) T;
					target->swap(data);
				}



				/**
					@brief Copies/moves one element to another

					Both objects have been constructed and should remain constructed

					@param from Element to copy/move from
					@param to Element to copy/move to
				*/
			template <typename T>
				inline static	void	move(T&from, T&to)
				{
					to.swap(from);
				}

				/**
					@brief Swaps data of @b u and @b v
				*/
			template <typename T>
				inline static	void	swap(T&u, T&v)
				{
					u.swap(v);
				}


			template <typename T0, typename T1>
				inline static	void	moveRange(T0*from_begin, T0*from_end, T1*to_begin)
				{
					Move<T0,T1>::execute(from_begin,from_end,to_begin);
				}


			/**
				@brief Shifts elements in reverse order

				This method is useful to shift data up

				@param shift_from_rbegin Last element of the range, first element that should be shifted
				@param shift_from_rend Element one before the first element of the range, one past the last element that should be shifted
				@param shift_to_rbegin Last element of the target range, first element that should be shifted to
			*/
			template <typename T>
				static	inline	void	reverseShiftRange(T*shift_from_rbegin, T*shift_from_rend, T*shift_to_rbegin)
				{
					while (shift_from_rbegin != shift_from_rend)
						(*shift_to_rbegin--).swap(*shift_from_rend--);
				}


			/**
				@brief Shifts elements in forward order

				This method is useful to shift data down

				@param shift_from_begin First element of the range, first element that should be shifted
				@param shift_from_end Element one past the last element of the range
				@param shift_to_begin First element of the target range, first element that should be shifted to
			*/
			template <typename T>
				static	inline	void	shiftRange(T*shift_from_begin, T*shift_from_end, T*shift_to_begin)
				{
					while (shift_from_begin != shift_from_end)
						(*shift_to_begin++).swap(*shift_from_end++);
				}


		};






		template <class Plugin>
			class CommonStrategy:public Plugin
			{
			public:
				/**
					@brief Reverts the order of elements in the specified field

					@param begin Element to begin from
					@param end	Element one past the end
				*/
				template <typename T>
					static inline	void	revert(T*begin, T*end)
					{
						if (begin == end)
							return;
						T	*front = begin,
							*back = end-1;
						T	extra;
						while (front < back)
						{
							Plugin::move(*front,extra);
							Plugin::move(*back,*front);
							Plugin::move(extra,*back);
							front++;
							back--;
						}
					}

				template <typename T0, typename T1>
					inline static	void	moveElements(T0*from, T1*to, count_t elements)
					{
						Plugin::moveRange(from,from+elements,to);
					}

				template <typename T0, typename T1>
					inline static	void	copyElements(const T0*from, T1*to, count_t elements)
					{
						Plugin::copyRange(from,from+elements,to);
					}

			};



		/**
			@brief Strategy class to use for objects that implement the swap() method, which swap data via that method
		*/
		typedef CommonStrategy<SwapStrategyPlugin>		SwapStrategy;
		/**
			@brief Strategy class to use for objects that implement the adoptFrom() method, which are initialized and moved via that method
		*/
		typedef CommonStrategy<AdoptStrategyPlugin>		AdoptStrategy;
		/**
			@brief Strategy class to use for traditional and primitive objects, which are initialized and moved via constructor and assignment-operator
		*/
		typedef CommonStrategy<CopyStrategyPlugin>		CopyStrategy;

		/**
			@brief Strategy class to use for primitive/native types and POD structs only. Certain operations copy binary without regards to the datatype's internal structure
		*/
		typedef CommonStrategy<PrimitiveStrategyPlugin>	PrimitiveStrategy;
		typedef CommonStrategy<PrimitiveStrategyPlugin>	NativeStrategy;		//!< See PrimitiveStrategy
		typedef CommonStrategy<PrimitiveStrategyPlugin>	PODStrategy;		//!< See PrimitiveStrategy


		typedef SwapStrategy		Swap;
		typedef SwapStrategy		Swappable;
		typedef AdoptStrategy		Adopt;
		typedef AdoptStrategy		Adopting;
		typedef AdoptStrategy		Adoptable;
		typedef CopyStrategy		Copy;
		typedef CopyStrategy		Copyable;
		typedef PrimitiveStrategy	Primitive;
		typedef NativeStrategy		Native;
		typedef PODStrategy			POD;




		template <class Strategy0, class Strategy1>
			class HybridStrategy:public CopyStrategy
			{};

		template <class MyStrategy>
			class HybridStrategy<MyStrategy,MyStrategy>:public MyStrategy
			{};



	template <typename T>
		class StrategySelector
		{
		public:
			typedef	Strategy::CopyStrategy	Default;
		};

		#undef DECLARE_DEFAULT_STRATEGY
		#define DECLARE_DEFAULT_STRATEGY(_type_, _strategy_)\
			namespace DeltaWorks{\
			namespace Strategy{\
			template <>\
				class StrategySelector<_type_>\
				{\
				public:\
				typedef	Strategy::_strategy_	Default;\
				};\
			}}

		#undef DECLARE__
		#define DECLARE__(_type_, _strategy_)	DECLARE_DEFAULT_STRATEGY(_type_, _strategy_)

		#undef DECLARE_T__
		#define DECLARE_T__(_type_, _strategy_)\
			namespace DeltaWorks{\
			namespace Strategy\
			{\
			template <typename T>\
				class StrategySelector<_type_<T> >\
				{\
				public:\
				typedef	Strategy::_strategy_	Default;\
				};\
			}}

		#undef DECLARE_T2__
		/**
			@brief Declares the default strategy for a two-component template type (e.g. MyTemplateClass<class T,size_t S> can be declared as adopting via DECLARE_T2__(MyTemplateClass,class,size_t,Adopting) )
		*/
		#define DECLARE_T2__(_type_, _t_type0_, _t_type1_,  _strategy_)\
			namespace DeltaWorks{\
			namespace Strategy\
			{\
			template <_t_type0_ T0, _t_type1_ T1>\
				class StrategySelector<_type_<T0, T1> >\
				{\
				public:\
				typedef	Strategy::_strategy_	Default;\
				};\
			}}
	 }

	#undef DECLARE_PRIMITIVE_TYPE
	#define DECLARE_PRIMITIVE_TYPE(_type_)	DECLARE_DEFAULT_STRATEGY(_type_,PrimitiveStrategy)



	using namespace Strategy;

}

#include <memory>


DECLARE__(char,Primitive)
DECLARE_PRIMITIVE_TYPE(unsigned char)
DECLARE_PRIMITIVE_TYPE(short)
DECLARE_PRIMITIVE_TYPE(unsigned short)
DECLARE_PRIMITIVE_TYPE(int)
DECLARE_PRIMITIVE_TYPE(unsigned int)
DECLARE_PRIMITIVE_TYPE(long)
DECLARE_PRIMITIVE_TYPE(unsigned long)
DECLARE_PRIMITIVE_TYPE(long long)
DECLARE_PRIMITIVE_TYPE(unsigned long long)
DECLARE_PRIMITIVE_TYPE(float)
DECLARE_PRIMITIVE_TYPE(double)
DECLARE_PRIMITIVE_TYPE(long double)
DECLARE_PRIMITIVE_TYPE(bool)


DECLARE_PRIMITIVE_TYPE(char*)
DECLARE_PRIMITIVE_TYPE(unsigned char*)
DECLARE_PRIMITIVE_TYPE(short*)
DECLARE_PRIMITIVE_TYPE(unsigned short*)
DECLARE_PRIMITIVE_TYPE(int*)
DECLARE_PRIMITIVE_TYPE(unsigned int*)
DECLARE_PRIMITIVE_TYPE(long*)
DECLARE_PRIMITIVE_TYPE(unsigned long*)
DECLARE_PRIMITIVE_TYPE(long long*)
DECLARE_PRIMITIVE_TYPE(unsigned long long*)
DECLARE_PRIMITIVE_TYPE(float*)
DECLARE_PRIMITIVE_TYPE(double*)
DECLARE_PRIMITIVE_TYPE(long double*)
DECLARE_PRIMITIVE_TYPE(bool*)
DECLARE_PRIMITIVE_TYPE(void*)


DECLARE_T__(std::shared_ptr,Swappable);
DECLARE_T__(std::weak_ptr,Swappable);

DECLARE_T2__(std::pair, typename, typename, Swappable);

#endif
