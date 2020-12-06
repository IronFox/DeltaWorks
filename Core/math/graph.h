#ifndef graphH
#define graphH


/******************************************************************

Collection of graph-algorithms.

******************************************************************/

#include <memory>

#include "../container/queue.h"
#include "../container/buffer.h"

namespace DeltaWorks
{

	namespace Math
	{
		class Graph;

		namespace GraphDetails
		{
			struct BaseAttachment
			{
				virtual				~BaseAttachment()	{}
			};
			typedef std::shared_ptr<BaseAttachment>	PBaseAttachment;

			class Node;
			class Edge;

			typedef std::weak_ptr<Node>		WNode;
			typedef std::weak_ptr<Edge>		WEdge;
			typedef std::shared_ptr<Node>	PNode;
			typedef std::shared_ptr<Edge>	PEdge;


			class Indexed
			{
				friend class Graph;
			public:
				index_t			GetIndex() const {return index;}
				void			UpdateIndex(index_t idx) {index = idx;}
			private:
				index_t			index = InvalidIndex;
			};

			class Node : public Indexed
			{
			public:
				PBaseAttachment	attachment;

				/**/			Node();
				index_t			Scan(index_t dfsIn);
				bool			OutboundConnectedTo(const PNode&) const;
				bool			InboundConnectedTo(const PNode&) const;
				bool			OutboundConnectedTo(const PEdge&) const;
				bool			InboundConnectedTo(const PEdge&) const;
				bool			ConnectedTo(const PNode&n) const {return OutboundConnectedTo(n) || InboundConnectedTo(n);}
				ArrayRef<const WNode>	GetOutNeighbors() const {return out.ToRef().Const();}
				ArrayRef<const WEdge>	GetOutEdges() const {return outEdges.ToRef().Const();}
				ArrayRef<const WNode>	GetInNeighbors() const {return in.ToRef().Const();}
				ArrayRef<const WEdge>	GetInEdges() const {return inEdges.ToRef().Const();}
				template <typename Visitor>
					void		VisitAllNeighbors(const Visitor&f)	const
					{
						foreach (out,o)
							f(o->lock());
						foreach (in,i)
							f(i->lock());
					}
				template <typename Visitor>
					void		VisitAllEdges(const Visitor&f)	const
					{
						foreach (outEdges,o)
							f(o->lock());
						foreach (inEdges,i)
							f(i->lock());
					}
			private:
				bool            isSelected;
				int             inDegree;
				Buffer0<WNode>	in,
								out;
				Buffer0<WEdge>	inEdges,
								outEdges;
				count_t			dfsIn,
								dfsOut;

				friend class	Edge;
				friend class	Graph;
			};

			class Edge : public std::enable_shared_from_this<Edge>, public Indexed
			{
			public:
				void			LinkTo(const PNode&from, const PNode&to);
				virtual			~Edge()	{};
			private:
				PNode			from,
								to;
				friend class	Graph;
			};


			template <typename T>
				class Storage
				{
				public:
					typedef std::shared_ptr<T>	Item;
					typedef Buffer0<Item> Store;

					typedef typename Store::const_iterator const_iterator;

					const_iterator begin() const {return store.begin();}
					const_iterator end() const {return store.end();}
					
					void			GarbageCollect()
					{
						Buffer0<Item>	next;
						foreach (store,s)
							if ((*s))
							{
								(*s)->UpdateIndex(next.Count());
								next << *s;
							}
						store.swap(next);
						erased = 0;
					}
					bool			ShouldGarbageCollect() const	{return erased*2 > store.Count();}

					void			Clear()
					{
						store.Clear();
						erased = 0;
					}

					count_t			CountOccurrences(const Item&item) const
					{
						count_t cnt = 0;
						foreach (store,n1)
							if (item == *n1)
								cnt++;
						return cnt;
					}

					const Item&		operator[](index_t idx) const {return store[idx];}

					void			CheckValidity(const TCodeLocation&loc, const Item&item) const
					{
						if (!item)
							Except::TriggerFatal(loc,"Parameter is empty");
						if (item->GetIndex() >= store.Count() || item != store[item->GetIndex()])
							Except::TriggerFatal(loc,"Given item is not part of the local graph");
					}

					void			Erase(const Item&item)
					{
						if (item->GetIndex() >= store.Count() || item != store[item->GetIndex()])
							throw Except::Program::ParameterFault(CLOCATION,"Given node is not part of the local graph");
						store[item->GetIndex()].reset();
						erased++;
						if (ShouldGarbageCollect())
							GarbageCollect();
					}

					void			operator<<(const Item&item)
					{
						item->UpdateIndex(store.Count());
						store << item;
					}

					void			Export(Array<Item>&outArray) const
					{
						outArray.SetSize(store.Count() - erased);
						auto ptr = outArray.begin();
						foreach (store,s)
							if ((*s))
								(*ptr++) = *s;
						ASSERT__(ptr == outArray.end());
					}

					count_t			Count() const {return store.Count();}
				private:
					count_t			erased = 0;
					Store			store;
				};
		}

		class Graph
		{
		public:
			typedef GraphDetails::PNode	PNode;
			typedef GraphDetails::WNode	WNode;
			typedef GraphDetails::PEdge	PEdge;
			typedef GraphDetails::WEdge	WEdge;
			typedef GraphDetails::BaseAttachment NodeAttachment;
			typedef GraphDetails::PBaseAttachment PNodeAttachment;

			void				Clear()	{nodes.Clear(); edges.Clear();}
			PNode				AddNode();
			void				EraseNode(const PNode&n);
			PNode				MergeNodes(const PNode&a, const PNode&b);
			PEdge				AddEdge(const PNode&node0, const PNode&node1);
			bool				CreateTopologicalOrder(Container::BasicBuffer<PNode>&out);
			void				PerformDepthFirstSearch();
			void				VerifyIntegrity(bool thoroughly=false) const;

			void				ExportNodes(Array<PNode>&ar)	const	{nodes.Export(ar);}
			void				ExportEdges(Array<PEdge>&ar)	const	{edges.Export(ar);}
		private:
			count_t				CountOccurrences(const PNode&)	const;
			count_t				CountOccurrences(const PEdge&)	const;
			void				Unlink(const PNode&);
			void				MarkErased(const PEdge&);
			void				MarkErased(const PNode&);
			void				CheckValidity(const TCodeLocation&, const PNode&)	const;
			void				CheckValidity(const TCodeLocation&, const PEdge&)	const;

			GraphDetails::Storage<GraphDetails::Node>	nodes;
			GraphDetails::Storage<GraphDetails::Edge>	edges;

		};

	}


}


#endif
