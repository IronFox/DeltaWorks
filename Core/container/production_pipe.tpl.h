#ifndef production_pipeTplH
#define production_pipeTplH



/******************************************************************

E:\include\list\production_pipe.tpl.h

******************************************************************/

template <typename T,typename MyStrategy>
	inline void WorkPipe<T,MyStrategy>::SignalRead()
	{
		DBG_ASSERT__(!locked);
		//mutex.lock();
		lock.lock();
		readAt = 0;
		locked = true;
	}

template <typename T,typename MyStrategy>
	inline bool WorkPipe<T,MyStrategy>::operator>>(T&out)
	{
		DBG_ASSERT__(locked);
		if (readAt >= Super::Count())
			return false;
		MyStrategy::move(Super::at(readAt++), out);
		return true;
	}

template <typename T,typename MyStrategy>
	inline void  WorkPipe<T,MyStrategy>::ExitRead()
	{
		DBG_ASSERT__(locked);
		Super::clear();
		locked = false;
		lock.unlock();
	}
#if __BUFFER_RVALUE_REFERENCES__
template <typename T,typename MyStrategy>
	inline WorkPipe<T,MyStrategy>&    WorkPipe<T,MyStrategy>::operator<<(T&&element)
	{
		lock.lock();
			Super::Append(std::forward<T&&>(element));
			contentSignal.signal();
		lock.unlock();
		return *this;

	}
#endif


template <typename T,typename MyStrategy>
	template <typename Strategy>
		count_t						WorkPipe<T,MyStrategy>::FlushContentTo(BasicVector<T,Strategy>&destination)
		{
			SignalRead();
				count_t rs = Super::Count();
				destination.MoveAppendAll(*this);
			ExitRead();
			return rs;
		}


template <typename T,typename MyStrategy>
	inline WorkPipe<T,MyStrategy>&    WorkPipe<T,MyStrategy>::operator<<(const T&pntr)
	{
		lock.lock();
			Super::Append(pntr);
			contentSignal.signal();
		lock.unlock();
		return *this;
	}
template <typename T,typename MyStrategy>
	inline WorkPipe<T,MyStrategy>&    WorkPipe<T,MyStrategy>::MoveAppend(T&pntr)
	{
		lock.lock();
			Super::MoveAppend(pntr);
			contentSignal.signal();
		lock.unlock();
		return *this;
	}

template <typename T,typename MyStrategy>
	inline	WorkPipe<T,MyStrategy>&    WorkPipe<T,MyStrategy>::MoveAppend(BasicVector<T>&buffer)
	{
		lock.lock();
			Super::MoveAppend(buffer);
			contentSignal.signal();
		lock.unlock();
		return *this;
	}



template <typename T,typename MyStrategy>
	inline void WorkPipe<T,MyStrategy>::clear()
	{
		lock.lock();
			Super::clear();
		lock.unlock();
	}


#endif
