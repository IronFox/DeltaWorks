#ifndef exceptionH
#define exceptionH

/*#include <iostream>
using namespace std;*/

/******************************************************************

General exception-container.

******************************************************************/


//#include <global_string.h>

/*
__DATE__  The date the file is compiled; for example, "Jul 14, 1995". This symbol is a predefined macro. 
__FILE__  The name of the file being compiled; for example "prog.c". This symbol is a predefined macro. 
__func__ The name of the function currently being compiled. This predefined identifier is only available under the emerging ANSI/ISO C99 standard. 
__LINE__  The number of the line being compiled (before including any header files). This symbol is a predefined macro. 
__TIME__  The time the file is compiled in 24-hour format; for example, "13:01:45". This symbol is a predefined macro. 
__STDC__  
*/


#include <exception>
#include <iostream>
#include "../interface/string.h"

#ifdef __BORLANDC__
    #define __func__    __FUNC__
#elif  defined(_MSC_VER)
	#define	__func__	__FUNCTION__
#endif


#if SYSTEM==UNIX
	#include <signal.h>
	#include <stdexcept>
#endif


namespace DeltaWorks
{

	template <size_t Length>
		struct TFixedString
		{
			static const size_t BufferSize = Length;

			char			*begin;	//!< Actual string to use. May or may not match the beginning of buffer


			char			buffer[BufferSize];
		};

	struct TCodeLocation
	{
		const char*		file="",*method="";
		unsigned		line=0;

		typedef TFixedString<512>	TOutString;
    
		/**/			TCodeLocation()	{}
		/**/			TCodeLocation(const char*file, const char*method, unsigned line):file(file),method(method),line(line)	{}
					
		void			Format(TOutString&)	const throw();
		void			FormatFilename(TOutString&)	const throw();
	};



	#include <string.h>

	#if 0

		extern TCodeLocation	exception_history[1000];
		extern unsigned			exception_history_length;
	
		#define FUNC_BEGIN		{if (exception_history_length >= ARRAYSIZE(exception_history)) FATAL__("Except history overflow"); exception_history[exception_history_length].file = __FILE__; exception_history[exception_history_length].method = __func__; exception_history[exception_history_length++] = __LINE__;}
		#define FUNC_END		{exception_history_length--;}
	#else
		#define FUNC_BEGIN
		#define FUNC_END
	#endif






	class FatalDescriptor:public TCodeLocation
	{
	private:
		char			message[10000],
						formatted[11000];

		
		void			Copy(const char*message, size_t length) throw();
		void			Reformat() throw();
	public:
						FatalDescriptor(const TCodeLocation&location, const char*msg):TCodeLocation(location)
						{
							Copy(msg,strlen(msg));
						}

	template <class C>  FatalDescriptor(const TCodeLocation&location, const C&msg):TCodeLocation(location)
						{
							char*terminal = msg.WriteTo(message,message+sizeof(message)-1);
							*terminal = 0;
							Reformat();
						}

		const char*		ErrorMessage()	const throw()
						{
							return formatted;
						}
		const char*		ToString()	const throw()
						{
							return formatted;
						}
		const char*		getMessage()	const throw()	{return message;}
	};


	/**
		@brief Except collection
 
		This namespace provides various exceptions for different applications
	 */
	namespace Except
	{




		#if SYSTEM==WINDOWS
			/**
			@brief Fatal exception, which should always lead to program termination
			*/
			class Fatal:public ::DeltaWorks::FatalDescriptor, public std::exception
			{
			public:
								Fatal(const TCodeLocation&location, const char*msg):FatalDescriptor(location,msg),std::exception(ToString(),1)
								{}
			template <class C>  Fatal(const TCodeLocation&location, const C&msg):FatalDescriptor(location,msg),std::exception(ToString(),1)
								{}

			};
		#else
			/**
			@brief Fatal exception, which should always lead to program termination
			*/
			class Fatal:public ::FatalDescriptor, public std::runtime_error
			{
				typedef std::runtime_error	Super;
			public:
								Fatal(const TCodeLocation&location, const char*msg):FatalDescriptor(location,msg),Super(ToString())
								{}
			template <class C>  Fatal(const TCodeLocation&location, const C&msg):FatalDescriptor(location,msg),Super(ToString())
								{}

			};
		#endif



		/**
			@brief This type tell exceptions that the provided string is global and should be referenced rather than copied
		
			Use like a function: throw Except::Runtime(Except::globalString("Hello world"));
		 */
		struct globalString
		{
			const char		*address;

							globalString(const char*addr):address(addr)
							{}
		};


		/**
			@brief Except name using constant arrays and automatic concatenation
		
			No dynamic memory allocation is used

		*/
		class Name
		{
		private:
				char			string[0x200];
				size_t			length;
		public:
			
