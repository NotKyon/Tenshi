#include "Window.hpp"
#include "../Allocation/Allocator.hpp"
#include "../Async/Mutex.hpp"
#include "../Core/Assert.hpp"
#include "../Core/Manager.hpp"

#if AX_OS(Windows)

# ifndef AX_WINDOWS_ICON_RESOURCE
#  define AX_WINDOWS_ICON_RESOURCE		1
# endif

# ifndef AX_WINDOWS_CURSOR_RESOURCE
#  define AX_WINDOWS_CURSOR_RESOURCE	2
# endif

#endif

namespace Ax { namespace Window {

	struct SInstance
	{
#if AX_OS(Windows)
		ATOM						WindowClass			= NULL;
#endif

		static SInstance &GetInstance()
		{
			static SInstance instance;
			return instance;
		}

	private:
		SInstance();
		~SInstance();
	};
	static TManager< SInstance >	WM;

	struct SWindow
	{
		SDelegate *					pTopDelegate		= NULL;
		void *						pUserData			= NULL;
		String						TitleText;
	};

	static SWindow &W( OSWindow Window )
	{
		static SWindow Dummy;

#if AX_OS(Windows)
		SWindow *p = ( SWindow * )GetWindowLongPtrW( ( HWND )Window, 0 );
		if( p != NULL ) {
			return *p;
		}
#endif

		return Dummy;
	}

#define DELEGATE_IMPL__(CallParms_)\
	if( !pDelegate || !ppfnFunc ) {\
		return false;\
	}\
	\
	const uintptr ByteOffset = uintptr( ppfnFunc ) - uintptr( pDelegate );\
	\
	for(;;) {\
		if( *ppfnFunc != NULL && !int( ( *ppfnFunc ) CallParms_ ) ) {\
			return true;\
		}\
		\
		pDelegate = pDelegate->pSuper;\
		if( !pDelegate ) {\
			break;\
		}\
		\
		ppfnFunc = ( tCallback * )( uintptr( pDelegate ) + ByteOffset );\
	}\
	\
	return false

	template< typename tCallback >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window )
	{
		DELEGATE_IMPL__((Window));
	}
	template< typename tCallback, typename tParm1 >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window, tParm1 Parm1 )
	{
		DELEGATE_IMPL__((Window, Parm1));
	}
	template< typename tCallback, typename tParm1, typename tParm2 >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window, tParm1 Parm1, tParm2 Parm2 )
	{
		DELEGATE_IMPL__((Window, Parm1, Parm2));
	}
	template< typename tCallback, typename tParm1, typename tParm2, typename tParm3 >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window, tParm1 Parm1, tParm2 Parm2, tParm3 Parm3 )
	{
		DELEGATE_IMPL__((Window, Parm1, Parm2, Parm3));
	}
	template< typename tCallback, typename tParm1, typename tParm2, typename tParm3, typename tParm4 >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window, tParm1 Parm1, tParm2 Parm2, tParm3 Parm3, tParm4 Parm4 )
	{
		DELEGATE_IMPL__((Window, Parm1, Parm2, Parm3, Parm4));
	}
	template< typename tCallback, typename tParm1, typename tParm2, typename tParm3, typename tParm4, typename tParm5 >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window, tParm1 Parm1, tParm2 Parm2, tParm3 Parm3, tParm4 Parm4, tParm5 Parm5 )
	{
		DELEGATE_IMPL__((Window, Parm1, Parm2, Parm3, Parm4, Parm5));
	}
	template< typename tCallback, typename tParm1, typename tParm2, typename tParm3, typename tParm4, typename tParm5, typename tParm6 >
	bool CallDelegate( SDelegate *pDelegate, tCallback *ppfnFunc, OSWindow Window, tParm1 Parm1, tParm2 Parm2, tParm3 Parm3, tParm4 Parm4, tParm5 Parm5, tParm6 Parm6 )
	{
		DELEGATE_IMPL__((Window, Parm1, Parm2, Parm3, Parm4, Parm5, Parm6));
	}

