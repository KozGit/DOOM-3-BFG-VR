/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.
Copyright (C) 2012 Robert Beckebans

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
/*
sys_event.cpp

Event are used for scheduling tasks and for linking script commands.

*/

#pragma hdrstop
#include "precompiled.h"


#include "../Game_local.h"

#define MAX_EVENTSPERFRAME			4096
//#define CREATE_EVENT_CODE

/***********************************************************************

  idEventDef

***********************************************************************/

idEventDef* idEventDef::eventDefList[MAX_EVENTS];
int idEventDef::numEventDefs = 0;

static bool eventError = false;
static char eventErrorMsg[ 128 ];

/*
================
idEventDef::idEventDef
================
*/
idEventDef::idEventDef( const char* command, const char* formatspec, char returnType )
{
	idEventDef*	ev;
	int			i;
	unsigned int	bits;
	
	assert( command );
	assert( !idEvent::initialized );
	
	// Allow NULL to indicate no args, but always store it as ""
	// so we don't have to check for it.
	if( !formatspec )
	{
		formatspec = "";
	}
	
	this->name = command;
	this->formatspec = formatspec;
	this->returnType = returnType;
	
	numargs = (int)strlen( formatspec );
	assert( numargs <= D_EVENT_MAXARGS );
	if( numargs > D_EVENT_MAXARGS )
	{
		eventError = true;
		sprintf( eventErrorMsg, "idEventDef::idEventDef : Too many args for '%s' event.", name );
		return;
	}
	
	// make sure the format for the args is valid, calculate the formatspecindex, and the offsets for each arg
	bits = 0;
	argsize = 0;
#ifdef IS64BIT
	argsize_32 = 0;
#else
	argsize_64 = 0;
#endif
	memset( argOffset, 0, sizeof( argOffset ) );
	for( i = 0; i < numargs; i++ )
	{
		argOffset[ i ] = (int)argsize;
		switch (formatspec[i])
		{
			case D_EVENT_FLOAT :
				bits |= 1 << i;
				// RB: 64 bit fix, changed sizeof( float ) to sizeof( intptr_t )
				argsize += sizeof( intptr_t );
				// RB end
#ifdef IS64BIT
				argsize_32 += 4;
#else
				argsize_64 += 8;
#endif
				break;
				
			case D_EVENT_INTEGER :
				// RB: 64 bit fix, changed sizeof( int ) to sizeof( intptr_t )
				argsize += sizeof( intptr_t );
				// RB end
#ifdef IS64BIT
				argsize_32 += 4;
#else
				argsize_64 += 8;
#endif
				break;
				
			case D_EVENT_VECTOR :
				// RB: 64 bit fix, changed sizeof( idVec3 ) to E_EVENT_SIZEOF_VEC
				argsize += E_EVENT_SIZEOF_VEC;
				// RB end
#ifdef IS64BIT
				argsize_32 += 12;
#else
				argsize_64 += 16;
#endif
				break;
				
			case D_EVENT_STRING :
				argsize += MAX_STRING_LEN;
#ifdef IS64BIT
				argsize_32 += MAX_STRING_LEN;
#else
				argsize_64 += MAX_STRING_LEN;
#endif
				break;
				
			case D_EVENT_ENTITY :
			case D_EVENT_ENTITY_NULL:
				// RB: 64 bit fix, sizeof( idEntityPtr<idEntity> ) to sizeof( intptr_t )
				argsize += sizeof( intptr_t );
				// RB end
#ifdef IS64BIT
				argsize_32 += 4;
#else
				argsize_64 += 8;
#endif
				break;
				
			case D_EVENT_TRACE :
			{
				// Carl Debug
#if 0
				trace_t t;
				size_t a = (char*)&t.endpos - (char*)&t; // 4 // 4
				size_t b = (char*)&t.endAxis - (char*)&t; // 16 // 16
				size_t c = (char*)&t.c - (char*)&t; // 52 // 56
				size_t d = (char*)&t.c.point - (char*)&t; // 56 // 60
				size_t e = (char*)&t.c.normal - (char*)&t; // 68 // 72
				size_t f = (char*)&t.c.dist - (char*)&t; // 80 // 84
				size_t g = (char*)&t.c.contents - (char*)&t; // 84 // 88
				size_t h = (char*)&t.c.material - (char*)&t; // 88 // 96
				size_t i = (char*)&t.c.modelFeature - (char*)&t; // 92 // 104
				size_t j = (char*)&t.c.id - (char*)&t; // 104 // 116
				size_t s = sizeof( t.c ); // 56 // 64
				size_t s2 = sizeof( t.c.material ); // 4 // 8
#endif

				argsize += sizeof( trace_t ) + MAX_STRING_LEN + sizeof( bool );
#ifdef IS64BIT
				argsize_32 += 1 + 108 + 128;
#else
				argsize_64 += 1 + 120 + 128;
#endif
				break;
			}
				
			default :
				eventError = true;
				sprintf( eventErrorMsg, "idEventDef::idEventDef : Invalid arg format '%s' string for '%s' event.", formatspec, name );
				return;
				break;
		}
	}
	
	// calculate the formatspecindex
	formatspecIndex = ( 1 << ( numargs + D_EVENT_MAXARGS ) ) | bits;
	
	// go through the list of defined events and check for duplicates
	// and mismatched format strings
	eventnum = numEventDefs;
	for( i = 0; i < eventnum; i++ )
	{
		ev = eventDefList[ i ];
		if( strcmp( command, ev->name ) == 0 )
		{
			if( strcmp( formatspec, ev->formatspec ) != 0 )
			{
				eventError = true;
				sprintf( eventErrorMsg, "idEvent '%s' defined twice with same name but differing format strings ('%s'!='%s').",
						 command, formatspec, ev->formatspec );
				return;
			}
			
			if( ev->returnType != returnType )
			{
				eventError = true;
				sprintf( eventErrorMsg, "idEvent '%s' defined twice with same name but differing return types ('%c'!='%c').",
						 command, returnType, ev->returnType );
				return;
			}
			// Don't bother putting the duplicate event in list.
			eventnum = ev->eventnum;
			return;
		}
	}
	
	ev = this;
	
	if( numEventDefs >= MAX_EVENTS )
	{
		eventError = true;
		sprintf( eventErrorMsg, "numEventDefs >= MAX_EVENTS" );
		return;
	}
	eventDefList[numEventDefs] = ev;
	numEventDefs++;
}

