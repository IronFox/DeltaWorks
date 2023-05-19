#include "profiler.h"

namespace Engine
{
	namespace Profiler
	{
	
		static unsigned channel_resolution=64;
		static float	record_interval=0.5,
						current_time=0;
		static Time::Stamp	last_change;
		static Channel*active_channel=NULL;
		static bool		record = false;
		static OrthographicAspect<>		orthographic_aspect;
		static Camera<>					projected_aspect;

		Ctr::Vector<Graph*>			orthographic_graphs, 
										perspective_graphs;
	
		Data		time_data,
					fps_data;
		
		/*virtual*/ Graph::~Graph()
		{
			if (!application_shutting_down)
				uninstall();
		}

		/*virtual*/ void			OrthographicGraph::install()
		{
			if (!orthographic_graphs.Contains(this))
				orthographic_graphs.append(this);
		}
		
		/*virtual*/ void			OrthographicGraph::uninstall()
		{
			orthographic_graphs.FindAndErase(this);
		}
		
		M::TVec3<>			C3dGraph::light = {1,1,1},
						C3dGraph::background = {0.5,0.5,0.5};
		
		
		/*virtual*/ void			C3dGraph::install()
		{
			if (!perspective_graphs.Contains(this))
				perspective_graphs.Append(this);
		}
		/*virtual*/ void			C3dGraph::uninstall()
		{
			perspective_graphs.FindAndErase(this);
		}


		
		const Time::Stamp&	lastChange()
		{
			return last_change;
		}
		
		static void pickColor(M::TVec3<>&color)
		{
			static const M::TVec3<>	colors[] = {
				{0,1,0},
				{1,1,0},
				{0,0,1},
				{0,1,1},
				{1,0,0},
				{0,1,0.5},
				{0.5,1,0},
				{0,0.5,1},
				{1,0.5,0},
				{1,1,1},
				{0,0,0}
			};
			static unsigned at=0;
			color = colors[at];
			at = (at+1)%ARRAYSIZE(colors);
		}
		
		
		
		
		void		Channel::setResolution(unsigned res)
		{
			if (res == history.GetLength())
				return;
			count_t old = history.GetLength();
			history.ResizePreserveContent(res);
			history.Fill(0,old);
			//weighted_history.resizeCopy(res);
			//weighted_history.Fill(0,old);
			minimum_history.ResizePreserveContent(res);
			minimum_history.Fill(0,old);
			maximum_history.ResizePreserveContent(res);
			maximum_history.Fill(0,old);
		}
		
		void		Channel::advance()
		{
			if (!history.GetLength())
				return;
			history.shiftDown();
			minimum_history.shiftDown();
			maximum_history.shiftDown();
			if (!counter)
				counter++;
			if (first_advance)
			{
				faccumulated = 0;
				accumulated = Time::Delta();
				//frame_accumulated = 0;
				//weighted_accumulated = 0;
				maximum = minimum = Time::Delta();
			}
			history.Last() = (faccumulated + accumulated.ToSeconds())/counter;
			if (faccumulated > 0)
			{
				if (accumulated.IsNotZero())
				{
					minimum_history.Last() = M::Min(fminimum,minimum.ToSeconds());
					maximum_history.Last() = M::vmax(fmaximum,maximum.ToSeconds());
				}
				else
				{
					minimum_history.Last() = fminimum;
					maximum_history.Last() = fmaximum;
				}
			}
			else
			{
				minimum_history.Last() = minimum.ToSeconds();
				maximum_history.Last() = maximum.ToSeconds();
			}
			
			
			/*
			weighted_history.shiftLeft();
			if (!weighted_counter)
				weighted_counter = 1;
			weighted_history.Last() = (timer.toSecondsf(weighted_accumulated))/weighted_counter;
			*/
			
		
			faccumulated = 0;
			accumulated = Time::Delta::Zero;
			fframe_accumulated = 0;
			frame_accumulated = Time::Delta::Zero;
			//weighted_accumulated = 0;
			fmaximum = 0;
			maximum = Time::Delta::Zero;
			fminimum = 0;
			minimum = Time::Delta::Zero;
			counter = 0;
			//weighted_counter = 0;
			first_advance = false;
		}
		
		
		void		Channel::nextFrame(float weight)
		{
			if (!counter)
			{
				fmaximum = fminimum = fframe_accumulated;
				maximum = minimum = frame_accumulated;
			}
			else
			{
				fmaximum = M::vmax(fmaximum,fframe_accumulated);
				fminimum = M::Min(fminimum,fframe_accumulated);
				maximum = M::vmax(maximum,frame_accumulated);
				minimum = M::Min(minimum,frame_accumulated);
			}
			
			frame_accumulated = Time::Delta::Zero;
			fframe_accumulated = 0;
			counter++;
			/*
			weighted_accumulated += (Timer::Time)(frame_accumulated*weight);
			weighted_counter += weight;
			*/
		}

		
		Channel::Channel():history(Config::channelResolution()),minimum_history(Config::channelResolution()),maximum_history(Config::channelResolution()),faccumulated(0),fminimum(0),fmaximum(0),counter(0),name("[unnamed]")
		{
			history.Fill(0);
			//weighted_history.Fill(0);
			maximum_history.Fill(0);
			minimum_history.Fill(0);
			first_advance = true;
		}
		
