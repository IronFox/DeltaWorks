#ifndef hashtableH
#define hashtableH

#include "common.h"
#include "../global_string.h"
#include "../interface/hashable.h"

#include <functional>

namespace DeltaWorks
{
	namespace Container
	{

		class StdHash
		{
		public:
			template <typename T>
				inline static hash_t	ComputeHash(const T&e)
				{
					using GlobalHashFunctions::Hash;
					return Hash(e);
				}
		};


		/**
		Warning:
		Use with extreme care.
		Hashing WILL FAIL if there are memory gaps between contained members due to structure packing.
		These gaps will never be initialized and contain garbage data in the least useful moment.
		*/
		struct TightlyPackedPODHash
		{
		template <typename T>
			static inline hash_t 		ComputeHash(const T&ident)	{return StdMemHashT<sizeof(T)>(&ident);}
		};




		template <class K, class KeyStrategy>
			struct THashSetCarrier	//! Carrier type used to store a single entry of a Hash set
			{
				typedef K					Key;
				typedef KeyStrategy			AppliedKeyStrategy;
				typedef THashSetCarrier<K,KeyStrategy> Self;

				K							key;		//!< Key of this entry
				hash_t						hashed;		//!< Hashed key
				bool						occupied;	//!< Set true if this entry is occupied

				/**/						THashSetCarrier():occupied(false)
				{}

				void						adoptData(THashSetCarrier<K,KeyStrategy>&other)
				{
					KeyStrategy::move(other.key,key);
					hashed = other.hashed;
					occupied = other.occupied;
				}
				void						occupy()
				{
					occupied = true;
				}
				void						free()
				{
					occupied = false;
				}
				bool						operator==(const Self&other) const
				{
					if (!occupied || !other.occupied)
						return occupied == other.occupied;
					return hashed == other.hashed && key == other.key;
				}
				bool						operator!=(const Self&other) const
				{
					return !operator==(other);
				}
			};

		template <class K, class C, class KeyStrategy, class DataStrategy>
			struct THashTableCarrier:public THashSetCarrier<K, KeyStrategy>	//! Carrier type used to store a single entry of a Hash table
			{
				typedef THashTableCarrier<K,C,KeyStrategy,DataStrategy>	ThisType;
				typedef THashSetCarrier<K, KeyStrategy>	Super;
				typedef C					Data;

				//this align is a pain in the arse in x86 for SOME reason
				/*ALIGN16*/ BYTE				entry_data[sizeof(C)];
				//C							entry;		//!< Local copy of the inserted data

				/**/						~THashTableCarrier()
				{
					if (Super::occupied)
						reinterpret_cast<C*>(entry_data)->~C();
				}

				ThisType&					operator=(const ThisType&other)
				{
					if (Super::occupied)
					{
						if (other.occupied)
							(*reinterpret_cast<C*>(entry_data)) =  (*reinterpret_cast<const C*>(other.entry_data));
						else
							reinterpret_cast<C*>(entry_data)->~C();
					}
					else
						if (other.occupied)
							new (entry_data) C(*reinterpret_cast<const C*>(other.entry_data));
					Super::operator=(other);
					return *this;
				}
				template <typename T2, class DataStrategy2>
				ThisType&					operator=(const THashTableCarrier<K,T2,KeyStrategy,DataStrategy2>&other)
				{
					if (Super::occupied)
					{
						if (other.occupied)
							(*reinterpret_cast<C*>(entry_data)) =  (*reinterpret_cast<const T2*>(other.entry_data));
						else
							reinterpret_cast<C*>(entry_data)->~C();
					}
					else
						if (other.occupied)
							new (entry_data) C(*reinterpret_cast<const T2*>(other.entry_data));
					Super::operator=(other);
					return *this;
				}

