#ifndef bvhH
#define bvhH


#include "../math/vector.h"
#include "../container/buffer.h"

#undef GetObject

namespace DeltaWorks
{
	struct NoBVHAttachment	{};


	template <typename Object, typename Float = float, typename Attachment = NoBVHAttachment>
		class BoxBVH
		{
		public:
			static const constexpr count_t NumAxes = 3;

			typedef M::Box<Float>	Bounds, BoxBounds;
			static void ProgressiveInclude(Bounds&box, Bounds&, const Bounds&bounds)
			{
				box.Include(bounds);
			}
			static void FinalizeEffectiveBounds(Bounds&effectiveBounds,const Bounds&boundingBox)
			{
				effectiveBounds = boundingBox;
			}
			static void IncludeCenterIntoBox(Bounds&box, const Bounds&bounds)
			{
				box.Include(bounds.center());
			}
			static Float CenterOf(const Bounds&b, int axis)
			{
				return b.axis[axis].center();
			}
		};

	template <typename Object, typename Float = float, typename Attachment = NoBVHAttachment>
		class SphereBVHNature
		{
		public:
			static const constexpr count_t NumAxes = 3;

			typedef M::Sphere<Float>	Bounds;
			typedef M::Box<Float>		BoxBounds;

			static void ProgressiveInclude(M::Box<Float>&box, Bounds&outBounds, const Bounds&bounds)
			{
				static M::Box<Float>	b;
				b.SetCenter(bounds.center,bounds.radius);
				box.Include(b);
				outBounds.Include(bounds);
			}
			static void FinalizeEffectiveBounds(Bounds&effectiveBounds,const M::Box<Float>&boundingBox)
			{
			
			}
			static void IncludeCenterIntoBox(M::Box<Float>&box, const Bounds&bounds)
			{
				box.Include(bounds.center);
			}
			static Float CenterOf(const Bounds&b, int axis)
			{
				return b.center.v[axis];
			}
	
		};

		/// <summary>
		/// Tuple that Contains both an element and its current bounding box.
		/// Primarily used internally, but may be used to more efficiently pass new entries to the hierarchy.
		/// </summary>
	template <typename Object, typename Bounds>
		struct BVHEntry
		{
			Object			element;
			Bounds			bounds;

			/**/			BVHEntry() :element(NULL)	{}
			/**/			BVHEntry(const Object&element, const Bounds&box) : element(element), bounds(box)	{}
		};

	template <typename Object, typename Bounds, count_t MaxObjectsPerLeaf>
		struct BVHStorage
		{
			Ctr::Array<Object>	objects;

			void			Store(const BVHEntry<Object, Bounds>*elements, count_t numElements)
			{
				objects.SetSize(numElements);
				for (index_t i = 0; i < numElements; i++)
					objects[i] = elements[i].element;
			}

			const Object&			Get(index_t object) const
			{
				return objects[object];
			}

			void			GetAll(Container::BasicBuffer<Object>&out) const
			{
				out.AppendAll(objects);
			}

			count_t			CountObjects() const { return objects.Count(); }
		};

	template <typename Object, typename Bounds>
		struct BVHStorage<Object,Bounds,1>
		{
			Object			object;

			void			Store(const BVHEntry<Object, Bounds>*elements, count_t numElements)
			{
				DBG_ASSERT__(numElements == 1);
				object = elements[0].element;
			}

			const Object&	Get(index_t objectIndex) const
			{
				DBG_ASSERT__(objectIndex == 0);
				return object;
			}
			void			GetAll(Container::BasicBuffer<Object>&out) const
			{
				out << object;
			}

			count_t			CountObjects() const { return 1; }
		};


	template <typename Object, count_t MaxObjectsPerLeaf, typename Float = float, typename Attachment = NoBVHAttachment, typename Nature = BoxBVH<Object, Float, Attachment> >
		class BVH
		{
		public:
			typedef typename Nature::Bounds		Bounds;
			typedef typename Nature::BoxBounds	BoxBounds;
	
	

			typedef BVHEntry<Object, Bounds>	Entry;


			struct TCompactNode
			{
				union
				{
					Object		object;		//!< occupied if this IS a leaf-node
					TCompactNode*secondChild;	//!< occupied if this is not a leaf-node
				};
				Bounds			bounds;
				char			axis;	//!< is -1 if this is a leaf-node
			};


