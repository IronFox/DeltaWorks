#include "../global_root.h"
#include "thread_system.h"
#if SYSTEM==WINDOWS
#include <Windows.h>
#endif
#include <algorithm>
#include "../io/sharedLibrary.h"

namespace DeltaWorks
{

	namespace System
	{
		bool							report_kernel_activity(false);

		Mutex	thread_report_mutex;
	

	
		//#define THREAD_REPORT(message)

	#if SYSTEM==WINDOWS

		pHandleFunction		*cancelSynchronousIo = NULL;

	#endif



		class Init
		{
			static Init			instance;

			#if SYSTEM == WINDOWS
				SharedLibrary	kernel_library;
			#endif

			Init()
			{
				#if SYSTEM == WINDOWS
					bool success = kernel_library.Open(FileSystem::StringToPath("kernel32.dll"));
					const String error = success?"":kernel_library.GetOpenError();
					ASSERT1__(success,error);

					if (!kernel_library.FindFunction("CancelSynchronousIo",cancelSynchronousIo))
						cancelSynchronousIo = NULL;

				
					//CancelSynchronousIo(handle);
				#endif


			}
		};

		/*static*/ Init		Init::instance;
	

	
	
	
		Operation::Operation():executing(false),object(NULL),method_pointer(NULL)
		{}
	
		Operation::Operation(ThreadMainObject*object_):executing(false),object(object_),method_pointer(NULL)
		{}
	
		Operation::Operation(ThreadMainObject*object_, pMethod method_pointer_):executing(false),object(object_),method_pointer(method_pointer_)
		{}
	
	
	
	

		Sequencer::Sequencer():remaining_runs(0),decrement(true),do_quit(false)
		{}
	
		bool	Sequencer::start(unsigned num_runs_)
		{
			if (remaining_runs)
				return false;
			remaining_runs = num_runs_;
			decrement = true;
			ThreadObject::start();
			job_signal.trigger();
			return true;
		}
	
		bool	Sequencer::startInfinitely()
		{
			if (remaining_runs)
				return false;
			remaining_runs = 1;
			decrement = false;
			ThreadObject::start();
			job_signal.trigger();
			return true;
		}
	
		bool	Sequencer::isActive() const					//!< Queries the current status @return true if the local thread is created and active and the local operation is currently executing or the number of remaining runs is greater 0
		{
			return Thread::isActive() && (remaining_runs || operation.isExecuting());
		}
	
		void	Sequencer::finish()
		{
			mutex.lock();
				remaining_runs = 0;
			mutex.release();
		}
	
		void	Sequencer::awaitCompletion()
		{
			finish_signal.wait();
		}
	
		void	Sequencer::terminate()
		{
			do_quit = true;
			remaining_runs = 0;
			job_signal.trigger();
		}
	
		void	Sequencer::ThreadMain()
		{
			while (!do_quit)
			{
				job_signal.wait();
				while (remaining_runs)
				{
					operation.Execute();
					mutex.lock();
						if (remaining_runs && decrement)
							remaining_runs--;
					mutex.release();
				}
				finish_signal.trigger();
			}
			job_signal.reset();
			remaining_runs = 0;
		}
	
	
	
		Thread::Thread()
		#if SYSTEM==WINDOWS
			:handle(NULL)
		#endif
		{}
	
	
		Thread::Thread(ThreadMainObject*object_,pMethod method, bool start)
		#if SYSTEM==WINDOWS
			:handle(NULL)
		#endif
		{
			create(object_,method,start);
		}

		Thread::Thread(ThreadMainObject&object_,pMethod method,bool start)
		#if SYSTEM==WINDOWS
			:handle(NULL)
		#endif
		{
			create(&object_,method,start);
		}

		Thread::Thread(ThreadMainObject*object_, bool start)
		#if SYSTEM==WINDOWS
			:handle(NULL)
		#endif
		{
			create(object_,NULL,start);
		}

		Thread::Thread(ThreadMainObject&object_, bool start)
		#if SYSTEM==WINDOWS
			:handle(NULL)
		#endif
		{
			create(&object_,NULL,start);
		}
	

		Thread::~Thread()
		{
			terminate();
		}
	
		void*			Thread::id()
		{
			#if SYSTEM==UNIX
				return ((void*)pthread_self());
			#elif SYSTEM==WINDOWS
				return ((void*)(size_t)GetCurrentThreadId());
			#endif
			return NULL;
		}
	

		#if SYSTEM==UNIX
		void* Thread::thread_exec(void*self_)
		{
			Thread*self = reinterpret_cast<Thread*>(self_);
			self->operation.Execute();
			self->onComplete.Wake();
			return NULL;
		}
		#elif SYSTEM==WINDOWS
		DWORD WINAPI Thread::thread_exec(LPVOID self_)
		{
			Thread*self = reinterpret_cast<Thread*>(self_);
			self->operation.Execute();
			if (cancelSynchronousIo)
				cancelSynchronousIo(self->handle);
			self->onComplete.Wake();
			return (DWORD)0;
		}
		#endif

		void        Thread::cleanUp()
		{
			#if SYSTEM==WINDOWS
				if (handle==NULL)
					return;
				if (!operation.executing)
				{
					CloseHandle(handle);
					handle = NULL;
				}
			#else
				//unnecessary

			#endif
		}


		void        Thread::create(ThreadMainObject*object_,pMethod method, bool start)
		{
			terminate();
			operation.object = object_;
			operation.method_pointer = method;
			onComplete.Reset();
			#if SYSTEM==WINDOWS
				if (start)
				{
					handle = CreateThread(NULL , 0 , thread_exec , this , 0 , NULL);    //CREATE_SUSPENDED instead of 0?
				}
				else
					handle = NULL;
				operation.executing = handle!=NULL;
			#elif SYSTEM==UNIX
				operation.executing = start && !pthread_create(&handle,NULL,thread_exec,this);
			#endif
		}

		void        Thread::create(ThreadMainObject&object_,pMethod method,bool start)
		{
			create(&object_,method,start);
		}

