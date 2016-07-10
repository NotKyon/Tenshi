#pragma once

/*

	!!! STOP !!!

	This library is only meant for use internally.
	
	If you're writing a GUI based application then use the SystemUI library
	instead.

	------------------------------------------------------------------------

	This interface operates directly on the native window handles. Internal
	data structures may be assigned to the native handles depending on the
	platform's implementation.

*/

#include "../Core/Types.hpp"
#include "../Core/String.hpp"
#include "../Input/Key.hpp"

namespace Ax { namespace Window {

	// Forward declarations
	struct SNativeWindow;

	// Handle to a window
	typedef SNativeWindow *			OSWindow;

	// Flags detailing how a window should be styled
	enum EStyleFlags
	{
		// Window will not have a close button
		kSF_NoCloseButton			= 1<<0,
		// Window will not have a minimize button
		kSF_NoMinimizeButton		= 1<<1,
		// Window will not have a maximize button
		kSF_NoMaximizeButton		= 1<<2,
		// Window will not be resizeable
		kSF_NoResize				= 1<<3,
		// Window will have a thin title bar (like tool windows)
 		kSF_ThinCaption				= 1<<4,
		// Window will not have any system-supplied parts (like caption, border, etc)
		kSF_NoOSParts				= 1<<5,
		// Window is top-most
		kSF_TopMost					= 1<<6,
		// Window supports translucency
		kSF_Translucent				= 1<<7
	};

	// Modifier key flags
	enum EModKeyFlags
	{
		kMF_LShift					= 1<<0,
		kMF_RShift					= 1<<1,
		kMF_LAlt					= 1<<2,
		kMF_RAlt					= 1<<3,
		kMF_LControl				= 1<<4,
		kMF_RControl				= 1<<5,
		kMF_Mouse1					= 1<<6,
		kMF_Mouse2					= 1<<7,
		kMF_Mouse3					= 1<<8,
		kMF_Mouse4					= 1<<9,
		kMF_Mouse5					= 1<<10
	};

	// Reply to an event
	enum class EReply
	{
		// The event was not handled
		NotHandled = 0,
		// The event has been handled
		Handled
	};

	// Hit test results
	enum class EHitTest
	{
		// The event was not handled
		NotHandled = 0,

		// Nothing was in this area
		Miss,

		// Client area (main content part of the window)
		ClientArea,
		// Caption area (where the title text and buttons go) -- allows window dragging
		TitleBar,
		// System menu (where the icon for the system menu is)
		SystemMenu,
		// Minimize button
		MinimizeButton,
		// Maximize/restore button
		MaximizeButton,
		// Close button
		CloseButton,
		// Top border of the window (for resize)
		TopSizer,
		// Top-right border of the window (for resize)
		TopRightSizer,
		// Right border of the window (for resize)
		RightSizer,
		// Bottom-right border of the window (for resize)
		BottomRightSizer,
		// Bottom border of the window (for resize)
		BottomSizer,
		// Bottom-left border of the window (for resize)
		BottomLeftSizer,
		// Left border of the window (for resize)
		LeftSizer,
		// Top-left border of the window (for resize)
		TopLeftSizer
	};

	// Just before a window is closed
	typedef EReply( *FnOnClose )( OSWindow );
	// Just before a window is minimized
	typedef EReply( *FnOnMinimize )( OSWindow );
	// Just before a window is maximized
	typedef EReply( *FnOnMaximize )( OSWindow );

	// Just after a window is "closed" (but not yet destroyed)
	typedef EReply( *FnOnClosed )( OSWindow );
	// Just after a window is minimized
	typedef EReply( *FnOnMinimized )( OSWindow );
	// Just after a window is maximized
	typedef EReply( *FnOnMaximized )( OSWindow );

	// When the application itself becomes active
	typedef EReply( *FnOnAppActivate )( OSWindow );
	// When the application itself is put in the background
	typedef EReply( *FnOnAppDeactivate )( OSWindow );

	// When a window becomes the "main window" of the app (front)
	typedef EReply( *FnOnAcceptMain )( OSWindow );
	// When a window is no longer the "main window" of the app
	typedef EReply( *FnOnResignMain )( OSWindow );