				void						adoptData(ThisType&other)
				{
					if (other.occupied)
					{
						if (Super::occupied)
							DataStrategy::move(*reinterpret_cast<C*>(other.entry_data),*reinterpret_cast<C*>(entry_data));
						else
							DataStrategy::constructSingleFromFleetingData(reinterpret_cast<C*>(entry_data),*reinterpret_cast<C*>(other.entry_data));
					}
					else
						if (Super::occupied)
							reinterpret_cast<C*>(entry_data)->~C();
					Super::adoptData(other);
				}
				C&							cast()
				{
					return *reinterpret_cast<C*>(entry_data);
				}
				const C&					cast() const
				{
					return *reinterpret_cast<const C*>(entry_data);
				}
				void						free()
				{
					if (Super::occupied)
						reinterpret_cast<C*>(entry_data)->~C();
					Super::occupied = false;
				}
				void						occupy()
				{
					new (entry_data) C;
					Super::occupied = true;
				}
				bool						operator==(const ThisType&other) const
				{
					return Super::operator==(other) && cast() == other.cast();
				}
				bool						operator!=(const ThisType&other) const
				{
					return !operator==(other);
				}
			};


		template <class Carrier>
			class GenericHashBase
			{
				typedef GenericHashBase<Carrier>	Self;
				typedef Array<Carrier,AdoptStrategy>	Store;
			protected:
				static const size_t 			InitialSize = 4;	//!< Initial entries in the table.
			
				Store			array;				//!< Entry table
				size_t			entries;			//!< Number of entries that are occupied in the table
			
				/**/			GenericHashBase();
											
				template <class Key>
					Carrier*	find(hash_t Hash, const Key&key, bool occupy,bool*did_occupy=NULL)	//!< Carrier lookup via a key and its Hash-value. \param Hash Hashvalue of the provided string (usually the result of hashString(key)) \param key Key to look for \param occupy Forces the hashtable to occupy a new carrier for the specified key if it could not be found. \param did_occupy If non NULL then \b did_occupy will be set true if a carrier was occupied, false otherwise. \return Pointer to the occupied carrier if the key could be found or \b occupy was set true. Otherwise to a non-occupied carrier.
					{
						if (occupy && entries+1 >= array.GetLength()*0.8)
							Resize(array.GetLength()*2);
						size_t offset = Hash%array.GetLength();

						Carrier*const raw = array.pointer();	//avoid redundant check
						while (raw[offset].occupied && (raw[offset].hashed != Hash || raw[offset].key != key))
							offset = (offset+1)%array.GetLength();
						Carrier&entry = raw[offset];
						if (occupy && !entry.occupied)
						{

							entry.hashed = Hash;
							entry.key = typename Carrier::Key(key);
							entry.occupy();
							entries++;
							if (entries == array.GetLength())
								FATAL__("hashtable overflow");
							if (did_occupy)
								(*did_occupy) = true;
						}
						else
							if (did_occupy)
								(*did_occupy) = false;


						return array+offset;
					}		
				template <class Key>
					const Carrier*find(hash_t Hash, const Key&key)			const	//!< Carrier lookup via a key and its Hash-value. \param Hash Hashvalue of the provided string (usually the result of hashString(key)) \param key Key to look for \return Pointer to the occupied carrier if the key could be found. Otherwise to a non-occupied carrier.
					{
						size_t offset = Hash%array.GetLength();
						const Carrier*const raw = array.pointer();	//avoid redundant check
						while (raw[offset].occupied && (raw[offset].hashed != Hash || raw[offset].key != key))
							offset = (offset+1)%array.GetLength();

						return raw+offset;
					}
				inline void		Resize(size_t new_size);							//!< Resizes the hashtable to the specified size and remaps all table entries \param new_size New number of entries to use
				inline void 	remove(Carrier*c);									//!< Removes the specified carrier from the table. Invokes Tidy(). \param c Carrier to remove.

				/**
				Shrinks the local array if necessary
				@return true if the local array was shrunk, false otherwise
				*/
				bool			Tidy();
	