		void		Channel::generateColor(index_t channel_index)
		{
			/*color[0] = (float)(channel_index%3)/2.0f;
			color[1] = (float)((channel_index/3)%3)/2.0f;
			color[2] = (float)((channel_index/9)%3)/2.0f;
			_normalize0(color);*/
			if (channel_index%2)
				M::Vec::set(color,0.2);
			else
				M::Vec::set(color,1);
		}
		
		bool		Channel::isActive()	const
		{
			return active_channel == this;
		}
		
		void		Channel::setColor(float r, float g, float b)
		{
			M::Vec::def(color,r,g,b);
		}
		
		void		Channel::adoptData(Channel&other)
		{
			history.adoptData(other.history);
			maximum_history.adoptData(other.maximum_history);
			minimum_history.adoptData(other.minimum_history);
			//weighted_history.adoptData(other.weighted_history);
			started = other.started;
			accumulated = other.accumulated;
			faccumulated = other.faccumulated;
			frame_accumulated = other.frame_accumulated;
			fframe_accumulated = other.fframe_accumulated;
			fminimum = other.fminimum;
			fmaximum = other.fmaximum;
			minimum = other.minimum;
			maximum = other.maximum;
			counter = other.counter;
			//weighted_counter = other.weighted_counter;
			color = other.color;
			name.adoptData(other.name);
		}
		
		void		Channel::start()
		{
			if (active_channel)
				active_channel->stop();
			active_channel = this;
			started.Update();
		}
		
		Time::Delta		Channel::stop()
		{
			if (record)
			{
				Time::Delta delta = started.Elapsed();
				accumulated += delta;
				frame_accumulated += delta;
				total += delta;
				active_channel = NULL;
				return delta;
			}
			active_channel = NULL;
			return Time::Delta::Zero;
		}
		
		
		
		
		void		Channel::accumulatei(const Time::Delta&t)
		{
			accumulated += t;
			frame_accumulated += t;
			total += t;
		}

		
		void		Channel::accumulate(float value)
		{
			faccumulated += value;
			fframe_accumulated += value;
		}
		
		
		void		Group::adoptData(Group&other)
		{
			Array<Channel,Adopt>::adoptData(other);
			color = other.color;
			name.adoptData(other.name);
		}
		
		Group::Group():res(Config::channelResolution()),name("[unnamed]")
		{
			pickColor(color);
		}
		
		void Group::setResolution(unsigned res_)
		{
			if (res == res_)
				return;
			for (unsigned i = 0; i < Count(); i++)
				operator[](i).setResolution(res_);
			res = res_;
		}

		
		
		namespace Config
		{
			unsigned	channelResolution()
						{
							return channel_resolution;
						}
			void		setChannelResolution(unsigned resolution)
						{
							channel_resolution = resolution;
							fps_data.setResolution(resolution);
							time_data.setResolution(resolution);
						}
			void		setRecordInterval(float interval)
						{
							record_interval = interval;
						}
						
			float		recordInterval()
						{
							return record_interval;
						}
		}
		
		Data::Data():res(Config::channelResolution())
		{}
		
		count_t		Data::countChannels()	const
		{
			count_t rs = 0;
			for (index_t i = 0; i < Count(); i++)
				rs += operator[](i).channels();
			return rs;
		
		}
		
		Channel&		Data::openChannel(index_t major_channel, index_t minor_channel)
		{
			if (major_channel >= Count())
			{
				count_t pre = Count();
				ResizePreserveContent(major_channel+1);
				for (index_t i = pre; i <= major_channel; i++)
					Array<Group,Adopt>::operator[](i).setResolution(res);
			}
			Group&group = Array<Group,Adopt>::operator[](major_channel);
			if (minor_channel >= group.channels())
			{
				count_t pre = group.channels();
				group.ResizePreserveContent(minor_channel+1);
				for (index_t i = pre; i <= minor_channel; i++)
				{
					group.channel(i).setResolution(res);
					group.channel(i).generateColor(i);
				}
				
			}
			Channel&ch = group.channel(minor_channel);
			return ch;
		}
		
