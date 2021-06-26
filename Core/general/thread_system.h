#ifndef thread_systemH
#define thread_systemH

/*****************************************************************************

Classes for platform-independent handling of Threads, Mutexes, and Thread Pools

*****************************************************************************/

#include "system.h"

#include "../container/queue.h"

#include <cstdlib>
#include <iostream>
#include <limits.h>
//#if defined _MSC_VER && _MSC_VER >= 1700
//	#define USE_ATOMIC
//	#include <atomic>
//#else
//	#undef USE_ATOMIC
//#endif
#undef USE_ATOMIC

#include <atomic>



namespace DeltaWorks
{

	namespace System
	{
		#define QUAD_JOBS	1
	
		extern bool	report_kernel_activity;


		#if SYSTEM==UNIX
			typedef pthread_t		thread_handle_t;
		#elif SYSTEM==WINDOWS
			typedef DWORD			thread_handle_t;
			typedef WINBASEAPI BOOL WINAPI pHandleFunction(__in HANDLE);

			extern pHandleFunction		*cancelSynchronousIo;

		#else
			#error stub: thread_t;
		#endif

		inline	thread_handle_t		thisThread()
		{
			#if SYSTEM==UNIX
				return pthread_self();
			#elif SYSTEM==WINDOWS
				return GetCurrentThreadId();
			#else
				#error stub: thisThread();
				return 0;
			#endif
		}

		#define THREAD_REPORT(message)	if (System::report_kernel_activity){System::thread_report_mutex.lock(); std::cout << ((void*)(size_t)System::thisThread())<<" (this="<<this<<")<"<<__func__<<">: "<<message<<std::endl;  System::thread_report_mutex.release();}
	

		class ThreadMainObject;
		class Thread;
		class Operation;
		class AbstractThreadMain;
	
		#undef THREAD_ENTRY
		#define THREAD_ENTRY
	
		#undef THREAD
		#define THREAD	THREAD_ENTRY
	
		#undef TENTRY
		#define TENTRY	THREAD_ENTRY
	
		typedef void(ThreadMainObject::*pThreadMethod)();	 			//!< Executable function format

	
	
		class ThreadMainObject
		{
		public:
		virtual	void			ThreadMain()=0;
		};
	

		class Operation	//!< Singular operation containing both object and method (if any)
		{
		protected:
				volatile bool	executing;
				friend class Thread;
		public:
				typedef pThreadMethod	pMethod;
			
				pMethod 				method_pointer;							//!< Pointer to the method that should be executed or NULL if ThreadMain() should be executed
				ThreadMainObject		*object;								//!< Executed thread object;

								Operation();
								Operation(ThreadMainObject*object);
								Operation(ThreadMainObject*object, pMethod method_pointer);
			
		inline	void			Execute()	//!< Executes the operation
								{
									executing = true;
									try
									{
										if (method_pointer)
											(object->*method_pointer)();
										else
											object->ThreadMain();
									}
									catch (...){}
									executing = false;
								}
		inline	bool			isExecuting()	const	//!< Returns true if this operation is currently being executed
								{
									return executing;
								}
		};	

		

		class Mutex //! System independent mutex handler
		{
		private:
			#if SYSTEM==WINDOWS
				HANDLE			handle;
			#elif SYSTEM==UNIX
				pthread_mutex_t handle;
			#endif
			friend class Event;
		public:
								Mutex();
		virtual					~Mutex();
				void			p();	//!< Identical to lock()
				void			P();	//!< Identical to lock()
				void			v();	//!< Identical to release()
				void			V();	//!< Identical to release()
				void			lock();		 //!< Waits until access can be aquired
				bool			tryLock();	  //!< Tries to acquire access \return true on success
				bool			tryLock(long milliseconds);	  //!< Tries to acquire access for a certain timeinterval \param milliseconds Milliseconds to try acquire the lock \return true on success
				void			release();	  //!< Releases access to the mutex
				void			unlock();		//!< Identical to release()
		};


		class RecursiveMutex: private Mutex
		{
		private:
				thread_handle_t	locked_thread;
				uint32_t		lock_level;
				Mutex			operation_shield;
		public:
							RecursiveMutex():lock_level(0)
							{}
			bool			IsLocked()	const
			{
				return lock_level > 0;
			}

			bool			IsNotLocked() const
			{
				return lock_level == 0;
			}

			bool			IsLockedByMe()	const
			{
				return lock_level > 0 && locked_thread == thisThread();
			}

			void			p()	//!< Identical to lock()
			{
				Lock();
			}
			void			P()	//!< Identical to lock()
			{
				Lock();
			}
			void			v()	//!< Identical to release()
			{
				Unlock();
			}
			void			V()
			{
				Unlock();
			}
			void			Lock()
			{
				thread_handle_t self = thisThread();
				operation_shield.lock();
					if (lock_level > 0)
					{
						if (locked_thread == self)
						{
							lock_level++;
							operation_shield.unlock();
							return;
						}
					}
					Mutex::lock();
						if (lock_level != 0)	//technically, this should not be possible
						{
							operation_shield.unlock();
							FATAL__("Trying to lock recursive mutex on lock_level != 0");
							return;
						}

						lock_level = 1;
						locked_thread = self;
				operation_shield.unlock();
			}

			
			bool			TryLock()
			{
				thread_handle_t self = thisThread();
				operation_shield.lock();
					if (lock_level > 0)
					{
						if (locked_thread == self)
						{
							lock_level++;
							operation_shield.unlock();
							return true;
						}
					}
					if (Mutex::tryLock())
					{
						if (lock_level != 0)	//technically, this should not be possible
						{
							operation_shield.unlock();
							FATAL__("Trying to lock recursive mutex on lock_level != 0");
							return false;
						}

						lock_level = 1;
						locked_thread = self;
						operation_shield.unlock();
						return true;
					}
				operation_shield.unlock();
				return false;
			}