			public:
				virtual			~GenericHashBase()	{};
	
				inline size_t	GetTotalSize()							const;	//!< Returns the total size of the set/table in bytes. \return Total size of the local set/table in bytes.
				inline GenericHashBase<Carrier>&operator=(const GenericHashBase<Carrier>&other);
				inline size_t	Count()								const;	//!< Returns the number of entries currently stored in the set/table. \return Number of entries in the set/table.
				inline operator size_t()							const;	//!< Implicit conversion to size_t. \return Returns the number of entries in the set/table.
				inline	void	clear();									//!< Resizes back to the initial set size and un-occupies all carriers. If no resizing took place then the respective objects are not reinitialized.
				inline	bool	IsEmpty()							const	/**!< Checks if the local table is empty**/	{return entries ==0;}
				inline	bool	IsNotEmpty()						const	/**!< Checks if the local table contains at least one element*/ 	{return entries !=0;}
				inline	void	Clear()										/** @copydoc clear() */ {clear();}
				inline	void	Import(GenericHashBase<Carrier>&list);		//!< Imports the content of the specified other hashset/hashtable in addition to the already contained entries. Existing entries are overwritten. \param list Hashset to import entries from.
				template <class Key>
					inline void	ExportKeys(Ctr::ArrayData<Key>&keys)	const;			//!< Exports the keys to the specified array. \param keys Reference to an array containing all associated keys after execution. 

				inline	void	Include(const Self&other);	//!< Loads the union of the local and the remote Hash table/set. Duplicate entries are taken once from the local table/set. @param other Hash table/set to calculate the union with.
				inline	void	Intersect(const Self&other);	//!< Loads the intersection of the local and the remote Hash table/set. Local entries that are not also contained by the remote table/set are dropped. @param other Hash table/set to calculate the intersection with.
				inline	void	Differentiate(const Self&other);		//!< Loads the difference (this - other). Removes all entries of the remote table/set from the local table/set. @param other Hash table/set to remove from the local one
				template <typename Carrier2>
					inline void	IncludeCast(const GenericHashBase<Carrier2>&other);	//!< @copydoc Include()

				inline void		adoptData(Self&other);
				inline void		swap(Self&other);
				friend void		swap(Self&a, Self&b)	{a.swap(b);}

				bool			operator==(const Self&other) const;
				bool			operator!=(const Self&other) const {return !operator==(other);}

				const Store&	GetStore() const {return array;}
			};

		template <class Carrier, class Hash>
			class ExtendedHashBase:public GenericHashBase<Carrier>
			{
			protected:
				typedef GenericHashBase<Carrier>	Base,Super;
				using Super::entries;
				using Super::array;
			public:
				typedef typename Carrier::Key	K;

				template <class Key>
					inline	bool					IsSet(const Key&ident)				const;	//!< Queries whether ot not data is associated with the specified key without setting it. \param ident Key to look for \return True if the specified key could be found, false otherwise.
				template <class Key>
					inline	bool					Unset(const Key&ident);						//!< Un-sets any data associated with the specifed key. Potentially resizes the internal set. \param ident Key to look for \return True if the specified key could be found and un-set, false otherwise.
				template <typename F>
					inline	void					visitAllKeys(const F&f) const;	//!< Passes each occupied element to the specified functor of type <tt>void f(const Key&)</tt>.
				template <typename F>
					inline	void					VisitAllKeys(const F&f) const				/** @copydoc visitAllKeys() */{visitAllKeys(f);}
				/**
				Visits all keys by invoking filter(key) for each contained key.
				@param filter Callback function to call. The function must return true if the respective element should be preserved, false if it should be unset
				*/
				void								FilterKeys(const std::function<bool(const K&)>&filter);

			};