								Name(const char*single)
								{
									if (!single)
									{
										string[0] = 0;
										length = 0;
									}
									else
									{
										length = strlen(single);
										if (length >= sizeof(string))
											length = sizeof(string)-1;
										memcpy(string,single,length);
										string[length] = 0;
									}
								}
								Name(const Name&parent_name, const char*child_name)
								{
									if (!child_name)
									{
										memcpy(string,parent_name.string,parent_name.length);
										length = parent_name.length;
										string[length] = 0;
									}
									else
									{
										memcpy(string,parent_name.string,parent_name.length);
										length = parent_name.length;
									
										size_t len = strlen(child_name);
										if (length+len >= sizeof(string))
											len = sizeof(string)-length-1;
										memcpy(string+length,child_name,len);
										length+=len;
										string[length] = 0;
									}

								}
	
								operator const char*()	const	{return string;}
		};





		//namespace Runtime
		//{


			#if SYSTEM==WINDOWS
				typedef std::exception	RootException;
			#else
				typedef std::runtime_error	RootException;
			#endif

			/**
			@brief Generic runtime exception
			*/
			class Exception:public RootException
			{
				typedef RootException	Super;
			protected:
				template <typename T>
					static inline const char*		castToChar(const IString<T>&string)
					{
						static StringConverter<char,0x1000>	converter;
						return converter.convert(string);
					}
			public:
				TCodeLocation				location;
				bool						hasLocation;
				/**/						Exception():hasLocation(false)
												#if SYSTEM!=WINDOWS
													:Super("")
												#endif
											{}
				/**/						Exception(const TCodeLocation&loc):location(loc),hasLocation(true)
												#if SYSTEM!=WINDOWS
													:Super("")
												#endif
											{}
				/**/						Exception(const char*message):Super(message),hasLocation(false) {}
				/**/						Exception(const TCodeLocation&loc, const char*message):Super(message),location(loc),hasLocation(true) {}
				#if SYSTEM==WINDOWS
					/**/					Exception(const globalString&string):Super(string.address,1),hasLocation(false) {}
					/**/					Exception(const TCodeLocation&loc, const globalString&string):Super(string.address,1),location(loc),hasLocation(true) {}
				#else
					/**/					Exception(const globalString&string):Super(string.address),hasLocation(false) {}
					/**/					Exception(const TCodeLocation&loc, const globalString&string):Super(string.address),location(loc),hasLocation(true) {}
				#endif
				/**/						Exception(const Super&except):Super(except),hasLocation(false) {}
				template <class C>			Exception(const IString<C>&string):Super(castToChar(string)),hasLocation(false) { }
				template <class C>			Exception(const TCodeLocation&loc, const IString<C>&string):Super(castToChar(string)),location(loc),hasLocation(true) { }
				virtual	const char*			GetType()	const {return StaticGetName();}
				virtual	const char*			what()		const override
				{
					static char buffer[0x4000];
					if (!hasLocation)
					{
						sprintf_s<sizeof(buffer)>(buffer,"[%s] %s",GetType(),Super::what());
					}
					else
					{
						TCodeLocation::TOutString	formatted;
						location.Format(formatted);
						sprintf_s<sizeof(buffer)>(buffer,"%s: [%s] %s",formatted.begin,GetType(),Super::what());
					}
					buffer[sizeof(buffer)-1] = 0;
					return buffer;
				};
				/**
				Queries the raw exception message (without any code location)
				*/
				const char*					GetRawMessage() const {return Super::what();}
				static const Name&			StaticGetName()	{static const Name	name = Name("Except"); return name;}

				const std::exception&		c_str()	const	{return *this;}		//ok, this is close to a hack. let's forget i had to do this :P. WHY did i have to do this again...?
			};



		
			typedef Exception		Inherit;



