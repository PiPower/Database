#ifndef ENTRY
#define ENTRY
#include <inttypes.h>

//bits 0-13 are used for offset
//bit 14 is used as dead-alive flag
//bits 15 unused
typedef uint16_t EntryDescriptor;


inline void SET_ALIVE(EntryDescriptor& value)
{
    value |= (uint16_t)(1 << 14);
}

inline void SET_DEAD(EntryDescriptor& value)
{
    value &= (uint16_t)~(1 << 14);
}

inline void SET_OFFSET(EntryDescriptor& value, uint16_t& offset)
{
    value = ( value & 0xc000) | (0x3FFF & offset) ;
}


inline bool IS_ALIVE(EntryDescriptor& value)
{
    return (value & (1 << 14) ) > 0;
}

inline bool IS_DEAD(EntryDescriptor& value)
{
    return (value & (1 << 14) ) == 0;
}

inline uint16_t GET_OFFSET(EntryDescriptor& value)
{
   return  value & 0x3FFF;
}
#endif