			bool			TryLock(long milliseconds)
			{
				thread_handle_t self = thisThread();
				operation_shield.lock();
					if (lock_level > 0)
					{
						if (locked_thread == self)
						{
							lock_level++;
							operation_shield.unlock();
							return true;
						}
					}
					if (Mutex::tryLock(milliseconds))
					{
						if (lock_level != 0)	//technically, this should not be possible
						{
							operation_shield.unlock();
							FATAL__("Trying to lock recursive mutex on lock_level != 0");
							return false;
						}
						lock_level = 1;
						locked_thread = self;
						operation_shield.unlock();
						return true;
					}
				operation_shield.unlock();
				return false;
			}

			void			Unlock()
			{
				thread_handle_t self = thisThread();
				operation_shield.lock();
					if (self != locked_thread)
					{
						operation_shield.unlock();
						FATAL__("Trying to unlock mutex that was not locked by this thread");
						return;
					}
					if (!lock_level)
					{
						operation_shield.unlock();
						FATAL__("Trying to unlock mutex that was not locked at all");
						return;
					}
					lock_level--;
					if (!lock_level)
						Mutex::unlock();
				operation_shield.unlock();
			}

			void			Release()
			{
				Unlock();
			}

		};
	
		
	
	
		class Semaphore //! System independent semaphore handler
		{
		private:
			#if SYSTEM==WINDOWS
				HANDLE			handle;
			#elif SYSTEM==UNIX
				int				handle;
			#endif
		public:
								Semaphore(long initial_value=0);   //!< Initial constructor \param initial_value Initial semaphore value
		virtual					~Semaphore();
	
				void			enter();
				bool			tryEnter();
				void			leave();
				void			release(unsigned inc_by=1);

				void			request()	{enter();}	//!< Identical to enter()
				void			grant()		{leave();}	//!< Identical to leave() or release(1)
		};
		typedef Semaphore  RestrictedArea;

		/*!
			\fn void Semaphore::enter()
			\brief Attemps to enter the semaphore area
		
			Enters the semaphore area if the current semaphore value is 0 or greater.
			If so, the semaphore value will be decremented by one and the method returns.
			Otherwise the active thread will wait until the semaphore area can be entered.

			\fn bool Semaphore::tryEnter()
			\brief Attemps to enter the semaphore area
			\return true if the semaphore could be locked

			Enters the semaphore area if the current semaphore value is 0 or greater.
			If so, the semaphore value will be decremented by one and the method returns
			true. Otherwise the method returns false.

			\fn void Semaphore::leave()
			\brief Releases the lock on the semaphore

			Leaves the semaphore area by increasing its value by 1. Any waiting thread
			can now again enter the semaphore.

			\fn void Semaphore::release(unsigned inc_by)
			\brief Releases a number of slots on the semaphore

			Releases the semaphore by increasing its value by \a inc_by. A number
			of \a inc_by waiting threads may now again enter the semaphore area.
		*/
	

	

		/**
		Handler class for thread signal handling
		*/
		class ThreadWaitLock
		{
		public:
			/**
			Wakes all waiting threads.
			New threads can no longer enter
			and will immediately return upon calling Wait()
			*/
			void					Wake();
			/**
			Resets the lock for new signalling
			*/
			void					Reset();
			/**
			Waits for the signal. May return immediately if the signal was
			already set
			*/
			void					Wait();
		private:
			RecursiveMutex			mutex;
			Semaphore				wait = Semaphore(0);
			std::atomic<count_t>	numWaiting = 0;
			volatile bool			waking = false;
		};


		class Thread		   //! System independent thread handler
		{
		public:
			#if SYSTEM==WINDOWS
				typedef HANDLE	handle_t;
		private:
				typedef DWORD /*WINAPI*/ (*_threadPointer)(LPVOID);
		static  DWORD WINAPI 	thread_exec(LPVOID self_);
			#elif SYSTEM==UNIX
				typedef thread_handle_t	handle_t;
		private:
				typedef void* (*_threadPointer)(void*);
		static  void*	   		thread_exec(void*self_);
			#endif
				handle_t		handle;

				void			cleanUp();
		public:
			typedef pThreadMethod	pMethod;

			Operation			operation;		//!< operation to execute

			ThreadWaitLock		onComplete;


			/**/				Thread();
			/**/				Thread(ThreadMainObject*object,pMethod method, bool start);   		//!< Creates the thread. The thread needs to be started manually using the start() method if \b start is set false \param object Base object to execute \param method Method pointer to execute on the specified object or NULL to execute ThreadMain() \param start Set true to execute immideately
			/**/				Thread(ThreadMainObject&object,pMethod method, bool start);		//!< Creates the thread (using NULL as parameter). The thread needs to be started manually using the start() method if \b start is set false \param object Base object to execute \param method Method pointer to execute on the specified object or NULL to execute ThreadMain()  \param start Set true to execute immideately
			/**/				Thread(ThreadMainObject*object, bool start);						//!< Create the thread using a thread object. The thread needs to be started manually using the start() method if \b start is set false \param object Thread object to execute \param start Set true to execute immideately
			/**/				Thread(ThreadMainObject&object, bool start);						//!< Create the thread using a thread object. The thread needs to be started manually using the start() method if \b start is set false \param object Thread object to execute \param start Set true to execute immideately
			virtual				~Thread();
	