#if AX_OS(Windows)
	namespace Windows
	{

		static uint32 GetMods()
		{
			uint32 Mods = 0;
			
			Mods |= ( GetKeyState( VK_LSHIFT   ) != 0 ) ? kMF_LShift   : 0;
			Mods |= ( GetKeyState( VK_RSHIFT   ) != 0 ) ? kMF_RShift   : 0;
			Mods |= ( GetKeyState( VK_LMENU    ) != 0 ) ? kMF_LAlt     : 0;
			Mods |= ( GetKeyState( VK_RMENU    ) != 0 ) ? kMF_RAlt     : 0;
			Mods |= ( GetKeyState( VK_LCONTROL ) != 0 ) ? kMF_LControl : 0;
			Mods |= ( GetKeyState( VK_RCONTROL ) != 0 ) ? kMF_RControl : 0;
			Mods |= ( GetKeyState( VK_LBUTTON  ) != 0 ) ? kMF_Mouse1   : 0;
			Mods |= ( GetKeyState( VK_RBUTTON  ) != 0 ) ? kMF_Mouse2   : 0;
			Mods |= ( GetKeyState( VK_MBUTTON  ) != 0 ) ? kMF_Mouse3   : 0;
			Mods |= ( GetKeyState( VK_XBUTTON1 ) != 0 ) ? kMF_Mouse4   : 0;
			Mods |= ( GetKeyState( VK_XBUTTON2 ) != 0 ) ? kMF_Mouse5   : 0;

			return Mods;
		}
		static uint32 GetScancode( LPARAM lParm )
		{
			return ( ( lParm & 0x00FF0000 )>>16 ) + ( ( lParm & 0x01000000 )>>11 );
		}
		static bool IsRepeat( LPARAM lParm )
		{
			return ( lParm & ( 1<<30 ) ) != 0;
		}
		static uint32 ConvertUTF16ToUTF32( const uint16( &src )[ 2 ] )
		{
			// check if the value is encoded as one UTF16 word
			if( *src < 0xD800 || *src > 0xDFFF ) {
				return uint32( *src );
			}
		
			// check for an error (0xD800..0xDBFF)
			if( *src > 0xDBFF ) {
				// this is an invalid character; replace with U+FFFD
				return 0xFFFD;
			}

			// if no character follows, or is out of range, then this is an invalid encoding
			if( *( src + 1 ) < 0xDC00 || *( src + 1 ) > 0xDFFF ) {
				// replace with U+FFFD
				return 0xFFFD;
			}

			// encode
			return 0x10000 + ( ( ( ( *src ) & 0x3FF ) << 10 ) | ( ( *( src + 1 ) ) & 0x3FF ) );
		}

		static LRESULT ConvertHitTest( EHitTest In )
		{
			switch( In )
			{
			case EHitTest::NotHandled:
			case EHitTest::Miss:
				return HTNOWHERE;

			case EHitTest::ClientArea:
				return HTCLIENT;
			case EHitTest::TitleBar:
				return HTCAPTION;
			case EHitTest::SystemMenu:
				return HTSYSMENU;

			case EHitTest::MinimizeButton:
				return HTMINBUTTON;
			case EHitTest::MaximizeButton:
				return HTMAXBUTTON;
			case EHitTest::CloseButton:
				return HTCLOSE;

			case EHitTest::TopSizer:
				return HTTOP;
			case EHitTest::TopRightSizer:
				return HTTOPRIGHT;
			case EHitTest::RightSizer:
				return HTRIGHT;
			case EHitTest::BottomRightSizer:
				return HTBOTTOMRIGHT;
			case EHitTest::BottomSizer:
				return HTBOTTOM;
			case EHitTest::BottomLeftSizer:
				return HTBOTTOMLEFT;
			case EHitTest::LeftSizer:
				return HTLEFT;
			case EHitTest::TopLeftSizer:
				return HTTOPLEFT;

			default:
				AX_ASSERT_MSG( false, "Invalid input" );
				break;
			}

			return HTNOWHERE;
		}

		LRESULT CALLBACK MsgProc_f( HWND hWindow, UINT uMessage, WPARAM wParm, LPARAM lParm )
		{
#define DELEGATE__(Name_) Window.pTopDelegate, &Window.pTopDelegate->pfn##Name_, pWindow

			if( uMessage == WM_NCCREATE ) {
				SetWindowLongPtrW( hWindow, 0, ( LONG_PTR )( ( ( CREATESTRUCTW * )lParm )->lpCreateParams ) );
				return DefWindowProcW( hWindow, uMessage, wParm, lParm );
			}

			OSWindow pWindow = OSWindow( hWindow );
			SWindow &Window = W( pWindow);

			switch( uMessage )
			{
				// close
				case WM_CLOSE:
					if( !CallDelegate( DELEGATE__(OnClose) ) ) {
						DestroyWindow( hWindow );
					}

					return 0;
				// closed
				case WM_DESTROY:
					CallDelegate( DELEGATE__(OnClosed) );
					SetWindowLongPtrW( hWindow, 0, 0 );
					Destroy( Window );
					return 0;

				// minimize/maximize
				case WM_SYSCOMMAND:
					if( ( wParm & 0xFFF0 ) == SC_MINIMIZE ) {
						CallDelegate( DELEGATE__(OnMinimize) );
					} else if( ( wParm & 0xFFF0 ) == SC_MAXIMIZE ) {
						CallDelegate( DELEGATE__(OnMaximize) );
					}

					break;

# if 0 // is sending a notification on style change unnecessary? only results from SetWindowLong, might cause WM_SYSCOMMAND anyway
				case WM_STYLECHANGING:
					if( wParm == GWL_STYLE ) {
						const STYLESTRUCT *const pStyles = ( const STYLESTRUCT * )lParm;
						if( !pStyles ) {
							break;
						}

						// The flags that have changed between the new and old styles
						const DWORD dwDifference = pStyles->styleNew ^ pStyles->styleOld;
						// The flags that were added in the new style that weren't in the old style
						const DWORD dwAdded = dwDifference & pStyles->styleNew;
	
						// If we now have a minimize flag then we are about to be minimized
						if( dwAdded & WS_MINIMIZE ) {
							CallDelegate( DELEGATE__(OnMinimize) );
							return 0;
						}
						
						// If we now have a maximize flag then we are about to be maximized
						if( dwAdded & WS_MAXIMIZE ) {
							CallDelegate( DELEGATE__(OnMaximize) );
							return 0;
						}
					}

					break;
# endif

				// app activate/deactivate
				case WM_ACTIVATEAPP:
					if( wParm == TRUE ) {
						CallDelegate( DELEGATE__(OnAppActivate) );
					} else {
						CallDelegate( DELEGATE__(OnAppDeactivate) );
					}
					return 0;

				// accept/resign main
				case WM_ACTIVATE:
					if( wParm != WA_INACTIVE ) {
						CallDelegate( DELEGATE__(OnAcceptMain) );
					} else {
						CallDelegate( DELEGATE__(OnResignMain) );
					}
					return 0;

				// accept key
				case WM_SETFOCUS:
					CallDelegate( DELEGATE__(OnAcceptKey) );
					return 0;
				// resign key
				case WM_KILLFOCUS:
					CallDelegate( DELEGATE__(OnResignKey) );
					return 0;

				// enable/disable
				case WM_ENABLE:
					if( wParm == TRUE ) {
						CallDelegate( DELEGATE__(OnEnabled) );
					} else {
						CallDelegate( DELEGATE__(OnDisabled) );
					}
					return 0;

				// visible/invisible
				case WM_SHOWWINDOW:
					if( wParm == TRUE ) {
						CallDelegate( DELEGATE__(OnVisible) );
					} else {
						CallDelegate( DELEGATE__(OnInvisible) );
					}
					return 0;

				// move
				case WM_MOVE:
					{
						RECT Screen;

						if( !GetWindowRect( hWindow, &Screen ) ) {
							return 0;
						}

						CallDelegate( DELEGATE__(OnMoved), Screen.left, Screen.top );
					}
					return 0;
				// size
				case WM_SIZE:
					{
						const uint32 uResX = uint32( LOWORD( lParm ) );
						const uint32 uResY = uint32( HIWORD( lParm ) );

						switch( wParm ) {
						case SIZE_RESTORED:
							CallDelegate( DELEGATE__(OnSized), uResX, uResY );
							break;

						case SIZE_MINIMIZED:
							CallDelegate( DELEGATE__(OnMinimized) );
							break;

						case SIZE_MAXIMIZED:
							CallDelegate( DELEGATE__(OnSized), uResX, uResY );
							CallDelegate( DELEGATE__(OnMaximized) );
							break;

						default:
							break;
						}
					}
					return 0;

				// moving
				case WM_MOVING:
					{
						RECT *const pArea = ( RECT * )lParm;
						if( !pArea ) {
							return FALSE;
						}

						int32 FrameLeft = pArea->left;
						int32 FrameTop = pArea->top;
						int32 FrameRight = pArea->right;
						int32 FrameBottom = pArea->bottom;

						if( !CallDelegate( DELEGATE__(OnMoving), FrameLeft, FrameTop, FrameRight, FrameBottom ) ) {
							break;
						}

						pArea->left = FrameLeft;
						pArea->top = FrameTop;
						pArea->right = FrameRight;
						pArea->bottom = FrameBottom;
					}
					return TRUE;
				// sizing
				case WM_SIZING:
					{
						RECT *const pArea = ( RECT * )lParm;
						if( !pArea ) {
							return FALSE;
						}

						int32 ResX = pArea->right - pArea->left;
						int32 ResY = pArea->bottom - pArea->top;

						if( !CallDelegate( DELEGATE__(OnSizing), ResX, ResY ) ) {
							break;
						}

						// handle x-resolution adjustment
						switch( wParm ) {
						case WMSZ_LEFT:
						case WMSZ_TOPLEFT:
						case WMSZ_BOTTOMLEFT:
							pArea->left = pArea->right - ResX;
							break;

						case WMSZ_RIGHT:
						case WMSZ_TOPRIGHT:
						case WMSZ_BOTTOMRIGHT:
							pArea->right = pArea->left + ResX;
							break;

						default:
							break;
						}

						// handle y-resolution adjustment
						switch( wParm ) {
						case WMSZ_TOP:
						case WMSZ_TOPLEFT:
						case WMSZ_TOPRIGHT:
							pArea->top = pArea->bottom - ResY;
							break;

						case WMSZ_BOTTOM:
						case WMSZ_BOTTOMLEFT:
						case WMSZ_BOTTOMRIGHT:
							pArea->bottom = pArea->top + ResY;
							break;

						default:
							break;
						}
					}
					return TRUE;
					
				// mouse press/release and movement
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_XBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				case WM_MBUTTONUP:
				case WM_XBUTTONUP:
				case WM_MOUSEMOVE:
					{
						const uint32 Mods = GetMods();
						const int32 ClientPosX = int32( short( LOWORD( lParm ) ) );
						const int32 ClientPosY = int32( short( HIWORD( lParm ) ) );

						if( uMessage == WM_MOUSEMOVE ) {
							TRACKMOUSEEVENT Tracker;

							Tracker.cbSize = sizeof( Tracker );
							Tracker.dwFlags = TME_LEAVE;
							Tracker.hwndTrack = hWindow;
							Tracker.dwHoverTime = 0;

							TrackMouseEvent( &Tracker );

							CallDelegate( DELEGATE__(OnMouseMove), ClientPosX, ClientPosY, Mods );
						} else {
							uint32 uButtonId = 0;

							if( uMessage == WM_RBUTTONDOWN || uMessage == WM_RBUTTONUP ) {
								uButtonId = 1;
							} else if( uMessage == WM_MBUTTONDOWN || uMessage == WM_MBUTTONUP ) {
								uButtonId = 2;
							} else if( uMessage == WM_XBUTTONDOWN || uMessage == WM_XBUTTONUP ) {
								uButtonId = 2 + uint32( HIWORD( wParm ) );
							}

							if( uMessage ==  WM_LBUTTONDOWN || uMessage == WM_RBUTTONDOWN || uMessage == WM_MBUTTONDOWN || uMessage == WM_XBUTTONDOWN ) {
								CallDelegate( DELEGATE__(OnMousePress), uButtonId, ClientPosX, ClientPosY, Mods );
							} else {
								CallDelegate( DELEGATE__(OnMouseRelease), uButtonId, ClientPosX, ClientPosY, Mods );
							}
						}
					}
					return 0;

				// mouse wheel
				case WM_MOUSEWHEEL:
					{
						const uint32 Mods = GetMods();
						const int32 ClientPosX = int32( short( LOWORD( lParm ) ) );
						const int32 ClientPosY = int32( short( HIWORD( lParm ) ) );
						const float fDelta = float( short( HIWORD( wParm ) )/WHEEL_DELTA );

						CallDelegate( DELEGATE__(OnMouseWheel), fDelta, ClientPosX, ClientPosY, Mods );
					}
					return 0;

				// mouse exit
				case WM_MOUSELEAVE:
					CallDelegate( DELEGATE__(OnMouseExit), GetMods() );
					return 0;

				// key press
				case WM_KEYDOWN:
					{
						const uint32 Mods = GetMods();
						const uint32 uScancode = GetScancode( lParm );
						const Input::EKey Key = Input::EKey( uScancode );
						const bool bIsRepeat = IsRepeat( lParm );

						CallDelegate( DELEGATE__(OnKeyPress), Key, Mods, bIsRepeat );
					}
					return 0;

				// key release
				case WM_KEYUP:
					{
						const uint32 Mods = GetMods();
						const uint32 uScancode = GetScancode( lParm );
						const Input::EKey Key = Input::EKey( uScancode );

						CallDelegate( DELEGATE__(OnKeyRelease), Key, Mods );
					}
					return 0;

				// key char
				case WM_CHAR:
					{
						const uint16 UTF16Char[ 2 ] = {
							uint16( ( wParm >>  0 ) & 0xFFFF ),
							uint16( ( wParm >> 16 ) & 0xFFFF )
						};

						const uint32 UTF32Char = ConvertUTF16ToUTF32( UTF16Char );

						CallDelegate( DELEGATE__(OnKeyChar), UTF32Char );
					}
					return 0;
				case WM_UNICHAR:
					{
						if( wParm == UNICODE_NOCHAR ) {
							return TRUE;
						}

						const uint32 UTF32Char = uint32( wParm );

						CallDelegate( DELEGATE__(OnKeyChar), UTF32Char );
					}
					return FALSE;

				// hit test
				case WM_NCHITTEST:
					{
						const int32 MouseScreenPosX = int32( short( LOWORD( lParm ) ) );
						const int32 MouseScreenPosY = int32( short( HIWORD( lParm ) ) );

						RECT Area;
						if( !GetWindowRect( hWindow, &Area ) ) {
							Area.left = MouseScreenPosX;
							Area.top = MouseScreenPosY;
							Area.right = 0;
							Area.bottom = 0;
						}

						const int32 MouseFramePosX = MouseScreenPosX - Area.left;
						const int32 MouseFramePosY = MouseScreenPosY - Area.top;

						EHitTest Result = EHitTest::NotHandled;
						SDelegate *pTest = Window.pTopDelegate;
						while( pTest != nullptr ) {
							SDelegate *const pCheck = pTest;
							pTest = pTest->pSuper;

							if( !pCheck->pfnOnHitTest ) {
								continue;
							}

							Result = pCheck->pfnOnHitTest( pWindow, MouseScreenPosX, MouseScreenPosY, MouseFramePosX, MouseFramePosY );
							if( Result == EHitTest::NotHandled ) {
								continue;
							}

							break;
						}

						if( Result != EHitTest::NotHandled ) {
							return ConvertHitTest( Result );
						}
					}
					break;

				// change title
				case WM_SETTEXT:
					{
						if( !AX_VERIFY( Window.TitleText.AssignUTF16( ( const wchar_t * )lParm ) ) ) {
							return FALSE;
						}

						bool bHasChangeText = false;
						SDelegate *pTest = Window.pTopDelegate;
						while( pTest != nullptr ) {
							SDelegate *const pCheck = pTest;
							pTest = pTest->pSuper;

							if( !pCheck->pfnOnChangeTitle ) {
								continue;
							}

							bHasChangeText = true;
							break;
						}

						if( !bHasChangeText ) {
							//break;
							return TRUE;
						}

						if( !CallDelegate( DELEGATE__(OnChangeTitle), Window.TitleText ) ) {
							//break;
							return TRUE;
						}
					}
					return TRUE;
				// retrieve the title
				case WM_GETTEXT:
					{
						Window.TitleText.ConvertUTF16( ( Ax::uint16 * )lParm, uintptr( wParm ) );
						return wcslen( ( const wchar_t * )lParm );
					}

				// erase background
				case WM_ERASEBKGND:
					return TRUE;

				// paint
				case WM_PAINT:
					{
						const bool bSysDevCtx = wParm != NULL;
						const HDC hDevCtx = bSysDevCtx ? HDC( wParm ) : GetDC( hWindow );

						RECT Area;
						GetClientRect( hWindow, &Area );

						static const COLORREF NiceBlueGray1	= RGB( 104, 128, 154 );
						static const COLORREF NiceBlueGray2	= RGB(  96, 128, 160 );
						static const COLORREF LighterBlue	= RGB(   0, 144, 208 );
						static const COLORREF Forest1		= RGB( 107, 189, 147 );
						static const COLORREF Forest2		= RGB( 112, 192, 144 );
						static const COLORREF Sakura1		= RGB( 155, 146, 185 );
						static const COLORREF Sakura2		= RGB( 160, 144, 192 );
						static const COLORREF DarkForest1	= RGB(  40,  70,  43 );
						static const COLORREF DarkForest2	= RGB(  32,  64,  32 );

						const COLORREF RandomColor = RGB( rand()%256, rand()%256, rand()%256 );

						SetBkColor( hDevCtx, NiceBlueGray1 );
						ExtTextOutW( hDevCtx, 0, 0, ETO_OPAQUE, &Area, L"", 0, nullptr );

						if( !bSysDevCtx ) {
							ReleaseDC( hWindow, hDevCtx );
						}
					}
			}

			return DefWindowProcW( hWindow, uMessage, wParm, lParm );

#undef DELEGATE__
		}

	}
