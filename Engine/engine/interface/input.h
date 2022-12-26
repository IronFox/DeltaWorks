#ifndef engine_interface_inputH
#define engine_interface_inputH

#include <string/keys.h>
//#include "../../io/config.h"
#include <container/vectorList.h>
#include <container/hashtable.h>
#include <global_string.h>
#include <string/string_converter.h>	//for string name lookup
#include <iostream>
#include <functional>

namespace Engine
{
	using namespace DeltaWorks;



	typedef std::function<void()>			FSimpleKeyHandler;
	typedef std::function<void(Key::Name)>	FKeyHandler;
		
	typedef void(*FRawSimpleKeyHandler)();
	typedef void(*FRawKeyHandler)(Key::Name);
		
	class KeyHandler
	{
	public:
		typedef void		(*const* FRawPointer)();
	private:
		FRawPointer			ptr = NULL;
		FKeyHandler			handler;
		typedef KeyHandler	Self;
	public:
		/**/				KeyHandler()	{}
		/**/				KeyHandler(const FKeyHandler&h):handler(h){ptr = h.target<void(*)()>();}
		/**/				KeyHandler(const FSimpleKeyHandler&h):handler( h ? [h](Key::Name){h();}:FKeyHandler()){ptr = h.target<void(*)()>();}
		explicit			KeyHandler(const FRawKeyHandler&h):handler(h){ptr = (FRawPointer)h;}
		explicit			KeyHandler(const FRawSimpleKeyHandler&h):handler( h ? [h](Key::Name){h();}:FKeyHandler()){ptr = (FRawPointer)h;}

		void				Clear()	{handler = FKeyHandler();ptr = NULL;}
		bool				IsSet() const {return !!handler;}
		bool				IsEmpty() const {return !handler;}
		bool				IsNotEmpty() const {return IsSet();}
		FRawPointer			GetRawPointer() const {return ptr;}
		
		KeyHandler&			operator=(const KeyHandler&other) {handler = other.handler; ptr = other.ptr; return *this;}
		KeyHandler&			operator=(const FKeyHandler&h) { return this->operator=(KeyHandler(h));}
		KeyHandler&			operator=(const FSimpleKeyHandler&h) { return this->operator=(KeyHandler(h));}
		KeyHandler&			operator=(const FRawKeyHandler&h) { return this->operator=(KeyHandler(h));}
		KeyHandler&			operator=(const FRawSimpleKeyHandler&h) { return this->operator=(KeyHandler(h));}
		
		bool				operator()(Key::Name k)	{if (handler) {handler(k); return true;} return false;}
		void				swap(Self&other)	{handler.swap(other.handler);}

		friend void			swap(Self&a, Self&b)	{a.swap(b);}
	};



	struct TKeyLink	//! Eve keyboard link container
	{
		KeyHandler			onPressed,
							onReleased,
							onPressedWithCtrl,
							onReleasedWithCtrl;

		void				Clear()
		{
			onPressed.Clear();
			onReleased.Clear();
			onPressedWithCtrl.Clear();
			onReleasedWithCtrl.Clear();
		}

		void				swap(TKeyLink&other)
		{
			onPressed.swap(other.onPressed);
			onReleased.swap(other.onReleased);
			onPressedWithCtrl.swap(other.onPressedWithCtrl);
			onReleasedWithCtrl.swap(other.onReleasedWithCtrl);
		}

		friend void			swap(TKeyLink&a, TKeyLink&b)
		{
			a.swap(b);
		}
	};

	struct TKeyPreparation	//! Eve keyboard preparation container
	{
		String		name;			//!< Name of the key event
		KeyHandler	onPressed,	//!< Key down event pointer
					onReleased;		//!< Key up event pointer
		bool		done;
		Key::Name	keyID;

		void swap(TKeyPreparation&other)
		{
			name.swap(other.name);
			onPressed.swap(other.onPressed);
			onReleased.swap(other.onReleased);
			std::swap(keyID,other.keyID);
		}
		friend void				swap(TKeyPreparation&a, TKeyPreparation&b)
		{
			a.swap(b);
		}
	};

	struct TAnalogSource	//! Analog resource containing
	{
		String		name;	//!< Source name
		float		*value,	//!< Pointer to the resource's current value
					min,	//!< Minimum value
					max;	//!< Maximum value
	};
	