			void				create(ThreadMainObject*object,pMethod method, bool start);   		//!< Creates the thread. The thread needs to be started manually using the start() method if \b start is set false \param object Base object to execute \param method Method pointer to execute on the specified object or NULL to execute ThreadMain() \param start Set true to execute immideately
			void				create(ThreadMainObject&object,pMethod method, bool start);		//!< Creates the thread (using NULL as parameter). The thread needs to be started manually using the start() method if \b start is set false \param object Base object to execute \param method Method pointer to execute on the specified object or NULL to execute ThreadMain()  \param start Set true to execute immideately
			void				create(ThreadMainObject*object, bool start);						//!< Create the thread using a thread object. The thread needs to be started manually using the start() method if \b start is set false \param object Thread object to execute \param start Set true to execute immideately
			void				create(ThreadMainObject&object, bool start);						//!< Create the thread using a thread object. The thread needs to be started manually using the start() method if \b start is set false \param object Thread object to execute \param start Set true to execute immideately
			void				start();														//!< Starts the created thread
			inline void			Start()	{start();}
			void				terminate();													//!< Terminates thread execution. A thread should rather return than self terminate
			inline void			Terminate() {terminate();}
			bool				AwaitCompletion(DWORD maxWaitMilliseconds=0xFFFFFFFF);		//!< Waits until thread has returned, then closes the local handle. Do not call from the thread itself
			bool				isActive()	const;											//!< Query thread status \return true, if the thread is currently running
			inline bool			IsActive() const {return isActive();}
			bool				isSelf();													   //!< Checks if the calling thread matches the internal handle of the local object
			inline bool			IsSelf() {return isSelf();}
			handle_t			getHandle()	const	{return handle;}
			handle_t			GetHandle() const {return handle;}
			static void*		id();															//!< Retrieves the thread id of the active thread
			static void*		GetID() {return id();}
			
			static  thread_handle_t	startAndForget(void (*func_pointer)(void*), void*parameter);	   //!< Creates and starts a new self-cleaning thread. No external holding structure necessary. \param func_pointer Function pointer to execute \param parameter Parameter handed to the function on execute
		};
	
		class ThreadObject:public ThreadMainObject, protected Thread 	//! Abstract object passed to a thread handler instead of a thread function
		{
		public:
			/**/				ThreadObject();
			virtual				~ThreadObject()		{};
			
				using Thread::start;
				using Thread::Start;
				using Thread::isActive;
				using Thread::IsActive;
				using Thread::AwaitCompletion;
				using Thread::terminate;
				using Thread::Terminate;
				using Thread::getHandle;
				using Thread::GetHandle;
				using Thread::onComplete;
			
		};
	




		template <typename Mutex>
			class MutexLockT	//! Automatic localized mutex lock. The class prevents double locking or unlocking and automatically aquires/releases the lock on the referenced mutex on construction/destruction.
			{
			protected:
				Mutex*			reference;
				bool			locked;

								//SyncLock(const SyncLock&other)	{}
				MutexLockT&		operator=(const MutexLockT&lock)	{return *this;}
			
								MutexLockT()
								{}
			public:
								MutexLockT(Mutex&mutex):reference(&mutex),locked(false) {Acquire();}

								~MutexLockT()
								{
									Release();
								}
				void			Acquire()	//!< Acquires a lock on the referenced mutex if a lock has not yet been acquired
								{
									if (!locked)
									{
										reference->P();
										locked = true;
									}
								}
				void			Release()	//!< Releases the lock on the referenced mutex if a lock has been acquired
								{
									if (locked)
									{
										reference->V();
										locked = false;
									}
								}
				operator bool()	{Acquire(); return true;}	//!< Aquires a lock if not already done
	
			};
		typedef MutexLockT<Mutex>	MutexLock;
		typedef MutexLockT<RecursiveMutex>	RecursiveMutexLock;
	
	
		/**
			@brief Automatic localized timeout mutex lock
		
			The class prevents double locking or unlocking and automatically aquires/releases the lock on the referenced mutex on construction/destruction.
			Upon acquiring a timeout of 1000 is passed 
		*/
		class SoftMutexLock	//! Automatic localized mutex lock. The class prevents double locking or unlocking and automatically aquires/releases the lock on the referenced mutex on construction/destruction.
		{
		protected:
			union
			{
				Mutex*			reference;
				RecursiveMutex*	recursive_reference;
			};
				bool			locked,is_recursive;


								//SyncLock(const SyncLock&other)	{}
				SoftMutexLock&	operator=(const SoftMutexLock&lock)	{return *this;}
			
								SoftMutexLock()
								{}
		public:
								SoftMutexLock(Mutex&mutex, const TCodeLocation&lock_location):reference(&mutex),locked(false),is_recursive(false) {acquire(lock_location);}
								SoftMutexLock(RecursiveMutex&mutex, const TCodeLocation&lock_location):recursive_reference(&mutex),locked(false),is_recursive(true) {acquire(lock_location);}

								~SoftMutexLock()
								{
									release();
								}
				void			acquire(const TCodeLocation&lock_location)	//!< Acquires a lock on the referenced mutex if a lock has not yet been acquired
								{
									if (!locked)
									{
										if (is_recursive)
										{
											if (!recursive_reference->TryLock(1000))
												Except::TriggerFatal(lock_location,"Failed to acquire lock in 1000 mseconds. Deadlock assumed.");
										}
										else
											if (!reference->tryLock(1000))
												Except::TriggerFatal(lock_location,"Failed to acquire lock in 1000 mseconds. Deadlock assumed.");
										locked = true;
									}
								}
				void			release()	//!< Releases the lock on the referenced mutex if a lock has been acquired
								{
									if (locked)
									{
										if (is_recursive)
											recursive_reference->Release();
										else
											reference->release();
										locked = false;
									}
								}
				operator bool()	{ASSERT__(locked); return true;}	//!< Aquires a lock if not already done
	
		};
	
	

