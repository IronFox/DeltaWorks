#include "../global_root.h"
#include "cgs_track_segment.h"

namespace DeltaWorks
{

	namespace CGS
	{

		static void linearize(CGS::SubGeometryA<>&sub,PreAllocVector<CGS::SubGeometryA<>*>&objects, bool visible_only)
		{
			if (!visible_only || (sub.vs_hull_field.GetLength() && sub.vs_hull_field.First().vertex_field.GetLength()))
				objects << &sub;
			for (index_t i = 0; i < sub.child_field.GetLength(); i++)
				linearize(sub.child_field[i],objects,visible_only);
		}
		static void linearize(const CGS::SubGeometryA<>&sub,PreAllocVector<const CGS::SubGeometryA<>*>&objects, bool visible_only)
		{
			if (!visible_only || (sub.vs_hull_field.GetLength() && sub.vs_hull_field.First().vertex_field.GetLength()))
				objects << &sub;
			for (index_t i = 0; i < sub.child_field.GetLength(); i++)
				linearize(sub.child_field[i],objects,visible_only);
		}
	
		bool		Tile::loadFromFile(const PathString&filename)
		{
			if (!geometry.LoadFromFile(filename.c_str()))
				return false;
			this->filename = filename;
			return geometry.extractDimensions(dim);
		}

	
	
		void		TrackConnector::update(const M::TMatrix4<>&parent_system, bool final)
		{
			if (segment)
			{
				ASSERT_IS_NULL__(stub);
				//lout << "feeding parent configuration into segment end point "<<segment->GetIndexOf(this)<<nl;
				TTrackNode&end_point = ((TTrackNode&)segment->end_point[segment->GetIndexOf(this)]);
				end_point = *parent;
			
				if (flipped)
					M::Vec::mult(end_point.direction,-1);
			
				segment->changed = true;
				segment->OnChanged(final);
				return;
			}
			if (!stub)
				return;
	
			ASSERT_EQUAL__(connectors.Count(),stub->geometry.connector_field.GetLength());
			ASSERT_EQUAL__(nodes.Count(),stub->geometry.connector_field.GetLength());
		
			float offset = -stub->dim.z.max;
			//(is_outbound)?-stub->dim[2]:-stub->dim[5];	//stubs are rotated. always upper boundary :P
			//lout << "outbound is "<<(is_outbound?"true":"false")<<nl;
			//lout << "stub offset is "<<offset<<nl;
			M::Mat::copy(parent_system,instance->matrix);
		
			if (is_outbound)
			{
				M::Vec::mult(instance->matrix.x.xyz,-1);
				M::Vec::mult(instance->matrix.z.xyz,-1);
			}
			M::Vec::mad(instance->matrix.w.xyz,instance->matrix.z.xyz,offset);
		
			instance->update();
		
		
		
			for (index_t i = 0; i < stub->geometry.connector_field.GetLength(); i++)
			{
				const CGS::TConnector<>&cc=stub->geometry.connector_field[i];
				TTrackNode&tn = nodes[i];
			
				M::Vec::set(tn.scale,M::Vec::distance(cc.p0,cc.p1)/2.0f);
				M::Vec::center(cc.p0,cc.p1,tn.coordinates.remainder);
				M::TVec3<> d,dir;
				M::Vec::sub(cc.p0,cc.p1,d);
	
				M::Vec::copy(cc.direction,dir);

				tn.coordinates.remainder.z += offset;
				if (is_outbound)
				{
					tn.coordinates.remainder.x *= -1;
					tn.coordinates.remainder.z *= -1;
					d.x *= -1;
					d.z *= -1;
					dir.x *= -1;
					dir.z *= -1;
				}
			
				M::Mat::Transform(parent_system,tn.coordinates.remainder);
				M::Vec::copy(parent->coordinates.sector,tn.coordinates.sector);
				
				M::Vec::cross(d,dir,tn.up);
				M::Vec::normalize0(tn.up);
				M::Vec::copy(dir,tn.direction);
				M::Vec::normalize0(tn.direction);
				/*if (is_outbound)
				{
					tn.direction.x *= -1;
					tn.direction.z *= -1;
				}*/
			
				M::Mat::rotate(parent_system,tn.direction);
				M::Mat::rotate(parent_system,tn.up);
						
				M::Vec::clear(tn.scale_direction);
			
				if (connectors[i].segment)
				{
					//lout << "feeding node configuration into stub segment end point "<<connectors[i].segment->GetIndexOf(this)<<nl;
					TTrackNode&end_point = ((TTrackNode&)connectors[i].segment->end_point[connectors[i].segment->GetIndexOf(this)]);
					end_point = nodes[i];
				
					if (connectors[i].flipped)
						M::Vec::mult(end_point.direction,-1);
					//lout << "  final end point direction is "<<M::Vec::toString(end_point.direction)<<nl;
					connectors[i].segment->changed = true;
					connectors[i].segment->OnChanged(final);
				}
			}
		}
	
