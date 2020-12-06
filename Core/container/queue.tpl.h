#ifndef queueTplH
#define queueTplH

/******************************************************************

Collection of different queues.

******************************************************************/


template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>::QueueIterator(Element*begin,Element*end,Element*sectionBegin,Element*c):field_begin(begin),field_end(end),sectionBegin(sectionBegin),current(c)
	{}
	
template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>&	QueueIterator<Entry,Element,MyStrategy>::operator++()
	{
		current++;
		if (current == field_end)
			current = field_begin;
		return *this;
	}
	
template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>	QueueIterator<Entry,Element,MyStrategy>::operator++(int)
	{
		It rs(*this);
		operator++();
		return rs;
	}

template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>	QueueIterator<Entry,Element,MyStrategy>::operator+(difference_type delta)	const
	{
		It rs(*this);
		rs.current += delta;
		while (rs.current >= field_end)
			rs.current -= (field_end-field_begin);
		while (rs.current < field_begin)
			rs.current += (field_end-field_begin);
		return rs;
	}
	
template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>&	QueueIterator<Entry,Element,MyStrategy>::operator--()
	{
		current--;
		if (current < field_begin)
			current = field_end-1;
		return *this;
	}

template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>	QueueIterator<Entry,Element,MyStrategy>::operator--(int)
	{
		It rs(*this);
		operator--();
		return rs;
	}

template <class Entry, class Element, class MyStrategy>
	QueueIterator<Entry,Element,MyStrategy>	QueueIterator<Entry,Element,MyStrategy>::operator-(difference_type delta)	const
	{
		It rs(*this);
		rs.current -= delta;
		while (rs.current >= field_end)
			rs.current -= (field_end-field_begin);
		while (rs.current < field_begin)
			rs.current += (field_end-field_begin);
		return rs;
	}
		
	
template <class Entry, class Element, class MyStrategy>
	size_t		QueueIterator<Entry,Element,MyStrategy>::GetIndex()	const
	{
		if (current >= sectionBegin)
			return current - sectionBegin;
		return current-field_begin;
	}
	


template <class Entry,class MyStrategy>
	Queue<Entry,MyStrategy>::Queue(size_t size):Super(size)
	{
		section_begin = section_end = Super::pointer();
		//field_end = section_begin + Super::GetLength();
	}



template <class Entry,class MyStrategy>
	Queue<Entry,MyStrategy>::Queue(const Self&other)
	{
		Super::SetSize(other.Super::Count());
		section_begin = section_end = Super::pointer();
		//field_end = section_begin + Super::GetLength();
		for (index_t i = 0; i < other.Count(); i++)
			(*section_end++).Copy(other[i]);
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
	}

template <class Entry,class MyStrategy>
	Queue<Entry,MyStrategy>::Queue(Self&&other):section_begin(nullptr),section_end(nullptr)//,field_end(nullptr)
	{
		this->adoptData(other);
	}

template <class Entry,class MyStrategy>
	void Queue<Entry,MyStrategy>::operator=(const Queue<Entry,MyStrategy>&other)
	{
		Super::SetSize(other.Super::Count());
		section_begin = section_end = Super::pointer();
		//field_end = section_begin + Super::GetLength();
		for (index_t i = 0; i < other.Count(); i++)
			(*section_end++).Copy(other[i]);
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

	}

template <class Entry,class MyStrategy>
	void Queue<Entry,MyStrategy>::swap(Queue<Entry,MyStrategy>&other)
	{
		Super::swap(other);
		std::swap(section_begin,other.section_begin);
		std::swap(section_end,other.section_end);
		//std::swap(field_end,other.field_end);
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

	}
template <class Entry,class MyStrategy>
	void Queue<Entry,MyStrategy>::adoptData(Queue<Entry,MyStrategy>&other)
	{
		Super::adoptData(other);
		section_begin = other.section_begin;
		section_end = other.section_end;
		//field_end = other.field_end;

		//other.field_end = other.section_begin = other.section_end = NULL;
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

	}



template <class Entry,class MyStrategy>
	typename Queue<Entry,MyStrategy>::iterator	Queue<Entry,MyStrategy>::begin()
	{
		return iterator(Super::begin(),Super::end(),section_begin,section_begin);
	}

template <class Entry,class MyStrategy>
	typename Queue<Entry,MyStrategy>::iterator	Queue<Entry,MyStrategy>::end()
	{
		return iterator(Super::begin(),Super::end(),section_begin,section_end);
	}

template <class Entry,class MyStrategy>
	typename Queue<Entry,MyStrategy>::const_iterator	Queue<Entry,MyStrategy>::begin()	const
	{
		return const_iterator(Super::begin(),Super::end(),section_begin,section_begin);
	}

template <class Entry,class MyStrategy>
	typename Queue<Entry,MyStrategy>::const_iterator	Queue<Entry,MyStrategy>::end()	const
	{
		return const_iterator(Super::begin(),Super::end(),section_begin,section_end);
	}