		#define synchronized(mutex)		if (MutexLock __synchronized_lock__ = mutex)
		#define softsync(mutex)			if (SoftMutexLock __synchronized_lock__ = SoftMutexLock(mutex,CLOCATION))

		
	
		class Event //! System independent event handler
		{
		private:
			#if SYSTEM==WINDOWS
				HANDLE			handle;
			#elif SYSTEM==UNIX
				pthread_cond_t	condition;
				pthread_mutex_t	mutex;
			#endif
		public:
								Event();
		virtual					~Event();
				void			trigger();	  //!< Signals the event
				void			signal();		//!<Identical to trigger()
				void			wait();		 //!< Waits for the event to be signaled
			#if SYSTEM==UNIX
				void			wait(Mutex&mutex);		 //!< Waits for the event to be signaled using mutex (linux only)
			#endif
		};
	
		/**
			@brief Asynchronous event
	
			Similar to an event except that triggering the event before the waiting thread is actually waiting will not cause deadlocks.
			Signal is primarily designed for a n-1-relation, meaning the signal is signaled one or more times and waited for exactly once before it can be signaled again.
			Any number of threads can signal, but only exactly one thread may wait for this particular signal at any given time.
		*/
		class Signal:protected Semaphore
		{
		protected:
				//volatile bool	signaled;
				Mutex			mutex;
				//std::atomic<bool>	signalCheck;
				std::atomic<count_t>	grantCount;
		public:
								Signal();
		virtual					~Signal();
	
				void			wait();
				void			signal();
				void			trigger();
				void			reset();
				bool			isSignaled()	const;	//!< Queries whether or not the signal has been triggered
				bool			isTriggered()	const;	//!< @overload
		};

	
		/**
			@brief Sequence thread
		
			A sequencer reexecutes the same thread object a number of times or until stopped
		*/
	
		class Sequencer:protected ThreadObject
		{
		protected:
				unsigned		remaining_runs;
				bool			decrement,
								do_quit;
				Signal			job_signal,
								finish_signal;
				Mutex			mutex;
	
		virtual	void			ThreadMain();
		public:
				Operation		operation;
			
								Sequencer();
				bool			start(unsigned num_runs=1);	//!< Executes the set operation. If the local thread has not previously been created then it is created. @param num_runs Number of times that @b operation should be executed. The method returns if @b num_runs is 0
				bool			startInfinitely();			//!< Reexecutes the set operation until the object is destroyed or finish() is invoked.
				bool			isActive()	const;			//!< Queries the current status @return true if the local thread is created and active and the local operation is currently executing or the number of remaining runs is greater 0
				void			finish();					//!< Sets the number of remaining runs to zero effectivly preventing the thread of continuing execution once the current operation has been completed. Does @b not stop the thread itself
				void			awaitCompletion();				//!< Waits for the thread to finish executing all remaining runs. If the local object is configured to run infinitly then the thread invoking this method will wait until another thread invokes the finish() method (or forever)
				void			terminate();				//!< Causes the local thread to terminate upon completion of any currently processed operations
		};
	
	
	
		/**
			@brief Standard barrier implementation for a fixed number of threads.
		
			The barrier object halts threads calling the wait() method until all other threads have come to a halt as well and then reactivates all threads.<br />
			This implementation requires a pre-defined fixed number of threads.
		*/
		class Barrier
		{
		private:
				Mutex			mutex;
				Semaphore		semaphore;
				unsigned		counter,
								num_threads;
		public:
								Barrier(unsigned thread_count);
				void			wait();
		};
	
	
		class ReadWriteMutex //! System independent read/write mutex handler
		{
		private:
				Mutex			mutex;
				Semaphore		semaphore;
				unsigned		max_readers;
				volatile bool	lockedDown;
		public:
								ReadWriteMutex(unsigned max_readers=32);
				void			BeginRead();	   //!< Waits until read-access can be aquired
				/**
				Attempts to enter writing phase for the local thread. The invocation fails immediately, if the local instance is locked down.
				@param lockdown Lock out any other threads calling SignalWrite() until ExitRead() is called. For those threads, calling SignalWrite() will return false immediately, rather than block\
				@return true, if the lock was acquired, false otherwise.
				*/
				bool			BeginWrite(bool lockdown = false);
				void			EndRead();		 //!< Releases read-access
				void			EndWrite();		//!< Releases write-access
		};
	
		/*!
			\brief System independent counter object
	
			The CounterObject provides a mutex shielded counter (initialized with 0).
			It can be increased or decreased without fear of synchronization issues.
			Also a thread may wait until the counter reaches 0.
		*/
		class CounterObject
		{
		private:
				Mutex			mutex;//,zero_mutex;
				Event			zero_event;
				unsigned		count;
		public:
								CounterObject();
				void			operator++(int);				//!< Increments the counter
				void			operator--(int);				//!< Decrements the counter
				void			operator+=(unsigned delta);	 //!< Increases the counter \param delta Value to increase by
				void			operator-=(unsigned delta);	 //!< Decreases the counter \param delta Value to decrease by
								operator	unsigned();		 //!< Implict type conversion. Queries the counter \return Current counter value
				void			waitForZero();				  //!< Waits until counter hits zero
		};

	
	
