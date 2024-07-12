#ifndef ENTRY
#define ENTRY
#include <inttypes.h>

//bits 0-12 are used for offset
//bit 13 is used as dead-alive flag
//bits 14-15 unused
typedef uint16_t EntryDescriptor;


inline void SET_ALIVE(EntryDescriptor& value)
{
    value |= (uint16_t)0x2000;
}

inline void SET_DEAD(EntryDescriptor& value)
{
    value &= (uint16_t)~0x2000;
}

inline void SET_OFFSET(EntryDescriptor& value, uint16_t& offset)
{
   value |= 0x1FFF & offset ;
}


inline bool IS_ALIVE(EntryDescriptor& value)
{
    return (value & 0x2000 ) > 0;
}

inline bool IS_DEAD(EntryDescriptor& value)
{
    return (value & 0x2000 ) == 0;
}

inline uint16_t GET_OFFSET(EntryDescriptor& value)
{
   return  value & 0x1FFF;
}
#endif