template <class Entry,class MyStrategy>
	bool			Queue<Entry,MyStrategy>::Contains(const Entry&entry)	const
	{
		foreach (*this,it)
			if (*it == entry)
				return true;
		return false;
	}


template <class Entry,class MyStrategy>
	typename Queue<Entry,MyStrategy>::iterator			Queue<Entry,MyStrategy>::erase(iterator it)
	{
		ASSERT__(it.field_begin == Super::begin());
		ASSERT__(it.field_end == Super::end());
		ASSERT__(it.sectionBegin == section_begin);

		if (it.current >= section_begin)
		{
			const index_t idx = it.current - section_begin;
			if (idx == 0)
			{
				EraseFront();
				return begin();
			}

			for (index_t i = idx; i > 0; i--)
				MyStrategy::move(section_begin[i-1].Cast(),section_begin[i].Cast());
			section_begin->Destruct();
			section_begin++;
		}
		else
		{
			ASSERT__(it.current < section_end);
			const index_t idx = it.current - Super::data;
			const index_t ide = section_end - Super::data;

			for (index_t i = idx; i+1 < ide; i++)
				MyStrategy::move(Super::data[i+1].Cast(),Super::data[i].Cast());
			EraseBack();
		}
		return it;
	}

template <class Entry,class MyStrategy>
	count_t	Queue<Entry,MyStrategy>::Pop(Entry*out_field, count_t count)
	{
		count_t written = 0;
		while (section_begin != section_end && count > 0)
		{
			MyStrategy::move(section_begin->Cast(),*out_field);
			section_begin->Destruct();
			out_field++;
			WrapInc(section_begin);
			count--;
			written++;
			//DBG_ASSERT__(Super::Owns(section_end));
			//DBG_ASSERT__(Super::Owns(section_begin));
		}
		return written;
	}

template <class Entry,class MyStrategy>
	bool	Queue<Entry,MyStrategy>::Pop(Entry&out)
	{
		if (section_begin == section_end)
			return false;
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
		MyStrategy::move(section_begin->Cast(),out);
		section_begin->Destruct();
		WrapInc(section_begin);
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

		return true;
	}

template <class Entry,class MyStrategy>
	Entry	Queue<Entry,MyStrategy>::Pop()
	{
		if (section_begin == section_end)
			return Entry();
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
		Entry out;
		MyStrategy::move(section_begin->Cast(),out);
		section_begin->Destruct();
		WrapInc(section_begin);
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
		return out;
	}

template <class Entry,class MyStrategy>
	void	Queue<Entry,MyStrategy>::EraseFront()
	{
		if (section_begin == section_end)
			return;
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

		section_begin->Destruct();
		WrapInc(section_begin);
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

	}

template <class Entry,class MyStrategy>
	void	Queue<Entry,MyStrategy>::EraseBack()
	{
		if (section_begin == section_end)
			return;
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
		if (section_end == Super::data)
			section_end = Super::end()-1;
		else
			section_end--;
		section_end->Destruct();
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
	}

template <class Entry, class MyStrategy>
	void	Queue<Entry,MyStrategy>::increaseSize(count_t new_size)
	{
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
		size_t old_usage = GetLength();
		Array<Element>	new_field(new_size);
		Element*out = new_field.pointer();
		while (section_begin != section_end)
		{
			out->adoptData(*section_begin);
			out++;
			WrapInc(section_begin);
		}
		

		Super::adoptData(new_field);
		section_begin = Super::pointer();
		section_end = section_begin+old_usage;
		//field_end = section_begin+Super::GetLength();
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
	}

template <class Entry, class MyStrategy>
	void	Queue<Entry,MyStrategy>::Push( const Ctr::ArrayData<Entry>&entries )
	{
		Push(entries.pointer(),entries.Count());
	}

template <class Entry, class MyStrategy>
	void	Queue<Entry,MyStrategy>::Push( const Entry*data, count_t count )
	{
		if (!count)
			return;
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));

		count_t target = Super::GetLength(),
				need = GetLength()+count;
				
		while (target < need)
			target <<= 1;
		if (target != Super::GetLength())
		{
			increaseSize(target);
			for (index_t i = 0; i < count; i++)
			{
				section_end->Construct(data[i]);
				//Strategy::move(data[i],section_end->Cast());
				section_end++;
			}
			//::Strategy::Adopt::copyRange(data,data+count,section_end);	//easy case
			//section_end += count;
		}
		else
		{
			while (count--)
			{
				section_end->Construct(*data);
				data ++;
				WrapInc(section_end);
			}
		}
		//DBG_ASSERT__(Super::Owns(section_end));
		//DBG_ASSERT__(Super::Owns(section_begin));
	}


template <class Entry, class MyStrategy>
	void	Queue<Entry,MyStrategy>::Push( const Entry&data )
	{
		Push() = data;
	}