		/*!
			\brief Root Hashset

			The generic hashset is a type-invariant root class of all other sets in this file.
			Instances of GenericHashSet are natively copyable and require no additional copy constructor or assignment operator.
		*/
		template <class K, class Hash=StdHash, typename KeyStrategy = typename StrategySelector<K>::Default>
			class HashSet:public ExtendedHashBase<THashSetCarrier<K,KeyStrategy>,Hash>
			{
			protected:
				typedef THashSetCarrier<K,KeyStrategy>		Carrier;
		
				typedef ExtendedHashBase<THashSetCarrier<K,KeyStrategy>,Hash>	Base, Super;

			public:
					typedef K						KeyType;	//!< Key type

				template <class Key>
					inline	void					Set(const Key&ident);						//!< Sets the specified key (if not set already). \param ident Key to set
				template <class Key>
					inline	bool					SetNew(const Key&ident);		//!< Attemps to insert the specified new identifier. @return true, if the key was not previously set, false otherwise
				template <class Key>
					inline	void					SetAll(const Ctr::ArrayData<Key>&idents);				//!< Sets the specified range of keys (if not set already). @param idents Ctr::Array of keys to set
			};

	
		typedef HashSet<String>			StringSet;				//!< String based hash set
		typedef HashSet<StringW>		StringWSet;				//!< String based hash set
		typedef HashSet<index_t>		IndexSet;				//!< Index set
		typedef HashSet<const void*>	PointerSet;				//!< Pointer set

		/*!
			\brief Root Hashtable

			The generic hashtable is a type-invariant root class of all other tables and containers in this file.
			Instances of HashTable are natively copyable and require no additional copy constructor or assignment operator.
		*/
		template <class K, class C, class Hash=StdHash, typename KeyStrategy = typename StrategySelector<K>::Default, typename DataStrategy = typename StrategySelector<C>::Default>
			class HashTable:public ExtendedHashBase<THashTableCarrier<K,C,KeyStrategy,DataStrategy>,Hash>
			{
			protected:
		
					typedef THashTableCarrier<K,C,KeyStrategy,DataStrategy>	Carrier;

					typedef ExtendedHashBase<THashTableCarrier<K,C, KeyStrategy,DataStrategy>,Hash>	Base,Super;

					using Super::entries;
					using Super::array;

			public:
					typedef K						KeyType;	//!< Key type
					typedef C						DataType;		//!< Data type