	// When a window gains keyboard focus
	typedef EReply( *FnOnAcceptKey )( OSWindow );
	// When a window loses keyboard focus
	typedef EReply( *FnOnResignKey )( OSWindow );

	// When a window becomes enabled
	typedef EReply( *FnOnEnabled )( OSWindow );
	// When a window becomes disabled
	typedef EReply( *FnOnDisabled )( OSWindow );

	// When a window becomes visible
	typedef EReply( *FnOnVisible )( OSWindow );
	// When a window becomes invisible
	typedef EReply( *FnOnInvisible )( OSWindow );

	// Just after (or during) a resize event
	typedef EReply( *FnOnSized )( OSWindow, uint32 ClientResX, uint32 ClientResY );
	// Just after (or during) a movement event
	typedef EReply( *FnOnMoved )( OSWindow, int32 FramePosX, int32 FramePosY );

	// When a window is being resized and its resolution needs to be checked - the parameters can be modified to adjust
	typedef EReply( *FnOnSizing )( OSWindow, int32 &ClientResX, int32 &ClientResY );
	// When a window is being moved and its location needs to be checked - the parameters can be modified to adjust
	typedef EReply( *FnOnMoving )( OSWindow, int32 &FrameLeft, int32 &FrameTop, int32 &FrameRight, int32 &FrameBottom );

	// Sent when a key is pressed (or automatically repeated)
	typedef EReply( *FnOnKeyPress )( OSWindow, Input::EKey, uint32 uModFlags, bool bIsRepeat );
	// Sent when a key is released
	typedef EReply( *FnOnKeyRelease )( OSWindow, Input::EKey, uint32 uModFlags );
	// Sent automatically to designate a given character from a combination of key presses
	typedef EReply( *FnOnKeyChar )( OSWindow, uint32 UTF32Char );

	// Sent when a mouse button is pressed (0 = left, 1 = right, 2 = middle, 3 = thumb 1, 4 = thumb 2)
	typedef EReply( *FnOnMousePress )( OSWindow, uint32 uMouseButton, int32 ClientPosX, int32 ClientPosY, uint32 uModFlags );
	// Sent when a mouse button is released
	typedef EReply( *FnOnMouseRelease )( OSWindow, uint32 uMouseButton, int32 ClientPosX, int32 ClientPosY, uint32 uModFlags );
	// Sent when the mouse wheel is scrolled
	typedef EReply( *FnOnMouseWheel )( OSWindow, float fDelta, int32 ClientPosX, int32 ClientPosY, uint32 uModFlags );
	// Sent when the mouse moves within the window
	typedef EReply( *FnOnMouseMove )( OSWindow, int32 ClientPosX, int32 ClientPosY, uint32 uModFlags );
	// Sent when a mouse leaves a window
	typedef EReply( *FnOnMouseExit )( OSWindow, uint32 uModFlags );

	// Sent to determine where the mouse is within a window
	typedef EHitTest( *FnOnHitTest )( OSWindow, int32 MouseScreenPosX, int32 MouseScreenPosY, int32 MouseFramePosX, int32 MouseFramePosY );
	// Sent to a window to notify it that the title is changed
	typedef EReply( *FnOnChangeTitle )( OSWindow, const char *pszTitleUTF8 );

	// Event handling chain
	struct SDelegate
	{
		// Next delegate to try if the event is unhandled
		SDelegate *					pSuper;

		// Just before a window is closed
		FnOnClose					pfnOnClose;
		// Just before a window is minimized
		FnOnMinimize				pfnOnMinimize;
		// Just before a window is maximized
		FnOnMaximize				pfnOnMaximize;

		// Just after a window is "closed" (but not yet destroyed)
		FnOnClosed					pfnOnClosed;
		// Just after a window is minimized
		FnOnMinimized				pfnOnMinimized;
		// Just after a window is maximized
		FnOnMaximized				pfnOnMaximized;

		// When the application itself becomes active
		FnOnAppActivate				pfnOnAppActivate;
		// When the application itself is put in the background
		FnOnAppDeactivate			pfnOnAppDeactivate;

		// When a window becomes the "main window" of the app (front)
		FnOnAcceptMain				pfnOnAcceptMain;
		// When a window is no longer the "main window" of the app
		FnOnResignMain				pfnOnResignMain;