		void        Thread::create(ThreadMainObject*object_, bool start)
		{
			create(object_,NULL,start);
		}

		void        Thread::create(ThreadMainObject&object_, bool start)
		{
			create(&object_,NULL,start);
		}

		void        Thread::start()
		{
			if (operation.executing)
				return;
			onComplete.Reset();
			#if SYSTEM==WINDOWS
				if (handle != NULL)
					CloseHandle(handle);
				handle = CreateThread(NULL , 0 , thread_exec , this , 0 , NULL);        //CREATE_SUSPENDED instead of 0?
				operation.executing = handle!=NULL;
			#elif SYSTEM==UNIX
				operation.executing = !pthread_create(&handle,NULL,thread_exec,this);
			#endif
		}





		thread_handle_t        Thread::startAndForget(void (*func_pointer)(void*), void*parameter)
		{
			thread_handle_t rs;
			#if SYSTEM==WINDOWS
				CloseHandle(CreateThread(NULL , 0 ,  (LPTHREAD_START_ROUTINE)func_pointer , parameter , 0 , &rs));
			#elif SYSTEM==UNIX
				pthread_create(&rs,NULL,(_threadPointer)func_pointer,parameter);
			#endif
			return rs;
		}


		/*
		void        Thread::suspend()
		{
			cleanUp();
			#if SYSTEM==WINDOWS
				if (!handle)
					return;
				SuspendThread(handle);
			#elif SYSTEM==UNIX
				if (!started)
					return;
				thr_suspend(handle);
			#endif
		}

		void        Thread::resume()
		{
			cleanUp();
			#if SYSTEM==WINDOWS
				if (!handle)
					return;
				ResumeThread(handle);
			#elif SYSTEM==UNIX
				if (!started)
					return;
				thr_continue(handle);
			#endif
		}
		*/
	
	
		void        Thread::terminate()
		{
			#if SYSTEM==WINDOWS
				if (handle == NULL)
					return;
				//CancelSynchronousIo(handle);
				if (cancelSynchronousIo)
					cancelSynchronousIo(handle);
				if (operation.executing)
				{
					THREAD_REPORT("killing thread "<<((void*)handle));
					TerminateThread(handle , 0);
				}
				#ifndef __GNUC__
				__try
				#endif
				{
					if (!application_shutting_down)	//i wonder if this could cause issues beyond application scope. documentation says handles are closed when the application is terminated anyway
	            		CloseHandle(handle);
				}
				#ifndef __GNUC__
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
				  // don't know. don't care. shouldn't happen
				}
				#endif
				handle = NULL;
				operation.executing = false;
			#elif SYSTEM==UNIX
				if (operation.executing)
				{
					THREAD_REPORT("killing thread "<<((void*)handle))
					pthread_kill(handle,0);
				}
				operation.executing = false;
			#endif
		}

		bool    Thread::isActive() const
		{
			return operation.executing;
		}

		bool        Thread::isSelf()
		{
			#if SYSTEM==WINDOWS
				return GetCurrentThreadId() == GetThreadId(handle);
				//return false; //no idea how to do this yet
			#elif SYSTEM==UNIX
				return operation.executing && pthread_equal(pthread_self(),handle);
			#endif
		}

		
		bool        Thread::AwaitCompletion(DWORD maxWaitMilliseconds/*=0xFFFFFFFF*/)
		{
			if (!operation.executing)
				return true;
			ASSERT__(!isSelf());	//waiting for self doesn't work so well (or at all)
			#if SYSTEM==WINDOWS
				if (handle == NULL)
					FATAL__("bad state");
				DWORD waitingForThreadID = GetThreadId(handle);	//in case it 
				DWORD rs = WaitForSingleObject(handle,maxWaitMilliseconds);
				if (rs == WAIT_TIMEOUT)
				{
					return false;
				}
				elif (rs != 0)
				{
					//FATAL__("Some unknown error occFailed to await completion of thread in allotted amount of time");
				}

				CloseHandle(handle);
				handle = NULL;
			#elif SYSTEM==UNIX
				pthread_join(handle,NULL);
				handle = 0;
			#endif
			operation.executing = false;
			return true;
		}
		ThreadObject::ThreadObject()
		{
			Thread::create(this,false);
		}





		Semaphore::Semaphore(long initial_value)
		{
			#if SYSTEM==WINDOWS
				handle =    CreateSemaphore( NULL, initial_value, LONG_MAX, NULL );
			#elif SYSTEM==UNIX
				THREAD_REPORT("semaphore created with initial value="<<initial_value);
				handle = semget(IPC_PRIVATE, 1, SEM_R|SEM_A);
				semctl(handle,0,SETVAL,initial_value);
			#endif


		}

		Semaphore::~Semaphore()
		{
			#if SYSTEM==WINDOWS
				ReleaseSemaphore(handle,1,NULL);
				CloseHandle(handle);
			#elif SYSTEM==UNIX
				THREAD_REPORT("destroying semaphore");
				semctl(handle,0,IPC_RMID);
			#endif
		}

		void        Semaphore::enter()
		{
			#if SYSTEM==WINDOWS
				WaitForSingleObject(handle, INFINITE );
			#elif SYSTEM==UNIX
				THREAD_REPORT("attempting to decrement semaphore value");
				sembuf op;
				op.sem_num = 0;
				op.sem_op = -1;
				op.sem_flg = 0;
				semop(handle,&op,1);
			#endif
		}

		bool        Semaphore::tryEnter()
		{
			#if SYSTEM==WINDOWS
				return WaitForSingleObject(handle,0L)==WAIT_OBJECT_0;
			#elif SYSTEM==UNIX
				THREAD_REPORT("trying to decrement semaphore value");
				sembuf op;
				op.sem_num = 0;
				op.sem_op = -1;
				op.sem_flg = IPC_NOWAIT;
				return semop(handle,&op,1)==0;
			#endif
		}

		void        Semaphore::leave()
		{
			#if SYSTEM==WINDOWS
				ReleaseSemaphore(handle,1,0);
			#elif SYSTEM==UNIX
				THREAD_REPORT("incrementing semaphore value");
				sembuf op;
				op.sem_num = 0;
				op.sem_op = 1;
				op.sem_flg = 0;
				semop(handle,&op,1);
			#endif
		}