		class AtomicLong
		{
		protected:
				#if SYSTEM!=WINDOWS
					typedef long int		long_t;
					std::atomic<long_t>		value;
				#else
					typedef volatile LONG	long_t;
				long_t						value;
				#endif
			
		public:
											AtomicLong(long value_=0):value(value_)
											{}
			
				AtomicLong&					operator=(long value);
				long						operator++();
				long						operator++(int);
				long						operator--();
				long						operator--(int);
				long						operator+=(long value);
				long						operator-=(long value);

				long						Set(long value);
			
				operator long() const		{return value;}
		};


		class SpinLock
		{
			//from http://en.cppreference.com/w/cpp/atomic/atomic_exchange

			#ifdef USE_ATOMIC
				std::atomic<bool> _lock; // holds true when locked
			#else
				AtomicLong		_lock;
			#endif
											// holds false when unlocked

		public:

			/**/		SpinLock():_lock(false)	{}

			inline void lock()
			{
				#ifdef USE_ATOMIC
					while(std::atomic_exchange_explicit(&_lock, true, std::memory_order_acquire));
				#else
					while (_lock.Set(1));
				#endif
			}

			inline void unlock()
			{
				#ifdef USE_ATOMIC
					std::atomic_store_explicit(&_lock, false, std::memory_order_release);
				#else
					_lock.Set(0);
				#endif
			}
		};
	
	
	
		template <class Queue>
			class ThreadPoolWorker;
		class ThreadPoolJob;
		template <class Entry>
			class BlockingQueue;
		template <class Entry>
			class BlockingPriorityQueue;
	
		class ThreadPoolContext
		{
		public:
				BlockingPipe		wait_pipe;		//!< Pipe used to signal waiting threads that they may continue
				size_t				counter,		//!< Number of operations left to do until the context finalizes
									waiting;		//!< Number of threads that await finalization of this context
				Mutex				counter_mutex;	//!< Mutex shielding the \b counter and \b waiting variables
			
				void				inc()
									{
										counter_mutex.lock();
											counter++;
										counter_mutex.release();
									}
				void				inc(size_t count)
									{
										counter_mutex.lock();
											counter+=count;
										counter_mutex.release();
									}
		public:
									ThreadPoolContext():counter(0),waiting(0)
									{}
		};

		/**
			@brief Singular thread job container
		
			Depending on the precompiler configuration (QUAD_JOBS=0/1) thread jobs feature a single or four jobs at once which are executed in order by the same thread before it attempts to acquire a new job.
		*/
		class ThreadPoolJob
		{
			public:
				
				union
				{
					struct
					{
					#if QUAD_JOBS
						ThreadMainObject	*obj0,
											*obj1,
											*obj2,
											*obj3;
					#else
						ThreadMainObject	*obj0;
					#endif
					};
					#if QUAD_JOBS
						ThreadMainObject	*object[4];
					#endif
				};
					Thread::pMethod		method;
					ThreadPoolContext		*context;

											ThreadPoolJob();
											ThreadPoolJob(ThreadMainObject*object,Thread::pMethod method=NULL);
											ThreadPoolJob(ThreadMainObject&object,Thread::pMethod method=NULL);
											ThreadPoolJob(ThreadMainObject*object,ThreadPoolContext*context);
											ThreadPoolJob(ThreadMainObject&object,ThreadPoolContext*context);
											ThreadPoolJob(ThreadMainObject*object,Thread::pMethod method, ThreadPoolContext*context);
											ThreadPoolJob(ThreadMainObject&object,Thread::pMethod method, ThreadPoolContext*context);	
					#if QUAD_JOBS
											ThreadPoolJob(ThreadMainObject*object0,ThreadMainObject*object1,ThreadMainObject*object2,ThreadMainObject*object3,Thread::pMethod method, ThreadPoolContext*context);
					#endif
	
		};
	
		#ifndef __CUDACC__
	
		/**
			@brief Mutex protected object-queue
		
			BlockingQueue provides a number of protected methods to access the underlying queue object.
			When trying to read an element from the queue the operation halts if the queue is empty.
	
		*/
		template <class Entry>
			class BlockingQueue:private Container::Queue<Entry>
			{
			private:
					Semaphore					semaphore;
					Mutex						mutex;
					size_t						sequence_counter;
				
			public:
												BlockingQueue();
												BlockingQueue(size_t size);	//!< Constructor featuring an initial queue size (the queue still auto-increases its size if necessary)
					void						read(Entry&out);				//!< Attempts to read an entry from the queue. This method blocks until an entry is available. Any read entry is removed from the queue. @param out [out] Reference to copy the entry from
					void						write(const Entry&data);		//!< Writes a single entry to the end of the queue, thus increasing the number of contained elements by one.  @param data [in] Entry to copy to the end of the queue
					void						beginSequence();					//!< Begins a protected write sequence. Must be terminated by invoking endSequence()
					void						writeElement(const Entry&data);		//!< Appends a single element to the end of the queue. This method is considerably faster than write() but must be called inbetween beginSequence() and endSequence()
					void						endSequence();						//!< Ends a protected write sequence. Must be called once all elements of a sequence have been appended to the queue.
					bool						IsEmpty()	const;					//!< Queries if the queue is currently empty
					void						insertPrimary(const Entry&data, size_t count);	//!< The method is named to allow seamless handling of different types of queues by template classes. It does append the specified object @a count times to the end of the queue, @b not to the beginning as the method name might imply. @param data Data to append to the end of the queue @param count Number of times that the object should be appended
				