		bool		TrackConnector::IsValid(String*error_out)	const
		{
			if (stub)
			{
				if (segment)
				{
					if (error_out)
						(*error_out) = "Both stub and segment are not NULL";
					return false;
				}
				if (nodes.Count() != stub->geometry.connector_field.GetLength())
				{
					if (error_out)
						(*error_out) = "Node count ("+String(nodes.Count())+") differs from base connector field length ("+String(stub->geometry.connector_field.GetLength())+")";
					return false;
				}
				if (connectors.Count() != stub->geometry.connector_field.GetLength())
				{
					if (error_out)
						(*error_out) = "Connector count ("+String(connectors.Count())+") differs from base connector field length ("+String(stub->geometry.connector_field.GetLength())+")";
					return false;
				}
				if (!instance || instance->target != &stub->geometry)
				{
					if (error_out)
						(*error_out) = "Stub instance invalid or non existent";
					return false;
				}
				for (index_t i = 0; i < connectors.Count(); i++)
					if (connectors[i].segment)
					{
						if (connectors[i].segment->connector[0] != this
							&&
							connectors[i].segment->connector[1] != this)
							{
								if (error_out)
									(*error_out) = "Stub connector segment "+String(i)+" does not not know this";
								return false;
							}
					}
				return true;
			}
		
			if (segment)
			{
				if (segment->connector[0] != this && segment->connector[1] != this)
				{
					if (error_out)
						(*error_out) = "Linked segment does not know this";
					return false;
				}
				if (instance)
				{
					if (error_out)
						(*error_out) = "Connector is configured as segment connector but does have a stub geometry instance";
					return false;
				}
				if (nodes.GetLength() || connectors.GetLength())
				{
					if (error_out)
						(*error_out) = "Connector is configured as segment connector but does have nodes and/or connectors";
					return false;
				}
				return true;
			}
		
			if (nodes.GetLength() || connectors.GetLength())
			{
				if (error_out)
					(*error_out) = "Connector is configured for neither stub nor segment but does have nodes and/or connectors";
				return false;
			}
			return true;
		}
	
		void			TrackConnector::setStub(Tile*stub_, bool preserve_where_possible)
		{
			if (stub == stub_)
				return;
			Unset(preserve_where_possible);
			if (!stub_)
				return;
			stub = stub_;
			if (preserve_where_possible)
			{
				for (index_t i = stub->geometry.connector_field.GetLength(); i < connectors.Count(); i++)
					if (connectors[i].segment)
						connectors[i].segment->disconnectFrom(this);
				connectors.ResizePreserveContent(stub->geometry.connector_field.GetLength());
			
				if (segment)
				{
					if (connectors.Count())
					{
						connectors[0] = *this;
						segment = NULL;
					}
					else
					{
						segment->disconnectFrom(this);
						segment = NULL;
					}
				}
			}
			else
				connectors.SetSize(stub->geometry.connector_field.GetLength());
			nodes.SetSize(stub->geometry.connector_field.GetLength());
		
			instance = stub->geometry.createInstance();
		
			parent->scale.x = 1;
			parent->scale.y = 1;
		
			M::TMatrix4<> system;
			parent->makeSystem(system);
			update(system,true);
		
			ASSERT__(IsValid());
		}
	

		bool			TrackConnector::getIndexOfSegment(const TrackSegment*segment, TNodeSegmentSlot&slot_out)	const
		{
			slot_out.is_outbound = (this == &parent->outbound);
			if (!segment)
				return false;
			if (stub != NULL)
			{
				for (index_t i = 0; i < connectors.Count(); i++)
					if (connectors[i].segment == segment)
					{
						slot_out.flipped = connectors[i].flipped;
						slot_out.slot_index = i;
						return true;
					}
				return false;
			}
			if (this->segment == segment)
			{
				slot_out.flipped = flipped;
				slot_out.slot_index = 0;
				return true;
			}
			return false;
		}
	
