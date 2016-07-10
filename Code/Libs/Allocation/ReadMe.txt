ALLOCATION
==========

The Allocation library handles memory allocation, deallocation, and statistics.
A system of "memory tags" is implemented such that information about any given
subsystem's memory usage and requests can be queried.


CONFIGURATION MACROS
--------------------
`AX_MEMTAGS_ENABLED` is by default defined to 1. This enables memory statistics
to be recorded. When memory tags are enabled a little extra memory is used to
retain information about the tagged memory. It's recommended to keep this
enabled for at least development builds.

`AX_DEFAULT_MEMTAG` specifies what the default memory tag (integer) is when a
memory tag is not explicitly given to one of the allocation commands. This
defaults to tag 0. It is recommended that this be defined per-project (in the
project's settings as an additional preprocessor macro).


SETTING THE NAME OF A MEMORY TAG
--------------------------------
For development purposes it can be helpful to specify a canonical name for a
given memory tag. The names of memory tags can be set and queried with the
following commands.

	void Ax::SetAllocatorName( const char *name, int tag = AX_DEFAULT_MEMTAG );
	const char *Ax::GetAllocatorName( int tag = AX_DEFAULT_MEMTAG );


CUSTOM ALLOCATION
-----------------
If you need a custom allocator for specific systems it's possible to provide
one. You implement an Ax::IAllocator interface (see Allocator.hpp) then set the
allocator with

	void Ax::SetCustomAllocator( IAllocator *allocator, int tag = AX_DEFAULT_MEMTAG );

Custom allocators do not affect memory tagging, so they are safe to use with the
memory tagging system.


CHECKING MEMTAG STATS
---------------------
You can easily check memory tag statistics with a call to the query function
(listed below).

	Ax::STagStats &Ax::GetAllocStats( int tag = AX_DEFAULT_MEMTAG );

This returns a reference to the given tag's stats. (See Allocator.hpp.) The
reference can be modified, however synchronization is up to you.


THREAD SAFETY
-------------
The Allocation library is thread-safe except for the custom allocators. (Custom
allocators will need to implement their own thread-safety systems if necessary.)

The following functions are also not thread-safe:

	void Ax::SetCustomAllocator( IAllocator *pAllocator, int tag = AX_DEFAULT_MEMTAG );
	Ax::IAllocator *Ax::GetCustomAllocator( int tag = AX_DEFAULT_MEMTAG );
	
	void Ax::SetAllocatorName( const char *psName, int tag = AX_DEFAULT_MEMTAG );
	const char *Ax::GetAllocatorName( int tag = AX_DEFAULT_MEMTAG );

Any calls to the above functions should be done carefully.