#endif

	SInstance::SInstance()
	{
#if AX_OS(Windows)
		WNDCLASSEXW wc;

		wc.cbSize = sizeof( wc );
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = &Windows::MsgProc_f;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof( void * );
		wc.hInstance = GetModuleHandleW( NULL );
		wc.hIcon = ( HICON )LoadImageW( wc.hInstance, MAKEINTRESOURCEW( AX_WINDOWS_ICON_RESOURCE ), IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION );
		wc.hCursor = ( HCURSOR )LoadImageW( wc.hInstance, MAKEINTRESOURCEW( AX_WINDOWS_CURSOR_RESOURCE ), IMAGE_CURSOR, 0, 0, LR_CREATEDIBSECTION );
		wc.hbrBackground = GetSysColorBrush( COLOR_BTNFACE );
		wc.lpszMenuName = NULL;
		wc.lpszClassName = L"FloatingWindow";
		wc.hIconSm = ( HICON )LoadImageW( wc.hInstance, MAKEINTRESOURCEW( AX_WINDOWS_ICON_RESOURCE ), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION );

		WindowClass = RegisterClassExW( &wc );
		AX_EXPECT_NOT_NULL( WindowClass );
#endif
	}
	SInstance::~SInstance()
	{
#if AX_OS(Windows)
		UnregisterClassW( ( LPCWSTR )WindowClass, GetModuleHandleW( NULL ) );
#endif
	}

	static SDelegate *DuplicateDelegate_r( const SDelegate &Source, SDelegate *pTopDelegate = NULL )
	{
		SDelegate *pNewDelegate = Alloc< SDelegate >();
		AX_EXPECT_NOT_NULL( pNewDelegate );

		memcpy( ( void * )pNewDelegate, ( const void * )&Source, sizeof( SDelegate ) );
		pNewDelegate->pSuper = Source.pSuper != NULL ? DuplicateDelegate_r( *Source.pSuper, pTopDelegate ) : pTopDelegate;

		return pNewDelegate;
	}
	static bool CompareDelegates( const SDelegate &First, const SDelegate &Second )
	{
		const uintptr *const pFirst = ( const uintptr * )&First;
		const uintptr *const pSecond = ( const uintptr * )&Second;

		static const uintptr uStart = 1;
		static const uintptr uEnd = sizeof( SDelegate )/sizeof( void * );

		for( uintptr i = uStart; i < uEnd; ++i ) {
			if( pFirst[ i ] != pSecond[ i ] ) {
				return false;
			}
		}

		return true;
	}

