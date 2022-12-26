#include "entity_tree.h"

namespace DeltaWorks
{
	namespace C = Container;

	bool			CompositeEntityTree::verbose(false),
					EntityTree::verbose(false);

	using namespace Math;

	static bool		intersect(const CompositeEntityTree::Volume&a, const CompositeEntityTree::Volume&b, float sector_size)
	{
		M::TVec3<> dif;
		Composite::sub(a.lower,b.upper,dif,sector_size);
		if (M::Vec::oneGreater(dif,Vector3<>::zero))
			return false;
	
		Composite::sub(b.lower,a.upper,dif,sector_size);
		if (M::Vec::oneGreater(dif,Vector3<>::zero))
			return false;
	
		return true;
	}


	static bool		intersect(const Composite::Coordinates&e0, const Composite::Coordinates&e1, const CompositeEntityTree::Volume&b, float sector_size)
	{
		M::TVec3<double>	_e0,_e1,box_extend;
		Composite::sub(e0,b.lower,_e0,sector_size);
		Composite::sub(e1,b.lower,_e1,sector_size);
		Composite::sub(b.upper,b.lower,box_extend,sector_size);

		M::Box<double>	box(Vector3<double>::zero,box_extend);
		return _oIntersectsBox(_e0, _e1, box);
	}



	static bool		intersect(const EntityTree::Volume&a, const EntityTree::Volume&b)
	{
		return a.Intersects(b);
		//M::TVec3<> dif;
		//if (M::Vec::oneGreater(a.lower,b.upper))
		//	return false;
		//
		//if (M::Vec::oneGreater(b.lower,a.upper))
		//	return false;
		//
		//return true;
	}