template <class Entry, class MyStrategy>
	Entry&	Queue<Entry,MyStrategy>::Push( )
	{
		Element* newEnd = section_end;
		WrapInc(newEnd);
		if (newEnd == section_begin)
		{
			size_t old_len = Super::GetLength();
			Super	new_field(Super::GetLength() > 0 ? Super::GetLength()*2 : 2);
			Element*out = new_field.pointer();
			for (iterator it = begin(); it != end(); ++it)
			{
				out->adoptData(*it.current);
				out++;
			}
			Super::adoptData(new_field);
			section_begin = Super::pointer();
			section_end = old_len > 0 ? section_begin+old_len-1 : section_begin;
			newEnd = section_end+1;
			//field_end = section_begin+Super::GetLength();
			#ifdef _DEBUG
				for (iterator it = begin(); it != end(); ++it)
				{
					volatile Entry&test = *it;
				}
			#endif
		}

		Element* rs = section_end;
		section_end = newEnd;
		return rs->ConstructAndCast();
	}

template <class Entry, class MyStrategy>
	template <typename T>
		typename T*	Queue<Entry,MyStrategy>::Increment(T*el)	const
		{
			el++;
			if (el >= Super::end())
				el = Super::data;
			return el;
		}

//
//template <class Entry, class Strategy>
//	void	Queue<Entry,Strategy>::push(const Ctr::ArrayData<Entry>&entries)
//	{
//		push(entries.pointer(),entries.Count());
//	}
//
//template <class Entry, class Strategy>
//	void	Queue<Entry,Strategy>::push(const Entry*data, count_t count)
//	{
//		if (!count)
//			return;
//		count_t target = Super::GetLength(),
//				need = GetLength()+count;
//				
//		while (target < need)
//			target <<= 1;
//		if (target != Super::GetLength())
//		{
//			increaseSize(target);
//			Strategy::copyRange(data,data+count,section_end);	//easy case
//			section_end += count;
//		}
//		else
//		{
//			while (count--)
//			{
//				(*section_end++) = (*data++);
//				if (section_end >= field_end)
//					section_end = Super::pointer();
//			}
//		}
//	}
//

template <class Entry, class MyStrategy>
	void	Queue<Entry,MyStrategy>::PushFront( const Entry&data )
	{
		PushFront() = data;
	}

template <class Entry, class MyStrategy>
	Entry&	Queue<Entry,MyStrategy>::PushFront( )
	{
		#ifdef _DEBUG
			count_t cnt0 = Count();
		#endif

		Element* newStart = section_begin;
		if (newStart == Super::pointer())
			newStart = Super::IsNotEmpty() ? Super::end()-1 : Super::end();
		else
			newStart--;
		if (section_end == newStart)
		{
			size_t old_len = Super::GetLength();
			Super	new_field(Super::GetLength() > 0 ? Super::GetLength()*2 : 2);
			Element*out = new_field.pointer()+1;
			for (iterator it = begin(); it != end(); ++it)
			{
				out->adoptData(*it.current);
				out++;
			}
			Super::adoptData(new_field);
			section_begin = Super::pointer();
			section_end = out;
//			field_end = section_begin+Super::GetLength();
			//DBG_ASSERT__(old_len==GetLength()+1);

			//for (iterator it = begin(); it != end(); ++it)
			//{
			//	volatile Entry&test = *it;
			//}
		}
		else
			section_begin = newStart;
		DBG_ASSERT__(cnt0+1 == Count());
		return section_begin->ConstructAndCast();
	}

template <class Entry,class MyStrategy>
	void				Queue<Entry,MyStrategy>::WrapInc(const Element*&ptr) const
	{
		ptr = Increment(ptr);
	}
template <class Entry,class MyStrategy>
	void				Queue<Entry,MyStrategy>::WrapInc(Element*&ptr)
	{
		ptr = Increment(ptr);
	}


template <class Entry,class MyStrategy>
	void	Queue<Entry,MyStrategy>::clear()
	{
		while (section_begin != section_end)
		{
			section_begin->Destruct();
			WrapInc(section_begin);
		}
		//section_end = section_begin;
	}
	

template <class Entry,class MyStrategy>
	bool	Queue<Entry,MyStrategy>::IsEmpty()					const
	{
		return section_begin == section_end;
	}
template <class Entry,class MyStrategy>
	bool	Queue<Entry,MyStrategy>::IsNotEmpty()					const
	{
		return section_begin != section_end;
	}

template <class Entry,class MyStrategy>	
	size_t	Queue<Entry,MyStrategy>::GetLength()					const
	{
		return section_end >= section_begin?section_end-section_begin:section_end+Super::GetLength()-section_begin;
	}

template <class Entry,class MyStrategy>	
	bool	Queue<Entry,MyStrategy>::operator==(const Self&other) const
	{
		auto i0 = other.begin();
		auto i1 = begin();
		while (i0 != other.end() && i1 != end())
		{
			if (*i0 != *i1)
				return false;
			++i0; ++i1;
		}
		return i0 == other.end() && i1 == end();
	}
template <class Entry,class MyStrategy>	
	bool	Queue<Entry,MyStrategy>::operator!=(const Self&other) const
	{
		auto i0 = other.begin();
		auto i1 = begin();
		while (i0 != other.end() && i1 != end())
		{
			if (*i0 == *i1)
				return false;
			++i0; ++i1;
		}
		return i0 != other.end() || i1 != end();
	}

	