#if AX_OS(Windows)
	static void GetStyleAndExstyle( uint32 uStyleFlags, DWORD &OutStyle, DWORD &OutExstyle )
	{
		OutStyle = WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		OutExstyle = 0;

		if( ~uStyleFlags & kSF_NoCloseButton ) {
			OutStyle |= WS_SYSMENU;
		}
		if( ~uStyleFlags & kSF_NoMinimizeButton ) {
			OutStyle |= WS_MINIMIZEBOX;
		}
		if( ~uStyleFlags & kSF_NoMaximizeButton ) {
			OutStyle |= WS_MAXIMIZEBOX;
		}
		if( ~uStyleFlags & kSF_NoResize ) {
			OutStyle |= WS_THICKFRAME;
		}
		if( uStyleFlags & kSF_ThinCaption ) {
			OutExstyle |= WS_EX_TOOLWINDOW;
		}
		if( uStyleFlags & kSF_NoOSParts ) {
			OutStyle |= WS_POPUP;
		}
		if( uStyleFlags & kSF_TopMost ) {
			OutExstyle |= WS_EX_TOPMOST;
		}
		if( uStyleFlags & kSF_Translucent ) {
			OutExstyle |= WS_EX_LAYERED | WS_EX_COMPOSITED;
		}
	}