/*
================
idEventDef::NumEventCommands
================
*/
int	idEventDef::NumEventCommands()
{
	return numEventDefs;
}

/*
================
idEventDef::GetEventCommand
================
*/
const idEventDef* idEventDef::GetEventCommand( int eventnum )
{
	return eventDefList[ eventnum ];
}

/*
================
idEventDef::FindEvent
================
*/
const idEventDef* idEventDef::FindEvent( const char* name )
{
	idEventDef*	ev;
	int			num;
	int			i;
	
	assert( name );
	
	num = numEventDefs;
	for( i = 0; i < num; i++ )
	{
		ev = eventDefList[ i ];
		if( strcmp( name, ev->name ) == 0 )
		{
			return ev;
		}
	}
	
	return NULL;
}

/***********************************************************************

  idEvent

***********************************************************************/

static idLinkList<idEvent> FreeEvents;
static idLinkList<idEvent> EventQueue;
static idLinkList<idEvent> FastEventQueue;
static idEvent EventPool[ MAX_EVENTS ];

bool idEvent::initialized = false;

idDynamicBlockAlloc<byte, 16 * 1024, 256>	idEvent::eventDataAllocator;

/*
================
idEvent::~idEvent()
================
*/
idEvent::~idEvent()
{
	Free();
}

/*
================
idEvent::Alloc
================
*/
idEvent* idEvent::Alloc( const idEventDef* evdef, int numargs, va_list args )
{
	idEvent*		ev;
	size_t		size;
	const char*	format;
	idEventArg*	arg;
	byte*		dataPtr;
	int			i;
	const char*	materialName;
	
	if( FreeEvents.IsListEmpty() )
	{
		gameLocal.Error( "idEvent::Alloc : No more free events" );
	}
	
	ev = FreeEvents.Next();
	ev->eventNode.Remove();
	
	ev->eventdef = evdef;
	
	if( numargs != evdef->GetNumArgs() )
	{
		gameLocal.Error( "idEvent::Alloc : Wrong number of args for '%s' event.", evdef->GetName() );
	}
	
	size = evdef->GetArgSize();
	if( size )
	{
		ev->data = eventDataAllocator.Alloc( (int)size );
		memset( ev->data, 0, size );
	}
	else
	{
		ev->data = NULL;
		return ev;
	}
	
	format = evdef->GetArgFormat();
	for( i = 0; i < numargs; i++ )
	{
		arg = va_arg( args, idEventArg* );
		if( format[ i ] != arg->type )
		{
			// when NULL is passed in for an entity, it gets cast as an integer 0, so don't give an error when it happens
			if( !( ( ( format[ i ] == D_EVENT_TRACE ) || ( format[ i ] == D_EVENT_ENTITY ) ) && ( arg->type == 'd' ) && ( arg->value == 0 ) ) )
			{
				gameLocal.Error( "idEvent::Alloc : Wrong type passed in for arg # %d on '%s' event.", i, evdef->GetName() );
			}
		}
		
		dataPtr = &ev->data[ evdef->GetArgOffset( i ) ];
		
		switch( format[ i ] )
		{
			case D_EVENT_FLOAT :
			case D_EVENT_INTEGER :
				*reinterpret_cast<int*>( dataPtr ) = arg->value;
				break;
				
			case D_EVENT_VECTOR :
				if( arg->value )
				{
					*reinterpret_cast<idVec3*>( dataPtr ) = *reinterpret_cast<const idVec3*>( arg->value );
				}
				break;
				
			case D_EVENT_STRING :
				if( arg->value )
				{
					idStr::Copynz( reinterpret_cast<char*>( dataPtr ), reinterpret_cast<const char*>( arg->value ), MAX_STRING_LEN );
				}
				break;
				
			case D_EVENT_ENTITY :
			case D_EVENT_ENTITY_NULL :
				*reinterpret_cast< idEntityPtr<idEntity> * >( dataPtr ) = reinterpret_cast<idEntity*>( arg->value );
				break;
				
			case D_EVENT_TRACE :
				if( arg->value )
				{
					*reinterpret_cast<bool*>( dataPtr ) = true;
					*reinterpret_cast<trace_t*>( dataPtr + sizeof( bool ) ) = *reinterpret_cast<const trace_t*>( arg->value );
					
					// save off the material as a string since the pointer won't be valid in save games.
					// since we save off the entire trace_t structure, if the material is NULL here,
					// it will be NULL when we process it, so we don't need to save off anything in that case.
					if( reinterpret_cast<const trace_t*>( arg->value )->c.material )
					{
						materialName = reinterpret_cast<const trace_t*>( arg->value )->c.material->GetName();
						idStr::Copynz( reinterpret_cast<char*>( dataPtr + sizeof( bool ) + sizeof( trace_t ) ), materialName, MAX_STRING_LEN );
					}
				}
				else
				{
					*reinterpret_cast<bool*>( dataPtr ) = false;
				}
				break;
				
			default :
				gameLocal.Error( "idEvent::Alloc : Invalid arg format '%s' string for '%s' event.", format, evdef->GetName() );
				break;
		}
	}
	
	return ev;
}