template <class Entry,class MyStrategy>
	bool		Queue<Entry,MyStrategy>::operator>>(Entry&entry)
	{
		return Pop(entry);
	}
	
template <class Entry,class MyStrategy>
	Queue<Entry,MyStrategy>&		Queue<Entry,MyStrategy>::operator<<(const Entry&entry)
	{
		Push(entry);
		return *this;
	}
	
template <class Entry,class MyStrategy>
	Entry&			Queue<Entry,MyStrategy>::GetOldest()
	{
		return section_begin->Cast();
	}
	
template <class Entry,class MyStrategy>
	const Entry&	Queue<Entry,MyStrategy>::GetOldest()						const
	{
		return section_begin->Cast();
	}

template <class Entry,class MyStrategy>
	Entry&			Queue<Entry,MyStrategy>::Peek()
	{
		return section_begin->Cast();
	}

template <class Entry,class MyStrategy>
	const Entry&	Queue<Entry,MyStrategy>::Peek()						const
	{
		return section_begin->Cast();
	}

template <class Entry, class MyStrategy>
	void		Queue<Entry,MyStrategy>::CopyToArray(Ctr::ArrayData<Entry>&out) const
	{
		out.SetSize(CountEntries());
		for (index_t i = 0; i < out.Count(); i++)
			out[i] = operator[](i);
	}

template <class Entry, class MyStrategy>
	void		Queue<Entry,MyStrategy>::MoveToArray(Ctr::ArrayData<Entry>&out, bool clearSelfWhenDone/*=true*/)
	{
		out.SetSize(CountEntries());
		for (index_t i = 0; i < out.Count(); i++)
			MyStrategy::move(operator[](i),out[i]);
		if (clearSelfWhenDone)
			Clear();
	}


template <class Entry,class MyStrategy>
	Entry&			Queue<Entry,MyStrategy>::GetNewest()
	{
		return (section_end!=Super::data?(section_end-1):Super::end()-1)->Cast();
	}
	
template <class Entry,class MyStrategy>
	const Entry&	Queue<Entry,MyStrategy>::GetNewest()						const
	{
		return (section_end!=Super::data?(section_end-1):Super::end()-1)->Cast();
	}
	
template <class Entry,class MyStrategy>
	Entry&			Queue<Entry,MyStrategy>::operator[](size_t index)
	{
		if (section_begin + index >= Super::end())
		{
			return Super::at(index- (Super::end()-section_begin)).Cast();
		}
		return section_begin[index].Cast();
	}
	
template <class Entry,class MyStrategy>
	const Entry&			Queue<Entry,MyStrategy>::operator[](size_t index)	const
	{
		if (section_begin + index >= Super::end())
		{
			return Super::at(index- (Super::end()-section_begin)).Cast();
		}
		return section_begin[index].Cast();
	}


template <class Entry, class Priority, class MyStrategy>
	PriorityQueue<Entry, Priority, MyStrategy>::PriorityQueue(size_t size):entry_field(size),priority_field(size),section_begin(0),section_end(0)
	{}
			
template <class Entry, class Priority, class MyStrategy>
	typename PriorityQueue<Entry,Priority,MyStrategy>::iterator	PriorityQueue<Entry, Priority, MyStrategy>::begin()
	{
		return iterator(entry_field.begin(),entry_field.end(),entry_field+section_begin,entry_field+section_begin);
	}
		
template <class Entry, class Priority, class MyStrategy>
	typename PriorityQueue<Entry,Priority,MyStrategy>::iterator	PriorityQueue<Entry, Priority, MyStrategy>::end()
	{
		return iterator(entry_field.begin(),entry_field.end(),entry_field+section_begin,entry_field+section_end);
	}


template <class Entry, class Priority, class MyStrategy>
	typename PriorityQueue<Entry,Priority,MyStrategy>::const_iterator	PriorityQueue<Entry, Priority, MyStrategy>::begin() const
	{
		return const_iterator(entry_field.begin(),entry_field.end(),entry_field+section_begin,entry_field+section_begin);
	}
		
template <class Entry, class Priority, class MyStrategy>
	typename PriorityQueue<Entry,Priority,MyStrategy>::const_iterator	PriorityQueue<Entry, Priority, MyStrategy>::end() const
	{
		return const_iterator(entry_field.begin(),entry_field.end(),entry_field+section_begin,entry_field+section_end);
	}

template <class Entry, class Priority, class MyStrategy>
	bool	PriorityQueue<Entry, Priority, MyStrategy>::pop(Entry&out, Priority&pout)
	{
		if (section_begin == section_end)
			return false;
		MyStrategy::move(entry_field[section_begin].Cast(),out);
		entry_field[section_begin].Destruct();
		PriorityArray::AppliedStrategy::move(priority_field[section_begin],pout);
		section_begin++;
		if (section_begin >= entry_field.GetLength())
			section_begin = 0;
		return true;
	}
	