#endif

	OSWindow Open( const SCreateInfo &Info )
	{
#if AX_OS(Windows)
		SWindow *pWindow = Alloc< SWindow >();
		AX_EXPECT_NOT_NULL( pWindow );

		Construct( *pWindow );

		DWORD Style;
		DWORD Exstyle;

		GetStyleAndExstyle( Info.uStyleFlags, Style, Exstyle );

		RECT Area = {
			LONG( Info.PosX ),
			LONG( Info.PosY ),
			LONG( Info.PosX + Info.ResX ),
			LONG( Info.PosY + Info.ResY )
		};
		AX_EXPECT( AdjustWindowRectEx( &Area, Style, FALSE, Exstyle ) );

		wchar_t wszBuf[ 256 ];
		HWND hWindow = CreateWindowExW
		(
			Exstyle,
			( LPCWSTR )WM->WindowClass,
			String( Info.pszTitle ).ToWStr( wszBuf ),
			Style,
			Area.left,
			Area.top,
			Area.right - Area.left,
			Area.bottom - Area.top,
			NULL,
			NULL,
			GetModuleHandleW( NULL ),
			( void * )pWindow
		);
		if( !hWindow ) {
			return NULL;
		}

		return OSWindow( hWindow );
#endif
	}
	void Close( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		DestroyWindow( HWND(Window) );
#endif
	}
	void Minimize( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		ShowWindow( HWND(Window), SW_MINIMIZE );
#endif
	}
	void Maximize( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		ShowWindow( HWND(Window), SW_MAXIMIZE );
#endif
	}

	void AddDelegate( OSWindow Window, const SDelegate &Delegate )
	{
		AX_ASSERT_NOT_NULL(Window);

		SWindow &Self = W(Window);

		Self.pTopDelegate = DuplicateDelegate_r( Delegate, Self.pTopDelegate );
		AX_ASSERT_NOT_NULL( Self.pTopDelegate );
	}
	void RemoveDelegate( OSWindow Window, const SDelegate &Delegate )
	{
		AX_ASSERT_NOT_NULL(Window);

		SWindow &Self = W(Window);

		SDelegate **ppSet = &Self.pTopDelegate;
		for( SDelegate *pTest = Self.pTopDelegate; pTest != NULL; pTest = pTest->pSuper ) {
			if( !CompareDelegates( *pTest, Delegate ) ) {
				ppSet = &pTest->pSuper;
				continue;
			}

			*ppSet = pTest->pSuper;
			Dealloc( pTest );
			break;
		}
	}

	void SetTitle( OSWindow Window, const char *pszTitleUTF8 )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		wchar_t wszBuf[ 256 ];
		SetWindowTextW( HWND(Window), String( pszTitleUTF8 ).ToWStr( wszBuf ) );