			#undef BEGIN_EXCEPTION_GROUP
			#define BEGIN_EXCEPTION_GROUP(_name_)\
				struct _PreType##_name_: public Inherit\
				{\
					typedef Inherit Super;\
					/**/								_PreType##_name_(){}\
					/**/								_PreType##_name_(const char*message):Inherit(message) {}\
					/**/								_PreType##_name_(const globalString&string):Inherit(string){}\
					/**/								_PreType##_name_(const TCodeLocation&loc):Super(loc){}\
					/**/								_PreType##_name_(const TCodeLocation&loc, const char*message):Super(loc,message) {}\
					/**/								_PreType##_name_(const TCodeLocation&loc, const globalString&string):Super(loc,string){}\
					template <class C>					_PreType##_name_(const IString<C>&string):Super(string){}\
					template <class C>					_PreType##_name_(const TCodeLocation&loc, const IString<C>&string):Super(loc,string){}\
					static const Name&					StaticGetName()	{static const Name name=Name(Super::StaticGetName(),"/"#_name_); return name;}\
					virtual	const char*					GetType()	const override					{return StaticGetName();}\
				};\
				struct _name_: public _PreType##_name_\
				{\
					typedef _PreType##_name_	Super;\
					/**/								_name_() {}\
					/**/								_name_(const char*message):Super(message) {}\
					/**/								_name_(const globalString&string):Super(string) {}\
					template <class C>					_name_(const IString<C>&string):Super(string) {}\
					/**/								_name_(const TCodeLocation&loc):Super(loc) {}\
					/**/								_name_(const TCodeLocation&loc, const char*message):Super(loc,message) {}\
					/**/								_name_(const TCodeLocation&loc, const globalString&string):Super(loc,string) {}\
					template <class C>					_name_(const TCodeLocation&loc, const IString<C>&string):Super(loc,string) {}

			#undef END_EXCEPTION_GROUP
			#define END_EXCEPTION_GROUP\
				};


			#undef DEFINE_MEMBER_EXCEPTION
			#define DEFINE_MEMBER_EXCEPTION(_name_)\
						BEGIN_EXCEPTION_GROUP(_name_)\
						END_EXCEPTION_GROUP


			/**
				@brief Program logic related exceptions
			 */
			BEGIN_EXCEPTION_GROUP(Program)
				/**
					@brief General program logic fault
				 */
				DEFINE_MEMBER_EXCEPTION(GeneralFault)
				/**
					@brief An function or method internal self-test has failed
				 */
				DEFINE_MEMBER_EXCEPTION(InternalVerificationFault)
				/**
					@brief A method or function's functionality is not yet implemented (or may never be)
				 */
				DEFINE_MEMBER_EXCEPTION(FunctionalityNotImplemented)

				/**
					@brief A method or function invocation could not satisfy a requested functionality
				
					This most typically revolves around dynamic functionality requests via parameter.
				 */
				DEFINE_MEMBER_EXCEPTION(UnsupportedRequest)

				/**
					@brief Program execution exception due to invalid parameters
					*/
				DEFINE_MEMBER_EXCEPTION(ParameterFault)

				/**
					@brief Program execution exception due to a parameter not being unique (this generally applies to names during insert operations)
					*/
				DEFINE_MEMBER_EXCEPTION(UniquenessViolation)

				/**
					@brief The internal data consistency of a module is compromised
					*/
				DEFINE_MEMBER_EXCEPTION(DataConsistencyFault)

				/**
					@brief The resquested member element could not be found
				*/
				DEFINE_MEMBER_EXCEPTION(MemberNotFound)

			END_EXCEPTION_GROUP

			/**
				@brief Memory management related exceptions
			 */
			BEGIN_EXCEPTION_GROUP(Memory)
				/**
				@brief General memory fault
				 */
				DEFINE_MEMBER_EXCEPTION(GeneralFault)

				/**
				Fault indicating some logic failure during serialization or deserialization
				*/
				DEFINE_MEMBER_EXCEPTION(SerializationFault)

			END_EXCEPTION_GROUP

			/**
				@brief Rendering related exceptions
			 */
			BEGIN_EXCEPTION_GROUP(Renderer)
				/**
					@brief General rendering exception
				 */
				DEFINE_MEMBER_EXCEPTION(GeneralFault)

				DEFINE_MEMBER_EXCEPTION(DeviceConnectionLost)

				DEFINE_MEMBER_EXCEPTION(ShaderRejected)
				/**
					@brief Texture transfer (upload or download) related exceptions
				 */
				BEGIN_EXCEPTION_GROUP(TextureTransfer)
					/**
						@brief General texture transfer exception
					 */
					DEFINE_MEMBER_EXCEPTION(GeneralFault)
					/**
						@brief Texture transfer exception due to invalid parameters
					 */
					DEFINE_MEMBER_EXCEPTION(ParameterFault)

				END_EXCEPTION_GROUP

					


				/**
					@brief Geometry transfer (upload or download) related exceptions
				 */
				BEGIN_EXCEPTION_GROUP(GeometryTransfer)
					/**
						@brief General geometry transfer exception
					 */
					DEFINE_MEMBER_EXCEPTION(GeneralFault)
					/**
						@brief Geometry transfer exception due to invalid parameters
					 */
					DEFINE_MEMBER_EXCEPTION(ParameterFault)

				END_EXCEPTION_GROUP

			END_EXCEPTION_GROUP


			/**
				@brief IO (storage or network) related exceptions
			 */
			BEGIN_EXCEPTION_GROUP(IO)

				/**
					@brief General IO fault
				 */
				DEFINE_MEMBER_EXCEPTION(GeneralFault)

				/**
					@brief IO exception due to invalid parameters
					*/
				DEFINE_MEMBER_EXCEPTION(ParameterFault)


				/**
					@brief The assembly of a complex resource structure from various sources has failed
				
					This exception would be thrown if a resource structure combines various different
					 data structures and finds that some of them do not match. It may also be used if
					 the current data composition is not fit for export or processing in general.
				*/
				DEFINE_MEMBER_EXCEPTION(StructureCompositionFault)



				/**
					@brief Network (UDP, TCP, Pipe) related exceptions
				*/
				BEGIN_EXCEPTION_GROUP(Network)
					/**
						@brief General network fault
					 */
					DEFINE_MEMBER_EXCEPTION(GeneralFault)

					DEFINE_MEMBER_EXCEPTION(HostNotFound)

					DEFINE_MEMBER_EXCEPTION(ConnectionFailed)

					DEFINE_MEMBER_EXCEPTION(ConnectionLost)
					DEFINE_MEMBER_EXCEPTION(ProtocolViolation)

					/**
					@brief Network data not formatted as expected
					*/
					DEFINE_MEMBER_EXCEPTION(DataFormatFault)
					DEFINE_MEMBER_EXCEPTION(ReadLogicFault)
					DEFINE_MEMBER_EXCEPTION(WriteLogicFault)


				END_EXCEPTION_GROUP

				/**
					@brief Drive (HDD or removable) IO related exceptions
				 */
				BEGIN_EXCEPTION_GROUP(DriveAccess)
					/**
						@brief General drive fault
					 */
					DEFINE_MEMBER_EXCEPTION(GeneralFault)

					DEFINE_MEMBER_EXCEPTION(FileOpenFault)

					/**
						@brief File reading failed
						*/
					DEFINE_MEMBER_EXCEPTION(DataReadFault)

					/**
						@brief File writing failed
						*/
					DEFINE_MEMBER_EXCEPTION(DataWriteFault)


					/**
						@brief File data not formatted as expected
					 */
					DEFINE_MEMBER_EXCEPTION(FileFormatFault)

				
					/**
						@brief File data not as expected
					
						May also apply to data already loaded and stored.
						DataFault should be thrown where FileFormatFault is inappropriate
					 */
					DEFINE_MEMBER_EXCEPTION(FileDataFault)


				END_EXCEPTION_GROUP


			END_EXCEPTION_GROUP

		//}

	}

