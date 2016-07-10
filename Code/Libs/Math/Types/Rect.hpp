#pragma once

#include "../../Core/Types.hpp"
#include "IntVector2.hpp"

namespace Ax { namespace Math {
	
	/*!
	 *	Rectangle
	 *
	 *	Represents an area from (x1,y1) to (x2 - 1,y2 - 1) inclusively. 
	 */
	struct SRect
	{
		int32						x1;
		int32						y1;
		int32						x2;
		int32						y2;

		inline SRect()
		: x1( 0 )
		, y1( 0 )
		, x2( 0 )
		, y2( 0 )
		{
		}
		inline SRect( int32 x1, int32 y1, int32 x2, int32 y2 )
		: x1( x1 )
		, y1( y1 )
		, x2( x2 )
		, y2( y2 )
		{
		}
		inline SRect( const SIntVector2 &TopLeft, const SIntVector2 &BottomRight )
		: x1( TopLeft.x )
		, y1( TopLeft.y )
		, x2( BottomRight.x )
		, y2( BottomRight.y )
		{
		}
		inline SRect( const SRect &Other )
		: x1( Other.x1 )
		, y1( Other.y1 )
		, x2( Other.x2 )
		, y2( Other.y2 )
		{
		}

		/*! Puts the members in the correct order */
		inline SRect &FixSelf()
		{
			if( x2 < x1 ) { Swap( x1, x2 ); }
			if( y2 < y1 ) { Swap( y1, y2 ); }

			return *this;
		}
		/*! Duplicates this rectangle but with the members in the correct order */
		inline SRect Fixed() const
		{
			return SRect( *this ).FixSelf();
		}

		/*! Increases this rectangle's area by combining with another rectangle */
		inline SRect &CombineWith( const SRect &other )
		{
			if( x1 > other.x1 ) { x1 = other.x1; }
			if( y1 > other.y1 ) { y1 = other.y1; }
			if( x2 < other.x2 ) { x2 = other.x2; }
			if( y2 < other.y2 ) { y2 = other.y2; }

			return *this;
		}
		/*! Creates a rectangle that tightly fits the space of this rectangle and the other rectangle */
		inline SRect Combined( const SRect &other ) const
		{
			return SRect( *this ).CombineWith( other );
		}

		/*! Restrict this rectangle by a containing rectangle */
		inline SRect &RestrictBy( const SRect &other )
		{
			if( x1 < other.x1 ) { x1 = other.x1; }
			if( y1 < other.y1 ) { y1 = other.y1; }
			if( x2 > other.x2 ) { x2 = other.x2; }
			if( y2 > other.y2 ) { y2 = other.y2; }

			return *this;
		}
		/*! Creates a rectangle that matches the current rectangle restricted by another */
		inline SRect Restricted( const SRect &other ) const
		{
			return SRect( *this ).RestrictBy( other );
		}

		/*! Center this rectangle within another rectangle (retaining the same size) */
		inline SRect &CenterSelf( const SRect &within )
		{
			const int32 w1 = x2 - x1;
			const int32 h1 = y2 - y1;

			const int32 w2 = within.x2 - within.x1;
			const int32 h2 = within.y2 - within.y1;

			x1 = within.x1 + w2/2 - w1/2;
			y1 = within.y1 + h2/2 - h1/2;
			x2 = x1 + w1;
			y2 = y1 + h1;

			return *this;
		}
		/*! Creates a rectangle that matches the current rectangle centered within another */
		inline SRect Centered( const SRect &within ) const
		{
			return SRect( *this ).CenterSelf( within );
		}