		void			TrackConnector::requireIndexOfSegment(const TrackSegment*segment, TNodeSegmentSlot&slot_out)	const
		{
			slot_out.is_outbound = (this == &parent->outbound);
			ASSERT_NOT_NULL__(segment);

			if (stub != NULL)
			{
				for (index_t i = 0; i < connectors.Count(); i++)
					if (connectors[i].segment == segment)
					{
						slot_out.flipped = connectors[i].flipped;
						slot_out.slot_index = i;
						return;
					}
				FATAL__("Stub is set, but none of the "+String(connectors.Count())+" connectors matches the specifies segment");
				return;
			}
			if (this->segment == segment)
			{
				slot_out.flipped = flipped;
				slot_out.slot_index = 0;
				return;
			}
			FATAL__("No stub is set, and the specified segment ("+String(segment)+") does not match the locally linked one ("+String(this->segment)+")");
			return;
		}
	
	
		bool			TrackConnector::setStubSegment(index_t index, TrackSegment*segment_, bool flip)
		{
			if (!stub)
				return false;
			if (index >= connectors.Count())
				return false;
			if (connectors[index].segment)
				connectors[index].segment->disconnectFrom(this);
		
			connectors[index].segment = NULL;
			connectors[index].flipped = flip;
		
			if (!segment_->connector[0])
			{
				segment_->connector[0] = this;
				((TTrackNode&)segment_->end_point[0]) = nodes[index];
				if (flip)
					M::Vec::mult(segment_->end_point[0].direction,-1);

			}
			elif (!segment_->connector[1])
			{
				segment_->connector[1] = this;
				((TTrackNode&)segment_->end_point[1]) = nodes[index];
				if (flip)
					M::Vec::mult(segment_->end_point[1].direction,-1);
			}
			else
				return false;
			
			connectors[index].segment = segment_;
		
			String error;
			ASSERT1__(IsValid(&error),error);
			segment_->onConnectTo(this);
		
		
			return true;
		}
	
		bool			TrackConnector::setSegment(TrackSegment*segment_, bool flip)
		{
			if (segment == segment_)
				return true;
			Unset();
			if (!segment_)
				return false;
			flipped = flip;
		
			BYTE index = 0;
		
			if (!segment_->connector[0])
			{
				segment_->connector[0] = this;
				((TTrackNode&)segment_->end_point[0]) = *parent;
				if (flip)
					M::Vec::mult(segment_->end_point[0].direction,-1);
			}
			elif (!segment_->connector[1])
			{
				segment_->connector[1] = this;
				((TTrackNode&)segment_->end_point[1]) = *parent;
				if (flip)
					M::Vec::mult(segment_->end_point[1].direction,-1);

			}
			else
				return false;
			
			segment = segment_;
			String error;
			ASSERT1__(IsValid(&error),error);
		
			segment->onConnectTo(this);
			return true;
		}

		bool			TrackConnector::setSegment(index_t slot_index, TrackSegment*segment, bool flip)
		{
			if (stub)
				return setStubSegment(slot_index,segment,flip);
			return setSegment(segment,flip);
		}
	
		bool			TrackConnector::setSegment(TrackSegment*segment_, bool end, bool flip)
		{
			if (segment == segment_)
				return true;
			Unset();
			if (!segment_)
				return false;
			flipped = flip;
		
			BYTE index = 0;
		
			if (!segment_->connector[end])
				segment_->connector[end] = this;
			else
				return false;
			
			segment = segment_;
			String error;
			ASSERT1__(IsValid(&error),error);
		
			segment->onConnectTo(this);
			return true;
		}

			
		void			TrackConnector::disconnect(TrackSegment*seg, bool cascade_event)
		{
			if (stub)
			{
				for (index_t i = 0; i < connectors.Count(); i++)
					if (connectors[i].segment==seg)
					{
						if (cascade_event)
							connectors[i].segment->disconnectFrom(this);
						connectors[i].segment = NULL;
					}
			}
			elif (segment == seg)
			{
				if (cascade_event)
					segment->disconnectFrom(this);
				segment = NULL;
			}
		}
	
		bool			TrackNode::setSegment(bool is_outbound, index_t slot_index, TrackSegment*segment, bool flip)
		{
			if (is_outbound)
				return outbound.setSegment(slot_index,segment,flip);
			return inbound.setSegment(slot_index,segment,flip);
	
		}

		bool			TrackNode::setSegment(const TNodeSegmentSlot&slot, TrackSegment*segment)
		{
			if (slot.is_outbound)
				return outbound.setSegment(slot.slot_index,segment,slot.flipped);
			return inbound.setSegment(slot.slot_index,segment,slot.flipped);
	
		}

		/*virtual*/			TrackNode::~TrackNode()
		{
			inbound.Unset(false);
			outbound.Unset(false);
		}

		void			TrackNode::disconnect(TrackSegment*seg, bool cascade_event)
		{
			inbound.disconnect(seg,cascade_event);
			outbound.disconnect(seg,cascade_event);
		}
	
