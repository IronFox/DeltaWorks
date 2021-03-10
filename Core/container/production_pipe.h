#ifndef production_pipeH
#define production_pipeH

#include "buffer.h"
#include "../general/thread_system.h"


/******************************************************************

E:\include\list\production_pipe.h

******************************************************************/

namespace DeltaWorks
{
	namespace Container
	{

		/**
		Thread-safe production queue designed for many threads writing and one thread reading.
		Copies objects as they are using the provided strategy
		*/
		template <typename T, typename MyStrategy=typename StrategySelector<T>::Default>
			class WorkPipe : protected Buffer<T,0,MyStrategy>
			{
				typedef Buffer<T,0,MyStrategy>Super;
				typedef WorkPipe<T,MyStrategy>Self;
				volatile bool				locked;
				//Mutex						mutex;
				System::SpinLock			lock;
				index_t						readAt;
				System::Signal				contentSignal;
			public:
				typedef typename Super::iterator		iterator;
				typedef typename Super::const_iterator		const_iterator;

				/**/						WorkPipe():locked(false)	{}
				inline  void                SignalRead();		//!< Signals that the local object mutex should be locked and the read cursor reset for subsequent read access
				inline  bool                operator>>(T&);	//!< Retrieves the object currently apointed by the read cursor. Automatically advances the read cursor 	@return true if the object could be retrieves, false if the read cursor has reached the end of the queue
				inline  void                ExitRead();			//!< Signals that the local list should be flushed and the local object mutex released
				inline  Self&				operator<<(const T&);		//!< Inserts an element to the end of the queue. This operation is mutex protected
				#if __BUFFER_RVALUE_REFERENCES__
					inline Self&			operator<<(T&&element);
				#endif

				inline  Self&				MoveAppend(T&);		//!< Similar to the << operator but uses strategy move, rather than forced copy
				inline	Self&				MoveAppend(BasicBuffer<T>&buffer);
				inline  void                clear();			//!< Erases each object and flushs the queue. This operation is mutex protected
				inline	void				Clear() {clear();}
				inline	void				WaitForContent()	{contentSignal.wait();}
				inline  void				OverrideSignalNow() {contentSignal.signal();}
		
				typename Super::iterator		begin()	{ASSERT__(locked); return Super::begin();}
				typename Super::const_iterator	begin()	const {ASSERT__(locked); return Super::begin();}
				typename Super::iterator		end()	{return Super::end();}
				typename Super::const_iterator	end() const	{return Super::end();}

				inline count_t				Count() const {return Super::Count();}
				inline count_t				size() const {return Super::size();}
				inline bool					IsEmpty() const {return Super::IsEmpty();}
				/**
				Moves all local items to the end of the specified destination buffer in local order.
				Uses destination strategy to move elements.
				The local queue will be left empty as a result.
				Thread safe. DO NOT call SignalRead() and ExitRead(), or else the system will enter deadlock
				@param[in,out] destination List to append to. NOT CLEARED ahead of operation
				@return The number of appended elements
				*/
				template <typename Strategy>
					count_t					FlushContentTo(BasicBuffer<T,Strategy>&destination);

				//the following operations are provided for convenience. They are _not_ mutex protected and (except for Count() maybe) should be invoked only from inside a signalRead()/exitRead() block.
				//Super::IsEmpty;
				using Super::IsNotEmpty;
				using Super::operator[];
				using Super::pointer;
			
				typedef T	Type;
	
			};

		/**
			@brief Pointer queue for scenarios where many threads write and one reads
		*/
		template <class T>
			class ProductionPipe:public WorkPipe<std::shared_ptr<T> >
			{
				typedef WorkPipe<std::shared_ptr<T> >Super;
			protected:
			public:

					//ReferenceVector<T>::Type;
			};



		#include "production_pipe.tpl.h"
	}
}

#endif