	void		CompositeEntityTree::recursiveRemap(const C::PreAllocVector<Entity*>&source, float sector_size)
	{
		if (verbose)
			std::cout << "now processing node on level "<<level<<" with "<<source.GetLength()<<" input entities"<<std::endl;
	
		bool self = &source == &entities;

		if (!self)
			entities.reset();
		if (has_children)
		{
			for (BYTE k = 0; k < 8; k++)
				if (child[k])
				{
					Discard(child[k]);
					child[k] = NULL;
				}
			has_children = false;
		}
		split.reset();

		if (verbose)
			std::cout << " determining center"<<std::endl;
		for (unsigned i = 0; i < source.GetLength(); i++)
		{
			Entity*entity = source[i];
		
		
			if (!intersect(volume,entity->volume,sector_size))
				continue;
		
			Composite::Coordinates center;
			Composite::center(entity->volume.lower,entity->volume.upper,center,sector_size);
		
			Composite::add(split,center,split);

			if (!self)
				entities << entity;
		}
		entities.compact();
		if (verbose)
			std::cout << " compactified. keeping "<<entities.Count()<<" relevant entity/ies"<<std::endl;
	
		if (!level || entities.Count()<2)
		{
			if (verbose)
				if (entities.Count()<2)
					std::cout << " insufficient entities given for further subdivision. terminating effort"<<std::endl;
			return;
		}
		split.divInt(int(entities.Count()),sector_size);
		if (verbose)
			std::cout << " center is "<<split.ConvertToString(sector_size)<<std::endl;
	
	
		BYTE greatest(0),collapsed(0);
		Composite::Scalar greatest_range(0,0);
	
		Composite::Coordinates new_split;
		for (BYTE k = 0; k < 3; k++)
		{
			if (verbose)
				std::cout << "  processing axis "<<(int)k<<std::endl;
			Composite::Scalar				delta,delta2,lower,upper,val,original_split;
	
			Composite::sub(volume.upper.axis(k),volume.lower.axis(k),delta);
			delta.divInt(5,sector_size);
		
			//determine lower and upper boundaries:
			Composite::add(volume.lower.axis(k),delta,lower);
			Composite::sub(volume.upper.axis(k),delta,upper);
		
			lower.wrap(sector_size);
			upper.wrap(sector_size);
		
			if (lower.CompareTo(split.axis(k),sector_size)>0)
				split.setAxis(k,lower);
			
			if (upper.CompareTo(split.axis(k),sector_size)<0)
				split.setAxis(k,upper);
		
			split.getAxis(k,val);
			split.getAxis(k,original_split);
		
			bool moved;
		
			moved = true;
			while (moved)
			{
				moved = false;
				for (unsigned i = 0; i < entities.Count(); i++)
				{
					Entity*object = entities[i];
				
					Composite::Scalar upper,lower,separation;
					object->volume.lower.getAxis(k,lower);
					object->volume.upper.getAxis(k,upper);
					Composite::sub(upper,lower,separation);
					separation.divInt(20,sector_size);
				
					if (lower.CompareTo(val,sector_size)<0 && upper.CompareTo(val,sector_size)>0)
					{
						if (verbose)
						{
							std::cout << "  entity "<<i<<" intersects new split at "<<val.ConvertToString(sector_size)<<std::endl;
							std::cout << "   entity range is "<<lower.ConvertToString(sector_size)<<" - "<<upper.ConvertToString(sector_size)<<std::endl;
						}
				
						if (val.CompareTo(original_split,sector_size)>0)
						{
							if (verbose)
								std::cout << "   moving new split to upper entity boundary"<<std::endl;
							Composite::add(upper,separation,val);
						}
						elif (val.CompareTo(original_split,sector_size)<0)
						{
							if (verbose)
								std::cout << "   moving new split to lower entity boundary"<<std::endl;
							Composite::sub(lower,separation,val);
						}
						else
						{
							if (verbose)
								std::cout << "   rare case"<<std::endl;
							Composite::Scalar center;
							Composite::center(object->volume.upper.axis(k),object->volume.lower.axis(k),center,sector_size);
							if (center.CompareTo(val,sector_size)>0)
							{
								if (verbose)
									std::cout << "   moving new split to lower entity boundary"<<std::endl;
								Composite::sub(lower,separation,val);
							}
							else
							{
								if (verbose)
									std::cout << "   moving new split to upper entity boundary"<<std::endl;
								Composite::add(upper,separation,val);
							}
						}
						//val.mult(1.01,sector_size);
						moved = true;
					}
				}
			}
			if (verbose)
				std::cout << " split vector component determined at "<<val.ConvertToString(sector_size)<<std::endl;
	
			if (val.CompareTo(volume.lower.axis(k),sector_size)<0)
				val = volume.lower.axis(k);
			
			if (val.CompareTo(volume.upper.axis(k),sector_size)>0)
				val = volume.upper.axis(k);

			if (verbose)
				std::cout << " split vector component clamped to "<<val.ConvertToString(sector_size)<<std::endl;

			delta.divInt(2,sector_size);
		
			Composite::add(volume.lower.axis(k),delta,lower);
			Composite::sub(volume.upper.axis(k),delta,upper);
		
			if (val.CompareTo(lower,sector_size)<0)
			{
				collapsed ++;
				val = lower;
			}
			elif (val.CompareTo(upper,sector_size)>0)
			{
				collapsed ++;
				val = upper;
			}
		
		
			new_split.setAxis(k,val);
			if (verbose)
				std::cout << " final split vector component determined at "<<val.ConvertToString(sector_size)<<std::endl;
		
			if (delta.CompareTo(greatest_range,sector_size)>0)
			{
				greatest_range = delta;
				greatest = k;
			}
		}
	
		if (collapsed == 3)
		{
			if (verbose)
				std::cout << " all collapsed"<<std::endl;
	
			if (entities.Count() > 3)
			{
				if (verbose)
					std::cout << "  but split required"<<std::endl;
				Composite::Scalar original = split.axis(greatest);
				split = new_split;
				split.setAxis(greatest,original);
			}
			else
			{
				return;
			}
		}
		else
			split = new_split;
	
	
		BYTE	num_children=0,
				unary_child = 0;
		for (BYTE k = 0; k < 8; k++)
		{
			BYTE p[3] = { (BYTE)(k / 4),
						 (BYTE)(k % 4 / 2),
						 (BYTE)(k % 4 % 2)};
			Volume d;
			bool collapsed(false);
			for (BYTE j = 0; j < 3; j++)
			{
				d.lower.setAxis(j,p[j]?split.axis(j):volume.lower.axis(j));
				d.upper.setAxis(j,p[j]?volume.upper.axis(j):split.axis(j));
				collapsed |= d.upper.separation(j,d.lower,sector_size) < volume.upper.separation(j,volume.lower,sector_size)/20;
			}
		
			if (!collapsed)
			{
				child[k] = SignalNew(new CompositeEntityTree(d, level-1));
				num_children++;
				unary_child = k;
			}
			else
				child[k] = NULL;
		}
		has_children = true;
	
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
			{
				child[k]->recursiveRemap(entities,sector_size);
				if (!child[k]->entities.Count())
				{
					Discard(child[k]);
					child[k] = NULL;
					num_children--;
					if (k == unary_child && num_children == 1)
						for (BYTE k2 = 0; k2 < 8; k2++)
							if (child[k2])
								unary_child = k2;
					
				}
			}
		if (num_children==1 && !child[unary_child]->has_children)
		{
			Discard(child[unary_child]);
			child[unary_child] = NULL;
			has_children = false;
		}
	}