		bool			TrackNode::getClosestFreeSlot(const Composite::Coordinates&reference, float sector_size, TNodeSegmentSlot&slot)
		{
			TrackConnector*connector=NULL;
			index_t index=0;
			float dist=Composite::distanceSquare(reference,coordinates,sector_size);
			M::TVec3<>	axis;
			Composite::sub(coordinates,reference,axis,sector_size);
			if (outbound.stub)
			{
				for (index_t i = 0; i < outbound.connectors.Count(); i++)
					if (!outbound.connectors[i].segment)
					{
						float d = Composite::distanceSquare(reference,outbound.nodes[i].coordinates,sector_size);
						if (d < dist)
						{
							dist = d;
							index = i;
							connector = &outbound;
						}
					}
			}
			if (inbound.stub)
			{
				for (index_t i = 0; i < inbound.connectors.Count(); i++)
					if (!inbound.connectors[i].segment)
					{
						float d = Composite::distanceSquare(reference,inbound.nodes[i].coordinates,sector_size);
						if (d < dist)
						{
							dist = d;
							index = i;
							connector = &inbound;
						}
					}
			}

			if (connector)
			{
				slot.is_outbound = connector == &outbound;
				slot.slot_index = index;
				slot.flipped = M::Vec::dot(connector->nodes[index].direction,axis)>=0;
				return true;
			}
			else
			{
				bool flip = (M::Vec::dot(direction,axis)<0);
				/*if (end)
					flip = !flip;*/
				if (!outbound.stub && !outbound.segment)
				{
					slot.is_outbound = true;
					slot.slot_index = 0;
					slot.flipped = !flip;

					if (slot.flipped && !inbound.stub && !inbound.segment)
					{
						slot.is_outbound = false;
						//slot.flipped = false;	//can't unset flipped. only use more appropriate connector
					}
					return true;
				}
				if (!inbound.stub && !inbound.segment)
				{
					slot.is_outbound = false;
					slot.slot_index = 0;
					slot.flipped = !flip;
					return true;
				}
			}
			return false;
		}
	
		bool			TrackNode::setClosestSegment(const Composite::Coordinates&reference, TrackSegment*seg, bool end, float sector_size)
		{
			TrackConnector*connector=NULL;
			index_t index=0;
			float dist=Composite::distanceSquare(reference,coordinates,sector_size);
			M::TVec3<>	axis;
			Composite::sub(coordinates,reference,axis,sector_size);
			if (outbound.stub)
			{
				for (index_t i = 0; i < outbound.connectors.Count(); i++)
					if (!outbound.connectors[i].segment)
					{
						float d = Composite::distanceSquare(reference,outbound.nodes[i].coordinates,sector_size);
						if (d < dist)
						{
							dist = d;
							index = i;
							connector = &outbound;
						}
					}
			}
			if (inbound.stub)
			{
				for (index_t i = 0; i < inbound.connectors.Count(); i++)
					if (!inbound.connectors[i].segment)
					{
						float d = Composite::distanceSquare(reference,inbound.nodes[i].coordinates,sector_size);
						if (d < dist)
						{
							dist = d;
							index = i;
							connector = &inbound;
						}
					}
			}

			if (connector)
			{
				connector->setStubSegment(index,seg,M::Vec::dot(connector->nodes[index].direction,axis)<0);
				return true;
			}
			else
			{
				bool flip = (M::Vec::dot(direction,axis)<0);
				/*if (end)
					flip = !flip;*/
				if (!outbound.stub && !outbound.segment)
				{
					outbound.setSegment(seg,end,flip);
					return true;
				}
				if (!inbound.stub && !inbound.segment)
				{
					inbound.setSegment(seg,end,!flip);
					return true;
				}
			}
			return false;
		}
	
		void			TrackConnector::Unset(bool preserve_connections)
		{
			if (stub)
			{
				instance.reset();
				if (!preserve_connections)
				{
					for (index_t i = 0; i < connectors.Count(); i++)
						if (connectors[i].segment)
							connectors[i].segment->disconnectFrom(this);
					connectors.free();
				}
				nodes.free();
				stub = NULL;
			}
		
			if (segment && !preserve_connections)
			{
				segment->disconnectFrom(this);
				segment = NULL;
			}
		}
	
		void			TrackConnector::unsetStub(bool preserve_where_possible)
		{
			if (!stub)
				return;
			Unset(preserve_where_possible);
			if (!preserve_where_possible)
				return;
			bool set = !segment;
			for (index_t i = 0; i < connectors.Count(); i++)
				if (connectors[i].segment)
					if (!set)
					{
						(BaseTrackConnector&)*this = connectors[i];
						set = true;
					}
					else
						connectors[i].segment->disconnectFrom(this);
		}
	
	
	
	
		void	TrackNode::makeSystem(M::TMatrix4<>&system)	const
		{
			system.z.xyz = direction;
			system.y.xyz = up;
			M::Vec::cross(up,direction,system.x.xyz);
			M::Vec::normalize0(system.x.xyz);
			system.w.xyz = coordinates.remainder;
			M::Mat::resetBottomRow(system);
		}
	
		void			TrackNode::update(bool final)
		{
			M::TMatrix4<> system;
			makeSystem(system);
			inbound.update(system,final);
			outbound.update(system,final);
		}

		void			TrackNode::update(const M::TMatrix4<>&precompiled_system, bool final)
		{
			inbound.update(precompiled_system,final);
			outbound.update(precompiled_system,final);
		}

	
		
		/*virtual*/				TrackSegment::~TrackSegment()
		{
			disconnect();
		}

		/*virtual*/	void							TrackSegment::onDisconnectFrom(TrackConnector*connector_)
		{}

	
	