/*
================
idEvent::CopyArgs
================
*/
// RB: 64 bit fixes, changed int to intptr_t
void idEvent::CopyArgs( const idEventDef* evdef, int numargs, va_list args, intptr_t data[ D_EVENT_MAXARGS ] )
{
// RB end
	int			i;
	const char*	format;
	idEventArg*	arg;
	
	format = evdef->GetArgFormat();
	if( numargs != evdef->GetNumArgs() )
	{
		gameLocal.Error( "idEvent::CopyArgs : Wrong number of args for '%s' event.", evdef->GetName() );
	}
	
	for( i = 0; i < numargs; i++ )
	{
		arg = va_arg( args, idEventArg* );
		if( format[ i ] != arg->type )
		{
			// when NULL is passed in for an entity, it gets cast as an integer 0, so don't give an error when it happens
			if( !( ( ( format[ i ] == D_EVENT_TRACE ) || ( format[ i ] == D_EVENT_ENTITY ) ) && ( arg->type == 'd' ) && ( arg->value == 0 ) ) )
			{
				gameLocal.Error( "idEvent::CopyArgs : Wrong type passed in for arg # %d on '%s' event.", i, evdef->GetName() );
			}
		}
		
		data[ i ] = arg->value;
	}
}

/*
================
idEvent::Free
================
*/
void idEvent::Free()
{
	if( data )
	{
//TODO: this triggers an assert, but leaks data now
#ifdef _WIN32
		eventDataAllocator.Free( data );
#endif
		data = NULL;
	}
	
	eventdef	= NULL;
	time		= 0;
	object		= NULL;
	typeinfo	= NULL;
	
	eventNode.SetOwner( this );
	eventNode.AddToEnd( FreeEvents );
}

/*
================
idEvent::Schedule
================
*/
void idEvent::Schedule( idClass* obj, const idTypeInfo* type, int time )
{
	idEvent* event;
	
	assert( initialized );
	if( !initialized )
	{
		return;
	}
	
	object = obj;
	typeinfo = type;
	
	// wraps after 24 days...like I care. ;)
	this->time = gameLocal.time + time;
	
	eventNode.Remove();
	
	if( obj->IsType( idEntity::Type ) && ( ( ( idEntity* )( obj ) )->timeGroup == TIME_GROUP2 ) )
	{
		event = FastEventQueue.Next();
		while( ( event != NULL ) && ( this->time >= event->time ) )
		{
			event = event->eventNode.Next();
		}
		
		if( event )
		{
			eventNode.InsertBefore( event->eventNode );
		}
		else
		{
			eventNode.AddToEnd( FastEventQueue );
		}
		
		return;
	}
	else
	{
		this->time = gameLocal.slow.time + time;
	}
	
	event = EventQueue.Next();
	while( ( event != NULL ) && ( this->time >= event->time ) )
	{
		event = event->eventNode.Next();
	}
	
	if( event )
	{
		eventNode.InsertBefore( event->eventNode );
	}
	else
	{
		eventNode.AddToEnd( EventQueue );
	}
}

/*
================
idEvent::CancelEvents
================
*/
void idEvent::CancelEvents( const idClass* obj, const idEventDef* evdef )
{
	idEvent* event;
	idEvent* next;
	
	if( !initialized )
	{
		return;
	}
	
	for( event = EventQueue.Next(); event != NULL; event = next )
	{
		next = event->eventNode.Next();
		if( event->object == obj )
		{
			if( !evdef || ( evdef == event->eventdef ) )
			{
				event->Free();
			}
		}
	}
	
	for( event = FastEventQueue.Next(); event != NULL; event = next )
	{
		next = event->eventNode.Next();
		if( event->object == obj )
		{
			if( !evdef || ( evdef == event->eventdef ) )
			{
				event->Free();
			}
		}
	}
}

/*
================
idEvent::ClearEventList
================
*/
void idEvent::ClearEventList()
{
	int i;
	
	//
	// initialize lists
	//
	FreeEvents.Clear();
	EventQueue.Clear();
	
	//
	// add the events to the free list
	//
	for( i = 0; i < MAX_EVENTS; i++ )
	{
		EventPool[ i ].Free();
	}
}