		// When a window gains keyboard focus
		FnOnAcceptKey				pfnOnAcceptKey;
		// When a window loses keyboard focus
		FnOnResignKey				pfnOnResignKey;

		// When a window becomes enabled
		FnOnEnabled					pfnOnEnabled;
		// When a window becomes disabled
		FnOnDisabled				pfnOnDisabled;

		// When a window becomes visible
		FnOnVisible					pfnOnVisible;
		// When a window becomes invisible
		FnOnInvisible				pfnOnInvisible;

		// Just after (or during) a resize event
		FnOnSized					pfnOnSized;
		// Just after (or during) a movement event
		FnOnMoved					pfnOnMoved;

		// When a window is being resized and its resolution needs to be checked - the parameters can be modified to adjust
		FnOnSizing					pfnOnSizing;
		// When a window is being moved and its location needs to be checked - the parameters can be modified to adjust
		FnOnMoving					pfnOnMoving;

		// Sent when a key is pressed (or automatically repeated)
		FnOnKeyPress				pfnOnKeyPress;
		// Sent when a key is released
		FnOnKeyRelease				pfnOnKeyRelease;
		// Sent automatically to designate a given character from a combination of key presses
		FnOnKeyChar					pfnOnKeyChar;

		// Sent when a mouse button is pressed (0 = left, 1 = right, 2 = middle, 3 = thumb 1, 4 = thumb 2)
		FnOnMousePress				pfnOnMousePress;
		// Sent when a mouse button is released
		FnOnMouseRelease			pfnOnMouseRelease;
		// Sent when the mouse wheel is scrolled
		FnOnMouseWheel				pfnOnMouseWheel;
		// Sent when the mouse moves within the window
		FnOnMouseMove				pfnOnMouseMove;
		// Sent when a mouse leaves a window
		FnOnMouseExit				pfnOnMouseExit;

		// Sent to determine where the mouse is within a window
		FnOnHitTest					pfnOnHitTest;
		// Sent to a window to notify it that the title is changed
		FnOnChangeTitle				pfnOnChangeTitle;

		inline SDelegate()
		: pSuper					( NULL )
		, pfnOnClose				( NULL )
		, pfnOnMinimize				( NULL )
		, pfnOnMaximize				( NULL )
		, pfnOnClosed				( NULL )
		, pfnOnMinimized			( NULL )
		, pfnOnMaximized			( NULL )
		, pfnOnAppActivate			( NULL )
		, pfnOnAppDeactivate		( NULL )
		, pfnOnAcceptMain			( NULL )
		, pfnOnResignMain			( NULL )
		, pfnOnAcceptKey			( NULL )
		, pfnOnResignKey			( NULL )
		, pfnOnEnabled				( NULL )
		, pfnOnDisabled				( NULL )
		, pfnOnVisible				( NULL )
		, pfnOnInvisible			( NULL )
		, pfnOnSized				( NULL )
		, pfnOnMoved				( NULL )
		, pfnOnSizing				( NULL )
		, pfnOnMoving				( NULL )
		, pfnOnKeyPress				( NULL )
		, pfnOnKeyRelease			( NULL )
		, pfnOnKeyChar				( NULL )
		, pfnOnMousePress			( NULL )
		, pfnOnMouseRelease			( NULL )
		, pfnOnMouseWheel			( NULL )
		, pfnOnMouseMove			( NULL )
		, pfnOnMouseExit			( NULL )
		, pfnOnHitTest				( NULL )
		, pfnOnChangeTitle			( NULL )
		{
		}
	};

	// Window creation parameters
	struct SCreateInfo
	{
		// UTF-8 encoded title string
		const char *				pszTitle;
		// x-position of the window
		int32						PosX;
		// y-position of the window
		int32						PosY;
		// x-resolution of the window's client area (width)
		uint32						ResX;
		// y-resolution of the window's client area (height)
		uint32						ResY;
		// Style flags for the window
		uint32						uStyleFlags;
		// Delegate to handle events for the window
		SDelegate *					pDelegate;
		// Extra data pointer for the window
		void *						pData;