		void	TrackSegment::updateControls(float sector_size, float control_factor0, float control_factor1)
		{
			M::TVec3<>	d;
			M::TVec2<>	sd;
			float	len,slen;
		
			M::TVec3<>	delta;
			M::Vec::sub(end_point[1].coordinates.sector,end_point[0].coordinates.sector,delta);
			M::Vec::mult(delta,sector_size);
		
			end_point[1].coordinates.sector = end_point[0].coordinates.sector;
			M::Vec::add(end_point[1].coordinates.remainder,delta);
		
		
		
			Composite::sub(end_point[1].coordinates,end_point[0].coordinates,d,sector_size);
			M::Vec::sub(end_point[1].scale,end_point[0].scale,sd);
			len = M::Vec::length(d);
			slen = M::Vec::length(sd);
		
			//lout << "updating segment controls with end point directions:"<<nl;
			//lout << " "<<M::Vec::toString(end_point[0].direction)<<nl;
			//lout << " "<<M::Vec::toString(end_point[1].direction)<<nl;
		
			M::Vec::mad(end_point[0].coordinates.remainder,end_point[0].direction,control_factor0*len,end_point[0].position_control);
			end_point[0].scale_control.x = end_point[0].scale.x + end_point[0].scale_direction.x*control_factor0*(end_point[1].scale.x-end_point[0].scale.x);
			end_point[0].scale_control.y = end_point[0].scale.y + end_point[0].scale_direction.y*control_factor0*(end_point[1].scale.y-end_point[0].scale.y);
		
			M::Vec::mad(end_point[1].coordinates.remainder,end_point[1].direction,control_factor1*len,end_point[1].position_control);
			end_point[1].scale_control.x = end_point[1].scale.x + end_point[1].scale_direction.x*control_factor1*(end_point[1].scale.x-end_point[0].scale.x);
			end_point[1].scale_control.y = end_point[1].scale.y + end_point[1].scale_direction.y*control_factor1*(end_point[1].scale.y-end_point[0].scale.y);
		}


		void	TrackSegment::interpolate(float fc, TFrame&rs)	const
		{
			//cout << _toString(end_point[0].scale,2)<<", "<<_toString(end_point[0].scale_control,2)<<", "<<_toString(end_point[1].scale_control,2)<<", "<<_toString(end_point[1].scale,2)<<" ("<<fc<<") => "<<rs.position
			M::Vec::resolveBezierCurvePoint(end_point[0].scale, end_point[0].scale_control, end_point[1].scale_control, end_point[1].scale, fc, rs.scale);
		
			#if 1
				M::Vec::resolveBezierCurvePoint(end_point[0].coordinates.remainder, end_point[0].position_control, end_point[1].position_control, end_point[1].coordinates.remainder, fc, rs.position);
				M::Vec::resolveBezierCurveAxis(end_point[0].coordinates.remainder, end_point[0].position_control, end_point[1].position_control, end_point[1].coordinates.remainder, fc, rs.system.z);
			#else
				M::Vec::resolveUCBS(end_point[0].coordinates.remainder, end_point[0].position_control, end_point[1].position_control, end_point[1].coordinates.remainder, fc, rs.position);
				M::Vec::resolveUCBSaxis(end_point[0].coordinates.remainder, end_point[0].position_control, end_point[1].position_control, end_point[1].coordinates.remainder, fc, rs.system.z);
			#endif
		
		
			M::Vec::normalize0(rs.system.z);
			M::Vec::mult(rs.system.z,-1);
			M::Vec::interpolate(end_point[0].up,end_point[1].up,fc,rs.system.y);
			M::Vec::cross(rs.system.y,rs.system.z,rs.system.x);
			M::Vec::normalize0(rs.system.x);
			M::Vec::cross(rs.system.z,rs.system.x,rs.system.y);
			M::Vec::normalize0(rs.system.y);
		}