#endif
	}
	uintptr GetTitle( OSWindow Window, char *pszOutTitleUTF8, uintptr cMaxOutBytes )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
# if 0
		static Async::CMutex BufferAccessor;
		static TArray< wchar_t > UTF16Chars;
		static String Title;

		if( !pszOutTitleUTF8 || !cMaxOutBytes ) {
			return GetWindowTextLengthW( HWND(Window) )*4;
		}

		*pszOutTitleUTF8 = '\0';

		{
			Async::MutexGuard Guard( BufferAccessor );

			if( !AX_VERIFY( UTF16Chars.Reserve( GetWindowTextLengthW( HWND(Window) ) ) ) ) {
				return 0;
			}

			if( !AX_VERIFY( Title.Reserve( UTF16Chars.NumAllocated()*4 ) ) ) {
				return 0;
			}

			GetWindowTextW( HWND(Window), UTF16Chars.Pointer(), int( UTF16Chars.NumAllocated() ) );

			Title.AssignUTF16( UTF16Chars.Pointer() );

			StrCpy( pszOutTitleUTF8, cMaxOutBytes, Title.CString() );
		}

		return strlen( pszOutTitleUTF8 ) + 1;
# else
		SWindow &IntWindow = W(Window);
		if( !pszOutTitleUTF8 || !cMaxOutBytes ) {
			return IntWindow.TitleText.Num();
		}

		StrCpy( pszOutTitleUTF8, cMaxOutBytes, IntWindow.TitleText );
		return strlen( pszOutTitleUTF8 ) + 1;
