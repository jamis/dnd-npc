#ifndef __NPCHISTORY_H__
#define __NPCHISTORY_H__

#include "grammar.h"
#include "npcEngine.h"

grGRAMMAR* openNPCMotivationGrammar( char* path );
void getNPCMotivation( grGRAMMAR* grammar, NPC* npc, char* buffer, int size );
void getNPCRecentPast( grGRAMMAR* grammar, NPC* npc, char* buffer, int size );

#endif /* __NPCHISTORY_H__ */