		void        Semaphore::release(unsigned inc_by)
		{
			#if SYSTEM==WINDOWS
				ReleaseSemaphore(handle,inc_by,0);
			#elif SYSTEM==UNIX
				THREAD_REPORT("releasing semaphore by "<<inc_by);
				sembuf op;
				op.sem_num = 0;
				op.sem_op = inc_by;
				op.sem_flg = 0;
				semop(handle,&op,1);
			#endif
		}



		Mutex::Mutex()
		{
			#if SYSTEM==WINDOWS
				handle =    CreateMutex( NULL, 0, NULL );
			#elif SYSTEM==UNIX
				pthread_mutex_init(&handle,NULL);
			#endif
		}

		Mutex::~Mutex()
		{
			#if SYSTEM==WINDOWS
				ReleaseMutex(handle);
				CloseHandle(handle);
			#elif SYSTEM==UNIX
				pthread_mutex_destroy(&handle);
			#endif
		}

		void        Mutex::lock()
		{
			#if SYSTEM==WINDOWS
				WaitForSingleObject(handle, INFINITE );
			#elif SYSTEM==UNIX
				pthread_mutex_lock(&handle);
			#endif
		}

		bool        Mutex::tryLock()
		{
			#if SYSTEM==WINDOWS
				return WaitForSingleObject(handle,0L)==WAIT_OBJECT_0;
			#elif SYSTEM==UNIX
				return !pthread_mutex_trylock(&handle);
			#endif
		}
	
		bool        Mutex::tryLock(long milliseconds)
		{
			#if SYSTEM==WINDOWS
				return WaitForSingleObject(handle,milliseconds)==WAIT_OBJECT_0;
			#elif SYSTEM==UNIX
				for (long i = 0; i < milliseconds; i++)	//somehow i like the microsoft implementation better...
				{
					if (!pthread_mutex_trylock(&handle))
						return true;
					sleep(1);
				}
				return !pthread_mutex_trylock(&handle);
				/*	//would be neat, wouldn't it?:
				struct timespec deltatime;
				deltatime.tv_sec = 0;
				deltatime.tv_nsec = milliseconds*1000;		
				return !pthread_mutex_timedlock_np(&handle,&deltatime);
				*/
			#endif
		}

		void        Mutex::release()
		{
			#if SYSTEM==WINDOWS
				ReleaseMutex(handle);
			#elif SYSTEM==UNIX
				pthread_mutex_unlock(&handle);
			#endif
		}

		void		Mutex::unlock()
		{
			release();
		}

		void		Mutex::p()
		{
			lock();
		}

		void		Mutex::P()
		{
			lock();
		}

		void		Mutex::v()
		{
			release();
		}

		void		Mutex::V()
		{
			release();
		}


		Event::Event()
		{
			#if SYSTEM==WINDOWS
				handle =    CreateEvent( NULL, FALSE, FALSE, NULL );
			#elif SYSTEM==UNIX
				pthread_cond_init(&condition, NULL);
				pthread_mutex_init(&mutex,NULL);
			#endif
		}

		Event::~Event()
		{
			#if SYSTEM==WINDOWS
				CloseHandle(handle);
			#elif SYSTEM==UNIX
				pthread_cond_destroy(&condition);
				pthread_mutex_destroy(&mutex);
			#endif
		}

		void Event::trigger()
		{
	//        cout << " - signaling\n";
			#if SYSTEM==WINDOWS
				SetEvent(handle);
			#elif SYSTEM==UNIX
				pthread_cond_signal(&condition);
			#endif
	//        cout << " - signaled\n";
		}

		void Event::signal()
		{
			trigger();
		}

		void  Event::wait()
		{
	//        cout << " - waiting\n";
			#if SYSTEM==WINDOWS
				WaitForSingleObject(handle, INFINITE );
			#elif SYSTEM==UNIX
				pthread_mutex_lock(&mutex);
				pthread_cond_wait(&condition,&mutex);
				pthread_mutex_unlock(&mutex);
			#endif
	//        cout << " - returning\n";
		}

		#if SYSTEM==UNIX
			void        Event::wait(Mutex&remote_mutex)         //!< Waits for the event to be signaled using mutex (linux only)
			{
	//            cout << " - waiting(m)\n";
				pthread_cond_wait(&condition,&remote_mutex.handle);
	//            cout << " - returning(m)\n";
			}
		#endif

	
	
		Signal::Signal():grantCount(0)
		{}
	
		Signal::~Signal()
		{}
	
		void	Signal::wait()
		{
			count_t set;
			do
			{
				Semaphore::request();
				set = --grantCount;
			}
			while (set > 0);
			//signalCheck = false;
			//mutex.lock();
			//	if (signaled)
			//		signaled = false;
			//mutex.release();
			//Semaphore::request();
		}
	
		void	Signal::signal()
		{
			++grantCount;
			Semaphore::grant();

			//mutex.lock();
			//	if (signaled)
			//	{
			//		mutex.release();
			//		return;
			//	}
			//	signaled = true;
			//	Semaphore::release();
			//mutex.release();
		}
	
		void	Signal::reset()
		{
			while (grantCount--)
				Semaphore::request();
			//bool ref = true;
			//if (signalCheck.compare_exchange_strong(ref,false))
			//{
			//	Semaphore::request();
			//}

			//mutex.lock();
			//	if (signaled)
			//		Semaphore::enter();
			//	signaled = false;
			//mutex.release();
		}

		void	Signal::trigger()
		{
			signal();
		}

		bool	Signal::isSignaled()	const
		{
			return grantCount > 0;
		}
	
		bool	Signal::isTriggered()	const
		{
			return grantCount > 0;
		}

	
	
	
	
	
	
		Barrier::Barrier(unsigned threads):num_threads(threads),counter(0)
		{}