# endif
#endif
	}

	void SetData( OSWindow Window, void *pData )
	{
		AX_ASSERT_NOT_NULL(Window);
		W(Window).pUserData = pData;
	}
	void *GetData( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL(Window);
		return W(Window).pUserData;
	}

	void PerformClose( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		SendMessageW( HWND(Window), WM_SYSCOMMAND, SC_CLOSE, 0 );
#endif
	}
	void PerformMinimize( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		SendMessageW( HWND(Window), WM_SYSCOMMAND, SC_MINIMIZE, 0 );
#endif
	}
	void PerformMaximize( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		SendMessageW( HWND(Window), WM_SYSCOMMAND, SC_MAXIMIZE, 0 );
#endif
	}

	void SetVisible( OSWindow Window, bool bIsVisible )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		ShowWindow( HWND(Window), bIsVisible ? SW_SHOW : SW_HIDE );
		if( bIsVisible ) {
			UpdateWindow( HWND(Window) );
		}
#endif
	}
	bool IsVisible( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		return IsWindowVisible( HWND(Window) ) != FALSE;
#endif
	}

	void SetEnabled( OSWindow Window, bool bIsEnabled )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		EnableWindow( HWND(Window), bIsEnabled ? TRUE : FALSE );
#endif
	}
	bool IsEnabled( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL( Window );

#if AX_OS(Windows)
		return IsWindowEnabled( HWND(Window) ) != FALSE;
#endif
	}

	void Resize( OSWindow Window, uint32 ResX, uint32 ResY )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		const DWORD Style = GetWindowLongW( HWND(Window), GWL_STYLE );
		const DWORD ExStyle = GetWindowLongW( HWND(Window), GWL_EXSTYLE );
		const BOOL bHasMenu = GetMenu( HWND(Window) ) != NULL;

		RECT Adjust = { 0, 0, LONG( ResX ), LONG( ResY ) };

		if( !AX_VERIFY( AdjustWindowRectEx( &Adjust, Style, bHasMenu, ExStyle ) ) ) {
			return;
		}

		const uint32 ClientResX = Adjust.right - Adjust.left;
		const uint32 ClientResY = Adjust.bottom - Adjust.top;

		SetWindowPos( HWND(Window), NULL, 0, 0, ClientResX, ClientResY, SWP_NOMOVE|SWP_NOZORDER );