		private:
			class Node
			{
			public:
				const bool		isLeaf;
				const Bounds 	bounds;
				Attachment		attachment;


				/**/			Node(bool isLeaf, const Bounds&bounds) : isLeaf(isLeaf),bounds(bounds)	{}
				virtual			~Node()	{}
				count_t			RecursiveCount() const;
			};

			class LeafNode : public Node
			{
			public:
				BVHStorage < Object, Bounds, MaxObjectsPerLeaf >
								storage;
				//Object*			object;
			

				/**/			LeafNode(const Bounds&bounds) : Node(true, bounds) { }
			};

			class InnerNode : public Node
			{
			public:
				Node			*child0,
								*child1;
				int				axis;

				/**/			InnerNode(const Bounds&bounds) : Node(false, bounds), child0(NULL),child1(NULL),axis(0) { }
				virtual			~InnerNode()	{if (child0) delete child0; if (child1) delete child1;}
			};

			Node				*_root;
			Container::Buffer<Entry>_allItems;
			bool				_locked;
			count_t				_numBuckets;



			void				_RebuildIfNeeded()
			{
				//lock (this)
				{
					if (!_root && _allItems.IsNotEmpty())
						ForceReconstruction();
				}
			}

		public:
			class Accessor
			{
			private:
				Node*			_GetNode()	{return reinterpret_cast<Node*>(this);}
				const Node*		_GetNode()	const {return reinterpret_cast<const Node*>(this);}
			public:
				count_t			CountObjects()	const	{const Node*n = _GetNode(); return n->isLeaf ? ((LeafNode*)n)->storage.CountObjects() : 0;}
				void			GetObject(Object&, index_t index=0)	const;
				bool			IsLeaf() const	{return _GetNode()->isLeaf;}
				Accessor*		GetFirstChild() const {const Node*n = _GetNode(); return n->isLeaf ? NULL : reinterpret_cast<Accessor*>(((InnerNode*)n)->child0);}
				Accessor*		GetSecondChild() const {const Node*n = _GetNode(); return n->isLeaf ? NULL : reinterpret_cast<Accessor*>(((InnerNode*)n)->child1);}
				const Bounds&	GetBounds() const {return _GetNode()->bounds;}
				Attachment&		GetAttachment()	{return _GetNode()->attachment;}
			};

			typedef Accessor*	iterator;


			/**/				BVH():_root(NULL),_locked(false),_numBuckets(16)	{}
			/**/				~BVH()	{if (_root) delete _root;}

			void				DisallowAutomaticReconstruction()	{_locked = true;}
			void				AllowAutomaticReconstruction()	{_locked = false;_RebuildIfNeeded();}

			void				Insert(const Object&element, const Bounds&space)
			{
				Insert(Entry(element, space));
			}
			void				Insert(const Entry&entry)
			{
				//lock(_allItems)
				{
					_allItems << entry;
					if (!_locked)
						ForceReconstruction();
					else
					{
						if (_root)
							delete _root;
						_root = NULL;
					}
				}
			}

			void				Insert(const Entry*entries, count_t numEntries)
			{
				//lock (_allItems)
				{
					_allItems.Append(entries,numEntries);
					if (!_locked)
						ForceReconstruction();
					else
					{
						if (_root)
							delete _root;
						_root = NULL;
					}
				}
			}
			void				Insert(const Ctr::ArrayData<Entry>&entries)	{Insert(entries.pointer(),entries.Count());}

			count_t				Count() const
			{
				return _allItems.Count();
			}

			void				Remove(const Object&element)
			{
				//lock(_allItems)
					for (index_t i = 0; i < _allItems.Count(); i++)
						if (_allItems[i].element == element)
						{
							_allItems.Erase(i);
							if (!_locked)
								ForceReconstruction();
							else
							{
								if (_root)
									delete _root;
								_root = NULL;
							}
							return;
						}
			}

		private:
			class Bucket
			{
			public:
				count_t				numElements;
				Bounds				bounds;;
				float				volumeUntilThis,
									volumeFromThis;

				/**/				Bucket()	{Reset();}