					void						operator>>(Entry&out);				//!< Operator version of read()
					BlockingQueue<Entry>&		operator<<(const Entry&in);			//!< Operator version of write()
			};

		/**
			@brief Mutex protected priority object-queue
		
			BlockingPriorityQueue provides a number of protected methods to access the underlying priority queue object.
		*/
		template <class Entry>
			class BlockingPriorityQueue:private Container::PriorityQueue<Entry,int>
			{
			private:
					Semaphore					semaphore;
					Mutex						mutex;
					size_t						sequence_counter;
				
			public:
												BlockingPriorityQueue(size_t size=1024);	//!< Constructor featuring an initial queue size (the queue still auto-increases its size if necessary)
					void						read(Entry&out);							//!< Attempts to read the most important entry from the queue. This method blocks until an entry is available. Any read entry is removed from the queue. @param out [out] Reference to copy the entry from
					void						write(const Entry&data, int priority);		//!< Writes a single entry to an appropriate position in the queue, thus increasing the number of contained elements by one.  @param data [in] Entry to copy to the end of the queue @param priority [in] Priority to apply to the inserted object
					void						beginSequence();							//!< Begins a protected write sequence. Must be terminated by invoking endSequence()
					void						writeElement(const Entry&data, int priority);		//!< Inserts a single element into the queue. This method is considerably faster than write() but must be called inbetween beginSequence() and endSequence()
					void						endSequence();								//!< Ends a protected write sequence. Must be called once all elements of a sequence have been appended to the queue.
					bool						IsEmpty()	const;									//!< Queries if the queue is currently empty
					void						insertPrimary(const Entry&data, size_t count);	//!< Inserts a single object a number of times at top priority into the queue. The inserted object will be priorized over all currently inserted objects. @param data [in] Object to enqueue @param count [in] Number of times the object should be inserted in the queue
				
					void						operator>>(Entry&out);				//!< Operator version of read()
			};
	
	
		template <class Queue>
			class TmplThreadPool;
	
		/**
			@brief Pool worker thread
		*/
		template <class Queue>
			class ThreadPoolWorker:public ThreadObject
			{
			public:
					TmplThreadPool<Queue>	*pool;				//!< Pointer to the parent thread pool
					//unsigned			operation_counter;
				
					void				ThreadMain();
				
					friend class TmplThreadPool<Queue>;
					friend class ThreadPoolJob;
			};

		class JobSequence;
	
		/*!
			\brief Thread pool

			The ThreadPool object manages a constant number of threads that can be used for a dynamic number of jobs.
		*/
		template <class Queue>
			class TmplThreadPool
			{
			public:

					typedef ThreadPoolContext	Context;
					typedef ThreadPoolJob		Job;
				

			protected:
					typedef ThreadPoolWorker<Queue>	Worker;
					friend class ThreadPoolWorker<Queue>;
					friend class ThreadPoolJob;
					friend class ThreadPoolContext;
					friend class JobSequence;
			
			
					ThreadMainObject	*init_object;	//!< Object executed once by each new worker thread. May be NULL
					Thread::pMethod	init_method;	//!< Initialization method to invoke on the specified init_object. May be NULL resulting in init_object->ThreadMain() to be called instead (unless init_object is NULL)
					Ctr::Array<Worker>	workers;		//!< Worker field
					//BlockingPipe   	job_pipe;
					Queue				job_pipe;		//!< Pipe used to channel jobs to the workers
					//bool				terminated;		
					std::exception		exception;		//!< Caught exception thrown during the execution of a job. NULL if none.
					bool				exception_caught;
				
	//		virtual	void				enqueueTerminalJob()=0;
			public:

				typedef Context			Family;
				static Context			global_context;		//!< Context applied to standard context-less jobs
		
				// Constructor \param num_threads Number of threads to create \param init Pointer to execute on each new thread before executing jobs \param init_data Pointer to initiation parameter to be passed to each new thread together with \b init
										TmplThreadPool(size_t num_threads=0,ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL); 
			virtual						~TmplThreadPool();
					void				build(size_t num_threads,ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);  //!< Rebuild local  \param num_threads Number of threads to create \param init Pointer to execute on each new thread before executing jobs \param init_data Pointer to initiation parameter to be passed to each new thread together with \b init
				
				
					void				terminate();											//!< Sends terminal signals and destroys all threads. Should be used <b>exclusivly</b> to terminate the application faster in case of a critical exception.
				
					void				signalIdle(ThreadPoolContext*context=&global_context);	//!< Signals any threads waiting in the specified context to continue even if the working queue is not yet empty.
					void				flush(ThreadPoolContext*context=&global_context);	//!< 	Releases any threads waiting until idle and terminates. Should be used <b>exclusivly</b> to terminate the application faster in case of a critical exception.
				
					bool				busy(Context*context=&global_context);							 //!< Queries pool state \return true if busy
					void				waitUntilIdle(Context*context=&global_context);					//!< Waits until ThreadPool is idle
					void				implode();												//!< Send terminal symbols to all threads and waits until they finish
					size_t				workerCount()	const	{return workers.Count();}		//!< Retrieves the number of active workers
				
			};
	
	
		/**
			@brief Standard thread pool
	
		*/
		class ThreadPool:public TmplThreadPool<BlockingQueue<ThreadPoolJob> >
		{
		protected:
					friend class JobSequence;
		public:
				typedef TmplThreadPool<BlockingQueue<ThreadPoolJob> >	Parent;
			
									ThreadPool(size_t num_threads=0,ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL); 
									