		static void allocateBendableComponents(const CGS::SubGeometryA<>&input_object, CGS::SubGeometryA<>&object)
		{
			object.wheel_field.free();	//wheels can't be bent
			object.accelerator_field.free();	//accelerators can't be bent
			object.tracks_field.free();	//tracks can't be bent. ok, nothing can be bent that is not geometry
			object.mounting_field.free();
			object.meta = input_object.meta;
			object.name = input_object.name;	//hmmmm, no, no name needed. yes, yes, it is... gah
			//object.system_link = &object.path;	//done later
			object.vs_hull_field.SetSize(input_object.vs_hull_field.GetLength());

			object.child_field.SetSize(input_object.child_field.GetLength());

			for (index_t i = 0; i < object.child_field.GetLength(); i++)
				allocateBendableComponents(input_object.child_field[i],object.child_field[i]);
		}
			
			
		void	TrackSegment::bend(const Tile&source, CGS::Geometry<>&geometry)	const
		{
			//geometry.copy(source.geometry);	//!< Plain copy. Lots of redundancy but at least it's enclosed

			//geometry.clear();
			if (source.geometry.usesLocalTextureResource())
			{
				geometry.local_textures = source.geometry.local_textures;
				geometry.texture_resource = &geometry.local_textures;
			}
			else
				geometry.texture_resource = source.geometry.texture_resource;

			geometry.object_field.SetSize(source.geometry.object_field.GetLength());
			for (index_t i = 0; i < geometry.object_field.GetLength(); i++)
				allocateBendableComponents(source.geometry.object_field[i],geometry.object_field[i]);

			geometry.animator_field.free();	//can't be bent
			geometry.connector_field.free();

			geometry.material_field.SetSize(source.geometry.material_field.GetLength());
			for (index_t i = 0; i < geometry.material_field.GetLength(); i++)
			{
				const CGS::MaterialA<>&source_material = source.geometry.material_field[i];
				CGS::MaterialA<>&target_material = geometry.material_field[i];

				target_material.info = source_material.info;
				target_material.data.coord_layers = source_material.data.coord_layers;
				target_material.data.object_field.SetSize(source_material.data.object_field.GetLength());
			}

			geometry.resetLinkage();

		
			ASSERT2__(M::Vec::equal(end_point[0].coordinates.sector,end_point[1].coordinates.sector),M::Vec::toString(end_point[0].coordinates.sector),M::Vec::toString(end_point[1].coordinates.sector));
		
			/*TFrame		frame[Resolution];
			for (index_t i = 0; i < Resolution; i++)*/
		
			M::TVec3<>	range = source.dim.GetExtent();
		
			//::lout << "bending segment with"<<nl;
			//lout << " dim="<<M::Vec::toString(source.dim.lower)<<'-'<<M::Vec::toString(source.dim.upper)<<nl;
			#undef REPORT
			#define REPORT(CON)	{}	//lout << " "#CON":"<<nl<<"  .coordinates="<<CON.coordinates.ToString()<<nl<<"  .position_control="<<M::Vec::toString(CON.position_control)<<nl<<"  .scale="<<M::Vec::toString(CON.scale)<<nl<<"  .scale_control="<<M::Vec::toString(CON.scale_control)<<nl;
			REPORT(end_point[0])
			REPORT(end_point[1])
		
			float stretch = (end_point[0].scale.x+end_point[1].scale.x)/2.0;	//average stretch

			float spline_length = M::Vec::distance(end_point[0].coordinates.remainder,end_point[1].coordinates.remainder);

			M::TVec3<>	center;

			M::Vec::center(end_point[0].coordinates.remainder,end_point[1].coordinates.remainder,center);
		
			//lout << " stretch="<<stretch<<nl;
		
		
			UINT repeat = repetitive?(UINT)Round(spline_length/range.z/stretch):1;
			if (!repeat)
				repeat++;
			//lout << " repeat="<<repeat<<nl;
			//repeat = 1;

			float	segment_length = 1.0f/(float)repeat,
					zrange = range.z/segment_length;

		
		
			for (index_t i = 0; i < geometry.material_field.GetLength(); i++)
				for (index_t j = 0; j < geometry.material_field[i].data.object_field.GetLength(); j++)
				{
					CGS::RenderObjectA<>&target_robj = geometry.material_field[i].data.object_field[j];
					const CGS::RenderObjectA<>&source_robj = source.geometry.material_field[i].data.object_field[j];

					target_robj.detail = source_robj.detail;
					target_robj.tname = source_robj.target->name;
					target_robj.particle_field.free();
					target_robj.vpool.vcnt = source_robj.vpool.vcnt;
					target_robj.vpool.vlyr = source_robj.vpool.vlyr;
					target_robj.vpool.vflags = source_robj.vpool.vflags;

					count_t		frame_length = source_robj.vpool.vdata.GetLength(),
								vframe_length = source_robj.vpool.vcnt,
								vsize = source_robj.vpool.vsize();
					target_robj.vpool.vdata.SetSize(frame_length*repeat);	//simple clone here, i can't be arsed to merge them
					target_robj.vpool.vdata.copyFrom(source_robj.vpool.vdata.pointer(),source_robj.vpool.vdata.size());
					//float*vfield = target_robj.vpool.vdata;
					Concurrency::parallel_for(UINT32(0),source_robj.vpool.vcnt,[&target_robj,&source_robj,center,vsize,repeat,frame_length,this,zrange,&source,segment_length,stretch](UINT32 k)
					{
					//for (UINT32 k = 0; k < robj.vpool.vcnt; k++)
					//{
						M::TVec3<> p,n,t;
						float	*vfield = target_robj.vpool.vdata + vsize*k;
						float	*v=vfield;
						M::Mat::Transform(source_robj.target->path,M::Vec::ref3(v),p);	v+=3;
						if (source_robj.vpool.vflags&CGS::HasNormalFlag)
						{
							M::Mat::rotate(source_robj.target->path,M::Vec::ref3(v),n); v+=3;
						}
						if (source_robj.vpool.vflags&CGS::HasTangentFlag)
						{
							M::Mat::rotate(source_robj.target->path,M::Vec::ref3(v),t); v+=3;
						}
					
						for (UINT l = repeat-1; l < repeat; l--)
						{
							float*vout = vfield+l*frame_length;
							float z = p.z;
							float rel = 1.0f-((z-source.dim.z.min)/zrange+segment_length*l);
							TFrame vec;
							interpolate(rel,vec);
							{
								M::TVec3<>&ref = M::Vec::ref3(vout);
								M::Vec::mad(vec.position,vec.system.x,p.x*vec.scale.x, ref);
								M::Vec::mad(ref,vec.system.y,p.y*vec.scale.y);
								M::Vec::sub(ref,center);
							}
							vout+=3;
						
							if (source_robj.vpool.vflags&CGS::HasNormalFlag)
							{
								M::TVec3<> n2,n3;
								M::Vec::def(n2,n.x*vec.scale.y/vec.scale.x,n.y*vec.scale.x/vec.scale.y,n.z);
								M::Mat::Mult(vec.system,n2,M::Vec::ref3(vout));
								M::Vec::normalize0(M::Vec::ref3(vout));
								//M::Vec::copy(vec.system.x,M::Vec::ref3(vout));
								vout += 3;
							}
							if (source_robj.vpool.vflags&CGS::HasTangentFlag)
							{
								M::TVec3<> t2,t3;
								M::Vec::def(t2,	t.x*vec.scale.y/vec.scale.x,t.y*vec.scale.x/vec.scale.y,t.z);
								M::Mat::Mult(vec.system,t2,M::Vec::ref3(vout));
								M::Vec::normalize0(M::Vec::ref3(vout));
								//M::Vec::copy(vec.system.x,M::Vec::ref3(vout));
								vout += 3;
							}
							/*if (scale_texture)
							{
								float scale = (vec.scale[0]+vec.scale[1])/2.0f;
								for (index_t l = 0; l < robj.vpool.vlyr; l++)
								{
									_mult2(v,scale,vout);
									vout+=2;
									v+=2;
								}
							}
							else*/
								M::Vec::copyD(v,vout,2*source_robj.vpool.vlyr);
						}
						/*for (index_t l = 1; l < repeat; l++)
							M::Vec::copyD(vfield+3+3,vfield+l*frame_length+3+3,source_robj.vpool.vlyr*2);*/
						//vfield += vsize;
					});
					target_robj.vpool.vcnt*=repeat;
				
					if (repeat > 1)
					{
						count_t iframe_length = source_robj.ipool.idata.GetLength();
						target_robj.ipool.idata.SetSize(iframe_length*repeat);
						//Ctr::Array<CGS::StdDef::IndexType>	new_field(iframe_length*repeat);
						CGS::StdDef::IndexType*at = target_robj.ipool.idata.pointer();
						//for (index_t k = 0; k < robj.ipool.sdata.GetLength(); k++)
						{
							for (index_t m = 0; m < repeat; m++)
								for (index_t l = 0; l < source_robj.ipool.idata.GetLength(); l++)
									*at++ = CGS::StdDef::IndexType(source_robj.ipool.idata[l]+m*vframe_length);
							target_robj.ipool.triangles = source_robj.ipool.triangles * repeat;
							target_robj.ipool.quads =  source_robj.ipool.quads * repeat;
						}
						ASSERT_CONCLUSION(target_robj.ipool.idata,at);
					}
					else
					{
						target_robj.ipool = source_robj.ipool;
					}
				}
			
		
			//now for ze object geometry
		
		
			PreAllocVector<const CGS::SubGeometryA<>*>source_objects;
			PreAllocVector<CGS::SubGeometryA<>*>objects;
			for (index_t i = 0; i < geometry.object_field.GetLength(); i++)
			{
				linearize(geometry.object_field[i],objects,false);
				linearize(source.geometry.object_field[i],source_objects,false);
			}

			ASSERT_EQUAL__(source_objects.Count(),objects.Count());	//TRANSITORY
		
		
			for (index_t i = 0; i < objects.Count(); i++)
			{
				ASSERT_EQUAL__(objects[i]->vs_hull_field.GetLength(),source_objects[i]->vs_hull_field.GetLength());	//TRANSITORY

				objects[i]->meta.shortest_edge_length = source_objects[i]->meta.shortest_edge_length * spline_length / repeat;
				const M::TMatrix4<CGS::StdDef::SystemType>&path = source_objects[i]->path;

				for (index_t j = 0; j < objects[i]->vs_hull_field.GetLength(); j++)
				{
					typedef CGS::SubGeometryA<>::VsDef Def;
				
					const Mesh<Def>	&source_hull = source_objects[i]->vs_hull_field[j];
					Mesh<Def>	&target_hull = objects[i]->vs_hull_field[j];

					bendHull(source_hull,target_hull,repeat,source.dim.z.min, zrange, segment_length, stretch, path, center);
				
					//Mesh<Def>	nobj;
				}

				typedef CGS::SubGeometryA<>::PhDef Def;
				
				const Mesh<Def>	&source_hull = source_objects[i]->phHull;
				Mesh<Def>	&target_hull = objects[i]->phHull;

				bendHull(source_hull,target_hull,repeat,source.dim.z.min, zrange, segment_length, stretch, path, center);
			}
			/*for (index_t i = 0; i < objects.Count(); i++)
				M::Mat::Eye(objects[i]->meta.system);*/
			geometry.resetSystems();
			//geometry.root_system.loadIdentity(false);
			geometry.root_system.setPosition(center);
			geometry.remap(CGS::Everything);
		}