	//using namespace Except;




	#if SYSTEM==WINDOWS
		#define DISPLAY__(_MESSAGE_)    {MessageBoxA(NULL,(_MESSAGE_),"Fatal Error",MB_OK|MB_ICONERROR);}
	#elif SYSTEM==UNIX
		#include <iostream>
		#define DISPLAY__(_MESSAGE_)    {std::cerr<<"Fatal Error\n"<<(_MESSAGE_)<<std::endl;};
	#endif

	#define CLOCATION   ::DeltaWorks::TCodeLocation((const char*)__FILE__,__func__,__LINE__)


	namespace Except
	{
		extern	void (*onFatal)(const FatalDescriptor&descriptor);

		void	EndFatal();		//!< Leaves synchronizes fatal invokation
		void	BeginFatal();	//!< Begins fatal handling (synchronizes)

		void	HolyShit(const char*message);		//!< This is when shit hit the fan, most commonly somewhere where fatal event handling is no longer possible. Immediately shuts down application
		[[noreturn]]		
		void	TerminateApplication();				//!< Immediately terminates the application


		//#define EBACK__         throw(C_Exception)

		/**
		@brief Invokes fatal exception handler, then terminates application.

		Only the first thread to call this function, actually invokes the handler. Subsequent invocations of this function by any thread are rejected
		*/
		template <class C>
			inline void TriggerFatal(const TCodeLocation&location, const C&message)
			{
				BeginFatal();
					FatalDescriptor	descriptor(location,message);
					onFatal(descriptor);
					TerminateApplication();
				EndFatal();
			}
	}

	#define FATAL__(_MESSAGE_)					{::DeltaWorks::Except::TriggerFatal(CLOCATION,(_MESSAGE_));}

	#ifdef _DEBUG
		#define DBG_FATAL__(_MESSAGE_)			FATAL__(_MESSAGE_)
	#else
		#define DBG_FATAL__(_MESSAGE_)			{}
	#endif

	#define THROW_FATAL__(_message_)			throw ::DeltaWorks::Except::Fatal(CLOCATION,message);
	#define THROW_FATAL_EXCEPTION__(_message_)	THROW_FATAL__(_message_)