				void				Process(const Job&job);			//!< Enqueues a singular job
				void				Process(ThreadMainObject*object,Thread::pMethod method=NULL);	//!< Enqueues a singular job
				void				Process(ThreadMainObject&object,Thread::pMethod method=NULL);	//!< Enqueues a singular job
				void				Process(ThreadMainObject*object,ThreadPoolContext*context);	//!< Enqueues a singular job
				void				Process(ThreadMainObject&object,ThreadPoolContext*context);	//!< Enqueues a singular job
				void				Process(ThreadMainObject*object,Thread::pMethod method, ThreadPoolContext*context);	//!< Enqueues a singular job
				void				Process(ThreadMainObject&object,Thread::pMethod method, ThreadPoolContext*context);		//!< Enqueues a singular job
				void				process4(ThreadMainObject*obj0,ThreadMainObject*obj1,ThreadMainObject*obj2,ThreadMainObject*obj3,Thread::pMethod method, ThreadPoolContext*context);	//!< Enqueues 4 jobs using the same method and context on 4 different objects
				void				split(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context=&global_context);									//!< Separates the specified objects into quad jobs and mass enqueues them using the specified method and context
				void				split(Ctr::Array<ThreadMainObject*>&objects,size_t count, Thread::pMethod method, ThreadPoolContext*context=&global_context);									//!< Separates the specified objects into quad jobs and mass enqueues them using the specified method and context. Only the first @b count objects are enqueued
				void				splitIndividually(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context=&global_context);
			
				ThreadPool&		operator<<(const Job&job);

		};
	
		/**
			@brief Interface to quickly enqueue a sequence of jobs with reduced overhead
		
			Job sequences are designed to be temporary stack-only objects. Create a JobSequence object with the desired pool as
			constructor parameter and then assign job segments via split() or splitIndividually() . The object automatically
			releases the mutex lock on destruction.
		*/
		class JobSequence
		{
		protected:
				ThreadPool				&pool;
			
										JobSequence(const JobSequence&other):pool(other.pool)
										{}
				void					operator=(const JobSequence&other)
										{}
									
		public:
		typedef	ThreadPool::Job		Job;
			
										JobSequence(ThreadPool&pool_):pool(pool_)
										{
											pool.job_pipe.beginSequence();
										}
										~JobSequence()
										{
											pool.job_pipe.endSequence();
										}
				/**
					@brief Enqueues a series of objects using the same method and context
				
					@param objects Ctr::Array of objects to enqueue. May be empty causing the method to simply return.
					@param method Method to use on all objects. May be NULL causing the processor to invoke the ThreadMain() method of each object
					@param context Counter context associated with each object
				*/
				void					split(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context);
				/**
					@brief Enqueues a series of objects using the same method and context
				
					Similar to the above except that only the first @a count objects are actually enqueued. Any following objects are ignored.
				*/
				void					split(Ctr::Array<ThreadMainObject*>&objects,size_t count, Thread::pMethod method, ThreadPoolContext*context);
				/**
					@brief Quad job override version of the above method
				
					splitIndividually() overrides the QUAD_JOB macro switch and enforces one object per job. If said macro is false then splitIndividually() behaves identical
					to split()
				
					@param objects Ctr::Array of objects to enqueue. May be empty causing the method to simply return.
					@param method Method to use on all objects. May be NULL causing the processor to invoke the ThreadMain() method of each object
					@param context Counter context associated with each object
				*/
				void					splitIndividually(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context);
		};
	
		/*!
			\brief Priority thread pool

			The PriorityThreadPool object manages a constant number of threads that can be used
			for a dynamic number of jobs. Jobs are processed in order or priority primarily and in order
			of enqueueing secondarily.
		*/
		class PriorityThreadPool:public TmplThreadPool<BlockingPriorityQueue<ThreadPoolJob> >
		{
		public:
				typedef TmplThreadPool<BlockingPriorityQueue<ThreadPoolJob> >	Parent;
	
									PriorityThreadPool(size_t num_threads=0,ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL); 
			
				void				Process(int priority, const Job&job);
				void				Process(int priority, ThreadMainObject*object,Thread::pMethod method=NULL);
				void				Process(int priority, ThreadMainObject&object,Thread::pMethod method=NULL);
				void				Process(int priority, ThreadMainObject*object,ThreadPoolContext*context);
				void				Process(int priority, ThreadMainObject&object,ThreadPoolContext*context);
				void				Process(int priority, ThreadMainObject*object,Thread::pMethod method, ThreadPoolContext*context);
				void				Process(int priority, ThreadMainObject&object,Thread::pMethod method, ThreadPoolContext*context);	
				void				process4(int priority, ThreadMainObject*obj0,ThreadMainObject*obj1,ThreadMainObject*obj2,ThreadMainObject*obj3,Thread::pMethod method, ThreadPoolContext*context);
				void				split(const Ctr::Array<int>&priorities, Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context=&global_context);
				void				splitIndividually(const Ctr::Array<int>&priorities, Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context=&global_context);
			
			
		};
	
	
		/**
			@brief Pool-variable thread kernel
		
			Thread kernels allocate one worker thread per available processor and lock each to its respective CPU.
		*/
		template <class ThreadPool>
			class TmplKernel:public ThreadPool,protected ThreadObject
			{
			private:
					size_t				counter;
					Mutex		  		mutex;
					ThreadMainObject	*init_object;
					pThreadMethod		init_method;
			
