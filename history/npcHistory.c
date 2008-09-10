#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "dndconst.h"
#include "npcEngine.h"
#include "npcHistory.h"

grGRAMMAR* openNPCMotivationGrammar( char* path ) {
  char cwd[512];
  grGRAMMAR* g;

  getcwd( cwd, sizeof( cwd ) );

  chdir( path );
  g = grLoadGrammar( "npcbackground.txt" );
  chdir( cwd );

  return g;
}


void getNPCMotivation( grGRAMMAR* grammar, NPC* npc, char* buffer, int size ) {
  int   valid;
  char* s;
  char* p;
  int   alignments;

  grammar->startSymbol = "[motivation]";

  do {
    valid = 1;
    grBuildPhrase( grammar, buffer, size );
    s = strchr( buffer, '|' );
    if( s ) {
      alignments = 0;
      *s = 0;
      p = buffer;
      while( p != s ) {
        switch( *p ) {
          case 'l': alignments |= alLAWFUL; break;
          case 'N': alignments |= alLCNEUTRAL; break;
          case 'c': alignments |= alCHAOTIC; break;
          case 'g': alignments |= alGOOD; break;
          case 'n': alignments |= alGENEUTRAL; break;
          case 'e': alignments |= alEVIL; break;
        }
        p++;
      }
      if( ( npc->alignment & alignments ) != 0 ) {
        valid = 0;
      } else {
        s++;
        memmove( buffer, s, strlen( s ) + 1 );
      }
    }
  } while( !valid );
}


void getNPCRecentPast( grGRAMMAR* grammar, NPC* npc, char* buffer, int size ) {
  int   valid;
  char* s;
  char* p;
  int   alignments;

  grammar->startSymbol = "[recentpast]";

  do {
    valid = 1;
    grBuildPhrase( grammar, buffer, size );
  } while( !valid );
}