	#define ASSERT__(_EXPRESSION_)	{if (!(_EXPRESSION_)) FATAL__("[" #_EXPRESSION_ "] evaluates to false");}
	#define ASSERT_NOT_NULL__(_EXPRESSION_)	{if ((_EXPRESSION_)==NULL) FATAL__("[" #_EXPRESSION_ "] evaluates to NULL");}
	#define ASSERT_NOT_NULL1__(_EXPRESSION_,_PARAMETER0_)	{using ::DeltaWorks::StringConversion::ToString; if ((_EXPRESSION_)==NULL) FATAL__(#_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+"') evaluates to NULL");}
	#define ASSERT_NOT_NULL2__(_EXPRESSION_,_PARAMETER0_,_PARAMETER1_)	{using ::DeltaWorks::StringConversion::ToString; if ((_EXPRESSION_)==NULL) FATAL__(#_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+" and "#_PARAMETER1_"='"+ToString(_PARAMETER1_)+"') evaluates to NULL");}
	#define ASSERT_IS_NULL__(_EXPRESSION_)	{if ((_EXPRESSION_)!=NULL) FATAL__("[" #_EXPRESSION_ "] evaluates to not NULL");}

	#define ASSERT1__(_EXPRESSION_,_PARAMETER0_)	{using ::DeltaWorks::StringConversion::ToString; if (!(_EXPRESSION_)) FATAL__(#_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+"') evaluates to false");}
	#define ASSERT2__(_EXPRESSION_,_PARAMETER0_,_PARAMETER1_)	{using ::DeltaWorks::StringConversion::ToString; if (!(_EXPRESSION_)) FATAL__(#_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+"' and "#_PARAMETER1_"='"+ToString(_PARAMETER1_)+"') evaluates to false");}
	#define ASSERT3__(_EXPRESSION_,_PARAMETER0_,_PARAMETER1_,_PARAMETER2_)	{using ::DeltaWorks::StringConversion::ToString; if (!(_EXPRESSION_)) FATAL__( #_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+"', "#_PARAMETER1_"='"+ToString(_PARAMETER1_)+"', and "#_PARAMETER2_"='"+ToString(_PARAMETER2_)+"') evaluates to false");}
	#define ASSERT4__(_EXPRESSION_,_PARAMETER0_,_PARAMETER1_,_PARAMETER2_,_PARAMETER3_)	{using ::DeltaWorks::StringConversion::ToString; if (!(_EXPRESSION_)) FATAL__( #_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+"', "#_PARAMETER1_"='"+ToString(_PARAMETER1_)+"', "#_PARAMETER2_"='"+ToString(_PARAMETER2_)+"', and "#_PARAMETER3_"='"+ToString(_PARAMETER3_)+"') evaluates to false");}
	#define ASSERT5__(_EXPRESSION_,_PARAMETER0_,_PARAMETER1_,_PARAMETER2_,_PARAMETER3_,_PARAMETER4_)	{using ::DeltaWorks::StringConversion::ToString; if (!(_EXPRESSION_)) FATAL__( #_EXPRESSION_ " (with "#_PARAMETER0_"='"+ToString(_PARAMETER0_)+"', "#_PARAMETER1_"='"+ToString(_PARAMETER1_)+"', "#_PARAMETER2_"='"+ToString(_PARAMETER2_)+"', "#_PARAMETER3_"='"+ToString(_PARAMETER3_)+"', and "#_PARAMETER4_"='"+ToString(_PARAMETER4_)+"') evaluates to false");}

	#define CONSTRAINT_VIOLATION3__(component,vector,lower_bounds,upper_bounds)	{using ::DeltaWorks::StringConversion::ToString;  FATAL__("Constraint violation of component "#component" of vector "#vector" ("+ToString((vector)[0])+", "+ToString((vector)[1])+", "+ToString((vector)[2])+") against constraint ["+ToString(lower_bounds)+", "+ToString(upper_bounds)+"]");}
	#define ASSERT_COMPONENT_IS_CONSTRAINED3__(component,vector,lower_bounds,upper_bounds)	{using ::DeltaWorks::StringConversion::ToString; if ((vector)[component]<(lower_bounds) || (vector)[component]>(upper_bounds)) CONSTRAINT_VIOLATION3__(component,vector,lower_bounds,upper_bounds);}
	#define	ASSERT_IS_CONSTRAINED3__(vector,lower_bounds,upper_bounds)	{using ::DeltaWorks::StringConversion::ToString; ASSERT_COMPONENT_IS_CONSTRAINED3__(0,vector,lower_bounds,upper_bounds);ASSERT_COMPONENT_IS_CONSTRAINED3__(1,vector,lower_bounds,upper_bounds);ASSERT_COMPONENT_IS_CONSTRAINED3__(2,vector,lower_bounds,upper_bounds);}
	#define	ASSERT_IS_CONSTRAINED__(value,lower_bounds,upper_bounds)	{using ::DeltaWorks::StringConversion::ToString; if ((value)<(lower_bounds) || (value)>(upper_bounds)) FATAL__("Constraint violation of "#value" ("+ToString(value)+") against constraint ["+ToString(lower_bounds)+", "+ToString(upper_bounds)+"]");}

