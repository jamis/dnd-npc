#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "npcEngine.h"
#include "dndconst.h"
#include "dndutil.h"

#include "pcgen_interface.h"


static int writeFeat( FILE* output, NPCFEAT* feats, int type );
static int writeSpells( FILE* output, NPCCLASS* cls );
static int computeMinimumXP( NPC* npc );
static int getPCGenAlignmentCode( NPC* npc );

static int mixCase( char* to, char* from );


static int mixCase( char* to, char* from ) {
  int i;
  int upit;

  strcpy( to, from );

  upit = 1;
  for( i = 0; to[i] != 0; i++ ) {
    if( upit && isalpha( to[i] ) ) {
      to[i] = toupper( to[i] );
      upit = 0;
    }
    upit = ( !isalpha( to[i] ) && to[i] != '\'' );
  }

  return 0;
}


static int writeFeat( FILE* output, NPCFEAT* feats, int type ) {
  NPCFEAT*       feat;
  NPCFEATWEAPON* nfw;
  NPCFEATSKILL*  nfs;
  NPCFEATSCHOOL* nfsch;
  NPCFEATSPELLS* nfsp;
  int            count;
  int            i;
  char           buffer[ 128 ];

  count = 0;
  for( feat = feats; feat != 0; feat = feat->next ) {
    if( feat->type == type ) {
      count++;
    }
  }

  if( count < 1 ) {
    return 0;
  }

  mixCase( buffer, dndGetFeatName( type ) );
  fprintf( output, "%s:%d:", buffer, count );

  for( feat = feats; feat != 0; feat = feat->next ) {
    if( feat->type == type ) {
      switch( feat->type ) {
        case ftSPELLMASTERY:
          nfsp = feat->data;
          for( i = 0; i < 3; i++ ) {
            mixCase( buffer, dndGetSpellName( nfsp->spells[ i ] ) );
            fprintf( output, "%s:", buffer );
          }
          break;
        case ftSPELLFOCUS:
          nfsch = feat->data;
          mixCase( buffer, dndGetSchoolOfMagicName( nfsch->school ) );
          fprintf( output, "%s:", buffer );
          break;
        case ftWEAPONPROFICIENCY_SIMPLE:
        case ftWEAPONPROFICIENCY_MARTIAL:
        case ftWEAPONPROFICIENCY_EXOTIC:
        case ftIMPROVEDCRITICAL:
        case ftWEAPONFOCUS:
        case ftWEAPONFINESSE:
        case ftWEAPONSPECIALIZATION:
          nfw = feat->data;
          mixCase( buffer, dndGetWeaponName( nfw->weapon ) );
          fprintf( output, "%s:", buffer );
          break;
        case ftSKILLFOCUS:
          nfs = feat->data;
          mixCase( buffer, dndGetSkillName( nfs->skill ) );
          fprintf( output, "%s:", buffer );
          break;
      }
    }
  }

  return 0;
}


static int writeSpells( FILE *output, NPCCLASS* cls ) {
  NPCSPELL** spells;
  NPCSPELL*  spell;
  int i;
  char buffer[ 128 ];

  switch( cls->type ) {
    case pcWIZARD:   spells = ((NPCWIZARDDATA*)cls->data)->spells; break;
    case pcBARD:     spells = ((NPCBARDDATA*)cls->data)->spells; break;
    case pcSORCERER: spells = ((NPCSORCERERDATA*)cls->data)->spells; break;
    default:
      spells = 0;
  }

  if( spells != 0 ) {
    for( i = 0; i < 10; i++ ) {
      for( spell = spells[i]; spell != 0; spell = spell->next ) {
        mixCase( buffer, dndGetSpellName( spell->spell ) );
        fprintf( output, "%s|Known Spells|1:", buffer );
      }
    }
  }

  fprintf( output, "\n" );

  if( cls->type == pcCLERIC) {
    fprintf( output, "\n" ); /* for Domain spells */
  }

  return 0;
}


static int computeMinimumXP( NPC* npc ) {
  NPCCLASS* cls;
  int       i;
  int       xp;

  xp = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    for( i = 0; i < cls->level; i++ ) {
      xp += ( cls->level - 1 ) * 1000;
    }
  }

  return xp;
}


static int getPCGenAlignmentCode( NPC* npc ) {
  int code = 0;

  if( npc->alignment & alLCNEUTRAL ) {
    code = 3;
  } else if( npc->alignment & alCHAOTIC ) {
    code = 6;
  }

  if( npc->alignment & alGENEUTRAL ) {
    code++;
  } else if( npc->alignment & alEVIL ) {
    code += 2;
  }

  return code;
}