		void	Barrier::wait()
		{
			mutex.lock();
				counter++;
				bool do_wait = counter < num_threads;
				if (!do_wait)
					counter = 0;
			mutex.release();

			if (do_wait)
				semaphore.enter();
			else
				semaphore.release(num_threads-1);
		}




		ReadWriteMutex::ReadWriteMutex(unsigned max):semaphore(max),max_readers(max),lockedDown(false)
		{}

		void        ReadWriteMutex::BeginRead()
		{
			semaphore.enter();
		}

		void        ReadWriteMutex::EndRead()
		{
			semaphore.leave();
		}

		bool        ReadWriteMutex::BeginWrite(bool lockdown /*= false*/)
		{
			mutex.lock();
				if (lockedDown)
				{
					mutex.unlock();
					return false;
				}
				lockedDown = lockdown;
				for (unsigned i = 0; i < max_readers; ++i)
					semaphore.enter();
			mutex.release();
			return true;
		}

		void        ReadWriteMutex::EndWrite()
		{
			semaphore.release(max_readers);
			lockedDown = false;
		}


		CounterObject::CounterObject():count(0)
		{}

		void    CounterObject::operator++(int)
		{
			mutex.lock();
	//                 if (!count)
	//                     zero_mutex.lock();
				count++;
			mutex.release();
		}

		void    CounterObject::operator--(int)
		{
			mutex.lock();
				if (!--count)
					zero_event.trigger();
	//                     zero_mutex.release();
			mutex.release();
		}

		void    CounterObject::operator+=(unsigned delta)
		{
			mutex.lock();
	//                 if (!count)
	//                     zero_mutex.lock();
				count += delta;
			mutex.release();
		}

		void     CounterObject::operator-=(unsigned delta)
		{
			mutex.lock();
				count -= delta;
				if (!count)
						zero_event.trigger();
	//                     zero_mutex.release();
			mutex.release();
		}

		CounterObject::operator    unsigned()
		{
			mutex.lock();
				unsigned cnt = count;
			mutex.release();
			return cnt;
		}

		void        CounterObject::waitForZero()
		{
			#if SYSTEM==UNIX
				mutex.lock();
					if (!count)
					{
						mutex.release();
						return;
					}
					zero_event.wait(mutex);
				mutex.release();

			#else
				zero_event.wait();
			#endif
		}
	
	
		ThreadPoolJob::ThreadPoolJob(ThreadMainObject*Object,Thread::pMethod Method):obj0(Object),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(Method),context(&ThreadPool::global_context)
		{}
	
		ThreadPoolJob::ThreadPoolJob(ThreadMainObject&Object,Thread::pMethod Method):obj0(&Object),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(Method),context(&ThreadPool::global_context)
		{}
	
		ThreadPoolJob::ThreadPoolJob(ThreadMainObject*object_,ThreadPoolContext*context_):obj0(object_),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(NULL),context(context_)
		{}
	
		ThreadPoolJob::ThreadPoolJob(ThreadMainObject&object_,ThreadPoolContext*context_):obj0(&object_),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(NULL),context(context_)
		{}
	
		ThreadPoolJob::ThreadPoolJob(ThreadMainObject*object_,Thread::pMethod method_, ThreadPoolContext*context_):obj0(object_),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(method_),context(context_)
		{}
	
		ThreadPoolJob::ThreadPoolJob(ThreadMainObject&object_,Thread::pMethod method_, ThreadPoolContext*context_):obj0(&object_),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(method_),context(context_)
		{}
	
		ThreadPoolJob::ThreadPoolJob():obj0(NULL),
		#if QUAD_JOBS
			obj1(NULL),obj2(NULL),obj3(NULL),
		#endif
		method(NULL),context(&ThreadPool::global_context)
		{}

		#if QUAD_JOBS
   		ThreadPoolJob::ThreadPoolJob(ThreadMainObject*object0,ThreadMainObject*object1,ThreadMainObject*object2,ThreadMainObject*object3,Thread::pMethod method_, ThreadPoolContext*context_):obj0(object0),obj1(object1),obj2(object2),obj3(object3),method(method_),context(context_)
		{}
		#endif


		ThreadPool::ThreadPool(count_t num_threads,ThreadMainObject*init_object_, Thread::pMethod init_method_):Parent(num_threads,init_object_,init_method_)
		{}

	
		void		JobSequence::split(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context)
		{
			if (!objects.Count())
				return;
			THREAD_REPORT("enqueueing a total of "<<objects.Count()<<" job(s)")
			#if QUAD_JOBS
				count_t rest = objects.Count()%4;
				context->counter_mutex.lock();
					(context->counter)+=objects.Count()/4;
					if (rest)
						(context->counter)++;
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				for (unsigned i = 0; i < objects.Count()/4; i++)
					pool.job_pipe.writeElement(Job(objects[i*4],objects[i*4+1],objects[i*4+2],objects[i*4+3],method,context));
				if (rest)
				{
					index_t offset = objects.Count()-rest;
					Job job(objects[offset],method,context);
					for (index_t i = 1; i < rest; i++)
					{
						job.object[i] = objects[offset+i];
					}
					pool.job_pipe.writeElement(job);
				}			
			#else
				context->counter_mutex.lock();
					(context->counter)+=objects.Count();
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				/*for (unsigned i = 0; i < objects.Count(); i++)
					job_pipe << Job(objects[i],method,context);*/
				for (unsigned i = 0; i < objects.Count(); i++)
					pool.job_pipe.writeElement(Job(objects[i],method,context));
			#endif
			THREAD_REPORT("enqueued")
	
		}
	