		void	TrackSegment::disconnect()
		{
			if (connector[0])
			{
				connector[0]->disconnect(this,false);
				connector[0] = NULL;
			}
			if (connector[1])
			{
				connector[1]->disconnect(this,false);
				connector[1] = NULL;
			}
		}
	
		void	TrackSegment::disconnectFrom(TrackConnector*connector_)
		{
			if (connector[0] == connector_)
				connector[0] = NULL;
			elif (connector[1] == connector_)
				connector[1] = NULL;
			else
				return;
			onDisconnectFrom(connector_);
		}
	
		void	TrackSegment::smartConnect(TrackNode&node0, TrackNode&node1, float sector_size)
		{
			if (&node0 == &node1)
				return;
			if (connector[0])
				connector[0]->disconnect(this,false);
			if (connector[1])
				connector[1]->disconnect(this,false);
			connector[0] = NULL;
			connector[1] = NULL;
		
		
			node0.setClosestSegment(node1.coordinates,this,false,sector_size);
			node1.setClosestSegment(node0.coordinates,this,true,sector_size);
		
			//node0.update();
			//node1.update();
		}

		void	TrackSegment::connect(TrackConnector&from_, TrackConnector&to_, float sector_size)
		{
			if (&from_ == &to_)
				return;
			if (connector[0])
				connector[0]->disconnect(this,false);
			if (connector[1])
				connector[1]->disconnect(this,false);
			connector[0] = NULL;
			connector[1] = NULL;

			M::TVec3<> axis;
			Composite::sub(to_.parent->coordinates,from_.parent->coordinates,axis,sector_size);
			
			from_.setSegment(this,M::Vec::dot(from_.parent->direction,axis)<0);
			to_.setSegment(this,M::Vec::dot(to_.parent->direction,axis)<0);
		}
	