	count_t	CompositeEntityTree::recursiveLookup(const Composite::Coordinates&edge_point0, const Composite::Coordinates&edge_point1, C::PreAllocVector<Entity*>&buffer, float sector_size)
	{
		count_t cnt = entities.Count();
		if (!cnt)
			return 0;
		if (!level)
		{
			count_t c = 0;
			for (index_t i = 0; i < entities.Count(); i++)
			{
				Entity*object = entities[i];
				if (!intersect(edge_point0, edge_point1, object->volume,sector_size))
					continue;
				buffer << object;
				c++;
			}
			return c;
		}
		const Composite::Coordinates	edge[2] = {edge_point0,edge_point1};
		//M::TVec3<double>			half;
		//Composite::center(volume.lower,volume.upper,half,sector_size);
		bool set[8] = {false,false,false,false,false,false,false,false};
		BYTE define[3];

		for (BYTE j = 0; j < 2; j++)
				if (!Composite::oneLess(edge[j],volume.lower,sector_size)
					&&
					!Composite::oneGreater(edge[j],volume.upper,sector_size))
				{
					for (BYTE k = 0; k < 3; k++)
						define[k] = edge[j].axis(k).CompareTo(split.axis(k),sector_size) > 0;
					BYTE k = define[0] *4+ define[1] *2 + define[2];
					set[k] = true;
				}
		M::TVec3<double>	dir,p0,dsplit;
		Composite::sub(edge_point1, edge_point0, dir, sector_size);
		edge_point0.convertToAbsolute(p0,sector_size);
		split.convertToAbsolute(dsplit,sector_size);
		for (BYTE k = 0; k < 3; k++)
			if (dir.v[k])
			{
				BYTE	x = (k+1)%3,
						y = (k+2)%3;
				for (BYTE j = 0; j < 2; j++)
				{
						define[k] = j;
						double alpha = (Composite::sub(volume.coordinates(j).axis(k),edge_point0.axis(k)).toDouble(sector_size))/dir.v[k];
						if (alpha > 1 || alpha < 0)
							continue;
						double	vx = p0.v[x] + dir.v[x] *alpha,
								vy = p0.v[y] + dir.v[y] *alpha;
						if (!volume.containsAlongAxis(x,vx,sector_size) || !volume.containsAlongAxis(y,vy,sector_size))
							continue;
						define[x] = vx > dsplit.v[x];
						define[y] = vy > dsplit.v[y];
						set[define[0]*4 + define[1]*2 + define[2]] = true;
				}
				double	alpha = (dsplit.v[k] - p0.v[k]) / dir.v[k];
				if (alpha > 1 || alpha < 0)
					continue;
				double	vx = p0.v[x] + dir.v[x] *alpha,
						vy = p0.v[y] + dir.v[y] *alpha;
				if (!volume.containsAlongAxis(x,vx,sector_size) || !volume.containsAlongAxis(y,vy,sector_size))
					continue;
				define[k] = 0;
				define[x] = vx > dsplit.v[x];
				define[y] = vy > dsplit.v[y];
				set[define[0] *4 + define[1]*2 + define[2]] = true;
				define[k] = 1;
				set[define[0] *4 + define[1]*2 + define[2]] = true;
			}
		index_t at = buffer.Count();
		for (BYTE k = 0; k < 8; k++)
			if (set[k])
				if (child[k]->recursiveLookup(edge_point0,edge_point1,buffer,sector_size) == entities.Count())
				{
					buffer.Truncate(at);
					for (index_t i = 0; i < entities.Count(); i++)
						buffer << entities[i];
					return entities.Count();
				}
		return 0;
	}