		Channel&		Data::activate(index_t major_channel, index_t minor_channel)
		{
			Channel&ch = openChannel(major_channel,minor_channel);
			Profiler::start(&ch);
			return ch;
		}
		
		void			Data::setResolution(unsigned res_)
		{
			if (res == res_)
				return;
			for (unsigned i = 0; i < Count(); i++)
				Array<Group,Adopt>::operator[](i).setResolution(res_);
			res = res_;
		}
				
		/*void			Data::update()
		{
			update(timing.delta);
		}*/
		
		void			Data::update(float weight)
		{
			for (unsigned i = 0; i < Count(); i++)
				for (unsigned j = 0; j < Array<Group,Adopt>::operator[](i).channels(); j++)
					Array<Group,Adopt>::operator[](i).channel(j).nextFrame(weight);
		}
		
		void			Data::advance()
		{
			for (unsigned i = 0; i < Count(); i++)
				for (unsigned j = 0; j < Array<Group,Adopt>::operator[](i).channels(); j++)
					Array<Group,Adopt>::operator[](i).channel(j).advance();
		}

		
	
		Channel*		activeChannel()
						{
							return active_channel;
						}
		Channel*		openChannel(unsigned major_channel, unsigned minor_channel)
						{
							if (active_channel)
								active_channel->stop();
							return &time_data.openChannel(major_channel,minor_channel);
						}
		Channel*		start(unsigned major_channel, unsigned minor_channel)
						{
							if (active_channel)
								active_channel->stop();
							Channel*ch = openChannel(major_channel,minor_channel);
							ch->start();
							return ch;
						}
		Time::Delta		stop()
						{
							if (active_channel)
								return active_channel->stop();
							return Time::Delta::Zero;
						}
		Channel*		start(Channel*channel)
						{
							Channel*old_channel = active_channel;
							active_channel = channel;
							if (old_channel != active_channel)
							{
								if (old_channel)
									old_channel->stop();
								if (active_channel)
									active_channel->start();
							}
							return old_channel;
						}
		
		static Ctr::PreAllocVector<Channel*>	channel_stack;
		
		Channel*		replace(unsigned major_channel, unsigned minor_channel)
						{
							channel_stack << active_channel;
							return start(major_channel,minor_channel);
						}
						
		Time::Delta		restore()
						{
							Time::Delta	result;
							if (active_channel)
								result = active_channel->stop();
							else
								result = Time::Delta::Zero;
							start(channel_stack.pop());
							return result;
						}
					
		
		bool			update()
						{
							return update(timing.delta);
						}
						
		bool			update(float delta)
						{
			
							bool was_unset = !fps_data.groups();
							
							Channel&fps_channel = fps_data.openChannel(0,0);
							if (was_unset)
							{
								fps_data.group(0).Set("FPS",0.8,0.85,1);
								fps_channel.Set("FPS",0.8,0.85,1);
							}
							fps_channel.accumulate(timing.delta>0?1.0f/timing.delta:0);
							
							
							static Time::Stamp t;
							Time::Stamp now;
							
							static Time::Delta times[5] = {Time::Delta::Zero};
							Time::Delta d = now-t;
							for (index_t i = 0; i < ARRAYSIZE(times)-1; i++)
								times[i] = times[i+1];
							times[ARRAYSIZE(times)-1] = d;
							Time::Delta base = M::Min(M::Min(M::Min(times[0],times[1]),M::Min(times[2],times[3])),times[4]);
							float weight = d > base*2;;

							
							time_data.update(weight);
							fps_data.update(weight);
							t = now;
														
							
							current_time += delta;
							bool has_changed = false;
							if (current_time > record_interval)
								if (active_channel)
									active_channel->stop();
							while (current_time > record_interval)
							{
								has_changed = true;
								record = true;
								current_time -= record_interval;
								time_data.advance();
								fps_data.advance();
							}
							if (has_changed)
							{
								last_change.Update();
								if (active_channel)
									active_channel->start();
								foreach (orthographic_graphs,graph)
									(*graph)->update();
								foreach (perspective_graphs,graph)
									(*graph)->update();
							}
							return has_changed;

						}
		void			renderOrthographic()
						{
							foreach (orthographic_graphs,graph)
								(*graph)->Draw();
						}
		void			renderPerspective()
						{
							foreach (perspective_graphs,graph)
								(*graph)->Draw();
						}
	}
}