					void				ThreadMain();
			public:
										TmplKernel(ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
					void				build(ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
			};
	
		typedef TmplKernel<ThreadPool>			ThreadKernel;
		typedef TmplKernel<PriorityThreadPool>	PriorityKernel;
	
		/**
			@brief Pool-variable thread kernel
		
			Thread hyper kernels allocate a number of worker threads per available processor and lock each to its respective CPU.
			Typical scenarios are two or three worker threads per CPU.
		*/
		template <class ThreadPool>
			class TmplHyperKernel:public ThreadPool,protected ThreadObject
			{
			private:
					size_t				counter;
					Mutex		  		mutex;
					ThreadMainObject	*init_object;
					pThreadMethod		init_method;
			
					void				ThreadMain();
			public:
										TmplHyperKernel(ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
										TmplHyperKernel(unsigned factor, ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
					void				build(ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
					void				build(unsigned factor, ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
			};

		typedef TmplHyperKernel<ThreadPool>			HyperKernel;
		typedef TmplHyperKernel<PriorityThreadPool>	PriorityHyperKernel;
	




		class ParallelLoopBase
		{
		protected:
				Semaphore							job_semaphore,		//!< Semaphore set to the number of remaining jobs
													terminal_semaphore,
													destruct_semaphore;
				Mutex								mutex;
				#ifdef __GNUC__
					typedef long int				offset_t;
				#else
					typedef volatile LONG			offset_t;
				#endif
				#if SYSTEM==WINDOWS
					volatile BYTE					offset_field[2*sizeof(offset_t)];
					offset_t						*aligned_offset;	//!< Current job offset (located in offset_field and aligned to 32 bits)
				#else
					std::atomic<offset_t>			jobOffset;	//!< Current job offset
				#endif
				count_t								thread_counter,
													num_jobs,			//!< Total number of jobs
													last_job_iterations;		//!< Number of iterations in the last job
				bool								set_affinity;	//!< Set true to bind threads to processor cores
				ThreadMainObject					*init_object;	//!< Object executed once by each new worker thread. May be NULL
				Thread::pMethod						init_method;	//!< Initialization method to invoke on the specified init_object. May be NULL resulting in init_object->ThreadMain() to be called instead (unless init_object is NULL)
				BlockingPipe						wait_pipe;		//!< Pipe used to signal the main thread that it may return


													ParallelLoopBase();
		virtual										~ParallelLoopBase();
		};
	
		class ParallelLoop:protected ParallelLoopBase
		{
		public:
			class Operator
			{
			public:
				virtual	void			ExecuteRange(index_t begin, index_t end, index_t worker_index) = 0;
			};
		protected:
			class Worker:public ThreadObject
			{
				ParallelLoop			*parent;
				index_t					myIndex;
					
				void					Init();
			public:
				/**/					Worker();
				void					Setup(index_t workerIndex, ParallelLoop*parent) {this->parent = parent; myIndex = workerIndex;}
				void					ThreadMain();
			};
			
			
			friend class Worker;					//!< Should be implied but better to be safe than sorry
			count_t						iterationsPerJob;	//!< Number of iterations per job
			Operator					*op;
			Ctr::Array<Worker>			workers;		//!< Worker field
			
		public:

			static	ParallelLoop		globalInstance;

			/**/						ParallelLoop(ThreadMainObject*init_object=NULL, Thread::pMethod init_method=NULL);
			virtual						~ParallelLoop();

	
			/**
			@brief Executes a function on this object.
				
			Execute() splits the specified number of iterations into jobs of equal size depending on @a iterations_per_job . The remainder is executed by the calling thread.
			No two operations may be executed at the same time by a single object. If multiple threads execute operations on ParallelLoop instances, then create one instance per thread or protect Execute()
			using an external mutex.
			Execute() itself is not mutex protected.
			*/
			void						Execute(Operator*op, count_t iterations, count_t iterationsPerJob);

			count_t						GetWorkerCount()	const	{return workers.Count();}
			static count_t				GetGlobalWorkerCount() {return globalInstance.workers.Count();}
		};


		template <typename F>
			class ParallelOperator : public ParallelLoop::Operator
			{
			protected:
				const F					&function;
			public:
				/**/					ParallelOperator(const F& f):function(f) {}
				virtual	void			ExecuteRange(index_t begin, index_t end, index_t worker_index) override
										{
											while (begin != end)
											{
												function(begin,worker_index);
												begin++;
											}
										}
			};

		template <typename Function>
			inline void ParallelFor(count_t iterations, count_t iterationsPerJob, const Function& f)
			{
				ParallelOperator<Function>	op(f);
				ParallelLoop::globalInstance.Execute(&op,iterations,iterationsPerJob);
			}
		
		template <typename Function>
			inline void LinearFor(count_t iterations, count_t iterationsPerJob, const Function& f)
			{
				for (index_t i = 0; i < iterations; i++)
					f(i,0);
			}
		


		#if 0
		template <typename Parameter, void (*Callback)(unsigned,const Parameter&), unsigned IterationsPerJob, bool Reduced>
			class ParallelLoop
			{
			protected:
					class Worker:public ThreadObject
					{
					public:
							void					init();
							void					ThreadMain();
					};
					class Instance:public ParallelLoopBase
					{
					public:
							Ctr::Array<Worker>			workers;		//!< Worker field
							const Parameter			*parameter;

													Instance();
					virtual							~Instance();
					};
					friend class Worker;					//!< Should be implied but better to be safe than sorry

					static Instance				instance;
			public:
			static	void							execute(unsigned iterations, const Parameter&parameter);
			};
		#endif /*0*/
	
		#endif /*__CUDACC__*/
	

		extern Mutex	thread_report_mutex;
	
	}

	#ifndef __CUDACC__
		#include "thread_system.tpl.h"
	#endif
}

#endif