#endif
	}
	void ResizeFrame( OSWindow Window, uint32 ResX, uint32 ResY )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		SetWindowPos( HWND(Window), NULL, 0, 0, ResX, ResY, SWP_NOMOVE|SWP_NOZORDER );
#endif
	}

	void Position( OSWindow Window, int32 PosX, int32 PosY )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		SetWindowPos( HWND(Window), NULL, PosX, PosY, 0, 0, SWP_NOSIZE|SWP_NOZORDER );
#endif
	}

	void GetSize( OSWindow Window, uint32 &OutResX, uint32 &OutResY )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		RECT Client;

		if( !AX_VERIFY( GetClientRect( HWND(Window), &Client ) ) ) {
			OutResX = 0;
			OutResY = 0;

			return;
		}

		OutResX = Client.right;
		OutResY = Client.bottom;
#endif
	}
	void GetFrameSize( OSWindow Window, uint32 &OutResX, uint32 &OutResY )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		RECT Screen;

		if( !AX_VERIFY( GetWindowRect( HWND(Window), &Screen ) ) ) {
			OutResX = 0;
			OutResY = 0;

			return;
		}

		OutResX = Screen.right - Screen.left;
		OutResY = Screen.bottom - Screen.top;
#endif
	}

	void GetPosition( OSWindow Window, int32 &OutPosX, int32 &OutPosY )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		RECT Screen;

		if( !AX_VERIFY( GetWindowRect( HWND(Window), &Screen ) ) ) {
			OutPosX = 0;
			OutPosY = 0;

			return;
		}

		OutPosX = Screen.left;
		OutPosY = Screen.top;
#endif
	}

	void SetNeedsDisplay( OSWindow Window )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		InvalidateRect( HWND(Window), NULL, TRUE );
#endif
	}
	void SetRectNeedsDisplay( OSWindow Window, int32 ClientLeft, int32 ClientTop, int32 ClientRight, int32 ClientBottom )
	{
		AX_ASSERT_NOT_NULL(Window);

#if AX_OS(Windows)
		const RECT Area = { ClientLeft, ClientTop, ClientRight, ClientBottom };
		InvalidateRect( HWND(Window), &Area, TRUE );
#endif
	}

}}