				template <typename F>
					inline	void					visitAllValues(const F&f);	//!< Passes each occupied element to the specified functor of type <tt>void f(DataType&)</tt>. The functor may receive a copy, const, or non-const reference, even alter data if needed
				template <typename F>
					inline	void					VisitAllValues(const F&f)				/** @copydoc visitAllValues() */{visitAllValues(f);}
				template <typename F>
					inline	void					visitAllValues(const F&f) const;	//!< Constant version of visitAllValues(). Only copies or const references may be received by the functor.
				template <typename F>
					inline	void					VisitAllValues(const F&f) const				/** @copydoc visitAllValues() */{visitAllValues(f);}
				template <typename F>
					inline	void					visitAllEntries(const F&f);	//!< Passes each occupied element to the specified functor of type <tt>void f(const KeyType&, DataType&)</tt>. The functor may receive a copy, const, or non-const reference, even alter data if needed
				template <typename F>
					inline	void					VisitAllEntries(const F&f)				/** @copydoc visitAllEntries() */{visitAllEntries(f);}
				template <typename F>
					inline	void					visitAllEntries(const F&f)	const;	//!< Constant version of visitAllEntries(). Only copies or const references may be received by the functor.
				template <typename F>
					inline	void					VisitAllEntries(const F&f)	const		/** @copydoc visitAllEntries() */{visitAllEntries(f);}
				/**
				Iterates over all entries and aborts as soon as the given function returns true.
				@param f Scan function. Must be of the form bool(K,C). Once the function returns true, scanning aborts
				@return Key of the entry where @a f returned true, null if this did not happen
				*/
				template <typename F>
					inline	const KeyType*			ScanEntries(const F&f)	const;
				/**
				Visits keys and values entries by invoking filter(key, value) for each contained entry.
				@param filter Callback function to call. The function must return true if the respective element should be preserved, false if it should be unset
				*/
				inline	void						FilterEntries(const std::function<bool(const K&, DataType&)>&f);
				template <class Entry>
					inline	void					ExportTo(Ctr::ArrayData<K>&keys, Ctr::ArrayData<Entry>&values)	const;	//!< Exports keys and values to the respective arrays. \param keys Reference to an array containing all associated keys after execution. \param values Reference to an object array containing all contained data elements after execution. \b keys and \b values will be of the same size with each entry of \b keys associated with the entry in \b values of the same index.
				template <class Entry>
					inline	void					ExportTo(Ctr::ArrayData<Entry>&values)		const;	//!< Exports all entries. \param values Reference to an array containing all entries after execution.
				template <class Entry>
					inline	void					exportAddressesTo(Ctr::ArrayData<const K*>&keys, Ctr::ArrayData<Entry*>&values);
				template <class Entry>
					inline	void					exportAddressesTo(Ctr::ArrayData<Entry*>&values);
				template <class Entry>
					inline	void					exportAddressesTo(Ctr::ArrayData<const K*>&keys, Ctr::ArrayData<const Entry*>&values)const;
				template <class Entry>
					inline	void					exportAddressesTo(Ctr::ArrayData<const Entry*>&values)	const;
				template <class Entry>
					inline	bool					FindKeyOf(const Entry&entry, K&key)const;	//!< Finds the key of the specified entry and stores it in \b key. Operates in O(n). \param entry Entry to look for. Comparison will be done via operator== . \param key Out key variable. Found key will be assigned via operator= . \return True if the requested element could be found, false otherwise.
				inline	bool						Query(const K&ident, DataType&target)	const;	//!< Requests the content associated with the specified key without setting it. \param ident Key to look for \param target Out reference to copy the respective content to \return True if an entry matching the specified key could be found, false otherwise. \b target remains unchanged if \b key could not be found.
				inline	bool						queryAndUnset(const K&ident, DataType&target); 	//!< Requests the content associated with the specified key and un-sets it. \param ident K to look for \param target Out reference to copy the respective content to \return True if an entry matching the specified key could be found, false otherwise. \b target remains unchanged if \b key could not be found.
				inline	bool						QueryAndUnset(const K&ident, DataType&target)	/** @copydoc queryAndUnSet() */ {return queryAndUnset(ident,target);}
				inline	DataType*					queryPointer(const K&ident);				//!< Requests the content associated with the specified key without setting it. The method returns a pointer to the element ot NULL if no such could be found. \param ident Key to look for \return Pointer to the object matching the specified key or NULL if no such could be found
				inline	DataType*					QueryPointer(const K&ident)				/** @copydoc queryPointer() */{return queryPointer(ident);}
				inline	const DataType*				queryPointer(const K&ident)		const;	//!< Requests the content associated with the specified key without setting it. The method returns a pointer to the element ot NULL if no such could be found. \param ident Key to look for \return Pointer to the object matching the specified key or NULL if no such could be found
				inline	const DataType*				QueryPointer(const K&ident)		const	/** @copydoc queryPointer() */{return queryPointer(ident);}
				inline	bool						Owns(const DataType*data)	const;
				inline	DataType*					setNew(const K&ident);					//!< Sets the specified key if it is currently not set and returns the pointer its content. If the key is already set then NULL is returned @param ident Key to set @return Pointer to the set content associated with the specified key, or NULL if the key was previously set
				inline	DataType*					SetNew(const K&ident)						/** @copydoc setNew()*/ { return setNew(ident); }
				inline	bool						SetNew(const K&ident, const DataType&d)	{ DataType*into = setNew(ident); if (into) (*into) = d; return into != NULL;}
				template <class Entry>
					inline	bool					unsetEntry(const Entry&entry);				//!< Un-sets the key associated with the specified entry. Potentially resizes the internal table. \param entry Entry to look for \return True if the specified entry could be found and un-set, false otherwise.
				template <class Entry>
					inline	bool					UnsetEntry(const Entry&entry)				/** @copydoc unSetEntry() */{return unsetEntry(entry);}
				inline	DataType&					Reference(const K&ident, const DataType&initValue);		//!< Sets the specified key (to the specified init value, if not set already) and returns the currently stored value as a reference. \param ident Key to set \param initValue Value to assign to the associated data if the specified key was not previously specified.
				inline	DataType&					Set(const K&ident, const DataType&v);		//!< Sets the specified key (if not set already) and assigns \b v to the associated data. \param ident Key to set \param v Value to assign to the associated data.
				inline	DataType&					Set(const K&ident);						//!< Sets the specified key if it is currently not set and returns a reference to it. No change occurs if the specified key is already set (merely returns a reference to it)
				inline	DataType&					GetChar(const K&ident, DataType&except);		//!< Returns a (mutable) reference to the data associated with the specified key, or @a except if no such could be found.
				inline	DataType&					Get(const K&ident, DataType&except)		{return GetChar(ident,except);}
				inline	const C&					GetChar(const K&ident, const C&except)	const;			//!< Returns a (const) reference to the data associated with the specified key, or @a except if no such could be found.
				inline	const C&					Get(const K&ident, const C&except)	const	{return GetChar(ident,except);}
				inline	DataType&					require(const K&ident);					//!< Returns a (mutable) reference to the data associated with the specified key. An assertion will fire, if the requested element does not exist
				inline	DataType&					Require(const K&ident)					{return require(ident);}
				inline	const C&					require(const K&ident)	const;			//!< Returns a (const) reference to the data associated with the specified key. An assertion will fire, if the requested element does not exist
				inline	const C&					Require(const K&ident)	const			{return require(ident);}
				inline	DataType&					operator[](const K&ident);				//!< Standard access. If the specified key could not be found then an exception of type Program::MemberNotFound will be triggered. \param ident Key to look for \return Reference to the data associated with the specified key.
				inline	const DataType&				operator[](const K&ident)			const;	//!< Standard const access. If the specified key could not be found then an exception of type Program::MemberNotFound will be triggered. \param ident Key to look for \return Reference to the data associated with the specified key.
			};