		void		JobSequence::split(Ctr::Array<ThreadMainObject*>&objects,count_t count, Thread::pMethod method, ThreadPoolContext*context)
		{
			if (!count)
				return;
			ASSERT__(count <= objects.Count());
			THREAD_REPORT("enqueueing a total of "<<count<<" job(s)")
			#if QUAD_JOBS
				count_t rest = count%4;
				context->counter_mutex.lock();
					(context->counter)+=count/4;
					if (rest)
						(context->counter)++;
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				for (index_t i = 0; i < count/4; i++)
					pool.job_pipe.writeElement(Job(objects[i*4],objects[i*4+1],objects[i*4+2],objects[i*4+3],method,context));
				if (rest)
				{
					index_t offset = count-rest;
					Job job(objects[offset],method,context);
					for (index_t i = 1; i < rest; i++)
					{
						job.object[i] = objects[offset+i];
					}
					pool.job_pipe.writeElement(job);
				}			
			#else
				context->counter_mutex.lock();
					(context->counter)+=count;
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				/*for (unsigned i = 0; i < objects.Count(); i++)
					job_pipe << Job(objects[i],method,context);*/
				for (index_t i = 0; i < count; i++)
					pool.job_pipe.writeElement(Job(objects[i],method,context));
			#endif
			THREAD_REPORT("enqueued")
	
		}
		void			JobSequence::splitIndividually(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context)
		{
			if (!objects.Count())
				return;
			THREAD_REPORT("enqueueing a total of "<<objects.Count()<<" job(s)")
			context->counter_mutex.lock();
				(context->counter)+=objects.Count();
			
				THREAD_REPORT("counter incremented to "<<context->counter)
			context->counter_mutex.release();
			for (unsigned i = 0; i < objects.Count(); i++)
				pool.job_pipe.writeElement(Job(objects[i],method,context));
			THREAD_REPORT("enqueued")
		}

	
		void			ThreadPool::split(Ctr::Array<ThreadMainObject*>&objects,count_t count, Thread::pMethod method, ThreadPoolContext*context)
		{
			JobSequence	sequence(*this);
			sequence.split(objects,count,method,context);
		}
		
		void			ThreadPool::split(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context)
		{
			JobSequence	sequence(*this);
			sequence.split(objects,method,context);
		
			#if 0
			if (!objects.Count())
				return;
			THREAD_REPORT("enqueueing a total of "<<objects.Count()<<" job(s)")
			#if QUAD_JOBS
				unsigned rest = objects.Count()%4;
				context->counter_mutex.lock();
					(context->counter)+=objects.Count()/4;
					if (rest)
						(context->counter)++;
				
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				/*for (unsigned i = 0; i < objects.Count(); i++)
					job_pipe << Job(objects[i],method,context);*/
				job_pipe.beginSequence();
				for (unsigned i = 0; i < objects.Count()/4; i++)
					job_pipe.writeElement(Job(objects[i*4],objects[i*4+1],objects[i*4+2],objects[i*4+3],method,context));
				if (rest)
				{
					unsigned offset = objects.Count()-rest;
					Job job(objects[offset],method,context);
					for (unsigned i = 1; i < rest; i++)
					{
						job.object[i] = objects[offset+i];
					}
					job_pipe.writeElement(job);
				}			
				job_pipe.endSequence();
			#else
				context->counter_mutex.lock();
					(context->counter)+=objects.Count();
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				/*for (unsigned i = 0; i < objects.Count(); i++)
					job_pipe << Job(objects[i],method,context);*/
				job_pipe.beginSequence();
				for (unsigned i = 0; i < objects.Count(); i++)
					job_pipe.writeElement(Job(objects[i],method,context));
				job_pipe.endSequence();
			#endif
			THREAD_REPORT("enqueued")
			#endif
		}
	
		void			ThreadPool::splitIndividually(Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context)
		{
			if (!objects.Count())
				return;
			THREAD_REPORT("enqueueing a total of "<<objects.Count()<<" job(s)")
			context->counter_mutex.lock();
				(context->counter)+=objects.Count();
			
				THREAD_REPORT("counter incremented to "<<context->counter)
			context->counter_mutex.release();
			/*for (unsigned i = 0; i < objects.Count(); i++)
				job_pipe << Job(objects[i],method,context);*/
			job_pipe.beginSequence();
			for (unsigned i = 0; i < objects.Count(); i++)
				job_pipe.writeElement(Job(objects[i],method,context));
			job_pipe.endSequence();

			THREAD_REPORT("enqueued")
		}

		void			ThreadPool::Process(const Job&job)
		{
			THREAD_REPORT("enqueueing singular job")
			job.context->counter_mutex.lock();
				(job.context->counter)++;
				THREAD_REPORT("counter now "<<job.context->counter)
			job.context->counter_mutex.release();
			job_pipe << job;
			THREAD_REPORT("job enqueued")
		}
	
		void			ThreadPool::Process(ThreadMainObject*object,Thread::pMethod method)
		{
			Process(Job(object,method));
		}
	
		void			ThreadPool::Process(ThreadMainObject&object,Thread::pMethod method)
		{
			Process(Job(object,method));
		}
	
		void			ThreadPool::Process(ThreadMainObject*object,ThreadPoolContext*context)
		{
			Process(Job(object,context));
		}
	
		void			ThreadPool::Process(ThreadMainObject&object,ThreadPoolContext*context)
		{
			Process(Job(object,context));
		}
	
		void			ThreadPool::Process(ThreadMainObject*object,Thread::pMethod method, ThreadPoolContext*context)
		{
			Process(Job(object,method,context));
		}
	
		void			ThreadPool::Process(ThreadMainObject&object,Thread::pMethod method, ThreadPoolContext*context)
		{
			Process(Job(object,method,context));
		}
	
		void			ThreadPool::process4(ThreadMainObject*obj0,ThreadMainObject*obj1,ThreadMainObject*obj2,ThreadMainObject*obj3,Thread::pMethod method, ThreadPoolContext*context)
		{
			#if QUAD_JOBS
				Process(Job(obj0,obj1,obj2,obj3,method,context));
			#else
				THREAD_REPORT("enqueueing quad job")
				context->counter_mutex.lock();
					(context->counter)+=4;
					THREAD_REPORT("counter now "<<context->counter)
				context->counter_mutex.release();
				job_pipe.beginSequence();
					job_pipe.writeElement(Job(obj0,method,context));
					job_pipe.writeElement(Job(obj1,method,context));
					job_pipe.writeElement(Job(obj2,method,context));
					job_pipe.writeElement(Job(obj3,method,context));
				job_pipe.endSequence();
				THREAD_REPORT("job enqueued")
		
			#endif
		}
	
	