template <class Entry, class Priority, class MyStrategy>
	bool	PriorityQueue<Entry, Priority, MyStrategy>::pop(Entry&out)
	{
		if (section_begin == section_end)
			return false;
		MyStrategy::move(entry_field[section_begin].Cast(),out);
		entry_field[section_begin].Destruct();
		section_begin++;
		if (section_begin >= entry_field.GetLength())
			section_begin = 0;
		return true;
	}
	
template <class Entry, class Priority, class MyStrategy>
	Entry				PriorityQueue<Entry, Priority, MyStrategy>::Pop()
	{
		if (section_begin == section_end)
			return Entry();
		Entry rs = entry_field[section_begin].Cast();
		entry_field[section_begin].Destruct();
		section_begin++;
		if (section_begin >= entry_field.GetLength())
			section_begin = 0;
		return rs;
	}

template <class Entry, class Priority, class MyStrategy>
	void				PriorityQueue<Entry, Priority, MyStrategy>::EraseFront()
	{
		if (section_begin == section_end)
			return;
		entry_field[section_begin].Destruct();
		section_begin++;
		if (section_begin >= entry_field.GetLength())
			section_begin = 0;
	}



template <class Entry, class Priority, class MyStrategy>
	bool	PriorityQueue<Entry, Priority, MyStrategy>::PopLeast(Entry&out, Priority&pout)
	{
		if (section_begin == section_end)
			return false;
		index_t index = section_end?section_end-1:entry_field.Count()-1;
		MyStrategy::move(entry_field[index].Cast(),out);
		PriorityArray::AppliedStrategy::move(priority_field[index],pout);
		entry_field[index].Destruct();
		section_end = index;
		return true;
	}
	
template <class Entry, class Priority, class MyStrategy>
	bool	PriorityQueue<Entry, Priority, MyStrategy>::PopLeast(Entry&out)
	{
		if (section_begin == section_end)
			return false;
		index_t index = section_end?section_end-1:entry_field.Count()-1;
		MyStrategy::move(entry_field[index].Cast(),out);
		entry_field[index].Destruct();
		section_end = index;
		return true;
	}
	
template <class Entry, class Priority, class MyStrategy>
	Entry				PriorityQueue<Entry, Priority, MyStrategy>::PopLeast()
	{
		if (section_begin == section_end)
			return Entry();
		index_t index = section_end?section_end-1:entry_field.Count()-1;
		Entry rs = entry_field[index].Cast();
		entry_field[index].Destruct();
		section_end = index;
		return rs;
	}

template <class Entry, class Priority, class MyStrategy>
	void				PriorityQueue<Entry, Priority, MyStrategy>::EraseLeast()
	{
		if (section_begin == section_end)
			return;
		index_t index = section_end?section_end-1:entry_field.Count()-1;
		entry_field[index].Destruct();
		section_end = index;
	}

template <class Entry, class Priority, class MyStrategy>
	inline void			PriorityQueue<Entry, Priority, MyStrategy>::dec(size_t&offset)
	{
		if (offset)
			offset--;
		else
			offset = entry_field.GetLength()-1;
	}

template <class Entry, class Priority, class MyStrategy>
	void			PriorityQueue<Entry, Priority, MyStrategy>::eraseAddr(index_t index)
	{
		if (index >= entry_field.GetLength())
			return;
		if (section_begin < section_end)
		{
			if (index < section_begin || index >= section_end)
				return;
			section_end--;
			for (index_t i = index; i < section_end; i++)
			{
				MyStrategy::move(entry_field[i+1],entry_field[i]);
				PriorityArray::AppliedStrategy::move(priority_field[i+1],priority_field[i]);
			}
			entry_field[section_end].Destruct();
			return;
		}
		if (index < section_end)
		{
			section_end--;
			for (index_t i = index; i < section_end; i++)
			{
				MyStrategy::move(entry_field[i+1],entry_field[i]);
				PriorityArray::AppliedStrategy::move(priority_field[i+1],priority_field[i]);
			}
			entry_field[section_end].Destruct();
			return;
		}
		if (index < section_begin)
			return;
		if (section_end)
			section_end--;
		else
			section_end = entry_field.GetLength()-1;
		
		for (index_t i = index; i != section_end;)
		{
			index_t next = (i+1)%entry_field.GetLength();
			MyStrategy::move(entry_field[next],entry_field[i]);
			PriorityArray::AppliedStrategy::move(priority_field[next],priority_field[i]);
			i = next;
		}
		entry_field[section_end].Destruct();

	}


template <class Entry, class Priority, class MyStrategy>
	void			PriorityQueue<Entry, Priority, MyStrategy>::Erase(const iterator&it)
	{
		eraseAddr(it.GetAddr());

	}

template <class Entry, class Priority, class MyStrategy>
	count_t				PriorityQueue<Entry,Priority,MyStrategy>::CountCopies( const Entry&data )	const
	{
		count_t rs = 0;
		foreach (*this,el)
			if (*el == data)
				rs++;
		return rs;
	}