		inline SCreateInfo()
		: pszTitle( NULL )
		, PosX( 0 )
		, PosY( 0 )
		, ResX( 0 )
		, ResY( 0 )
		, uStyleFlags( 0 )
		, pDelegate( NULL )
		, pData( NULL )
		{
		}
	};

	// Create a window
	OSWindow Open( const SCreateInfo &Info );
	// Destroy a window
	void Close( OSWindow Window );
	// Minimize a window
	void Minimize( OSWindow Window );
	// Maximize a window
	void Maximize( OSWindow Window );

	// Add a delegate to the window's chain
	void AddDelegate( OSWindow Window, const SDelegate &Delegate );
	// Remove a delegate from the window's chain
	void RemoveDelegate( OSWindow Window, const SDelegate &Delegate );

	// Set the title of a window
	void SetTitle( OSWindow Window, const char *pszTitleUTF8 );
	// Retrieve the title of a window
	uintptr GetTitle( OSWindow Window, char *pszOutTitleUTF8, uintptr cMaxOutBytes );

	// Set the data of a window
	void SetData( OSWindow Window, void *pData );
	// Retrieve the data of a window
	void *GetData( OSWindow Window );

	// Send the close message to a window
	void PerformClose( OSWindow Window );
	// Send the minimize message to a window
	void PerformMinimize( OSWindow Window );
	// Send the maximize message to a window
	void PerformMaximize( OSWindow Window );

	// Set a window to be visible or invisible
	void SetVisible( OSWindow Window, bool bIsVisible = true );
	// Determine whether a window is visible
	bool IsVisible( OSWindow Window );

	// Set a window to be enabled or disabled
	void SetEnabled( OSWindow Window, bool bIsEnabled = true );
	// Determine whether a window is enabled
	bool IsEnabled( OSWindow Window );

	// Resize a window's client area
	void Resize( OSWindow Window, uint32 ResX, uint32 ResY );
	// Resize a window's screen area
	void ResizeFrame( OSWindow Window, uint32 ResX, uint32 ResY );

	// Position a window
	void Position( OSWindow Window, int32 PosX, int32 PosY );

	// Retrieve the size of a window's client area
	void GetSize( OSWindow Window, uint32 &OutResX, uint32 &OutResY );
	// Retrieve the size of a window's screen area
	void GetFrameSize( OSWindow Window, uint32 &OutResX, uint32 &OutResY );

	// Retrieve the position of a window
	void GetPosition( OSWindow Window, int32 &OutPosX, int32 &OutPosY );

	// Set the window to display again
	void SetNeedsDisplay( OSWindow Window );
	// Set a portion of the window to display again
	void SetRectNeedsDisplay( OSWindow Window, int32 ClientLeft, int32 ClientTop, int32 ClientRight, int32 ClientBottom );

	//------------------------------------------------------------------------//

	// Easier "get title"
	inline String GetTitle( OSWindow Window )
	{
		String Temp;

		if( !Window || !Temp.Reserve( GetTitle( Window, NULL, 0 ) ) ) {
			return String();
		}

		GetTitle( Window, Temp, Temp.NumAllocated() );
		Temp.CheckLength();

		return Temp;
	}
	// Easier "get title"
	template< uintptr tBufferSize >
	inline char *GetTitle( OSWindow Window, char( &OutBuffer )[ tBufferSize ] )
	{
		GetTitle( Window, OutBuffer, tBufferSize );
		return &OutBuffer[ 0 ];
	}

	// Templated "set data" - pointer
	template< typename tObject >
	inline void SetData( OSWindow Window, tObject *pData )
	{
		SetData( Window, ( void * )pData );
	}
	// Templated "set data" - reference
	template< typename tObject >
	inline void SetData( OSWindow Window, tObject &Data )
	{
		SetData( Window, ( void * )&Data );
	}

	// Templated "get data" - return pointer
	template< typename tObject >
	inline tObject *GetData( OSWindow Window )
	{
		return ( tObject * )GetData( Window );
	}
	// Templated "get data" - parameter pointer
	//
	// return: true if the data pointer isn't NULL; false if it is NULL
	template< typename tObject >
	inline bool GetData( OSWindow Window, tObject *&pOutData )
	{
		pOutData = ( tObject * )GetData( Window );
		return pOutData != NULL;
	}

}}