				void				Reset()
				{
					bounds = Bounds::Invalid;
					volumeUntilThis = 0;
					volumeFromThis = 0;
					numElements = 0;
				}

			};

			static TCompactNode* _Compact(Node*node,TCompactNode*&outField)
			{
				TCompactNode*_this = outField;
				outField++;
				if (node->isLeaf)
				{
					_this->object = ((LeafNode*)node)->storage.Get(0);
					_this->axis = -1;
					_this->bounds = node->bounds;
					return _this;
				}
				_this->axis = ((InnerNode*)node)->axis;
				_this->bounds = node->bounds;
				_Compact(((InnerNode*)node)->child0,outField);
				_this->secondChild = _Compact(((InnerNode*)node)->child1,outField);
				return _this;
			}

			static Node*		_BuildNode(Entry*const elements, const count_t numElements, Ctr::ArrayData<Bucket>&buckets)
			{
				BoxBounds boundingBox = BoxBounds::Invalid;
				Bounds	effectiveBounds = Bounds::Invalid;

				for (index_t i = 0; i < numElements; i++)
					Nature::ProgressiveInclude(boundingBox,effectiveBounds, elements[i].bounds);
				Nature::FinalizeEffectiveBounds(effectiveBounds,boundingBox);
				if (numElements <= MaxObjectsPerLeaf)
				{
					LeafNode*leaf = new LeafNode(effectiveBounds);
					leaf->storage.Store(elements, numElements);
		 //           leaf->
						//elements.resizeAndCopy(elements, numElements);
					return leaf;
				}


				BoxBounds centerVolume = BoxBounds::Invalid;
				for (index_t i = 0; i < numElements; i++)
					Nature::IncludeCenterIntoBox(centerVolume,elements[i].bounds);

				int useAxis = 0;
				Float dist = centerVolume.axis[0].GetExtent();
				for (int i = 1; i < Nature::NumAxes; i++)
				{
					Float e = centerVolume.axis[i].GetExtent();
					if (e > dist)
					{
						dist = e;
						useAxis = i;
					}
				}
				const M::TFloatRange<Float> axisRange2 = centerVolume.axis[useAxis];
				const Float extend2 = axisRange2.GetExtent();

				if (extend2 == 0)
				{
					LeafNode*leaf = new LeafNode(effectiveBounds);
					leaf->storage.Store(elements, numElements);
					return leaf;
				}


				for (index_t i = 0; i < numElements; i++)
				{
					const Entry&e = elements[i];
					index_t bucketIndex = std::min(buckets.Count() - 1, (index_t)(float(Nature::CenterOf(e.bounds,useAxis) - axisRange2.min) / extend2 * buckets.Count()));
					Bucket&bucket = buckets[bucketIndex];
					bucket.numElements ++;
					//DBG_ASSERT_GREATER__(e.bounds.GetVolume(),0);
					bucket.bounds.Include(e.bounds);
					//DBG_ASSERT_GREATER__(bucket.bounds.GetVolume(),0);
				}

				index_t from = 0, to = buckets.Count()-1;
				while (buckets[from].numElements == 0)
					from++;
				while (buckets[to].numElements == 0)
					to--;
				Bounds bounds = buckets[from].bounds;
				float volume = buckets[from].numElements;
				//bounds.GetVolume(volume);
				buckets[from].volumeUntilThis = volume;
				for (index_t i = from+1; i <= to; i++)
				{
					if (buckets[i].numElements > 0)
					{
						bounds.Include(buckets[i].bounds);
						//bounds.GetVolume(volume);
						volume += buckets[i].numElements;
						//DBG_ASSERT_GREATER__(volume,0);
					}
					buckets[i].volumeUntilThis = volume;
					// Debug.WriteLine("volumeUntil(axis " + axis + ", bucket " + i + ") = " + volume);

				}
				bounds = buckets[to].bounds;
				volume = buckets[to].numElements;
				//bounds.GetVolume(volume);
				DBG_ASSERT_GREATER__(volume,0);
				buckets[to].volumeFromThis = volume;
				for (index_t i = to - 1; i < buckets.Count(); i--)
				{
					if (buckets[i].numElements > 0)
					{
						bounds.Include(buckets[i].bounds);
						volume += buckets[i].numElements;
						//bounds.GetVolume(volume);
						//DBG_ASSERT_GREATER__(volume,0);
					}
					buckets[i].volumeFromThis = volume;
					//Debug.WriteLine("volumeFrom(axis " + axis + ", bucket " + i + ") = " + volume);
				}

				Float minVolume = std::numeric_limits<Float>::max();
				index_t splitAtBucket = from;
				for (index_t b = from+1; b <= to; b++)
				{
					volume = M::sqr(buckets[b-1].volumeUntilThis) + M::sqr(buckets[b].volumeFromThis);
					if (volume < minVolume)
					{
						minVolume = volume;
						splitAtBucket = b;
					}
					//Debug.WriteLine("volume(axis " + axis + ", split " + b + ") = " + volume);
				}
				foreach (buckets,b)
					b->Reset();





				Entry	*i0 = elements,
						*i1 = elements + numElements -1;
				do
				{
					while (i0 < i1)
					{

						index_t bucketIndex = std::min(buckets.Count() - 1, (index_t)(float(Nature::CenterOf(i0->bounds,useAxis) - axisRange2.min) / extend2 * buckets.Count()));
							//std::min(buckets.Count() - 1, (index_t)((i0->boundingBox.axis[useAxis].center() - axisRange2.min) / extend2 * buckets.Count()));
						if (bucketIndex >= splitAtBucket)
							break;
						i0++;
					}
					while (i0 < i1)
					{
						index_t bucketIndex = std::min(buckets.Count() - 1, (index_t)(float(Nature::CenterOf(i1->bounds,useAxis) - axisRange2.min) / extend2 * buckets.Count()));
						if (bucketIndex < splitAtBucket)
							break;
						i1--;
					}

					if (i0 < i1)
						swp(*i0,*i1);
				}
				while (i0 < i1);

				InnerNode*innerNode = new InnerNode(effectiveBounds);
				innerNode->axis = useAxis;
				innerNode->child0 = _BuildNode(elements, i0-elements, buckets);
				innerNode->child1 = _BuildNode(i0, elements + numElements - i0, buckets);
				return innerNode;
			}