template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry,Priority,MyStrategy>::Contains( const Entry&data, Priority&outPriority )	const
	{
		foreach (*this,el)
			if (*el == data)
			{
				outPriority = priority_field[el.GetAddr()];
				return true;
			}
		return false;
	}

template <class Entry, class Priority, class MyStrategy>
	template <typename EntryToBoolFunction>
		bool			PriorityQueue<Entry,Priority,MyStrategy>::ContainsF(const EntryToBoolFunction&f,Priority&outPriority)	const
		{
			foreach (*this,el)
				if (f(*el))// == data)
				{
					outPriority = priority_field[el.GetAddr()];
					return true;
				}
			return false;
		}



template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry,Priority,MyStrategy>::Find( const Priority&priority, iterator&it )
	{
		size_t len = entry_field.GetLength();
		size_t end = section_end;
		if (end < section_begin)
			end+=entry_field.GetLength();
		size_t half = (section_begin+end)/2;
		
		size_t	lower = section_begin;
		{
			size_t		num = end-section_begin,
						upper = end;
			while (lower< upper && upper <= end)
			{
		        size_t el = (lower+upper)/2;
				size_t real = el%len;
				
		        const Priority&p = priority_field[real];
				if (p == priority)
				{
					it = iterator(entry_field.begin(),entry_field.end(),entry_field+section_begin,entry_field+real);
					return true;
				}
		        if (p < priority)
		            upper = el;
		        else
					lower = el+1;
		    }
		}
		index_t real = lower%len;
		if (priority_field[real] == priority)
		{
			it = iterator(entry_field.begin(),entry_field.end(),entry_field+section_begin,entry_field+real);
			return true;
		}
		return false;
	}

template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry,Priority,MyStrategy>::Find( const Entry&data, const Priority&priority, iterator&it )
	{
		if (!Find(priority,it))
			return false;
		if (*it == data)
			return true;
		iterator origin = it;
		--it;
		while (it != begin()-1 && priorityOf(it) == priority)
		{
			if (*it == data)
				return true;
			--it;
		}
		it = origin;
		++it;
		while (it != end() && priorityOf(it) == priority)
		{
			if (*it == data)
				return true;
			++it;
		}
		return false;
	}

template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry,Priority,MyStrategy>::AlterPriority( const Entry&data, const Priority&old_priority, const Priority&new_priority )
	{
		iterator it;
		if (!Find(data,old_priority,it))
			return false;
		Erase(it);
		Push(data,new_priority);
		return true;
	}


template <class Entry, class Priority, class MyStrategy>
	void				PriorityQueue<Entry, Priority, MyStrategy>::Push( const Entry&data, const Priority&priority )
	{
		
		size_t len = entry_field.GetLength();
		size_t end = section_end;
		if (end < section_begin)
			end+=entry_field.GetLength();
		size_t half = (section_begin+end)/2;
		
		size_t	lower = section_begin;
		{
			size_t	num = end-section_begin,
						upper = end;
			while (lower< upper && upper <= end)
			{
		        size_t el = (lower+upper)/2;
				size_t real = el%len;
				
		        const Priority&p = priority_field[real];
		        if (p < priority)
		            upper = el;
		        else
					lower = el+1;
		    }
		}

		if (end-section_begin==len-1)
		{
			//cout << "queue has reached capacity ("<<len<<")"<<endl;
			size_t l = lower%len;
			EntryArray		new_entry_field(len<<1);
			PriorityArray	new_priority_field(len<<1);
			//cout << "new fields have length "<<new_entry_field.GetLength()<<endl;
			//cout << "element is "<<l<<endl;
			Element*out = new_entry_field.pointer();
			Priority*pout = new_priority_field.pointer();
			
			while (section_begin != l)
			{
				//cout << "processing element "<<section_begin<<endl;
				out->adoptData(entry_field[section_begin]);
				out++;
				PriorityArray::AppliedStrategy::move(priority_field[section_begin],*pout++);
				section_begin++;
				section_begin%=len;
			}
			out->Construct(data);
			out++;
			//(*out++) = data;
			(*pout++) = priority;
			while (section_begin != section_end)
			{
				//cout << "processing element "<<section_begin<<endl;
				out->adoptData(entry_field[section_begin]);
				out++;
//				Strategy::move(entry_field[section_begin],*out++);
				PriorityArray::AppliedStrategy::move(priority_field[section_begin],*pout++);
				section_begin++;
				section_begin%=len;
			}
			
			entry_field.adoptData(new_entry_field);
			priority_field.adoptData(new_priority_field);
			section_begin = 0;
			section_end = len;
			//cout << "new section is "<<section_begin<<"..."<<section_end<<endl;
		}
		else
		{
			//cout << "simple insert job at "<<lower<<endl;
			if (end-lower < lower-section_begin)
			{
				for (size_t i = end; i > lower; i--)
				{
					entry_field[i%len].adoptData(entry_field[(i-1)%len]);
					//Strategy::move(entry_field[(i-1)%len],entry_field[i%len]);
					PriorityArray::AppliedStrategy::move(priority_field[(i-1)%len],priority_field[i%len]);
				}
				section_end++;
				section_end%=len;

				const index_t at = lower%len;
				entry_field[at].Construct(data);

				//entry_field[lower%len] = data;
				priority_field[lower%len] = priority;
			}
			else
			{
				//ShowMessage("section is "+String(section_begin)+", "+String(end));
				//ShowMessage("lower is "+String(lower));
				
				if (section_begin != lower%len)
				{
					//ShowMessage("copying first");
					if (section_begin)
					{
						entry_field[section_begin-1].adoptData(entry_field[section_begin]);
						//Strategy::move(entry_field[section_begin],entry_field[section_begin-1]);
						PriorityArray::AppliedStrategy::move(priority_field[section_begin],priority_field[section_begin-1]);
					}
					else
					{
						entry_field.Last().adoptData(entry_field.First());
						//Strategy::move(entry_field.First(),entry_field.Last());
						PriorityArray::AppliedStrategy::move(priority_field.First(),priority_field.Last());
					}
				}
				for (size_t i = section_begin; i+1 < lower; i++)
				{
					entry_field[i%len].adoptData(entry_field[(i+1)%len]);
					//Strategy::move(entry_field[(i+1)%len],entry_field[i%len]);
					PriorityArray::AppliedStrategy::move(priority_field[(i+1)%len],priority_field[i%len]);
				}
				dec(section_begin);
				dec(lower);
				const index_t at = lower%len;
				entry_field[at].Construct(data);
				priority_field[at] = priority;
			}
			//cout << "section end now "<<section_end<<" (len="<<len<<")"<<endl;
		}
	}
	