		/*!
		\brief Standard String-mapped Hashtable

		The hashtable stores copies of inserted objects mapped via strings. Anything that can be cast into a string can be used as a key.
		*/
		template <class C, typename DataStrategy = typename StrategySelector<C>::Default>
			class StringTable:public HashTable<String,C,StdHash,SwapStrategy,DataStrategy>
			{};


		/*!
			\brief Index table

			The indextable stores copies of inserted objects mapped via indices. Anything that can be cast into an integer can be used as a key.
		*/
		template <class C, typename DataStrategy = typename StrategySelector<C>::Default>
			class IndexTable:public HashTable<index_t,C,StdHash,PrimitiveStrategy,DataStrategy>
			{};


		/*!
			\brief Pointer table

			The pointertable stores copies of inserted objects mapped via pointers. Anything that can be cast into a pointer can be used as a key.
		*/
		template <class C, typename DataStrategy = typename StrategySelector<C>::Default>
			class PointerTable:public HashTable<const void*,C, StdHash, PrimitiveStrategy, DataStrategy>
			{};

		/*!
			\brief Generic mapped container
	
			The GenericHashContainer class acts as a generic super class to all pointer managing mapped tables.
		*/
		template <class K, class C,class Hash=StdHash, typename KeyStrategy = typename StrategySelector<K>::Default>
			class GenericHashContainer:protected HashTable<K,C*,Hash,KeyStrategy,PrimitiveStrategy>
			{
			protected:
					typedef HashTable<K,C*,Hash,KeyStrategy,PrimitiveStrategy>	Root;
					typedef typename Root::Carrier								Carrier;
			