	#define	ASSERT_EQUAL__(exp0,exp1)						ASSERT2__((exp0)==(exp1),exp0,exp1)
	#define	ASSERT_EQUAL1__(exp0,exp1,exp2)					ASSERT3__((exp0)==(exp1),exp0,exp1,exp2)
	#define	ASSERT_EQUAL2__(exp0,exp1,exp2,exp3)			ASSERT4__((exp0)==(exp1),exp0,exp1,exp2,exp3)
	#define	ASSERT_EQUAL3__(exp0,exp1,exp2,exp3,exp4)		ASSERT5__((exp0)==(exp1),exp0,exp1,exp2,exp3,exp4)

	#define	ASSERT_NOT_EQUAL__(exp0,exp1)					ASSERT2__((exp0)!=(exp1),exp0,exp1)
	#define	ASSERT_NOT_EQUAL1__(exp0,exp1,exp2)				ASSERT3__((exp0)!=(exp1),exp0,exp1,exp2)
	#define	ASSERT_NOT_EQUAL2__(exp0,exp1,exp2,exp3)		ASSERT4__((exp0)!=(exp1),exp0,exp1,exp2,exp3)

	#define	ASSERT_LESS__(exp0,exp1)						ASSERT2__((exp0)<(exp1),exp0,exp1)
	#define	ASSERT_LESS1__(exp0,exp1,exp2)					ASSERT3__((exp0)<(exp1),exp0,exp1,exp2)
	#define	ASSERT_LESS2__(exp0,exp1,exp2,exp3)				ASSERT4__((exp0)<(exp1),exp0,exp1,exp2,exp3)

	#define	ASSERT_GREATER__(exp0,exp1)						ASSERT2__((exp0)>(exp1),exp0,exp1)
	#define	ASSERT_GREATER1__(exp0,exp1,exp2)				ASSERT3__((exp0)>(exp1),exp0,exp1,exp2)
	#define	ASSERT_GREATER2__(exp0,exp1,exp2,exp3)			ASSERT4__((exp0)>(exp1),exp0,exp1,exp2,exp3)

	#define	ASSERT_LESS_OR_EQUAL__(exp0,exp1)				ASSERT2__((exp0)<=(exp1),exp0,exp1)
	#define	ASSERT_LESS_OR_EQUAL1__(exp0,exp1,exp2)			ASSERT3__((exp0)<=(exp1),exp0,exp1,exp2)
	#define	ASSERT_LESS_OR_EQUAL2__(exp0,exp1,exp2,exp3)	ASSERT4__((exp0)<=(exp1),exp0,exp1,exp2,exp3)

	#define	ASSERT_GREATER_OR_EQUAL__(exp0,exp1)			ASSERT2__((exp0)>=(exp1),exp0,exp1)
	#define	ASSERT_GREATER_OR_EQUAL1__(exp0,exp1,exp2)		ASSERT3__((exp0)>=(exp1),exp0,exp1,exp2)
	#define	ASSERT_GREATER_OR_EQUAL2__(exp0,exp1,exp2,exp3)	ASSERT4__((exp0)>=(exp1),exp0,exp1,exp2,exp3)


	#ifdef _DEBUG
		#define DBG_ASSERT__			ASSERT__
		#define DBG_ASSERT1__(exp0,exp1)	ASSERT1__(exp0,exp1)
		#define DBG_ASSERT2__			ASSERT2__
		#define DBG_ASSERT3__			ASSERT3__
		#define DBG_ASSERT4__			ASSERT4__

		#define DBG_VERIFY__			ASSERT__
		#define DBG_VERIFY1__			ASSERT1__
		#define DBG_VERIFY2__			ASSERT2__
		#define DBG_VERIFY3__			ASSERT3__
		#define DBG_VERIFY4__			ASSERT4__

		#define DBG_ASSERT_NOT_NULL__	ASSERT_NOT_NULL__
		#define DBG_ASSERT_NOT_NULL1__	ASSERT_NOT_NULL1__
		#define DBG_ASSERT_IS_NULL__	ASSERT_IS_NULL__