template <class Entry, class Priority, class MyStrategy>	
	bool				PriorityQueue<Entry, Priority, MyStrategy>::checkIntegrity()
	{
		iterator b = begin();
		while (b != end())
		{
			iterator a = b++;
			if (b != end() && priorityOf(a)<priorityOf(b))
				return false;
		}
		return true;
	
	}
	
template <class Entry, class Priority, class MyStrategy>
	void	PriorityQueue<Entry, Priority, MyStrategy>::clear()
	{
		const count_t len = entry_field.Length();
		while (section_begin != section_end)
		{
			entry_field[section_begin].Destruct();
			section_begin++;
			section_begin%=len;
		}

		//section_end = section_begin;
	}
	
template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry, Priority, MyStrategy>::IsEmpty()					const
	{
		return section_begin == section_end;
	}
template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry, Priority, MyStrategy>::IsNotEmpty()					const
	{
		return section_begin != section_end;
	}
	
	
template <class Entry, class Priority, class MyStrategy>
	size_t			PriorityQueue<Entry, Priority, MyStrategy>::GetLength()					const
	{
		if (section_end >= section_begin)
			return section_end-section_begin;
		return section_end+entry_field.GetLength()-section_begin;
	}
	
template <class Entry, class Priority, class MyStrategy>
	bool				PriorityQueue<Entry, Priority, MyStrategy>::operator>>(Entry&entry)
	{
		return pop(entry);
	}
	
	
template <class Entry, class Priority, class MyStrategy>
	Entry&				PriorityQueue<Entry, Priority, MyStrategy>::peek()	
	{
		return entry_field[section_begin].Cast();
	}

template <class Entry, class Priority, class MyStrategy>
	const Priority&				PriorityQueue<Entry, Priority, MyStrategy>::peekPriority()	const
	{
		return priority_field[section_begin];
	}

	
template <class Entry, class Priority, class MyStrategy>
	const Entry&		PriorityQueue<Entry, Priority, MyStrategy>::peek()						const
	{
		return entry_field[section_begin].Cast();
	}
	
template <class Entry, class Priority, class MyStrategy>
	Entry&				PriorityQueue<Entry, Priority, MyStrategy>::peekLeast()
	{
		return (section_end?entry_field[section_end-1]:entry_field.Last()).Cast();
	}
	
template <class Entry, class Priority, class MyStrategy>
	const Entry&		PriorityQueue<Entry, Priority, MyStrategy>::peekLeast()						const
	{
		return (section_end?entry_field[section_end-1]:entry_field.Last()).Cast();
	}

	
template <class Entry, class Priority, class MyStrategy>
	const Priority&		PriorityQueue<Entry, Priority, MyStrategy>::peekLeastPriority()						const
	{
		return section_end?priority_field[section_end-1]:priority_field.Last();
	}
	
template <class Entry, class Priority, class MyStrategy>
	const Priority&			PriorityQueue<Entry, Priority, MyStrategy>::priorityOf(const iterator&it)	const
	{
		return priority_field[it.GetAddr()];
	
	
	}