	class InputMap;
	
	class InputProfile
	{
	public:
		static const unsigned	NumKeys = Key::Max+1;	//0x100 + 0x20 joystick buttons + 0x5 mouse buttons		
		bool		keyboard,
					device_buttons;
		//TKeyLink	key[NumKeys];
		Array<TKeyLink>	key;	

					InputProfile(bool keyboard=true, bool device_buttons=true);
		void		ImportFrom(InputMap*map);
		void		ExportTo(InputMap*map);
	};

	
	
	/**
		\brief Generalized input handling interface
	*/
	class InputMap
	{
	private:
		static const unsigned	MaxBindingLayers = 0x100;
		static const unsigned	NumKeys = InputProfile::NumKeys;	//0x100 + 0x20 joystick buttons + 0x5 mouse buttons		
			
		friend class InputProfile;

		//TKeyLink				key[NumKeys];
		Array<TKeyLink>			key;	
		InputProfile			default_profile,
								*active_profile;
			
		Ctr::PreAllocVector<TKeyPreparation,0,Swap>	preparation;

		Ctr::Vector<InputProfile*>	binding_stack;
		unsigned				//active_binding,
								stack_forward_depth;
			
		Ctr::StringTable<TAnalogSource>	analog_sources;
		TAnalogSource				empty_source;
		Ctr::PreAllocVector<TAnalogSource,0>		linear_analog_list;

	public:
		bool				verbose;			//!< Set true to print input changes and events to the console. False by default
	
		bool				pressed[NumKeys];	//!< Key pressed state. The array provides one state variable per key.
			
			
		bool				keyDown(unsigned key);
		bool				keyDown(Key::Name key)
							{
								return keyDown((unsigned)key);
							}
		bool				keyUp(unsigned key);
		bool				keyUp(Key::Name key)
							{
								return keyUp((unsigned)key);
							}
								
		void 				CascadeKeyPressed(Key::Name index);		//!< Forwards a key down event to the profile most recently pushed to the profile stack. The method returns if no such exists
		void 				CascadeKeyReleased(Key::Name index);		//!< Forwards a key up event to the profile most recently pushed to the profile stack. The method returns if no such exists
			
	public:
		typedef KeyHandler		Handler;



								InputMap();
		virtual					~InputMap();

		/**
		Binds the specified event handler to the specified key.
		The method unsets any potentially bound up-handler
		@param key Index of the key to bind.
		@param downHandler Callback handler to execute if the specified key has been pressed
		*/
		void					Bind(Key::Name key, const Handler& downHandler);						
		void					Bind(Key::Name key, const FKeyHandler& downHandler)	{Bind(key,Handler(downHandler));}
		void					Bind(Key::Name key, const FSimpleKeyHandler& downHandler)	{Bind(key,Handler(downHandler));}
		/**
		Binds the specified event handlers to the specified key.
		@param key Index of the key to bind.
		@param downHandler Pointer to a function to execute if the specified key has been pressed.
		@param upHandler Pointer to a function to execute if the specified key has been released.
		*/
		void					Bind(Key::Name key, const Handler& downHandler, const Handler& upHandler);
		void					Bind(Key::Name key, const FKeyHandler& downHandler, const FKeyHandler& upHandler)	{Bind(key,Handler(downHandler),Handler(upHandler));}
		void					Bind(Key::Name key, const FSimpleKeyHandler& downHandler, const FSimpleKeyHandler& upHandler)	{Bind(key,Handler(downHandler),Handler(upHandler));}
		/**
		Binds the specified event handler to the specified key (+ctrl).
		Regular key handling is not affected by this method.
		@param key Index of the key to bind.
		@param ctrlHandler Function to execute if the specified key has been pressed in combination with the ctrl key.
		*/
		void					BindCtrl(Key::Name key, const Handler& ctrlHandler, const Handler& upHandler=Handler());
		void					BindCtrl(Key::Name key, const FKeyHandler& ctrlHandler, const FKeyHandler& upHandler=FKeyHandler()) {BindCtrl(key,Handler(ctrlHandler),Handler(upHandler));}
		void					BindCtrl(Key::Name key, const FSimpleKeyHandler& ctrlHandler, const FSimpleKeyHandler& upHandler=FSimpleKeyHandler()) {BindCtrl(key,Handler(ctrlHandler),Handler(upHandler));}
		void					Unbind(Key::Name key);																//!< Removes all bound event handlers from the specified key. \param key Index of the key to unbind.
		/**
		Binds all keys to the specified handler, overwriting any currently bound handlers unless @a unboundOnly is set.
		@param unboundOnly If set, only keys are remapped where both down and up handlers are currently not set
		*/
		void					BindAll(const Handler&downHandler, bool unboundOnly);
		void					BindAll(const FKeyHandler&downHandler, bool unboundOnly)	{BindAll(Handler(downHandler),unboundOnly);}
		void					BindAll(const FSimpleKeyHandler&downHandler, bool unboundOnly)	{BindAll(Handler(downHandler),unboundOnly);}
		/**
		Binds all keys to the specified handlers, overwriting any currently bound handlers unless @a unboundOnly is set.
		@param unboundOnly If set, only keys are remapped where both down and up handlers are currently not set
		*/
		void					BindAll(const Handler&downHandler, const Handler&upHandler, bool unboundOnly);
		void					BindAll(const FKeyHandler&downHandler, const FKeyHandler&upHandler, bool unboundOnly)	{BindAll(Handler(downHandler),Handler(upHandler),unboundOnly);}
		void					BindAll(const FSimpleKeyHandler&downHandler, const FSimpleKeyHandler&upHandler, bool unboundOnly)	{BindAll(Handler(downHandler),Handler(upHandler),unboundOnly);}