			public:
					typedef typename Root::DataType	DataType;		//!< Data type (pointer to)
					typedef typename Root::KeyType	KeyType;
			


													GenericHashContainer()	{};
					virtual							~GenericHashContainer();

					inline	void					importAndFlush(GenericHashContainer<K,C,Hash,KeyStrategy>&list);		//!< Imports the content of the specified other hashcontainer and flushs it. \param list Hashcontainer to import entries from.

					inline	void					clear();									//!< Resets the internal table to the initial size, deletes all contained entries and sets their pointers to NULL.
					inline	void					Clear()	/**@copydoc clear()*/	{clear();}
					inline	void					flush();									//!< Resets the internal table to the initial size and sets all contained entries to NULL (does not delete the objects).

				template <class Key>
					inline	DataType				define(const Key&ident, DataType element);		//!< Defines the pointer associated with the specified key. \param ident Key to define \param element Pointer to the data that should be associated. \return \b element if the specified key was not defined and \b element could be associated with it, NULL otherwise.
				template <class Key>
					inline	DataType				define(const Key&ident);					//!< Defines the pointer associated with the specified key. \param ident Key to define \return Pointer to the data associated with the specified key or to a new object if the key was not previously defined.
				template <class Key>
					inline	DataType				drop(const Key&ident);						//!< Drops the data associated with the specified key without deleting it. Succeeding calls to lookup(ident) will return NULL. \param ident Key to look for. \return Pointer to the associated data or NULL if \b ident could not be found.
				template <class Key>
					inline	bool					Erase(const Key&ident);

				template <class Key>
					inline	DataType				lookup(const Key&ident);					//!< Identical to lookup(ident)
				template <class Key>
					inline	const DataType			lookup(const Key&ident)				const;	//!< Identical to lookup(ident)

				template <class Key>
					inline	DataType				operator[](const Key&ident);				//!< Identical to lookup(ident)
				template <class Key>
					inline	const DataType			operator[](const Key&ident)			const;	//!< Identical to lookup(ident)
					inline	operator 				size_t()							const;	//!< Implicit conversion to size_t. \return Returns the number of entries in the table.
			
					using Root::IsSet;
					using Root::Count;
					using Root::Get;
					using Root::GetTotalSize;
					#ifndef __BORLANDC__
					using Root::ExportTo;
					#else
				template <class Key, class Entry>
					inline	void					ExportTo(Ctr::ArrayData<Key>&keys, Ctr::ArrayData<Entry>&values)	const
					{
						Root::ExportTo(keys,values);
					}
				template <class Entry>
					inline	void					ExportTo(Ctr::ArrayData<Entry>&values)		const
					{
						Root::ExportTo(values);
					}
					#endif
					using Root::Query;
					using Root::FindKeyOf;
			
			
			};

		/*!
			\brief String Hash container

			The hashcontainer stores object pointers mapped via string keys.
			The container manages the inserted pointers, deleting them if the hashcontainer is deleted.
		*/

		template <class C>
			class HashContainer:public GenericHashContainer<String,C,StdHash,SwapStrategy>
			{};


		/*!
			\brief Indexed container

			The index container stores object pointers mapped via indices. Anything that can be cast into an integer can be used as a key.
			The container manages the inserted pointers, deleting them if the hashcontainer is deleted.
		*/

		template <class C>
			class IndexContainer:public GenericHashContainer<index_t,C,StdHash,PrimitiveStrategy>
			{};


		/*!
			\brief Pointer	container

			The pointer container stores object pointers mapped via pointers. Anything that can be cast into a pointer can be used as a key.
			The container manages the inserted pointers, deleting them if the hashcontainer is deleted.
		*/

		template <class C>
			class PointerContainer:public GenericHashContainer<const void*,C, StdHash,PrimitiveStrategy>
			{};



		#include "hashtable.tpl.h"
	}
}

#endif
