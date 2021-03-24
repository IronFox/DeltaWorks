#ifndef importerH
#define importerH

#include "cgs.h"
#include "../general/timer.h"

/******************************************************************

E:\include\importer\importer.h

******************************************************************/

/*
	todo:
		.md2 importer
		.map importer (quake3)
*/

namespace DeltaWorks
{
	namespace Converter	//! Geometry converter namespace
	{

		/*!
			\brief Converter progress callback used to inform the client interface of process updates
		*/
		class ProgressCallback
		{
		public:
		virtual	void			converterProgressUpdate(const String&message)		//! Signals a new process step and indicates that the progress bar should be hidden (if any) \param message Message to print
								{};
		virtual	void			converterProgressBarUpdate(float progress)			//! Signals that the progress bar should be made visible (if any) and updated to the specified progress value (0-1) \param progress New progress bar state (0-1)
								{};
		virtual	void			hideConverterProgressBar()							//! Signals that the progress bar should be hidden
								{};
		virtual	void			dropLastConverterProgressUpdate()					//! Signals that the most recent progress update should be dropped (if the client application uses some sort of update history)
								{};
		};

		/*!
			\brief Root converter class
		
			All format dependent converter classes are required to inherit this root class.
			Conversion processes of this class and its derivatives are NOT thread safe. To import or export geometries parallely use multiple converter instances.
		*/
		class Root
		{
		protected:
				String				last_error;			//!< String representation of the last occured error. Filled in by the inheriting converter class
				ProgressCallback	default_callback,	//!< Internal (mute) callback instance
									*current_callback;	//!< Effective callback instance
				float				seconds;			//!< Number of seconds passed during the last load/save operation
	
		virtual	bool				Read(CGS::Geometry<>&target, const PathString&filename)=0;		//!< Virtual abstract load method (defined by the inheriting converter class)
		virtual	bool				Write(const CGS::Geometry<>&source, const PathString&filename)=0;	//!< Virtual abstract save method (defined by the inheriting converter class)
		public:
			bool					positionObjects;	//!< Indicates that loaded objects should be located in the center of the extracted vertices. True by default
	
			/**/					Root() :current_callback(&default_callback), positionObjects(true)
									{}
								
				bool            	LoadFromFile(CGS::Geometry<>&structure, const PathString&filename, ProgressCallback*callback=NULL)	//! Attempts to import the specified geometrical file. \param structure Geometry to import into. Any existing geometrical data will be cleared \param filename Filename to load from \param callback Custom progress callback instance to report to or NULL to not generate any reports \return true on success, false otherwise
									{
										if (callback)
											current_callback = callback;
										else
											current_callback = &default_callback;
										last_error = "No Error";
										Time::Stamp t;
										bool result = Read(structure,filename);
										seconds = t.ElapsedSeconds();
										return result;
									}
								
				CGS::Geometry<>*  	LoadFromFile(const PathString&filename,ProgressCallback*callback=NULL)	//! Attempts to import the specified geometrical file into a new geometry \param callback Custom progress callback instance to report to or NULL to not generate any reports \param filename Filename to load from 
									{
										CGS::Geometry<>*struc = SignalNew(new CGS::Geometry<>());
										if (LoadFromFile(*struc,filename,callback))
											return struc;
										Discard(struc);
										return NULL;
									}
				bool            	SaveToFile(const CGS::Geometry<>&structure, const PathString&filename,ProgressCallback*callback=NULL)	//! Attempts to export the specified geometry to the specified file \param structure Geometry to export \param filename Filename to save to  \param callback Custom progress callback instance to report to or NULL to not generate any reports
									{
										if (callback)
											current_callback = callback;
										else
											current_callback = &default_callback;
										last_error = "No Error";
										Time::Stamp t;
										bool result = Write(structure,filename);
										seconds = t.ElapsedSeconds();
										return result;
									}
			virtual	const String&	GetError()	const					//! Returns a string representation of the last occured error (if any)
									{
										return last_error;
									}
				float				getSecondsf()	const				//! Returns the number of seconds passed during the last load/save operation
									{
										return seconds;
									}
		};
	}
}

#endif