/*
================
idEvent::ServiceEvents
================
*/
void idEvent::ServiceEvents()
{
	idEvent*		event;
	int			num;
	// RB: 64 bit fixes, changed int to intptr_t
	intptr_t	args[ D_EVENT_MAXARGS ];
	// RB end
	int			offset;
	int			i;
	int			numargs;
	const char*	formatspec;
	trace_t**		tracePtr;
	const idEventDef* ev;
	byte*		data;
	const char*  materialName;
	
	num = 0;
	while( !EventQueue.IsListEmpty() )
	{
		event = EventQueue.Next();
		assert( event );
		
		if( event->time > gameLocal.time )
		{
			break;
		}
		
		common->UpdateLevelLoadPacifier();
		
		// copy the data into the local args array and set up pointers
		ev = event->eventdef;
		formatspec = ev->GetArgFormat();
		numargs = ev->GetNumArgs();
		for( i = 0; i < numargs; i++ )
		{
			offset = ev->GetArgOffset( i );
			data = event->data;
			switch( formatspec[ i ] )
			{
				case D_EVENT_FLOAT :
				case D_EVENT_INTEGER :
					args[ i ] = *reinterpret_cast<int*>( &data[ offset ] );
					break;
					
				case D_EVENT_VECTOR :
					*reinterpret_cast<idVec3**>( &args[ i ] ) = reinterpret_cast<idVec3*>( &data[ offset ] );
					break;
					
				case D_EVENT_STRING :
					*reinterpret_cast<const char**>( &args[ i ] ) = reinterpret_cast<const char*>( &data[ offset ] );
					break;
					
				case D_EVENT_ENTITY :
				case D_EVENT_ENTITY_NULL :
					*reinterpret_cast<idEntity**>( &args[ i ] ) = reinterpret_cast< idEntityPtr<idEntity> * >( &data[ offset ] )->GetEntity();
					break;
					
				case D_EVENT_TRACE :
					tracePtr = reinterpret_cast<trace_t**>( &args[ i ] );
					if( *reinterpret_cast<bool*>( &data[ offset ] ) )
					{
						*tracePtr = reinterpret_cast<trace_t*>( &data[ offset + sizeof( bool ) ] );
						
						if( ( *tracePtr )->c.material != NULL )
						{
							// look up the material name to get the material pointer
							materialName = reinterpret_cast<const char*>( &data[ offset + sizeof( bool ) + sizeof( trace_t ) ] );
							( *tracePtr )->c.material = declManager->FindMaterial( materialName, true );
						}
					}
					else
					{
						*tracePtr = NULL;
					}
					break;
					
				default:
					gameLocal.Error( "idEvent::ServiceEvents : Invalid arg format '%s' string for '%s' event.", formatspec, ev->GetName() );
			}
		}
		
		// the event is removed from its list so that if then object
		// is deleted, the event won't be freed twice
		event->eventNode.Remove();
		//assert( event->object );
		if (event->object)
			event->object->ProcessEventArgPtr( ev, args );
		
#if 0
		// event functions may never leave return values on the FPU stack
		// enable this code to check if any event call left values on the FPU stack
		if( !sys->FPU_StackIsEmpty() )
		{
			gameLocal.Error( "idEvent::ServiceEvents %d: %s left a value on the FPU stack\n", num, ev->GetName() );
		}
#endif
		
		// return the event to the free list
		event->Free();
		
		// Don't allow ourselves to stay in here too long.  An abnormally high number
		// of events being processed is evidence of an infinite loop of events.
		num++;
		if( num > MAX_EVENTSPERFRAME )
		{
			gameLocal.Error( "Event overflow.  Possible infinite loop in script." );
		}
	}
}

/*
================
idEvent::ServiceFastEvents
================
*/
void idEvent::ServiceFastEvents()
{
	idEvent*	event;
	int			num;
	// RB: 64 bit fixes, changed int to intptr_t
	intptr_t	args[ D_EVENT_MAXARGS ];
	// RB end
	int			offset;
	int			i;
	int			numargs;
	const char*	formatspec;
	trace_t**		tracePtr;
	const idEventDef* ev;
	byte*		data;
	const char*  materialName;
	
	num = 0;
	while( !FastEventQueue.IsListEmpty() )
	{
		event = FastEventQueue.Next();
		assert( event );
		
		if( event->time > gameLocal.fast.time )
		{
			break;
		}
		
		// copy the data into the local args array and set up pointers
		ev = event->eventdef;
		formatspec = ev->GetArgFormat();
		numargs = ev->GetNumArgs();
		for( i = 0; i < numargs; i++ )
		{
			offset = ev->GetArgOffset( i );
			data = event->data;
			switch( formatspec[ i ] )
			{
				case D_EVENT_FLOAT :
				case D_EVENT_INTEGER :
					args[ i ] = *reinterpret_cast<int*>( &data[ offset ] );
					break;
					
				case D_EVENT_VECTOR :
					*reinterpret_cast<idVec3**>( &args[ i ] ) = reinterpret_cast<idVec3*>( &data[ offset ] );
					break;
					
				case D_EVENT_STRING :
					*reinterpret_cast<const char**>( &args[ i ] ) = reinterpret_cast<const char*>( &data[ offset ] );
					break;
					
				case D_EVENT_ENTITY :
				case D_EVENT_ENTITY_NULL :
					*reinterpret_cast<idEntity**>( &args[ i ] ) = reinterpret_cast< idEntityPtr<idEntity> * >( &data[ offset ] )->GetEntity();
					break;
					
				case D_EVENT_TRACE :
					tracePtr = reinterpret_cast<trace_t**>( &args[ i ] );
					if( *reinterpret_cast<bool*>( &data[ offset ] ) )
					{
						*tracePtr = reinterpret_cast<trace_t*>( &data[ offset + sizeof( bool ) ] );
						
						if( ( *tracePtr )->c.material != NULL )
						{
							// look up the material name to get the material pointer
							materialName = reinterpret_cast<const char*>( &data[ offset + sizeof( bool ) + sizeof( trace_t ) ] );
							( *tracePtr )->c.material = declManager->FindMaterial( materialName, true );
						}
					}
					else
					{
						*tracePtr = NULL;
					}
					break;
					
				default:
					gameLocal.Error( "idEvent::ServiceFastEvents : Invalid arg format '%s' string for '%s' event.", formatspec, ev->GetName() );
			}
		}
		
		// the event is removed from its list so that if then object
		// is deleted, the event won't be freed twice
		event->eventNode.Remove();
		assert( event->object );
		event->object->ProcessEventArgPtr( ev, args );
		
#if 0
		// event functions may never leave return values on the FPU stack
		// enable this code to check if any event call left values on the FPU stack
		if( !sys->FPU_StackIsEmpty() )
		{
			gameLocal.Error( "idEvent::ServiceEvents %d: %s left a value on the FPU stack\n", num, event->eventdef->GetName() );
		}
#endif
		
		// return the event to the free list
		event->Free();
		
		// Don't allow ourselves to stay in here too long.  An abnormally high number
		// of events being processed is evidence of an infinite loop of events.
		num++;
		if( num > MAX_EVENTSPERFRAME )
		{
			gameLocal.Error( "Event overflow.  Possible infinite loop in script." );
		}
	}
}