	count_t	CompositeEntityTree::recursiveLookup(const Volume&space, C::PreAllocVector<Entity*>&buffer, float sector_size)
	{
		const count_t cnt = entities.Count();
	
		if (!cnt || !intersect(space,volume,sector_size))
		{
			return 0;
		}
		if (!level || 8*level > cnt || !has_children)
		{
			count_t c = 0;
			for (index_t i = 0; i < cnt; i++)
			{
				Entity*object = entities[i];
				if (!intersect(space,object->volume,sector_size))
					continue;
				buffer << object;
				c++;
			}
			return c;
		}
	
		count_t at = buffer.Count();
		for (BYTE k = 0; k < 8; k++)
			if (child[k] && child[k]->recursiveLookup(space,buffer,sector_size) == cnt)
			{
				if (buffer.Count()-at > cnt)
				{
					buffer.Truncate(at);
					for (unsigned i = 0; i < cnt; i++)
						buffer << entities[i];
				}
				return cnt;
			}
		return 0;
	}


	CompositeEntityTree::CompositeEntityTree(const Volume&space, unsigned level_):volume(space),level(level_)
	{
		for (BYTE k = 0; k < 8; k++)
			child[k] = NULL;
		has_children = false;
	}

	CompositeEntityTree::CompositeEntityTree(const CompositeEntityTree&other)
	{
		for (unsigned i = 0; i < other.entities.Count(); i++)
			entities << other.entities[i];

		if (level && entities.Count())
		{
			for (BYTE k = 0; k < 8; k++)
				child[k] = SignalNew(new CompositeEntityTree(*other.child[k]));
			has_children = true;
		}
		else
		{
			for (BYTE k = 0; k < 8; k++)
				child[k] = NULL; //we will see if we actually need this
			has_children = false;
		}
	}

	CompositeEntityTree::CompositeEntityTree():level(1)
	{
		for (BYTE k = 0; k < 8; k++)
			child[k] = NULL;
		has_children = false;
	}


