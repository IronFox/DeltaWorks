#ifndef engine_interface_screenH
#define engine_interface_screenH

/******************************************************************

Collection of screen-query containers.

******************************************************************/


#include <global_string.h>
#include <container/vectorList.h>

namespace Engine
{
	using namespace DeltaWorks;

	struct					TDisplayFrequency;
	class					SupportedResolution;
	class					ResolutionList;
	class					FrequencyList;

	#if SYSTEM==WINDOWS
		String	mode2str(const DEVMODE&mode);
	#endif



	struct TDisplayFrequency	//! Display frequency (screen refresh rate) carrier
	{
			short						rate;		//!< Screen refresh rate
			unsigned					index;		//!< Internal index variable
			TDisplayFrequency			*next;		//!< Pointer to the next display frequency following in the list
	};

	struct TDisplayMode		//! Display mode (screen resolution) carrier
	{
			DWORD						width,		//!< Screen width in pixels
										height;		//!< Screen height in pixels
			short						frequency;	//!< Display frequency (0 for highest possible)
	};

	class ScreenResolution	//! Display mode (screen resolution) container including all supported display frequencies
	{
	public:
			DWORD						width,		//!< Screen width in pixels
										height;		//!< Screen height in pixels
	#if SYSTEM==UNIX
			int							id;				//!< Unix resolution index
	#endif
			TDisplayFrequency			*frequency;		//!< One directionally linked list containing all display frequencies supported by this resolution
			unsigned					frequencies;	//!< Number of display frequencies supported by this resolution

										ScreenResolution();
	#if SYSTEM==WINDOWS
										ScreenResolution(const DEVMODE&mode);
	#endif
	virtual								~ScreenResolution();
			void						insertFrequency(short frequency);					//!< Inserts the specified display frequency into the linear display frequency list.
			bool						supported(short frequency)					const;	//!< Queries whether or not the specified frequency is supported by this resolution
			short						closestPossibleRate(short target)			const;	//!< Queries the nearest supported display frequency supported by this resolution \param target Target display frequency or 0 for maxium display frequency \return Next greater or identical supported frequency
			TDisplayMode				createDisplayMode(short frequency);					//!< Creates a display mode instance containing the local screen resolution and the specified frequency.\param frequency Target frequency or 0 for maximum possible frequency
			bool						operator>(const ScreenResolution&other)	const;	//!< Ordering operator. Compares the local resolution to the specified location. \return true if the local resolution is greater than the specified resolution, false otherwise.
			bool						operator<(const ScreenResolution&other)	const;	//!< Ordering operator. Compares the local resolution to the specified location. \return true if the local resolution is less than the specified resolution, false otherwise.
	#if SYSTEM==WINDOWS
			bool						operator>(const DEVMODE&other)	const;
			bool						operator<(const DEVMODE&other)	const;
	#endif
	};

	class ResolutionList:public Ctr::Vector<ScreenResolution>
	{
	public:
		#if SYSTEM==WINDOWS
			void					insert(const DEVMODE&mode);
		#endif
		String						list();		//!< Retrieves a string listing of the local list content
	};

	class FrequencyList	//! Display frequency (screen refresh rate) list. Holds up to 255 different display frequencies
	{
	private:
			short						flist[0x100];
			BYTE						fcnt;
	public:

										FrequencyList();
			void						insert(short frequency);	//!< Inserts a display frequency into the list if it is not already listed.
			unsigned					find(short frequency);		//!< Queries the index of the specified frequency. \return Index of the specified frequency (0 = first frequency) or UNSIGNED_UNDEF (0xFFFFFFFF) if it could not be found.
			short						get(unsigned index);		//!< Retrieves the display frequency at the specified index. \param index Index of the frequency to retrieve
			unsigned					Count();					//!< Retrieves the number of display frequencies stored in the local display frequency list.
			String						list();						//!< Retrieves a string listing of the local frequencies
	};
}

#endif