		ThreadPool&	ThreadPool::operator<<(const Job&job)
		{
			Process(job);
			return *this;
		}




	
	
	

		PriorityThreadPool::PriorityThreadPool(count_t num_threads,ThreadMainObject*init_object_, Thread::pMethod init_method_):Parent(num_threads,init_object_,init_method_)
		{}

	

	
	
		void			PriorityThreadPool::split(const Ctr::Array<int>&priorities, Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context)
		{
			if (!objects.Count() || priorities.Count() != objects.Count())
				return;
			THREAD_REPORT("enqueueing a total of "<<objects.Count()<<" job(s)")
			#if QUAD_JOBS
				count_t rest = objects.Count()%4;
				context->counter_mutex.lock();
					(context->counter)+=objects.Count()/4;
					if (rest)
						(context->counter)++;
				
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				job_pipe.beginSequence();
				for (index_t i = 0; i < objects.Count()/4; i++)
					job_pipe.writeElement(Job(objects[i*4],objects[i*4+1],objects[i*4+2],objects[i*4+3],method,context),std::max(std::max(priorities[i*4],priorities[i*4+1]),std::max(priorities[i*4+2],priorities[i*4+3])));
				if (rest)
				{
					index_t offset = objects.Count()-rest;
					int priority = priorities[offset];
					Job job(objects[offset],method,context);
					for (index_t i = 1; i < rest; i++)
					{
						job.object[i] = objects[offset+i];
						priority+=priorities[offset+i];
					}
					job_pipe.writeElement(job,int(priority/rest));
				}
				job_pipe.endSequence();
			#else
				context->counter_mutex.lock();
					(context->counter)+=objects.Count();
				
					THREAD_REPORT("counter incremented to "<<context->counter)
				context->counter_mutex.release();
				job_pipe.beginSequence();
				for (unsigned i = 0; i < objects.Count(); i++)
					job_pipe.writeElement(Job(objects[i],method,context),priorities[i]);
				job_pipe.endSequence();
			#endif
			THREAD_REPORT("enqueued")
		}
	
		void			PriorityThreadPool::splitIndividually(const Ctr::Array<int>&priorities, Ctr::Array<ThreadMainObject*>&objects,Thread::pMethod method, ThreadPoolContext*context)
		{
			if (!objects.Count() || priorities.Count() != objects.Count())
				return;
			THREAD_REPORT("enqueueing a total of "<<objects.Count()<<" job(s)")
			context->counter_mutex.lock();
				(context->counter)+=objects.Count();
			
				THREAD_REPORT("counter incremented to "<<context->counter)
			context->counter_mutex.release();
			job_pipe.beginSequence();
			for (unsigned i = 0; i < objects.Count(); i++)
				job_pipe.writeElement(Job(objects[i],method,context),priorities[i]);
			job_pipe.endSequence();
			THREAD_REPORT("enqueued")
		}

		void			PriorityThreadPool::Process(int priority, const Job&job)
		{
			THREAD_REPORT("enqueueing singular job")
			job.context->counter_mutex.lock();
				(job.context->counter)++;
				THREAD_REPORT("counter now "<<job.context->counter)
			job.context->counter_mutex.release();
			job_pipe.write(job,priority);
			THREAD_REPORT("job enqueued")
		}
	
		void			PriorityThreadPool::Process(int priority, ThreadMainObject*object,Thread::pMethod method)
		{
			Process(priority, Job(object,method));
		}
	
		void			PriorityThreadPool::Process(int priority, ThreadMainObject&object,Thread::pMethod method)
		{
			Process(priority,Job(object,method));
		}
	
		void			PriorityThreadPool::Process(int priority, ThreadMainObject*object,ThreadPoolContext*context)
		{
			Process(priority,Job(object,context));
		}
	
		void			PriorityThreadPool::Process(int priority, ThreadMainObject&object,ThreadPoolContext*context)
		{
			Process(priority,Job(object,context));
		}
	
		void			PriorityThreadPool::Process(int priority, ThreadMainObject*object,Thread::pMethod method, ThreadPoolContext*context)
		{
			Process(priority,Job(object,method,context));
		}
	
		void			PriorityThreadPool::Process(int priority, ThreadMainObject&object,Thread::pMethod method, ThreadPoolContext*context)
		{
			Process(priority,Job(object,method,context));
		}
	
		void			PriorityThreadPool::process4(int priority, ThreadMainObject*obj0,ThreadMainObject*obj1,ThreadMainObject*obj2,ThreadMainObject*obj3,Thread::pMethod method, ThreadPoolContext*context)
		{
			#if QUAD_JOBS
				Process(priority,Job(obj0,obj1,obj2,obj3,method,context));
			#else
				THREAD_REPORT("enqueueing quad job")
				context->counter_mutex.lock();
					(context->counter)+=4;
					THREAD_REPORT("counter now "<<context->counter)
				context->counter_mutex.release();
				job_pipe.beginSequence();
					job_pipe.writeElement(Job(obj0,method,context),priority);
					job_pipe.writeElement(Job(obj1,method,context),priority);
					job_pipe.writeElement(Job(obj2,method,context),priority);
					job_pipe.writeElement(Job(obj3,method,context),priority);
				job_pipe.endSequence();
				THREAD_REPORT("job enqueued")
		
			#endif		
		}

	
		long				AtomicLong::Set(long value_)
		{
			#if SYSTEM==WINDOWS
				return InterlockedExchange(&value,value_);
			#else
				return value.exchange(value_);
			#endif
		}


		AtomicLong&			AtomicLong::operator=(long value_)
		{
			#if SYSTEM==WINDOWS
				InterlockedExchange(&value,value_);
			#else
				value = value_;
			#endif
			return *this;
		}
	
		long			AtomicLong::operator++()
		{
			#if SYSTEM==WINDOWS
				return InterlockedIncrement(&value);
			#else
				return ++value;
			#endif
		}
	
		long			AtomicLong::operator++(int)
		{
			#if SYSTEM==WINDOWS
				return InterlockedIncrement(&value);
			#else
				return value++;
			#endif
		}
	