/*
================
idEvent::Init
================
*/
void idEvent::Init()
{
	gameLocal.Printf( "Initializing event system\n" );
	
	if( eventError )
	{
		gameLocal.Error( "%s", eventErrorMsg );
	}
	
#ifdef CREATE_EVENT_CODE
	void CreateEventCallbackHandler();
	CreateEventCallbackHandler();
	gameLocal.Error( "Wrote event callback handler" );
#endif
	
	if( initialized )
	{
		gameLocal.Printf( "...already initialized\n" );
		ClearEventList();
		return;
	}
	
	ClearEventList();
	
	eventDataAllocator.Init();
	
	gameLocal.Printf( "...%i event definitions\n", idEventDef::NumEventCommands() );
	
	// the event system has started
	initialized = true;
}

/*
================
idEvent::Shutdown
================
*/
void idEvent::Shutdown()
{
	gameLocal.Printf( "Shutdown event system\n" );
	
	if( !initialized )
	{
		gameLocal.Printf( "...not started\n" );
		return;
	}
	
	ClearEventList();
	
	eventDataAllocator.Shutdown();
	
	// say it is now shutdown
	initialized = false;
}

/*
================
idEvent::Save
================
*/
void idEvent::Save( idSaveGame* savefile )
{
	char* str;
	int i, size;
	idEvent*	event;
	byte* dataPtr;
	bool validTrace;
	const char*	format;
	// RB: for missing D_EVENT_STRING
	idStr s;
	// RB end
	
	savefile->WriteInt( EventQueue.Num() );
	
	event = EventQueue.Next();
	while( event != NULL )
	{
		savefile->WriteInt( event->time );
		savefile->WriteString( event->eventdef->GetName() );
		savefile->WriteString( event->typeinfo->classname );
		savefile->WriteObject( event->object );
		savefile->WriteInt( (int)event->eventdef->GetArgSize() );
		format = event->eventdef->GetArgFormat();
		for( i = 0, size = 0; i < event->eventdef->GetNumArgs(); ++i )
		{
			dataPtr = &event->data[ event->eventdef->GetArgOffset( i ) ];
			switch( format[ i ] )
			{
				case D_EVENT_FLOAT :
					savefile->WriteFloat( *reinterpret_cast<float*>( dataPtr ) );
					// RB: 64 bit fix, changed sizeof( float ) to sizeof( intptr_t )
					size += sizeof( intptr_t );
					// RB end
					break;
				case D_EVENT_INTEGER :
					// RB: 64 bit fix, changed sizeof( int ) to sizeof( intptr_t )
					savefile->WriteInt( *reinterpret_cast<int*>( dataPtr ) );
					size += sizeof( intptr_t );
					break;
				// RB end
				case D_EVENT_ENTITY :
				case D_EVENT_ENTITY_NULL :
					// RB: 64 bit fix, changed alignment to sizeof( intptr_t )
					reinterpret_cast< idEntityPtr<idEntity> * >( dataPtr )->Save( savefile );
					size += sizeof( intptr_t );
					// RB end
					break;
				case D_EVENT_VECTOR :
					savefile->WriteVec3( *reinterpret_cast<idVec3*>( dataPtr ) );
					// RB: 64 bit fix, changed sizeof( int ) to E_EVENT_SIZEOF_VEC
					size += E_EVENT_SIZEOF_VEC;
					// RB end
					break;
#if 1
				// RB: added missing D_EVENT_STRING case
				case D_EVENT_STRING :
					s.Clear();
					s.Append( reinterpret_cast<char*>( dataPtr ) );
					savefile->WriteString( s );
					//size += s.Length();
					size += MAX_STRING_LEN;
					break;
					// RB end
#endif
				case D_EVENT_TRACE :
					validTrace = *reinterpret_cast<bool*>( dataPtr );
					savefile->WriteBool( validTrace );
					size += sizeof( bool );
					if( validTrace )
					{
						size += sizeof( trace_t );
						const trace_t& t = *reinterpret_cast<trace_t*>( dataPtr + sizeof( bool ) );
						SaveTrace( savefile, t );
						if( t.c.material )
						{
							size += MAX_STRING_LEN;
							str = reinterpret_cast<char*>( dataPtr + sizeof( bool ) + sizeof( trace_t ) );
							savefile->Write( str, MAX_STRING_LEN );
						}
					}
					break;
				default:
					break;
			}
		}
		assert( size == ( int )event->eventdef->GetArgSize() );
		event = event->eventNode.Next();
	}
	
	// Save the Fast EventQueue
	savefile->WriteInt( FastEventQueue.Num() );
	
	event = FastEventQueue.Next();
	while( event != NULL )
	{
		savefile->WriteInt( event->time );
		savefile->WriteString( event->eventdef->GetName() );
		savefile->WriteString( event->typeinfo->classname );
		savefile->WriteObject( event->object );
		savefile->WriteInt( (int)event->eventdef->GetArgSize() );
		savefile->Write( event->data, (int)event->eventdef->GetArgSize() );
		
		event = event->eventNode.Next();
	}
}

