#ifndef engine_controlH
#define engine_controlH

#include "textout.h"
#include "aspect.h"
#include <container/vectorList.h>
#include <container/hashtable.h>
#include <math/resolution.h>

namespace Engine
{

	class ControlCluster;

	typedef void(*FRenderInstruction)(const Aspect<>&, const Resolution&);



	/*abstract*/ class RenderInstructionWrapper
	{
	public:
		index_t				orderIndex;
		FRenderInstruction	instruction;
		/**/				RenderInstructionWrapper():orderIndex(InvalidIndex),instruction(NULL)	{}
		/**/				RenderInstructionWrapper(index_t id, FRenderInstruction instruction):
							orderIndex(id),instruction(instruction)	{}

		int					CompareTo(const RenderInstructionWrapper&other)
							{
								if (orderIndex < other.orderIndex)
									return -1;
								if (orderIndex > other.orderIndex)
									return 1;
								return 0;
							}
	};

	/**
	@brief Sequence of renderable objects
	*/
	class RenderSequence
	{
		typedef RenderSequence	Self;
	private:
		typedef PreAllocVector<RenderInstructionWrapper,0>	WrapperList;
		WrapperList			wrappers;
		bool				isSealed;
	public:
		/**/				RenderSequence():isSealed(false)	{}
		void				swap(RenderSequence&other)
							{
								wrappers.swap(other.wrappers);
								std::swap(isSealed,other.isSealed);
							}
		friend void			swap(Self&a, Self&b)	{a.swap(b);}
		void				Insert(FRenderInstruction instruction, index_t orderIndex);
		void				Seal();
		bool				IsSealed()	const	{return isSealed;}
		void				Execute(const Aspect<>&aspect, const Resolution&resolution)	const
							{
								DBG_ASSERT__(IsSealed());
								WrapperList::const_iterator	at = wrappers.begin(),
															end = wrappers.end();
								for (;at != end; ++at)
									at->instruction(aspect,resolution);
							}
	};

	typedef IndexTable<RenderSequence,Swap>	RenderSequenceMap;




	
	/**
	@brief Abstract control module class for sequential game cores
		
	A Control instance describes an input hook for user input reaction as well as standardized rendering methods.
	*/
	/*abstract*/ class Control
	{
	private:
		friend class		ControlCluster;			
	protected:
		ControlCluster		*cluster;
	public:
		Control()			:cluster(NULL){}
		virtual				~Control(){}
							
		virtual	bool		OnKeyDown(Key::Name)						{return false;};	//!< Sends a key down event to the local control
		virtual	bool		OnKeyUp(Key::Name)							{return false;};	//!< Sends a key up event to the local control 
		virtual	bool		OnMouseWheel(short delta)					{return false;};	//!< Sends a mouse wheel event to the local control
	
		virtual	void		Advance(float delta)						{};					//!< Advances the frame. @param Last frames frame length. The control is encouraged to use this delta value rather than the global timing variable if slowmotion effects should be possible
		virtual void		FixedUpdate(float delta)					{};					//!< Called with a fixed frame delta
		virtual void		PostAdvance()								{};					//!< Called once Advance() and FixedUpdate() have been called for the frame

		virtual	bool		DetectNearestGroundHeight(const M::TVec3<>&referencePosition,float&outHeight, M::TVec3<>&outNormal)	{return false;};
		virtual void		OnResolutionChange(const Resolution&newResolution, bool isFinal)		{};
		virtual void		Shutdown()									{};					//!< Signals that the applications is being shut down

		virtual	void		OnInstall(RenderSequenceMap&sequenceMap)	{};					//!< Invoked once this control module is installed on the specified control cluster
		virtual	void		OnUninstall()								{};					//!< Invoked once this control module is installed on the specified control cluster
		ControlCluster*		GetCluster()	const						{return cluster;}
	};
	
	/**
	@brief Sequential collection of Control instances
		
	ControlCluster manages a series of controls.
	*/
	class ControlCluster
	{
	private:
		PreAllocVector<Control*,0>	controlStack;
		float				accumulatedTime,
							fixedFrameDelta;
	public:
		VirtualTextout		*textout;
		RenderSequenceMap	sequenceMap;
	
		/**/				ControlCluster(unsigned fixedUpdateFrequency = 80)
							:accumulatedTime(0),fixedFrameDelta(1.f / float(fixedUpdateFrequency)), textout(NULL)	{}
		virtual				~ControlCluster();
	
		bool				OnKeyDown(Key::Name);					//!< Invokes onKeyDown() methods of all contained control instances. Walks backwards through the list and stops at the first that returns true @return true if any contained instance returned true, false otherwise
		bool				OnKeyUp(Key::Name);						//!< Invokes onKeyUp() methods of all contained control instances. Walks backwards through the list and stops at the first that returns true @return true if any contained instance returned true, false otherwise
		bool				OnMouseWheel(short delta);				//!< Invokes onMouseWheel() methods of all contained control instances. Walks backwards through the list and stops at the first that returns true @return true if any contained instance returned true, false otherwise
	
		void				Advance(float delta);					//!< Invokes advance() methods of all contained control instances. Walks forward through the list
			
		void				Shutdown();								//!< Signals that the application is being shut down

		void				Install(Control*);
		void				Uninstall(Control*);
		bool				DetectNearestGroundHeight(const M::TVec3<>&referencePosition,float&outHeight, M::TVec3<>&outNormal);
		void				SignalResolutionChange(const Resolution&newResolution, bool isFinal);
	};






}



#endif