		/*! Retrieve the top-left point as a vector */
		inline SIntVector2 TopLeft() const
		{
			return SIntVector2( x1, y1 );
		}
		/*! Retrieve the top-right point as a vector */
		inline SIntVector2 TopRight() const
		{
			return SIntVector2( x2, y1 );
		}
		/*! Retrieve the bottom-right point as a vector */
		inline SIntVector2 BottomRight() const
		{
			return SIntVector2( x2, y2 );
		}
		/* Retrieve the bottom-left point as a vector */
		inline SIntVector2 BottomLeft() const
		{
			return SIntVector2( x1, y2 );
		}
		/*! Retrieve the center of this rectangle as a vector */
		inline SIntVector2 Center() const
		{
			return SIntVector2( x1 + ( x2 - x1 ), y1 + ( y2 - y1 ) );
		}
		/*! Retrieve the position of this rectangle as a vector (top left) */
		inline SIntVector2 Origin() const
		{
			return SIntVector2( x1, y1 );
		}
		/*! Retrieve the size of this rectangle as a vector */
		inline SIntVector2 Size() const
		{
			return SIntVector2( x2 - x1, y2 - y1 );
		}
		/*! Determine whether a given point is inside this rectangle */
		inline bool Contains( const SIntVector2 &v ) const
		{
			return x1 <= v.x && y1 <= v.y && x2 >= v.x && y2 >= v.y;
		}
		/*! Determine whether a given rectangle is completely contained within this rectangle */
		inline bool Contains( const SRect &r ) const
		{
			return Contains( r.TopLeft() ) && Contains( r.TopRight() ) && Contains( r.BottomRight() ) && Contains( r.BottomLeft() );
		}
		/*! Determine whether a given rectangle intersects with this rectangle */
		inline bool Intersects( const SRect &r ) const
		{
			return Contains( r.TopLeft() ) || Contains( r.TopRight() ) || Contains( r.BottomRight() ) || Contains( r.BottomLeft() );
		}
		/*! Convert a vector to one relative to this rectangle */
		inline SIntVector2 Relative( const SIntVector2 &v ) const
		{
			return v - TopLeft();
		}
		/*! Convert a rectangle to one relative to this rectangle */
		inline SRect Relative( const SRect &r ) const
		{
			SRect temp;

			temp.x1 = r.x1 - x1;
			temp.y1 = r.y1 - y1;
			temp.x2 = temp.x1 + r.ResX();
			temp.y2 = temp.y1 + r.ResY();

			return temp;
		}
		/*! Move this rectangle by the given vector */
		inline SRect &MoveSelf( const SIntVector2 &distance )
		{
			x1 += distance.x;
			y1 += distance.y;
			x2 += distance.x;
			y2 += distance.y;
			return *this;
		}
		/*! Duplicate this rectangle and move it by the given vector */
		inline SRect Moved( const SIntVector2 &distance ) const
		{
			return SRect( *this ).MoveSelf( distance );
		}

		/*! Resize this rectangle */
		inline SRect &ResizeSelf( const SIntVector2 &size )
		{
			x2 = x1 + size.x;
			y2 = y1 + size.y;
			return *this;
		}
		/*! Duplicate this rectangle and resize it by the given vector */
		inline SRect Resized( const SIntVector2 &size ) const
		{
			return SRect( *this ).ResizeSelf( size );
		}

		/*! Position this rectangle */
		inline SRect &PositionSelf( const SIntVector2 &pos )
		{
			x2 = pos.x + ResX();
			y2 = pos.y + ResY();
			x1 = pos.x;
			y1 = pos.y;

			return *this;
		}
		/*! Duplicate this rectangle and position it at the given vector */
		inline SRect Positioned( const SIntVector2 &pos ) const
		{
			return SRect( *this ).PositionSelf( pos );
		}

		/*! Retrieve the width of this rectangle. Same as ResX(). */
		inline int32 Width() const { return x2 - x1; }
		/*! Retrieve the height of this rectangle. Same as ResY(). */
		inline int32 Height() const { return y2 - y1; }

		/*! Retrieve the x-resolution of this rectangle. Same as Width(). */
		inline int32 ResX() const { return x2 - x1; }
		/*! Retrieve the y-resolution of this rectangle. Same as Height(). */
		inline int32 ResY() const { return y2 - y1; }

		/*! Retrieve the x-origin of this rectangle. */
		inline int32 PosX() const { return x1; }
		/*! Retrieve the y-origin of this rectangle. */
		inline int32 PosY() const { return y1; }

		inline bool operator==( const SRect &Other ) const
		{
			return x1 == Other.x1 && y1 == Other.y1 && x2 == Other.x2 && y2 == Other.y2;
		}
		inline bool operator!=( const SRect &Other ) const
		{
			return x1 != Other.x1 || y1 != Other.y1 || x2 != Other.x2 || y2 != Other.y2;
		}
	};

	inline SIntVector2 &SIntVector2::ClampSelf( const SRect &bounds )
	{
		return ClampSelf( bounds.TopLeft(), bounds.BottomRight() );
	}

}}