/*
================
idEvent::Restore
================
*/
void idEvent::Restore( idRestoreGame* savefile )
{
	char*    str;
	int		num, argsize, i, j, size;
	idStr	name;
	byte* dataPtr;
	idEvent*	event;
	const char*	format;
	// RB: for missing D_EVENT_STRING
	idStr s;
	// RB end
	bool convertFrom32Bit = false;
	bool convertFrom64Bit = false;

	savefile->ReadInt( num );
	
	for( i = 0; i < num; i++ )
	{
		if( FreeEvents.IsListEmpty() )
		{
			gameLocal.Error( "idEvent::Restore : No more free events" );
		}
		
		event = FreeEvents.Next();
		event->eventNode.Remove();
		event->eventNode.AddToEnd( EventQueue );
		
		savefile->ReadInt( event->time );
		
		// read the event name
		savefile->ReadString( name );
		event->eventdef = idEventDef::FindEvent( name );
		if( event->eventdef == NULL )
		{
			savefile->Error( "idEvent::Restore: unknown event '%s'", name.c_str() );
			return;
		}
		
		// read the classtype
		savefile->ReadString( name );
		event->typeinfo = idClass::GetClass( name );
		if( event->typeinfo == NULL )
		{
			savefile->Error( "idEvent::Restore: unknown class '%s' on event '%s'", name.c_str(), event->eventdef->GetName() );
			return;
		}
		
		savefile->ReadObject( event->object );
		
		// read the args
		savefile->ReadInt( argsize );
		if( argsize != (int)event->eventdef->GetArgSize() )
		{
#ifdef IS64BIT
			// Carl: loading 32 bit file on 64 bit
			if( argsize == event->eventdef->argsize_32 )
				convertFrom32Bit = true;
			else
#else
			// Carl: loading 64 bit file on 32 bit
			if( argsize == event->eventdef->argsize_64 )
				convertFrom64Bit = true;
			else
#endif
			// RB: fixed wrong formatting // Carl: fixed RB's wrong formatting
				savefile->Error( "idEvent::Restore: arg size (%d) doesn't match saved arg size(%d) on event '%s'", (int)event->eventdef->GetArgSize(), (int)argsize, event->eventdef->GetName() );
			// RB end
		}
		if( argsize )
		{
			event->data = eventDataAllocator.Alloc( (int)event->eventdef->GetArgSize() );
			format = event->eventdef->GetArgFormat();
			assert( format );
			for( j = 0, size = 0; j < event->eventdef->GetNumArgs(); ++j )
			{
				dataPtr = &event->data[ event->eventdef->GetArgOffset( j ) ];
				switch( format[ j ] )
				{
					case D_EVENT_FLOAT :
						savefile->ReadFloat( *reinterpret_cast<float*>( dataPtr ) );
						// RB: 64 bit fix, changed sizeof( float ) to sizeof( intptr_t )
						size += sizeof( intptr_t );
						// RB end
						break;
					case D_EVENT_INTEGER :
						// RB: 64 bit fix
						savefile->ReadInt( *reinterpret_cast<int*>( dataPtr ) );
						size += sizeof( intptr_t );
						break;
					// RB end
					case D_EVENT_ENTITY :
					case D_EVENT_ENTITY_NULL :
						// RB: 64 bit fix, changed alignment to sizeof( intptr_t )
						reinterpret_cast<idEntityPtr<idEntity> *>( dataPtr )->Restore( savefile );
						size += sizeof( intptr_t );
						// RB end
						break;
					case D_EVENT_VECTOR :
						savefile->ReadVec3( *reinterpret_cast<idVec3*>( dataPtr ) );
						// RB: 64 bit fix, changed sizeof( int ) to E_EVENT_SIZEOF_VEC
						size += E_EVENT_SIZEOF_VEC;
						// RB end
						break;
#if 1
					// RB: added missing D_EVENT_STRING case
					case D_EVENT_STRING :
						savefile->ReadString( s );
						//idStr::Copynz(reinterpret_cast<char *>( dataPtr ), s, s.Length() );
						//size += s.Length();
						idStr::Copynz( reinterpret_cast<char*>( dataPtr ), s, MAX_STRING_LEN );
						size += MAX_STRING_LEN;
						break;
						// RB end
#endif
					case D_EVENT_TRACE :
						savefile->ReadBool( *reinterpret_cast<bool*>( dataPtr ) );
						size += sizeof( bool );
						if( *reinterpret_cast<bool*>( dataPtr ) )
						{
							size += sizeof( trace_t );
							trace_t& t = *reinterpret_cast<trace_t*>( dataPtr + sizeof( bool ) );
							RestoreTrace( savefile,  t ) ;
							if( t.c.material )
							{
								size += MAX_STRING_LEN;
								str = reinterpret_cast<char*>( dataPtr + sizeof( bool ) + sizeof( trace_t ) );
								savefile->Read( str, MAX_STRING_LEN );
							}
						}
						break;
					default:
						break;
				}
			}
			//assert( size == ( int )event->eventdef->GetArgSize() );
		}
		else
		{
			event->data = NULL;
		}
	}
	
	// Restore the Fast EventQueue
	savefile->ReadInt( num );
	
	for( i = 0; i < num; i++ )
	{
		if( FreeEvents.IsListEmpty() )
		{
			gameLocal.Error( "idEvent::Restore : No more free events" );
		}
		
		event = FreeEvents.Next();
		event->eventNode.Remove();
		event->eventNode.AddToEnd( FastEventQueue );
		
		savefile->ReadInt( event->time );
		
		// read the event name
		savefile->ReadString( name );
		event->eventdef = idEventDef::FindEvent( name );
		if( event->eventdef == NULL )
		{
			savefile->Error( "idEvent::Restore: unknown event '%s'", name.c_str() );
			return;
		}
		
		// read the classtype
		savefile->ReadString( name );
		event->typeinfo = idClass::GetClass( name );
		if( event->typeinfo == NULL )
		{
			savefile->Error( "idEvent::Restore: unknown class '%s' on event '%s'", name.c_str(), event->eventdef->GetName() );
			return;
		}
		
		savefile->ReadObject( event->object );
		
		// read the args
		savefile->ReadInt( argsize );
#ifdef IS64BIT
		// Carl: loading 32 bit file on 64 bit
		if( argsize == event->eventdef->argsize_32 )
			convertFrom32Bit = true;
		else
#else
		// Carl: loading 64 bit file on 32 bit
		if( argsize == event->eventdef->argsize_64 )
			convertFrom64Bit = true;
		else
#endif
		if( argsize != (int)event->eventdef->GetArgSize() )
		{
			savefile->Error( "idEvent::Restore(2): arg size (%d) doesn't match saved arg size(%d) on event '%s'", (int)event->eventdef->GetArgSize(), (int)argsize, event->eventdef->GetName() );
		}
		if( argsize )
		{
			event->data = eventDataAllocator.Alloc( (int)event->eventdef->GetArgSize() );
#ifdef IS64BIT
			if( convertFrom32Bit )
			{
				// Normally, we just read the entire parameter list straight into memory, which means everything is little-endian,
				// which is why we use raw Read(ptr, size) methods.
				format = event->eventdef->GetArgFormat();
				for( j = 0, size = 0; j < event->eventdef->GetNumArgs(); ++j )
				{
					dataPtr = &event->data[event->eventdef->GetArgOffset( j )];
					switch( format[j] )
					{
					case D_EVENT_FLOAT:
					case D_EVENT_INTEGER:
					case D_EVENT_ENTITY:
					case D_EVENT_ENTITY_NULL:
						savefile->Read( reinterpret_cast<void*>(dataPtr), 4 );
						break;
					case D_EVENT_VECTOR:
						savefile->Read( reinterpret_cast<void*>(dataPtr), 12 );
						break;
					case D_EVENT_STRING:
						savefile->Read( reinterpret_cast<void*>(dataPtr), 128 );
						break;
					case D_EVENT_TRACE:
					{
						savefile->ReadBool( *reinterpret_cast<bool*>(dataPtr) );
						trace_t& t = *reinterpret_cast<trace_t*>(dataPtr + sizeof( bool ));
						memset( &t, 0, sizeof( t ) );
						savefile->Read( reinterpret_cast<void*>(&t), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.endpos), 3 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.endAxis), 9 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.point), 3 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.normal), 3 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.dist), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.contents), 4 );
						// Carl: We are loading a 4-byte pointer into an 8-byte slot, but we only care if it's NULL or not.
						// The fast event queue never uses the material pointer directly, it just checks if it's NULL.
						savefile->Read( reinterpret_cast<void*>(&t.c.material), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.modelFeature), 4 * 4 );
						savefile->Read( reinterpret_cast<void*>(dataPtr + 1 + 120), 128 );
						break;
					}
					default:
						break;
					}
				}

			}