/*

template <class C, class IndexType> const C&Queue<C,IndexType>::get()
{
    if (first == last)
        return NULL;
    element = loop[first];
    first++;
    return element;
}

template <class C, class IndexType> Queue<C,IndexType>::Queue():first(0),last(0)
{}

template <class C, class IndexType> const C&Queue<C,IndexType>::peek()  const
{
    if (first == last)
        return NULL;
    return loop[first];
}

template <class C, class IndexType> const C&Queue<C,IndexType>::peek(IndexType element)  const
{
    return loop[first+element];
}

template <class C, class IndexType> void Queue<C,IndexType>::Insert(const C&item)
{
    if (last+1 == first)
        return;
    loop[last] = item;
    last++;
}

template <class C, class IndexType> void Queue<C,IndexType>::clear()
{
    first = last;
}

template <class C, class IndexType> IndexType Queue<C, IndexType>::Count() const
{
    return last-first;
}

template <class C, class IndexType> bool Queue<C,IndexType>::full() const
{
    return last+1 == first;
}

template <class C, class IndexType> bool Queue<C,IndexType>::empty() const
{
    return last == first;
}





template <class C, class IndexType> C*CPQueue<C,IndexType>::get()
{
    if (first == last)
        return NULL;
    C*result = loop[first];
    first++;
    return result;
}

template <class C, class IndexType> CPQueue<C,IndexType>::CPQueue():first(0),last(0)
{}

template <class C, class IndexType> C*CPQueue<C,IndexType>::peek()
{
    if (first == last)
        return NULL;
    return loop[first];
}

template <class C, class IndexType> C*CPQueue<C,IndexType>::peek(IndexType element)
{
    return loop[first+element];
}

template <class C, class IndexType> void CPQueue<C,IndexType>::Insert(C*item)
{
    if (last+1 == first)
    {
        Discard(item);
        return;
    }
    loop[last] = item;
    last++;
}

template <class C, class IndexType> void CPQueue<C,IndexType>::clear()
{
    while (first != last)
    {
        Discard(loop[first]);
        first++;
    }
}

template <class C, class IndexType> void CPQueue<C,IndexType>::flush()
{
    first = last;
}

template <class C, class IndexType> IndexType CPQueue<C, IndexType>::Count() const
{
    return last-first;
}

template <class C, class IndexType> bool CPQueue<C,IndexType>::full() const
{
    return last+1 == first;
}

template <class C, class IndexType> bool CPQueue<C,IndexType>::empty() const
{
    return last == first;
}







template <class C> DynamicQueue<C>::DynamicQueue():first(NULL),last(NULL),volume(0)
{}

template <class C> DynamicQueue<C>::~DynamicQueue()
{
    clear();
}


template <class C> const C& DynamicQueue<C>::peek()                  const
{
    return first?first->element:element;
}

template <class C> const C& DynamicQueue<C>::get()
{
    if (!first)
        return element;
    element = first->element;
    SDynamicPath<C>*next = first->next;
    Discard(first);
    if (!next)
        last = NULL;
    first = next;
    volume--;
    return element;
}

template <class C> void DynamicQueue<C>::Insert(const C&item)
{
    SDynamicPath<C>*append = SignalNew(new SDynamicPath<C>());
    append->element = item;
    append->next = NULL;
    if (last)
        last->next = append;
    else
        first = append;
    last = append;
    volume++;
}

template <class C> void DynamicQueue<C>::clear()
{
    SDynamicPath<C>*path = first;
    while (path)
    {
        SDynamicPath<C>*next = path->next;
        Discard(path);
        path = next;
    }
    first = NULL;
    last = NULL;
    volume = 0;
}

template <class C> size_t DynamicQueue<C>::Count()              const
{
    return volume;
}

template <class C> bool DynamicQueue<C>::empty()                 const
{
    return !volume;
}


template <class C> DynamicPQueue<C>::DynamicPQueue():first(NULL),last(NULL),volume(0)
{}

template <class C> DynamicPQueue<C>::~DynamicPQueue()
{
    clear();
}

template <class C> C*DynamicPQueue<C>::peek()
{
    return first?first->element:NULL;
}

template <class C> C*DynamicPQueue<C>::get()
{
    if (!first)
        return NULL;
    C*element = first->element;
    SDynamicPath<C*>*next = first->next;
    Discard(first);
    if (!next)
        last = NULL;
    first = next;
    volume--;
    return element;
}

template <class C> void DynamicPQueue<C>::Insert(C*item)
{
    SDynamicPath<C*>*append = SignalNew(new SDynamicPath<C*>());
    append->element = item;
    append->next = NULL;
    if (last)
        last->next = append;
    else
        first = append;
    last = append;
    volume++;
}

template <class C> void DynamicPQueue<C>::clear()
{
    SDynamicPath<C*>*path = first;
    while (path)
    {
        SDynamicPath<C*>*next = path->next;
        Discard(path->element);
        Discard(path);
        path = next;
    }
    first = NULL;
    last = NULL;
    volume = 0;
}

template <class C> void DynamicPQueue<C>::flush()
{
    SDynamicPath<C*>*path = first;
    while (path)
    {
        SDynamicPath<C*>*next = path->next;
        Discard(path);
        path = next;
    }
    first = NULL;
    last = NULL;
    volume = 0;
}


template <class C> size_t DynamicPQueue<C>::Count()              const
{
    return volume;
}

template <class C> bool DynamicPQueue<C>::empty()                 const
{
    return !volume;
}

*/

#endif