		/**
		Sets the key to auto cascade incoming key pressed and released events to stacked profiles
		@param key Index of the key to cascade.
		*/
		void					Cascade(Key::Name key);
		/**
		Unbinds all key handlers.
		This also includes cascading handlers
		*/
		void					ClearAllBindings();
		/**
		Binds all keys so that their event is forwarded to the profile stack
		*/
		void					CascadeAllKeys();
		void					RegInit();											//!< Resets key registration
		/**
		Registers a key command for import. 
		@param name Name of the command
		@param downHandler Function to execute if the specified key has been pressed
		@param key Index of the key to default to if the command is not bound
		*/
		void					RegKey(const String&name, const Handler& down_handler, Key::Name key);
		/**
		Registers a key command for import
		@param name Name of the command
		@param downHandler Function to execute if the specified key has been pressed
		@param upHandler Function to execute if the specified key has been released
		@param key Index of the key to default to if the command is not bound.
		*/
		void					RegKey(const String&name, const Handler& down_handler, const Handler& up_handler, Key::Name key);
		/**
		Link a registered command to the specified key.
		The method returns if the specified command was not registered.
		After execution the specified key will be bound to registered function(s).
		@param name Name of the command to link
		@param key Index of the key to link to
		*/
		void					LinkKey(const String&name, Key::Name key);
		/**
		Links all non-linked commands to their respective default keys
		*/
		void					LinkOthers();
			
		/**
		Activates the specified profile.
		Any succeeding Bind*() or LinkKey() calls will affect the now active profile.
		@param profile Reference to the binding profile to use.
		*/
		void					ChooseProfile(InputProfile&profile);			
		void					BindProfile(InputProfile&profile);		//!< Identical to ChooseProfile()
		void					ActivateProfile(InputProfile&profile);	//!< Identical to ChooseProfile()
		InputProfile*			GetCurrentProfile();					//!< Retrieves the currently bound profile
		void					PushProfile();						//!< Pushes the currently active profile to the stack.
		void					PopProfile();						//!< Pops the most recently pushed profile from the stack, replacing the currently active one.
			
			
		void					RegAnalog(const String&name, float&resource, float min, float max);	//!< Registers a new analog resource by the given name \param name Name of the new resource \param resource Reference to the float variable containing the current resource status \param min Minimum value of this resource \param max Maximum value of this resource
		void					UnregAnalog(const String&name);
		TAnalogSource*			FindAnalog(const String&name);											//!< Attempts to locate an analog input source by the specified name. \return Reference to a matching analog source or NULL if no such match was found
		void					ReleasePressedKeys();
	};
	
	


	extern InputMap			input;


}






#endif