#else
			if( convertFrom64Bit )
			{
				int32 temp;
				format = event->eventdef->GetArgFormat();
				for( j = 0, size = 0; j < event->eventdef->GetNumArgs(); ++j )
				{
					dataPtr = &event->data[event->eventdef->GetArgOffset( j )];
					switch( format[j] )
					{
					case D_EVENT_FLOAT:
					case D_EVENT_INTEGER:
					case D_EVENT_ENTITY:
					case D_EVENT_ENTITY_NULL:
						savefile->Read( reinterpret_cast<void*>(dataPtr), 4 );
						savefile->ReadInt( temp );
						break;
					case D_EVENT_VECTOR:
						savefile->Read( reinterpret_cast<void*>(dataPtr), 12 );
						savefile->ReadInt( temp );
						break;
					case D_EVENT_STRING:
						savefile->Read( reinterpret_cast<void*>(dataPtr), 128 );
						break;
					case D_EVENT_TRACE:
					{
						savefile->ReadBool( *reinterpret_cast<bool*>(dataPtr) );
						trace_t& t = *reinterpret_cast<trace_t*>(dataPtr + sizeof( bool ));
						memset( &t, 0, sizeof( t ) );
						savefile->Read( reinterpret_cast<void*>(&t), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.endpos), 3 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.endAxis), 9 * 4 );
						int32 padding;
						savefile->ReadInt( padding ); // There were 4 bytes of padding here on 64 bit
						savefile->Read( reinterpret_cast<void*>(&t.c), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.point), 3 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.normal), 3 * 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.dist), 4 );
						savefile->Read( reinterpret_cast<void*>(&t.c.contents), 4 );
						savefile->ReadInt( padding ); // t.c.contents was 8 bytes long on 64 bit
						// Carl: In the fast event queue, material is NOT a real pointer.
						// We only care whether the whole 8 saved bytes of material are NULL, which means our 4-byte pointer must be NULL,
						// or if either part of the 8 bytes is not null, then our 4-byte pointer must be not NULL.
						savefile->Read( reinterpret_cast<void*>(&t.c.material), 4 );
						if( t.c.material == NULL )
							savefile->Read( reinterpret_cast<void*>(&t.c.material), 4 ); // overwrite our NULL in case the other half isn't NULL
						else
							savefile->ReadInt( padding ); // Don't overwrite our non-null 4-bytes with something that might be NULL
						savefile->Read( reinterpret_cast<void*>(&t.c.modelFeature), 4 * 4 );
						savefile->Read( reinterpret_cast<void*>(dataPtr + 1 + 108), 128 );
						break;
					}
						break;
					default:
						break;
					}
				}
			}