	CompositeEntityTree::~CompositeEntityTree()
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
				Discard(child[k]);
	}

	void		CompositeEntityTree::clear()
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
			{
				Discard(child[k]);
				child[k] = NULL;
			}
		entities.reset();
		has_children = false;
	}

	void		CompositeEntityTree::remap(const C::PreAllocVector<Entity*>&source, float sector_size, unsigned depth)
	{
		this->sector_size = sector_size;

		clear();
		if (!source.Count())
			return;

		level = depth?depth:(unsigned)log((float)source.GetLength());	//may need to adjust this later
		//ShowMessage(IntToStr(source->Count())+" => "+IntToStr(level));

		{
			Entity*object = source.First();
			volume = object->volume;
		}
		for (unsigned i = 1; i < source.Count(); i++)
		{
			Entity*entity = source[i];
		
			Composite::min(volume.lower,entity->volume.lower,volume.lower,sector_size);
			Composite::max(volume.upper,entity->volume.upper,volume.upper,sector_size);
		}
		if (verbose)
			std::cout << "remapping entities from buffer. volume is "<<volume.lower.ConvertToString(sector_size)<<" <-> "<<volume.upper.ConvertToString(sector_size)<<std::endl;
	
		recursiveRemap(source,sector_size);
	}


	void	CompositeEntityTree::lookup(const Composite::Coordinates&edge_point0, const Composite::Coordinates&edge_point1, C::PreAllocVector<Entity*>&out)
	{
		recursiveLookup(edge_point0, edge_point1, out,sector_size);
	}


	void	CompositeEntityTree::lookup(const Volume&space, C::PreAllocVector<Entity*>&out)
	{
		recursiveLookup(space,out,sector_size);
	
		/*
		Buffer<Entity*>					buffer;
		unsigned offset = out.Count();
		for (unsigned i = 0; i < buffer.Count(); i++)
		{
			Entity*object = buffer[i];
			if (!object->added)
			{
				out << object;
				object->added = true;
			}
		}
		for (unsigned i = offset; i < out.Count(); i++)
			out[i]->added = false;*/
	}

	CompositeEntityTree&	CompositeEntityTree::operator=(const CompositeEntityTree&other)
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
				Discard(child[k]);
		entities.reset();
		for (unsigned i = 0; i < other.entities.Count(); i++)
			entities << other.entities[i];

		if (level && entities.Count())
			for (BYTE k = 0; k < 8; k++)
				child[k] = SignalNew(new CompositeEntityTree(*other.child[k]));
		else
			for (BYTE k = 0; k < 8; k++)
				child[k] = NULL; //we will see if we actually need this
		return *this;
	}

	CompositeEntityTree*						CompositeEntityTree::getChild(BYTE c)
	{
		if (c >= 8)
			return NULL;
		return child[c];
	}

	bool										CompositeEntityTree::hasChildren()	const
	{
		return has_children;
	}


	unsigned									CompositeEntityTree::getLevel()	const
	{
		return level;
	}

	const Composite::Coordinates&								CompositeEntityTree::getSplitVector()	const
	{
		return split;
	}

	const CompositeEntityTree::Volume&					CompositeEntityTree::getVolume()	const
	{
		return volume;
	}


	count_t					CompositeEntityTree::countElements()	const
	{
		return entities.Count();
	}

	void										CompositeEntityTree::getElements(C::PreAllocVector<Entity*>&out)	const
	{
		out.reset();
		for (index_t i = 0; i < entities.Count(); i++)
			out << entities[i];
	}

	const C::PreAllocVector<CompositeEntityTree::Entity*>& 	CompositeEntityTree::getElementList() const
	{
		return entities;
	}



















	void		EntityTree::_RecursiveRemap(const C::PreAllocVector<Entity*>&source)
	{
		if (verbose)
			std::cout << "now processing node on level "<<level<<" with "<<source.GetLength()<<" input entities"<<std::endl;
	
		bool self = &source == &entities;
		if (!self)
			entities.reset();
		if (hasChildren)
		{
			for (BYTE k = 0; k < 8; k++)
				if (child[k])
				{
					Discard(child[k]);
					child[k] = NULL;
				}
			hasChildren = false;
		}
		M::Vec::clear(split);

		if (verbose)
			std::cout << " determining center"<<std::endl;
		for (unsigned i = 0; i < source.GetLength(); i++)
		{
			Entity*entity = source[i];
		
		
			if (!intersect(volume,entity->volume))
				continue;
		
			M::TVec3<> center = entity->volume.center();
			//M::Vec::center(entity->volume.lower,entity->volume.upper,center);
		
			M::Vec::add(split,center);

			if (!self)
				entities << entity;
		}
		entities.compact();
		if (verbose)
			std::cout << " compactified. keeping "<<entities.Count()<<" relevant entity/ies"<<std::endl;
		if (!level || entities.Count()<2)
		{
			if (verbose)
				if (entities.Count()<2)
					std::cout << " insufficient entities given for further subdivision. terminating effort"<<std::endl;
	
			return;
		}
		M::Vec::div(split,entities.Count());
		if (verbose)
			std::cout << " center is "<<M::Vec::toString(split)<<std::endl;
	
	
		BYTE greatest(0),collapsed(0);
		float greatest_range(0);
	
		M::TVec3<> min,max;
		volume.GetMin(min);
		volume.GetMax(max);
		M::TVec3<> new_split;
		for (BYTE k = 0; k < 3; k++)
		{
			if (verbose)
				std::cout << "  processing axis "<<(int)k<<std::endl;
			float				delta,delta2,lower,upper,val,original_split;
	
			delta = (max.v[k]-min.v[k])*0.2;
		
			//determine lower and upper boundaries:
			lower = min.v[k]+delta;
			upper = max.v[k]-delta;
		
			if (lower > split.v[k])
				split.v[k] = lower;
			if (upper < split.v[k])
				split.v[k] = upper;
		
			val = split.v[k];
			original_split = split.v[k];
		
			bool moved;
		
			moved = true;
			while (moved)
			{
				moved = false;
				for (unsigned i = 0; i < entities.Count(); i++)
				{
					Entity*object = entities[i];
				
					float upper,lower,separation;
					const M::TFloatRange<>&axis = object->volume.axis[k];
					lower = axis.min;
					upper = axis.max;
					separation = (upper-lower)/20;
				
					if (lower < val && upper > val)
					{
						if (verbose)
						{
							std::cout << "  entity "<<i<<" intersects new split at "<<val<<std::endl;
							std::cout << "   entity range is "<<lower<<" <-> "<<upper<<std::endl;
						}
				
						if (val > original_split)
						{
							if (verbose)
								std::cout << "   moving new split to upper entity boundary"<<std::endl;
							val = upper+separation;
						}
						elif (val < original_split)
						{
							if (verbose)
								std::cout << "   moving new split to lower entity boundary"<<std::endl;
							val = lower-separation;
						}
						else
						{
							if (verbose)
								std::cout << "   rare case"<<std::endl;
							float center;
							center = object->volume.axis[k].center();
							if (center > val)
							{
								if (verbose)
									std::cout << "   moving new split to lower entity boundary"<<std::endl;
								val = lower-separation;
							}
							else
							{
								if (verbose)
									std::cout << "   moving new split to upper entity boundary"<<std::endl;
								val = upper+separation;
							}
						}
						//val.mult(1.01,sector_size);
						moved = true;
					}
				}
			}
			if (verbose)
				std::cout << " split vector component determined at "<<val<<std::endl;
	
			val = volume.axis[k].Clamp(val);

			if (verbose)
				std::cout << " split vector component clamped to "<<val<<std::endl;

			delta /= 2;
		
			lower = volume.axis[k].min+delta;
			upper = volume.axis[k].max-delta;
		
			if (val < lower)
			{
				collapsed ++;
				val = lower;
			}
			elif (val > upper)
			{
				collapsed ++;
				val = upper;
			}
		
		
			new_split.v[k] = val;
			if (verbose)
				std::cout << " final split vector component determined at "<<val<<std::endl;
		
			if (delta > greatest_range)
			{
				greatest_range = delta;
				greatest = k;
			}
		}
	
		if (collapsed == 3)
		{
			if (verbose)
				std::cout << " all collapsed"<<std::endl;
	
			if (entities.Count() > 3)
			{
				if (verbose)
					std::cout << "  but split required"<<std::endl;
				float original = split.v[greatest];
				split = new_split;
				split.v[greatest] = original;
			}
			else
			{
				return;
			}
		}
		else
			split = new_split;
	
	
		BYTE	num_children=0,
				unary_child = 0;
		for (BYTE k = 0; k < 8; k++)
		{
			BYTE p[3] = { (BYTE)(k / 4),
						 (BYTE)(k % 4 / 2),
						 (BYTE)(k % 4 % 2)};
			Volume d;
			bool collapsed(false);
			for (BYTE j = 0; j < 3; j++)
			{
				d.axis[j].min = p[j]?split.v[j]:volume.axis[j].min;
				d.axis[j].max = p[j]?volume.axis[j].max:split.v[j];
				collapsed |= d.axis[j].GetExtent() < (volume.axis[j].GetExtent())/20;
			}
		
			if (!collapsed)
			{
				child[k] = SignalNew(new EntityTree(d, level-1));
				num_children++;
				unary_child = k;
			}
			else
				child[k] = NULL;
		}
		hasChildren = true;
	
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
			{
				child[k]->_RecursiveRemap(entities);
				if (!child[k]->entities.Count())
				{
					Discard(child[k]);
					child[k] = NULL;
					num_children--;
					if (k == unary_child && num_children == 1)
						for (BYTE k2 = 0; k2 < 8; k2++)
							if (child[k2])
								unary_child = k2;
					
				}
			}
		if (num_children==1 && !child[unary_child]->hasChildren)
		{
			Discard(child[unary_child]);
			child[unary_child] = NULL;
			hasChildren = false;
		}
	}

	count_t EntityTree::_RecursionEnd(const M::TVec3<>&edge_point0, const M::TVec3<>&edge_point1, C::PreAllocVector<Entity*>&buffer)
	{
		count_t c = 0;
		for (index_t i = 0; i < entities.Count(); i++)
		{
			Entity*object = entities[i];
			if (!_oIntersectsBox(edge_point0,edge_point1,object->volume))
				continue;
			//std::cout << "  writing object to buffer"<<std::endl;
			buffer << object;
			c++;
		}
		return c;
	}

	count_t	EntityTree::_RecursiveLookup(const M::TVec3<>&p0, const M::TVec3<>&p1, C::PreAllocVector<Entity*>&buffer)
	{
		const count_t cnt = entities.Count();
		if (!cnt)
			return 0;
		if (!level)
			return _RecursionEnd(p0,p1,buffer);
		const M::TVec3<>	edge[2] = {p0,p1};
		//M::TVec3<>			half;
		//M::Vec::center(volume.min,volume.max,half);
		bool set[8] = {false,false,false,false,false,false,false,false};
		BYTE define[3];

		for (BYTE j = 0; j < 2; j++)
				if (volume.Contains(edge[j]))
				{
					for (BYTE k = 0; k < 3; k++)
						define[k] = edge[j].v[k] > split.v[k];
					BYTE k = define[0] *4+ define[1] *2 + define[2];
					set[k] = true;
				}
		M::TVec3<>	dir;
		M::Vec::subtract(p1, p0, dir);
		for (BYTE k = 0; k < 3; k++)
			if (dir.v[k])
			{
				BYTE	x = (k+1)%3,
						y = (k+2)%3;
				M::TVec3<> corner[2] = {volume.min(),volume.max()};
				for (BYTE j = 0; j < 2; j++)
				{
					define[k] = j;
					float alpha = (corner[j].v[k]-p0.v[k])/dir.v[k];
					if (alpha > 1 || alpha < 0)
						continue;
					float	vx = p0.v[x] + dir.v[x] *alpha,
							vy = p0.v[y] + dir.v[y] *alpha;
					if (!volume.axis[x].Contains(vx) || !volume.axis[y].Contains(vy))
						continue;
					define[x] = vx > split.v[x];
					define[y] = vy > split.v[y];
					set[define[0]*4 + define[1]*2 + define[2]] = true;
				}
				float	alpha = (split.v[k] - p0.v[k]) / dir.v[k];
				if (alpha > 1 || alpha < 0)
					continue;
				float	vx = p0.v[x] + dir.v[x] *alpha,
						vy = p0.v[y] + dir.v[y] *alpha;
				if (!volume.axis[x].Contains(vx) || !volume.axis[y].Contains(vy))
					continue;
				define[k] = 0;
				define[x] = vx > split.v[x];
				define[y] = vy > split.v[y];
				set[define[0] *4 + define[1]*2 + define[2]] = true;
				define[k] = 1;
				set[define[0] *4 + define[1]*2 + define[2]] = true;
			}
		index_t at = buffer.Count();
		for (BYTE k = 0; k < 8; k++)
			if (set[k] && child[k]!=NULL)
				if (child[k]->_RecursiveLookup(p0,p1,buffer) == cnt)
				{
					buffer.Truncate(at);
					return _RecursionEnd(p0,p1,buffer);
				}
		return 0;
	}



	count_t	EntityTree::_RecursiveLookup(const Volume&space, C::PreAllocVector<Entity*>&buffer)
	{
		const count_t cnt = entities.Count();
	
		if (!cnt || !intersect(space,volume))
		{
			return 0;
		}
		if (!level || 8*level > cnt || !hasChildren)
		{
			count_t c = 0;
			for (index_t i = 0; i < cnt; i++)
			{
				Entity*object = entities[i];
				if (!intersect(space,object->volume))
					continue;
				buffer << object;
				c++;
			}
			return c;
		}
	
		count_t at = buffer.Count();
		for (BYTE k = 0; k < 8; k++)
			if (child[k] && child[k]->_RecursiveLookup(space,buffer) == cnt)
			{
				if (buffer.Count()-at > cnt)
				{
					buffer.Truncate(at);
					for (index_t i = 0; i < cnt; i++)
						buffer << entities[i];
				}
				return cnt;
			}
		return 0;
	}


	EntityTree::EntityTree(const Volume&space, unsigned level_):volume(space),level(level_)
	{
		for (BYTE k = 0; k < 8; k++)
			child[k] = NULL;
		hasChildren = false;
	}

	EntityTree::EntityTree(const EntityTree&other)
	{
		for (unsigned i = 0; i < other.entities.Count(); i++)
			entities << other.entities[i];

		if (level && entities.Count())
		{
			for (BYTE k = 0; k < 8; k++)
				child[k] = SignalNew(new EntityTree(*other.child[k]));
			hasChildren = true;
		}
		else
		{
			for (BYTE k = 0; k < 8; k++)
				child[k] = NULL; //we will see if we actually need this
			hasChildren = false;
		}
	}

	EntityTree::EntityTree():level(1)
	{
		for (BYTE k = 0; k < 8; k++)
			child[k] = NULL;
		hasChildren = false;
	}


	EntityTree::~EntityTree()
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
				Discard(child[k]);
	}

	void		EntityTree::Clear()
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
			{
				Discard(child[k]);
				child[k] = NULL;
			}
		entities.reset();
		hasChildren = false;
	}

	void		EntityTree::Remap(const C::PreAllocVector<Entity*>&source, unsigned depth)
	{

		Clear();
		if (!source.Count())
			return;

		level = depth?depth:(unsigned)log((float)source.GetLength());	//may need to adjust this later
		//ShowMessage(IntToStr(source->Count())+" => "+IntToStr(level));

		{
			Entity*object = source.First();
			volume = object->volume;
		}
		for (index_t i = 1; i < source.Count(); i++)
		{
			Entity*entity = source[i];

			volume.Include(entity->volume);
		}
		_RecursiveRemap(source);
	}

	void	EntityTree::Lookup(const M::TVec3<>&edge_point0, const M::TVec3<>&edge_point1, C::PreAllocVector<Entity*>&out)
	{
		_RecursiveLookup(edge_point0,edge_point1, out);
	}


	void	EntityTree::Lookup(const Volume&space, C::PreAllocVector<Entity*>&out)
	{
		_RecursiveLookup(space,out);
	
		/*
		Buffer<Entity*>					buffer;
		unsigned offset = out.Count();
		for (unsigned i = 0; i < buffer.Count(); i++)
		{
			Entity*object = buffer[i];
			if (!object->added)
			{
				out << object;
				object->added = true;
			}
		}
		for (unsigned i = offset; i < out.Count(); i++)
			out[i]->added = false;*/
	}

	EntityTree&	EntityTree::operator=(const EntityTree&other)
	{
		for (BYTE k = 0; k < 8; k++)
			if (child[k])
				Discard(child[k]);
		entities.reset();
		for (index_t i = 0; i < other.entities.Count(); i++)
			entities << other.entities[i];

		if (level && entities.Count())
			for (BYTE k = 0; k < 8; k++)
				child[k] = SignalNew(new EntityTree(*other.child[k]));
		else
			for (BYTE k = 0; k < 8; k++)
				child[k] = NULL; //we will see if we actually need this
		return *this;
	}

	EntityTree*						EntityTree::GetChild(BYTE c)
	{
		DBG_ASSERT_LESS__(c,8);
		return child[c];
	}
	const EntityTree*						EntityTree::GetChild(BYTE c) const
	{
		DBG_ASSERT_LESS__(c,8);
		return child[c];
	}

	bool										EntityTree::HasChildren()	const
	{
		return hasChildren;
	}


	unsigned									EntityTree::GetLevel()	const
	{
		return level;
	}

	const M::TVec3<>&								EntityTree::GetSplitVector()	const
	{
		return split;
	}

	const EntityTree::Volume&					EntityTree::GetVolume()	const
	{
		return volume;
	}


	count_t					EntityTree::CountElements()	const
	{
		return entities.Count();
	}

	void										EntityTree::GetElements(C::PreAllocVector<Entity*>&out)	const
	{
		out.reset();
		for (index_t i = 0; i < entities.Count(); i++)
			out << entities[i];
	}

	const C::PreAllocVector<EntityTree::Entity*>& 	EntityTree::GetElementList() const
	{
		return entities;
	}
}