		#define DBG_VERIFY_NOT_NULL__	ASSERT_NOT_NULL__
		#define DBG_VERIFY_NOT_NULL1__	ASSERT_NOT_NULL1__
		#define DBG_VERIFY_IS_NULL__	ASSERT_IS_NULL__

		#define DBG_CONSTRAINT_VIOLATION3__				CONSTRAINT_VIOLATION3__
		#define DBG_ASSERT_COMPONENT_IS_CONSTRAINED3__	ASSERT_COMPONENT_IS_CONSTRAINED3__
		#define	DBG_ASSERT_IS_CONSTRAINED3__			ASSERT_IS_CONSTRAINED3__
		#define	DBG_ASSERT_IS_CONSTRAINED__				ASSERT_IS_CONSTRAINED__

		#define	DBG_ASSERT_EQUAL__		ASSERT_EQUAL__
		#define	DBG_ASSERT_EQUAL1__		ASSERT_EQUAL1__
		#define	DBG_ASSERT_EQUAL2__		ASSERT_EQUAL2__

		#define	DBG_ASSERT_NOT_EQUAL__	ASSERT_NOT_EQUAL__
		#define	DBG_ASSERT_NOT_EQUAL1__	ASSERT_NOT_EQUAL1__
		#define	DBG_ASSERT_NOT_EQUAL2__	ASSERT_NOT_EQUAL2__

		#define	DBG_ASSERT_LESS__		ASSERT_LESS__
		#define	DBG_ASSERT_LESS1__		ASSERT_LESS1__
		#define	DBG_ASSERT_LESS2__		ASSERT_LESS2__

		#define	DBG_ASSERT_GREATER__	ASSERT_GREATER__
		#define	DBG_ASSERT_GREATER1__	ASSERT_GREATER1__
		#define	DBG_ASSERT_GREATER2__	ASSERT_GREATER2__

		#define	DBG_ASSERT_LESS_OR_EQUAL__	ASSERT_LESS_OR_EQUAL__
		#define	DBG_ASSERT_LESS_OR_EQUAL1__	ASSERT_LESS_OR_EQUAL1__
		#define	DBG_ASSERT_LESS_OR_EQUAL2__	ASSERT_LESS_OR_EQUAL2__

		#define	DBG_ASSERT_GREATER_OR_EQUAL__	ASSERT_GREATER_OR_EQUAL__
		#define	DBG_ASSERT_GREATER_OR_EQUAL1__	ASSERT_GREATER_OR_EQUAL1__
		#define	DBG_ASSERT_GREATER_OR_EQUAL2__	ASSERT_GREATER_OR_EQUAL2__

	#else
		#define DBG_ASSERT__(exp0)
		#define DBG_ASSERT1__(exp0,exp1)
		#define DBG_ASSERT2__(exp0,exp1,exp2)
		#define DBG_ASSERT3__(exp0,exp1,exp2,exp3)
		#define DBG_ASSERT4__(exp0,exp1,exp2,exp3,exp4)

		#define DBG_VERIFY__(exp0)						exp0
		#define DBG_VERIFY1__(exp0,exp1)				exp0
		#define DBG_VERIFY2__(exp0,exp1,exp2)			exp0
		#define DBG_VERIFY3__(exp0,exp1,exp2,exp3)		exp0
		#define DBG_VERIFY4__(exp0,exp1,exp2,exp3,exp4)	exp0

		#define DBG_ASSERT_NOT_NULL__(exp0)
		#define DBG_ASSERT_NOT_NULL1__(exp0,exp1)
		#define DBG_ASSERT_IS_NULL__(exp0)

		#define DBG_VERIFY_NOT_NULL__(exp0)				exp0
		#define DBG_VERIFY_NOT_NULL1__(exp0,exp1)		exp0
		#define DBG_VERIFY_IS_NULL__(exp0)				exp0

		#define DBG_CONSTRAINT_VIOLATION3__(exp0,exp1,exp2,exp3)
		#define DBG_ASSERT_COMPONENT_IS_CONSTRAINED3__(exp0,exp1,exp2,exp3)
		#define	DBG_ASSERT_IS_CONSTRAINED3__(exp0,exp1,exp2)
		#define	DBG_ASSERT_IS_CONSTRAINED__(exp0,exp1,exp2)

		#define	DBG_ASSERT_EQUAL__(exp0,exp1)
		#define	DBG_ASSERT_EQUAL1__(exp0,exp1,exp2)
		#define	DBG_ASSERT_EQUAL2__(exp0,exp1,exp2,exp3)