			static void _WalkNode(Node*node, const Bounds&space, Container::BasicBuffer<Object>&found)
			{
				if (node->isLeaf)
				{
					LeafNode*leaf = (LeafNode*)node;
					if (space.Intersects(leaf->bounds))
					{
						leaf->storage.GetAll(found);
					}
						//foreach (leaf->elements,e)
						//    if (space.Intersects(e->bounds))
						//        found << e->element;
					return;
				}
				InnerNode*n = (InnerNode*)node;
				if (space.Intersects(n->bounds))
				{
					_WalkNode(n->child0, space, found);
					_WalkNode(n->child1, space, found);
				}
			}
			static count_t _WalkNode(Node*node, const Bounds&space)
			{
				if (node->isLeaf)
				{
					count_t result = 0;
					LeafNode*leaf = (LeafNode*)node;
					if (space.Intersects(leaf->bounds))
						result++;
		   //             foreach (leaf->elements,e)
		   //                 if (space.Intersects(e->bounds))
								//result ++;
					return result;
				}
				InnerNode*n = (InnerNode*)node;
				if (space.Intersects(n->bounds))
				{
					return _WalkNode(n->child0, space) + _WalkNode(n->child1, space);
				}
				return 0;
			}
			static void _RecursivelyWalkNode(Node*node, const M::TVec<Float,Nature::NumAxes>&point, Container::BasicBuffer<Object>&found)
			{
				if (node->isLeaf)
				{
					LeafNode*leaf = (LeafNode*)node;
					if (leaf->bounds.Contains(point))
						//foreach (leaf->elements,e)
						//    if (e->bounds.Contains(point))
						leaf->storage.GetAll(found);
					return;
				}
				InnerNode*n = (InnerNode*)node;
				if (n->bounds.Contains(point))
				{
					_RecursivelyWalkNode(n->child0, point, found);
					_RecursivelyWalkNode(n->child1, point, found);
				}
			}