int convertToPCGen( NPC* npc, FILE* output ) {
  NPCCLASS* cls;
  int       i;
  int       hp_per_lev;
  int       hp_extra;
  NPCFEAT*  feat;
  NPCSKILL* skill;
  NPCLANGUAGE* language;
  int       is_literate;
  char      buffer[ 128 ];
  NPCCLERICDATA* clericData;

  /* CAMPAIGNS line */
  fprintf( output, "WotC - DnD Core Rules I - Players Hand Book:WotC - DnD Core Rules II - Dungeon Masters Guide:WotC - DnD Core Rules III - Monster Manual:\n" );

  /* VERSION line */
  fprintf( output, "VERSION:3.0.0\n" );

  /* PC name */
  fprintf( output, "%s:\n", npc->name );

  /* ability scores */
  fprintf( output, "%d:%d:%d:%d:%d:%d:0:0\n",
          npc->strength - dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abSTRENGTH ),
          npc->dexterity - dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abDEXTERITY ),
          npc->constitution - dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abCONSTITUTION ),
          npc->intelligence - dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abINTELLIGENCE ),
          npc->wisdom - dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abWISDOM ),
          npc->charisma - dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abCHARISMA ) );
          
  /* classes */
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    mixCase( buffer, dndGetClassName( cls->type ) );
    fprintf( output, "%s:None :None :%d:", buffer, cls->level );
    hp_per_lev = cls->hp / cls->level;
    hp_extra = cls->hp % cls->level;
    for( i = 0; i < cls->level; i++ ) {
      fprintf( output, "%d:", hp_per_lev + ( hp_extra > 0 ? 1 : 0 ) );
      hp_extra--;
    }
    fprintf( output, "0:" );
    switch( cls->type ) {
      case pcWIZARD:   fprintf( output, "INT:" ); break;
      case pcBARD:
      case pcSORCERER: fprintf( output, "CHA:" ); break;
      case npcADEPT:
      case pcDRUID:
      case pcCLERIC:
      case pcRANGER:   fprintf( output, "WIS:" ); break;
    }
  }
  fprintf( output, "\n" );

  for( feat = npc->feats; feat != 0; feat = feat->next ) {
    switch( feat->type ) {
      case ftSPELLMASTERY:
      case ftSPELLFOCUS:
      case ftWEAPONPROFICIENCY_SIMPLE:
      case ftWEAPONPROFICIENCY_MARTIAL:
      case ftWEAPONPROFICIENCY_EXOTIC:
      case ftIMPROVEDCRITICAL:
      case ftWEAPONFOCUS:
      case ftWEAPONFINESSE:
      case ftWEAPONSPECIALIZATION:
      case ftSKILLFOCUS:
        continue;
    }
    mixCase( buffer, dndGetFeatName( feat->type ) );
    fprintf( output, "%s:0:", buffer );
  }
  writeFeat( output, npc->feats, ftSPELLMASTERY );
  writeFeat( output, npc->feats, ftSPELLFOCUS );
  writeFeat( output, npc->feats, ftWEAPONPROFICIENCY_SIMPLE );
  writeFeat( output, npc->feats, ftWEAPONPROFICIENCY_MARTIAL );
  writeFeat( output, npc->feats, ftWEAPONPROFICIENCY_EXOTIC );
  writeFeat( output, npc->feats, ftIMPROVEDCRITICAL );
  writeFeat( output, npc->feats, ftWEAPONFOCUS );
  writeFeat( output, npc->feats, ftWEAPONFINESSE );
  writeFeat( output, npc->feats, ftWEAPONSPECIALIZATION );
  writeFeat( output, npc->feats, ftSKILLFOCUS );
  fprintf( output, "\n" );

  for( skill = npc->skills; skill != 0; skill = skill->next ) {
    mixCase( buffer, dndGetSkillName( skill->type ) );
    fprintf( output, "%s:%g:", buffer, skill->rank );
  }
  fprintf( output, "\n" );

  /* Deity, and domains (for clerics) */
  fprintf( output, "None:" );
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    if( cls->type == pcCLERIC ) {
      clericData = (NPCCLERICDATA*)cls->data;
      mixCase( buffer, dndGetDomainName( clericData->domain[0] ) );
      fprintf( output, "%s:", buffer );
      mixCase( buffer, dndGetDomainName( clericData->domain[1] ) );
      fprintf( output, "%s:", buffer );
      break;
    }
  }
  fprintf( output, "\n" );

  switch( npc->race ) {
    case rcELF_HIGH: strcpy( buffer, "Elf" ); break;
    case rcDWARF_HILL: strcpy( buffer, "Dwarf" ); break;
    case rcGNOME_ROCK: strcpy( buffer, "Gnome" ); break;
    case rcHALFLING_LIGHTFOOT: strcpy( buffer, "Halfling" ); break;
    default:
      mixCase( buffer, dndGetRaceName( npc->race ) );
  }
  fprintf( output, "%s:%d:%d:%d:%d:%c:Right\n",
          buffer, getPCGenAlignmentCode( npc ), npc->height_ft * 12 + npc->height_in,
          npc->weight, npc->age, ( npc->gender == gMALE ? 'M' : 'F' ) );

  is_literate = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    writeSpells( output, cls );
    is_literate = ( is_literate || ( cls->type != pcBARBARIAN ) );
  }

  for( language = npc->languages; language != 0; language = language->next ) {
    mixCase( buffer, dndGetLanguageName( language->language ) );
    fprintf( output, "%s:", buffer );
  }
  if( is_literate ) fprintf( output, "Literacy:" );
  fprintf( output, "\n" );

  fprintf( output, "\n" ); /* proficient weapons -- unimplemented */
  fprintf( output, "0:0:\n" );
  fprintf( output, " : : : : : : : : : : : :\n" );
  fprintf( output, "\n" );
  fprintf( output, "%d:  :  :\n", npcGearValue( npc ) );

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    mixCase( buffer, dndGetClassName( cls->type ) );
    fprintf( output, "%s:\n", buffer );
    if( cls->type == pcCLERIC ) {
      fprintf( output, "Domain:\n" );
    }
  }
  fprintf( output, "%d: : : :0:0:\n", computeMinimumXP( npc ) );
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    mixCase( buffer, dndGetClassName( cls->type ) );
    fprintf( output, "%s:\n", buffer );
    if( cls->type == pcCLERIC ) {
      fprintf( output, "Domain:\n" );
    }
  }
  fprintf( output, "TEMPLATE:" );

  return 0;
}