		#define	DBG_ASSERT_NOT_EQUAL__(exp0,exp1)
		#define	DBG_ASSERT_NOT_EQUAL1__(exp0,exp1,exp2)
		#define	DBG_ASSERT_NOT_EQUAL2__(exp0,exp1,exp2,exp3)

		#define	DBG_ASSERT_LESS__(exp0,exp1)
		#define	DBG_ASSERT_LESS1__(exp0,exp1,exp2)
		#define	DBG_ASSERT_LESS2__(exp0,exp1,exp2,exp3)

		#define	DBG_ASSERT_GREATER__(exp0,exp1)
		#define	DBG_ASSERT_GREATER1__(exp0,exp1,exp2)
		#define	DBG_ASSERT_GREATER2__(exp0,exp1,exp2,exp3)

		#define	DBG_ASSERT_LESS_OR_EQUAL__(exp0,exp1)
		#define	DBG_ASSERT_LESS_OR_EQUAL1__(exp0,exp1,exp2)
		#define	DBG_ASSERT_LESS_OR_EQUAL2__(exp0,exp1,exp2,exp3)

		#define	DBG_ASSERT_GREATER_OR_EQUAL__(exp0,exp1)
		#define	DBG_ASSERT_GREATER_OR_EQUAL1__(exp0,exp1,exp2)
		#define	DBG_ASSERT_GREATER_OR_EQUAL2__(exp0,exp1,exp2,exp3)
	#endif






	#define EXCEPT_ASSERT__(ExceptionClass, expression)		if (!(expression)) throw ExceptionClass(Except::globalString("Assertion failed: '"#expression"'"));
	
	#define FILE_READ_ASSERT_ZERO__(expression) EXCEPT_ASSERT__(Except::IO::DriveAccess::DataReadFault,expression==0)
}	


#endif

/**
C++Builder predefines certain global identifiers known as manifest constants. Most global identifiers begin and end with two underscores (__).

Note: For readability, underscores are often separated by a single blank space. In your source code, you should never insert whitespace between underscores.

Macro	Value	Description

_ _BCOPT_ _	1	Defined in any compiler that has an optimizer.
_ _BCPLUSPLUS_ _	0x0560	Defined if you've selected C++ compilation; will increase in later releases.
_ _BORLANDC_ _	0x0560	Version number.
_ _CDECL_ _ 	1	Defined if Calling Convention is set to cdecl; otherwise undefined.
_CHAR_UNSIGNED	1	Defined by default indicating that the default char is unsigned char. Use the -K compiler option to undefine this macro.

_ _CODEGUARD_ _	 	Defined whenever one of the CodeGuard compiler options is used; otherwise it is undefined.
_ _CONSOLE_ _	1 	When defined, the macro indicates that the program is a console application.
_CPPUNWIND	1	Enable stack unwinding. This is true by default; use -xd- to disable.
_ _cplusplus	1	Defined if in C++ mode; otherwise, undefined.
_ _DATE_ _	String literal	Date when processing began on the current file.

_ _DLL_ _	1	Defined whenever the -WD compiler option is used; otherwise it is undefined.
_ _FILE_ _	String literal	Name of the current file being processed.
_ _FLAT_ _	1	Defined when compiling in 32-bit flat memory model.
_ _FUNC_ _	String literal	Name of the current function being processed. More details.
_ _LINE_ _	Decimal constant	Number of the current source file line being processed.

_M_IX86	0x12c	Always defined. The default  value is 300. You can change the value to 400 or 500 by using the /4 or /5 compiler options.
_ _MT_ _	1	Defined only if the -tWM option is used. It specifies that the multithread library is to be linked.
_ _PASCAL_ _ 	1	Defined if Calling Convention is set to Pascal; otherwise undefined.

_ _STDC_ _	1	Defined if you compile with the -A compiler option; otherwise, it is undefined.
_ _TCPLUSPLUS_ _ 	0x0560	Version number.
_ _TEMPLATES_ _	1	Defined as 1 for C++ files (meaning that templates are supported); otherwise, it is undefined.
_ _TIME_ _	String literal	Time when processing began on the current file.
_ _TLS_ _	1	Thread Local Storage. Always true in C++Builder.
_ _TURBOC_ _ 	0x0560	Will increase in later releases.

_WCHAR_T		Defined only for C++ programs to indicate that wchar_t is an intrinsically defined data type.
_WCHAR_T_DEFINED		Defined only for C++ programs to indicate that wchar_t is an intrinsically defined data type.
_Windows	 	Defined for Windows-only code.
_ _WIN32_ _	1	Defined for console and GUI applications.

Note:	_ _DATE_ _, _ _FILE_ _ , _ _ FUNC_ _, _ _LINE_ _, _ _STDC_ _, and _ _TIME_ _ cannot be redefined or undefined.
*/