			static void _RecursivelyFindClosest(Node*node, const M::TVec<Float,Nature::NumAxes>&point, Object&closest, Float&distance)
			{
				if (node->isLeaf)
				{
					M::TVec<Float,Nature::NumAxes>	c;
					LeafNode*leaf = (LeafNode*)node;
					if (leaf->bounds.Contains(point))
						//foreach (leaf->elements,e)
						//    if (e->bounds.Contains(point))
							{
								leaf->bounds.GetCenter(c);
								Float d = M::Vec::quadraticDistance(c, point);
								if (d < distance)
								{
									distance = d;
									closest = leaf->storage.Get(0);
								}
							}
					return;
				}
				InnerNode*n = (InnerNode*)node;
				if (n->bounds.Contains(point))
				{
					_RecursivelyFindClosest(n->child0, point, closest, distance);
					_RecursivelyFindClosest(n->child1, point, closest, distance);
				}
			}

		public:
			void ForceReconstruction(bool verbose = false)
			{
				//lock(_allItems)
				{
					if (_root)
						delete _root;
					_root = NULL;
					if (_allItems.IsEmpty())
					{
						//if (verbose)
						  //  Debug.WriteLine("Tree is empty");
						//_root = null;
						return;
					}
					Ctr::Array<Bucket>	buckets(_numBuckets);
					_root = _BuildNode(_allItems.pointer(),_allItems.Count(),buckets);
					//if (verbose)
					  //  printNode(_root,0);
				}
			}

			count_t	GetBucketCount() const {return _numBuckets;}
			void	SetBucketCount(index_t count) {_numBuckets = count;}

			bool	IsEmpty() const	{return _allItems.IsEmpty();}
			bool	IsNotEmpty() const {return _allItems.IsNotEmpty();}

			void Lookup(const Bounds&space, Container::BasicBuffer<Object>&found)	const
			{
				Node*root = _root;
				if (!root)
					return;
				_WalkNode(root, space, found);
			}
			count_t CountHits(const Bounds&space)	const
			{
				Node*root = _root;
				if (!root)
					return 0;
				return _WalkNode(root, space);
			}
			void Lookup(const M::TVec<Float,Nature::NumAxes>&position, Container::BasicBuffer<Object>&found)	const
			{
				Node*root = _root;
				if (!root)
					return;
				_RecursivelyWalkNode(root, position, found);
			}
			bool LookupClosest(const M::TVec<Float,Nature::NumAxes>&position, Object&outResult)	const
			{
				Node*root = _root;
				if (!root)
					return false;
				Float distance = std::numeric_limits<Float>::max();
				_RecursivelyFindClosest(root, position, outResult, distance);
				return distance < std::numeric_limits<Float>::max();
			}
			void Clear()
			{
				//lock (this)
				{
					_allItems.clear();
					if (_root)
						delete _root;
					_root = NULL;
				}
			}

			void	Compact(Ctr::Array<TCompactNode>&outNodes)
			{
				Node*root = _root;
				outNodes.SetSize(root->RecursiveCount());
				TCompactNode*n = outNodes.pointer();
				_Compact(root,n);
				ASSERT__(n == outNodes.end());
			}

			iterator	GetRoot()	const
			{
				return reinterpret_cast<Accessor*>(_root);
			}
		};



	template <typename Object, count_t MaxObjectsPerLeaf, typename Float, typename Attachment, typename Nature>
		void			BVH<Object,MaxObjectsPerLeaf,Float,Attachment,Nature>::Accessor::GetObject(Object&rs, index_t index /*=0*/)	const
		{
			const Node*n = _GetNode();
			if (!n->isLeaf)
				return;
			const LeafNode*l = (LeafNode*)n;
			rs = l->storage.Get(index);
		}


	template <typename Object, count_t MaxObjectsPerLeaf, typename Float, typename Attachment, typename Nature>
		count_t			BVH<Object,MaxObjectsPerLeaf,Float,Attachment,Nature>::Node::RecursiveCount() const
		{
			if (isLeaf)
				return 1;
			return 1 + ((InnerNode*)this)->child0->RecursiveCount() + ((InnerNode*)this)->child1->RecursiveCount();
		}
	
	
	
	template <typename Object, count_t MaxObjectsPerLeaf,  typename Float = float, typename Attachment = NoBVHAttachment>
		class SphereBVH : public BVH<Object, MaxObjectsPerLeaf,Float, Attachment, SphereBVHNature<Object, Float, Attachment> >
		{};
	
	
	
	
	
}	
		
	

#endif