#endif
			else
			{
				savefile->Read( event->data, argsize );
			}
		}
		else
		{
			event->data = NULL;
		}
	}
}

/*
 ================
 idEvent::ReadTrace

 idRestoreGame has a ReadTrace procedure, but unfortunately idEvent wants the material
 string name at the of the data structure rather than in the middle
 ================
 */
void idEvent::RestoreTrace( idRestoreGame* savefile, trace_t& trace )
{
	savefile->ReadFloat( trace.fraction );
	savefile->ReadVec3( trace.endpos );
	savefile->ReadMat3( trace.endAxis );
	savefile->ReadInt( ( int& )trace.c.type );
	savefile->ReadVec3( trace.c.point );
	savefile->ReadVec3( trace.c.normal );
	savefile->ReadFloat( trace.c.dist );
	savefile->ReadInt( trace.c.contents );
	savefile->ReadInt( ( int& )trace.c.material );
	savefile->ReadInt( trace.c.contents );
	savefile->ReadInt( trace.c.modelFeature );
	savefile->ReadInt( trace.c.trmFeature );
	savefile->ReadInt( trace.c.id );
}

/*
 ================
 idEvent::WriteTrace

 idSaveGame has a WriteTrace procedure, but unfortunately idEvent wants the material
 string name at the of the data structure rather than in the middle
================
 */
void idEvent::SaveTrace( idSaveGame* savefile, const trace_t& trace )
{
	savefile->WriteFloat( trace.fraction );
	savefile->WriteVec3( trace.endpos );
	savefile->WriteMat3( trace.endAxis );
	savefile->WriteInt( trace.c.type );
	savefile->WriteVec3( trace.c.point );
	savefile->WriteVec3( trace.c.normal );
	savefile->WriteFloat( trace.c.dist );
	savefile->WriteInt( trace.c.contents );
	savefile->WriteInt( ( int& )trace.c.material );
	savefile->WriteInt( trace.c.contents );
	savefile->WriteInt( trace.c.modelFeature );
	savefile->WriteInt( trace.c.trmFeature );
	savefile->WriteInt( trace.c.id );
}



#ifdef CREATE_EVENT_CODE
/*
================
CreateEventCallbackHandler
================
*/
void CreateEventCallbackHandler()
{
	int num;
	int count;
	int i, j, k;
	char argString[ D_EVENT_MAXARGS + 1 ];
	idStr string1;
	idStr string2;
	idFile* file;
	
	file = fileSystem->OpenFileWrite( "Callbacks.cpp" );
	
	file->Printf( "// generated file - see CREATE_EVENT_CODE\n\n" );
	
	for( i = 1; i <= D_EVENT_MAXARGS; i++ )
	{
	
		file->Printf( "\t/*******************************************************\n\n\t\t%d args\n\n\t*******************************************************/\n\n", i );
		
		for( j = 0; j < ( 1 << i ); j++ )
		{
			for( k = 0; k < i; k++ )
			{
				argString[ k ] = j & ( 1 << k ) ? 'f' : 'i';
			}
			argString[ i ] = '\0';
			
			string1.Empty();
			string2.Empty();
			
			for( k = 0; k < i; k++ )
			{
				if( j & ( 1 << k ) )
				{
					string1 += "const float";
					string2 += va( "*( float * )&data[ %d ]", k );
				}
				else
				{
					// RB: 64 bit fix, changed int to intptr_t
					string1 += "const intptr_t";
					// RB end
					string2 += va( "data[ %d ]", k );
				}
				
				if( k < i - 1 )
				{
					string1 += ", ";
					string2 += ", ";
				}
			}
			
			file->Printf( "\tcase %d :\n\t\ttypedef void ( idClass::*eventCallback_%s_t )( %s );\n", ( 1 << ( i + D_EVENT_MAXARGS ) ) + j, argString, string1.c_str() );
			file->Printf( "\t\t( this->*( eventCallback_%s_t )callback )( %s );\n\t\tbreak;\n\n", argString, string2.c_str() );
			
		}
	}
	
	fileSystem->CloseFile( file );
}

#endif