		void	TrackSegment::connectStub(TrackConnector&from_, index_t stub_connector, TrackConnector&to_, float sector_size)
		{
			if (&from_ == &to_)
				return;
			if (connector[0])
				connector[0]->disconnect(this,false);
			if (connector[1])
				connector[1]->disconnect(this,false);
			connector[0] = NULL;
			connector[1] = NULL;

			M::TVec3<> axis;
			Composite::sub(to_.parent->coordinates,from_.parent->coordinates,axis, sector_size);
		
			if (!from_.setStubSegment(stub_connector,this,M::Vec::dot(from_.nodes[stub_connector].direction,axis)<0))
				return;
		
			to_.setSegment(this,M::Vec::dot(to_.parent->direction,axis)<0);
		
		}
	
	
		void	TrackSegment::connectStub(TrackConnector&from_, TrackConnector&to_, index_t stub_connector, float sector_size)
		{
			if (&from_ == &to_)
				return;
			if (connector[0])
				connector[0]->disconnect(this,false);
			if (connector[1])
				connector[1]->disconnect(this,false);
			connector[0] = NULL;
			connector[1] = NULL;

			M::TVec3<> axis;
			Composite::sub(to_.parent->coordinates,from_.parent->coordinates,axis,sector_size);
		
			from_.setSegment(this,M::Vec::dot(from_.parent->direction,axis)<0);
			if (!to_.setStubSegment(stub_connector,this,M::Vec::dot(to_.nodes[stub_connector].direction,axis)<0))
				return;
		}
	
		void	TrackSegment::connectStubs(TrackConnector&from_, index_t from_stub_connector, TrackConnector&to_, index_t to_stub_connector, float sector_size)
		{
			if (&from_ == &to_)
				return;
			if (connector[0])
				connector[0]->disconnect(this,false);
			if (connector[1])
				connector[1]->disconnect(this,false);
			connector[0] = NULL;
			connector[1] = NULL;

			M::TVec3<> axis;
			Composite::sub(to_.parent->coordinates,from_.parent->coordinates,axis,sector_size);
		
			if (!from_.setStubSegment(from_stub_connector,this,M::Vec::dot(from_.nodes[from_stub_connector].direction,axis)<0))
				return;
		
			if (!to_.setStubSegment(to_stub_connector,this,M::Vec::dot(to_.nodes[to_stub_connector].direction,axis)<0))
				return;
		}
	

	}
}