		long			AtomicLong::operator--()
		{
			#if SYSTEM==WINDOWS
				return InterlockedDecrement(&value);
			#else
				return --value;
			#endif
		}
	
		long			AtomicLong::operator--(int)
		{
			#if SYSTEM==WINDOWS
				return InterlockedDecrement(&value);
			#else
				return value--;
			#endif
		}
	
		long			AtomicLong::operator+=(long value_)
		{
			#if SYSTEM==WINDOWS
				InterlockedExchangeAdd(&value,value_);
			#else
				value += value_;
			#endif
			return value;
		}
	
		long			AtomicLong::operator-=(long value_)
		{
			#if SYSTEM==WINDOWS
				InterlockedExchangeAdd(&value,-value_);
			#else
				value -= value_;
			#endif
			return value;
		}
			
	#if 0
	
		ParallelLoop::Worker::Worker():parent(NULL)
		{}

		void ParallelLoop::Worker::init()
		{
			if (parent->set_affinity)
			{
				#if SYSTEM==WINDOWS
					parent->mutex.lock();
						SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR(1)<<DWORD_PTR(parent->thread_counter++));	//this will work for up to 64 threads. hmm...
					parent->mutex.release();
				#elif SYSTEM==LINUX
					parent->mutex.lock();
						int index = (parent->thread_counter++);
						cpu_set_t	cpu_set;
						CPU_ZERO(&cpu_set);
						CPU_SET(index,&cpu_set);
						if (sched_setaffinity(0,sizeof(cpu_set),&cpu_set))
							cout << "Warning: CPU affinity set failed for processor #"<<index<<"\n";
					parent->mutex.release();
				#endif
			}
			if (parent->init_object)
			{

				if (parent->init_method)
					(parent->init_object->*parent->init_method)();
				else
					parent->init_object->ThreadMain();
			}
		}

	
		void ParallelLoop::Worker::ThreadMain()
		{
			init();


			while (!application_shutting_down)
			{
				THREAD_REPORT("waiting for jobs");
				parent->job_semaphore.enter();
				/*parent->mutex.lock();
					const unsigned offset = parent->offset++;
				parent->mutex.release();*/
			
				offset_t offset = InterlockedIncrement(parent->aligned_offset);

			
				if ((unsigned)offset > parent->num_jobs)
				{
					parent->destruct_semaphore.release();
					THREAD_REPORT("Job offset exceeds maximum. Exiting thread");
					return;
				}
				bool final = offset == parent->num_jobs;
				offset--;
			
				const unsigned	iterations = parent->iterations_per_job;
				unsigned		at = offset * iterations;
				const unsigned	end = at + (final?parent->last_job_iterations:iterations);
			
				THREAD_REPORT("processing range "<<at<<" .. "<<end);
			
			
				while (at < end)
				{
					parent->callback(at++);
				}
				bool terminal = !parent->terminal_semaphore.tryEnter();
				/*parent->mutex.lock();
					bool terminal = !--parent->jobs_left;
				parent->mutex.release();*/
			
				if (terminal)
				{
					THREAD_REPORT("sending terminal symbol");
					int token;
					parent->wait_pipe<<token;
				}
				THREAD_REPORT("loop completed");
			
			}
		}
	
		ParallelLoopBase::ParallelLoopBase(bool reduced):thread_counter(0),num_jobs(0),last_job_iterations(0),set_affinity(!reduced),init_object(NULL),init_method(NULL)
		{}
			
		ParallelLoop::ParallelLoop(ThreadMainObject*init_object_, Thread::pMethod init_method_, bool reduced):ParallelLoopBase(reduced),iterations_per_job(64)
		{
			init_object = init_object_;
			init_method = init_method_;
			volatile BYTE*address = offset_field;
			#ifdef __GNUC__
				while (((long)address)&0x3)
					address++;
			#else
				while (((__int64)address)&0x3)
					address++;
			#endif
		
			aligned_offset = (offset_t*)address;
			(*aligned_offset) = 0;
	
			unsigned num_threads = getProcessorCount();
			if (reduced && num_threads>1)
				num_threads--;
			workers.SetSize(num_threads);
		
		

			for (unsigned i = 0; i < num_threads; i++)
			{
				THREAD_REPORT("starting thread "<<i)
			   // workers[i].operation_counter = 0;
				workers[i].parent = this;;
				workers[i].start();
			}
		}
	
	
		ParallelLoopBase::~ParallelLoopBase()
		{
		}

		ParallelLoop::~ParallelLoop()
		{
			num_jobs = 0;
			(*aligned_offset) = 1;
			job_semaphore.release(unsigned(workers.Count()));
			for (index_t i = 0; i < workers.Count(); i++)
				destruct_semaphore.enter();
		}

	
		void		ParallelLoop::execute(pCallback function, unsigned iterations, unsigned iterations_per_job)
		{
			if (!function)
				return;
			(*aligned_offset) = 0;
			num_jobs = iterations/iterations_per_job;
			last_job_iterations = iterations%iterations_per_job;

			if (last_job_iterations)
				num_jobs++;
			else
				last_job_iterations = iterations_per_job;
			if (!num_jobs)
				return;
			if (num_jobs > 1)
			{
				/*if (unsigned remainder = num_jobs % workers.Count())
				{
					if (remainder < workers.Count()/2)	//remove jobs
					{
					
				
				
					}
					else	//add jobs
					{
				
				
					}
			
			
			
				}*/
				/*
				if (last_job_iterations+num_jobs <= iterations_per_job)
				{
					unsigned balance_steps = ((iterations_per_job-last_job_iterations)/(num_jobs));
					iterations_per_job -= balance_steps;
					last_job_iterations += balance_steps*(num_jobs-1);
				}
				*/
				callback = function;
			
				this->iterations_per_job = iterations_per_job;
				//jobs_left = num_jobs;
				THREAD_REPORT("configured. releasing "<<num_jobs<<" jobs");
				terminal_semaphore.release(num_jobs-1);
				job_semaphore.release(num_jobs);
			
				int token;
				wait_pipe >> token;
				THREAD_REPORT("token received");
			}
			else
			{
				for (unsigned i = 0; i < iterations; i++)
					function(i);
			}
		
		}


	#endif

	
	

	
		ParallelLoopBase::ParallelLoopBase():thread_counter(0),num_jobs(0),last_job_iterations(0),set_affinity(true),init_object(NULL),init_method(NULL)
		{}
			
	
	
		ParallelLoopBase::~ParallelLoopBase()
		{}








	
			ParallelLoop::Worker::Worker():parent(NULL)
			{}

			void ParallelLoop::Worker::Init()
			{
				if (parent->set_affinity)
				{
					#if SYSTEM==WINDOWS
						parent->mutex.lock();
							SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR(1)<<DWORD_PTR(parent->thread_counter++));	//this will work for up to 64 threads. hmm...
						parent->mutex.release();
					#elif SYSTEM==LINUX
						parent->mutex.lock();
							int index = (parent->thread_counter++);
							cpu_set_t	cpu_set;
							CPU_ZERO(&cpu_set);
							CPU_SET(index,&cpu_set);
							if (sched_setaffinity(0,sizeof(cpu_set),&cpu_set))
								std::cerr << "Warning: CPU affinity set failed for processor #"<<index<<"\n";
						parent->mutex.release();
					#endif
				}
				if (parent->init_object)
				{

					if (parent->init_method)
						(parent->init_object->*parent->init_method)();
					else
						parent->init_object->ThreadMain();
				}
			}

	
			void ParallelLoop::Worker::ThreadMain()
			{
				Init();


				while (!application_shutting_down)
				{
					THREAD_REPORT("waiting for jobs");
					parent->job_semaphore.enter();
					/*parent->mutex.lock();
						const unsigned offset = parent->offset++;
					parent->mutex.release();*/
			
					//offset_t offset = InterlockedIncrement(parent->aligned_offset);

					#if SYSTEM==WINDOWS
						offset_t offset = InterlockedIncrement(parent->aligned_offset);
					#else
						offset_t offset = ++parent->jobOffset;
					#endif
				
			
					if ((unsigned)offset > parent->num_jobs)
					{
						if (application_shutting_down)
							return;
						parent->destruct_semaphore.release();
						THREAD_REPORT("Job offset exceeds maximum. Exiting thread");
						return;
					}
					bool final = offset == parent->num_jobs;
					offset--;
			
					const count_t	iterations = parent->iterationsPerJob;
					index_t			at = offset * iterations;
					const index_t	end = at + (final?parent->last_job_iterations:iterations);
			
					THREAD_REPORT("processing range "<<at<<" .. "<<end);
			
					parent->op->ExecuteRange(at,end,myIndex);
					bool terminal = !parent->terminal_semaphore.tryEnter();
					/*parent->mutex.lock();
						bool terminal = !--parent->jobs_left;
					parent->mutex.release();*/
			
					if (terminal)
					{
						THREAD_REPORT("sending terminal symbol");
						int token;
						parent->wait_pipe<<token;
					}
					THREAD_REPORT("loop completed");
			
				}
			}

			ParallelLoop::ParallelLoop(ThreadMainObject*init_object_, Thread::pMethod init_method_):iterationsPerJob(64),op(NULL)
			{
				init_object = init_object_;
				init_method = init_method_;
				#if SYSTEM==WINDOWS
					volatile BYTE*address = offset_field;
					while (((__int64)address)&0x3)
						address++;
					aligned_offset = (offset_t*)address;
					(*aligned_offset) = 0;
				#else
					jobOffset = 0;
				#endif
		
	
				count_t num_threads = getProcessorCount();
				workers.SetSize(num_threads);
		
		

				for (index_t i = 0; i < num_threads; i++)
				{
					THREAD_REPORT("starting thread "<<i)
				   // workers[i].operation_counter = 0;
					workers[i].Setup(i,this);
					workers[i].start();
				}
			}



			ParallelLoop::~ParallelLoop()
			{
				num_jobs = 0;
				#if SYSTEM==WINDOWS
					(*aligned_offset) = 1;
				#else
					jobOffset = 1;
				#endif
				job_semaphore.release(unsigned(workers.Count()));
				if (!application_shutting_down)
					for (index_t i = 0; i < workers.Count(); i++)
						destruct_semaphore.enter();
			}

	
			void		ParallelLoop::Execute(Operator*op, count_t iterations, count_t iterationsPerJob)
			{
				if (!op || application_shutting_down)
					return;
				#if SYSTEM==WINDOWS
					(*aligned_offset) = 0;
				#else
					jobOffset = 0;
				#endif
				num_jobs = iterations/iterationsPerJob;
				last_job_iterations = iterations%iterationsPerJob;

				if (last_job_iterations)
					num_jobs++;
				else
					last_job_iterations = iterationsPerJob;
				if (!num_jobs)
					return;
				if (num_jobs > 1)
				{
					this->op = op;
			
					this->iterationsPerJob = iterationsPerJob;
					//jobs_left = num_jobs;
					THREAD_REPORT("configured. releasing "<<num_jobs<<" jobs");
					terminal_semaphore.release(static_cast<unsigned>(num_jobs-1));
					job_semaphore.release(static_cast<unsigned>(num_jobs));
			
					int token;
					wait_pipe >> token;
					THREAD_REPORT("token received");
				}
				else
				{
					op->ExecuteRange(0,iterations,0);
				}
		
			}

		/*static*/	ParallelLoop						ParallelLoop::globalInstance;



		void	ThreadWaitLock::Wake()
		{
			RecursiveMutexLock lock(mutex);
			waking = true;
			wait.release(numWaiting);
			numWaiting = 0;
		}

		void	ThreadWaitLock::Reset()
		{
			RecursiveMutexLock lock(mutex);
			Wake();
			waking = false;
			numWaiting = 0;
		}

		void	ThreadWaitLock::Wait()
		{
			{
				RecursiveMutexLock lock(mutex);

				if (waking)
					return;
				numWaiting++;
			}
			wait.enter();

		}


	}
}
