#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "gameutil.h"
#include "npcEngine.h"
#include "dndconst.h"
#include "dndutil.h"
#include "grammar.h"
#include "npcHistory.h"
#include "pcgen_interface.h"

#include "templates.h"
#include "extensions.h"
#include "qDecoder.h"

#define TEMPATH "tem/"

#ifdef WIN32
#define CGINAME     "npc2.exe"
#else
#define CGINAME     "npc2.cgi"
#endif

static char url[512];

static struct {
  char* name;
  int   id;
} racetypes[] = {
  { "any",        raceANY },
  { "anycore",    raceANY_CORE },
  { "anydmg",     raceANY_DMG },
  { "anymm",      raceANY_MM },
  { "anycc",      raceANY_CC },
  { "dwarf",      rcDWARF_HILL },
  { "elf",        rcELF_HIGH },
  { "gnome",      rcGNOME_ROCK },
  { "halfelf",    rcHALFELF },
  { "halfling",   rcHALFLING_LIGHTFOOT },
  { "halforc",    rcHALFORC },
  { "human",      rcHUMAN },
  { "aasimar",    rcAASIMAR },
  { "aranea",     rcARANEA },
  { "azer",       rcAZER },
  { "bugbear",    rcBUGBEAR },
  { "centaur",    rcCENTAUR },
  { "dopple",     rcDOPPLEGANGER },
  { "drider",     rcDRIDER },
  { "deepdwarf",  rcDWARF_DEEP },
  { "derro",      rcDWARF_DERRO },
  { "duergar",    rcDWARF_DUERGAR },
  { "mtndwarf",   rcDWARF_MOUNTAIN },
  { "aqelf",      rcELF_AQUATIC },
  { "drow",       rcELF_DROW },
  { "grayelf",    rcELF_GRAY },
  { "wildelf",    rcELF_WILD },
  { "woodelf",    rcELF_WOOD },
  { "ettin",      rcETTIN },
  { "hillg",      rcGIANT_HILL },
  { "stoneg",     rcGIANT_STONE },
  { "frostg",     rcGIANT_FROST },
  { "fireg",      rcGIANT_FIRE },
  { "cloudg",     rcGIANT_CLOUD },
  { "stormg",     rcGIANT_STORM },
  { "gnoll",      rcGNOLL },
  { "frstgnome",  rcGNOME_FOREST },
  { "svirf",      rcGNOME_SVIRFNEBLIN },
  { "goblin",     rcGOBLIN },
  { "grimlock",   rcGRIMLOCK },
  { "hags",       rcHAG_SEA },
  { "haga",       rcHAG_ANNIS },
  { "hagg",       rcHAG_GREEN },
  { "deephalf",   rcHALFLING_DEEP },
  { "tallflw",    rcHALFLING_TALLFELLOW },
  { "harpy",      rcHARPY },
  { "hobgob",     rcHOBGOBLIN },
  { "kobold",     rcKOBOLD },
  { "kuotoa",     rcKUOTOA },
  { "lizard",     rcLIZARDFOLK },
  { "locathah",   rcLOCATHAH },
  { "medusa",     rcMEDUSA },
  { "mindflayer", rcMINDFLAYER },
  { "minotaur",   rcMINOTAUR },
  { "ogre",       rcOGRE },
  { "ogremage",   rcOGREMAGE },
  { "orc",        rcORC },
  { "sahuagin",   rcSAHUAGIN },
  { "tiefling",   rcTIEFLING },
  { "troglodyte", rcTROGLODYTE },
  { "troll",      rcTROLL },
  { "yuantip",    rcYUANTI_PUREBLOOD },
  { "yuantih",    rcYUANTI_HALFBLOOD },
  { "yuantia",    rcYUANTI_ABOMINATION },
  { "abandoned",  rcCC_ABANDONED },
  { "asaath",     rcCC_ASAATH },
  { "batdevil",   rcCC_BATDEVIL },
  { "pwretch",    rcCC_PLAGUEWRETCH },
  { "charduni",   rcCC_CHARDUNI },
  { "cgoblin",    rcCC_COALGOBLIN },
  { "fdwarf",     rcCC_DWARF_FORSAKEN },
  { "felf",       rcCC_ELF_FORSAKEN },
  { "iceghoul",   rcCC_ICE_GHOUL },
  { "mcora",      rcCC_MANTICORA },
  { "morgaunt",   rcCC_MORGAUNT },
  { "proud",      rcCC_PROUD },
  { "ratman",     rcCC_RATMAN },
  { "bgratman",   rcCC_RATMAN_BROWNGORGER },
  { "dratman",    rcCC_RATMAN_DISEASED },
  { "fratman",    rcCC_RATMAN_FOAMER },
  { "rwratman",   rcCC_RATMAN_REDWITCH },
  { "skindev",    rcCC_SKINDEVIL },
  { "segoblin",   rcCC_SPIDEREYEGOBLIN },
  { "stroll",     rcCC_STEPPETROLL },
  { "selement",   rcCC_STRIFEELEMENTAL },
  { "tokal",      rcCC_TOKALTRIBESMAN },
  { "ubantu",     rcCC_UBANTUTRIBESMAN },
  { 0,            0 }
};


typedef struct __lf__ LOOKANDFEEL;

struct __lf__ {
  char* name;
  char* main;
  char* config;
  char* printable;
  char* longdesc;
};

LOOKANDFEEL lookAndFeel[] = {
  { "standard", TEMPATH "npc2_main.tem", TEMPATH "npc2_statconfig.tem", TEMPATH "npc2_printable.tem", TEMPATH "npc2_longdesc.tem" },
  { 0, 0, 0, 0, 0 }
};


/* logs access to the CGI */
void logAccess( long seed, char* alignment, char* gender, char* race, char* cls, char* level, char* cls2, char* level2, char* cls3, char* level3, int count ) {
#ifdef USE_LOG
  FILE *f;
  char buffer[512];
  time_t t;
  char *host;

  f = fopen( LOGLOCATION, "at" );
  if( f == NULL ) {
    printf( "content-type: text/html\r\n\r\n" );
    printf( "could not open log file: %d (%s)\n", errno, strerror( errno ) );
    return;
  }

  t = time( NULL );
  strftime( buffer, sizeof( buffer ), "[%d %b %Y %H:%M:%S]", localtime( &t ) );

  host = getenv( "REMOTE_ADDR" );
  fprintf( f, "%s %s ", buffer, host );

  if( alignment == 0 ) {
    fprintf( f, "start\n" );
  } else {
    fprintf( f, "%s:%s:%s:%s:%s:%s:%s:%s:%s:%d (%d)\n", 
        alignment, gender, race, cls, level, cls2, level2, cls3, level3, count,
        seed );
  }

  fclose( f );
#endif
}


int getCounter() {
#ifdef USE_COUNTER
  FILE *f;
  char  buffer[32];
  int   count;

  f = fopen( CTRLOCATION, "r+t" );
  if( f == NULL ) {
    f = fopen( CTRLOCATION, "w+t" );
    if( f == NULL ) {
      return 0;
    }
  }

  fgets( buffer, sizeof( buffer ), f );
  count = atoi( buffer ) + 1;
  rewind( f );
  fprintf( f, "%d", count );
  fclose( f );

  return count;
#else
  return -1;
#endif
}


LOOKANDFEEL* getLookAndFeel( char* name ) {
  int i;

  for( i = 0; lookAndFeel[ i ].name != 0; i++ ) {
    if( strcmp( name, lookAndFeel[ i ].name ) == 0 ) {
      return &(lookAndFeel[i]);
    }
  }

  return &(lookAndFeel[0]);
}


static int icomp( const void* x, const void* y ) {
  int ix = *(int*)x;
  int iy = *(int*)y;

  if( ix < iy ) return 1;
  if( ix > iy ) return -1;
  return 0;
}


void heroicAbilities( int* scores, void* data ) {
  int i;

  npcAbScoreStrategy_BestOf4d6( scores, data );
  scores[0] = 18;
  scores[1] = 18;

  for( i = 2; i < 6; i++ ) {
    if( scores[i] < 12 ) {
      scores[ i ] = 12;
    }
  }
}


void straight18s( int* scores, void* data ) {
  int i;

  for( i = 0; i < 6; i++ ) {
    scores[ i ] = 18;
  }
}


void averageAbilities( int* scores, void* data ) {
  int i;

  for( i = 0; i < 6; i++ ) {
    scores[i] = 10 + ( rand() % 2 );
  }
}


NPCABSCOREGENFUNC strategies[] = {
  0, 
  npcAbScoreStrategy_BestOf4d6, 
  npcAbScoreStrategy_Straight3d6,
  heroicAbilities,
  averageAbilities,
  straight18s
};

static void strrepl( char* buf, char* srch, char* repl ) {
  char* p;
  int   slen;
  int   rlen;

  slen = strlen( srch );
  rlen = strlen( repl );

  p = strstr( buf, srch );
  while( p != NULL ) {
    memmove( p+rlen, p+slen, strlen( p + slen ) + 1 );
    strncpy( p, repl, rlen );
    p = strstr( p+rlen, srch );
  }
}


typedef struct {
  STANDARD_TAG_HDR;
  int (*func_ptr)( void* );
  void* parm;
} t_ae_local_fn_tag;


static int static_ae_local_fn_apply( t_ae_tag tag,
                                     CONST char* text,
                                     t_ae_template_mgr mgr,
                                     FILE* output )
{
  t_ae_generic_tag *tag_data = (t_ae_generic_tag*)tag;

  if( strcmp( text, tag_data->m_tag ) != 0 )
    return 0;

  return tag_data->process( tag, text, mgr, output );
}


static int static_ae_local_fn_process( t_ae_tag tag,
                                       CONST char* text,
                                       t_ae_template_mgr mgr,
                                       FILE* output )
{
  t_ae_local_fn_tag *tag_data = (t_ae_local_fn_tag*)tag;
  tag_data->func_ptr( tag_data->parm );
  return 0;
}


t_ae_tag ae_local_fn_tag( CONST char* name, int (*func_ptr)(void*), void* parm ) {
  t_ae_local_fn_tag* tag;

  tag = (t_ae_local_fn_tag*)ae_tag_new( name, sizeof( t_ae_local_fn_tag ) );
  tag->func_ptr = func_ptr;
  tag->parm = parm;
  tag->process = static_ae_local_fn_process;
  tag->apply = static_ae_local_fn_apply;

  return (t_ae_tag)tag;
}


int displayFeats( void* data ) {
  NPCFEAT*  feat;
  char      buffer[100];
  NPCFEATWEAPON* nfw;
  NPCFEATSKILL* nfs;
  NPCFEATSCHOOL* nfsch;
  NPCFEATSPELLS* nfsp;
  int i;

  printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n" );
  printf( "<UL>\n" );

  feat = (NPCFEAT*)data;
  while( feat != 0 ) {
    strcpy( buffer, dndGetFeatName( feat->type ) );
    buffer[0] = toupper( buffer[0] );

    if( feat->autoAdd ) {
      printf( "<LI><I>%s", buffer );
    } else {
      printf( "<LI>%s", buffer );
    }

    switch( feat->type ) {
      case ftSPELLMASTERY:
        nfsp = feat->data;
        printf( " (" );
        for( i = 0; i < 3; i++ ) {
          printf( "%s%s", 
                    ( i > 0 ? ", " : "" ),
                    dndGetSpellName( nfsp->spells[ i ] ) );
        }
        printf( ")" );
        break;
      case ftSPELLFOCUS:
        nfsch = feat->data;
        printf( " (%s)", dndGetSchoolOfMagicName( nfsch->school ) );
        break;
      case ftWEAPONPROFICIENCY_SIMPLE:
      case ftWEAPONPROFICIENCY_MARTIAL:
      case ftWEAPONPROFICIENCY_EXOTIC:
      case ftIMPROVEDCRITICAL:
      case ftWEAPONFOCUS:
      case ftWEAPONSPECIALIZATION:
        nfw = feat->data;
        printf( " (%s)", dndGetWeaponName( nfw->weapon ) );
        break;
      case ftSKILLFOCUS:
        nfs = feat->data;
        printf( " (%s)", dndGetSkillName( nfs->skill ) );
        break;
    }

    if( feat->autoAdd ) {
      printf( "</I>\n" );
    } else {
      printf( "\n" );
    }

    feat = feat->next;
  }

  printf( "</UL>\n" );
  printf( "</TD></TR>\n" );

  return 0;
}


int displayClasses( void* data ) {
  NPCCLASS* cls;
  char      buffer[100];
  int       totalLevel;

  printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n" );
  printf( "<UL>\n" );

  cls = (NPCCLASS*)data;
  while( cls != 0 ) {
    strcpy( buffer, dndGetClassName( cls->type ) );
    buffer[0] = toupper( buffer[0] );

    printf( "<LI>%s %d\n", buffer, cls->level );

    cls = cls->next;
  }

  printf( "</UL>\n" );
  printf( "</TD></TR>\n" );

  return 0;
}


int displaySkillPoints( void* data ) {
  NPC*      npc;
  NPCCLASS* cls;
  char      buffer[512];

  npc = (NPC*)data;

  printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n" );

  printf( "<TABLE BORDER=0 WIDTH=\"100%\">\n" );
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    char cls_name[100];

    strcpy( cls_name, dndGetClassName( cls->type ) );
    cls_name[0] = toupper( cls_name[0] );

    printf( "<TR><TD CLASS=\"NORMAL\">%s</TD><TD CLASS=\"NORMAL\"><B>%d</B></TD></TR>\n",
              cls_name,
              cls->skill_points );
  }
  printf( "</TABLE>\n" );

  printf( "</TD></TR>\n" );

  return 0;
}

int displaySkills( void* data ) {
  NPCSKILL* skill;
  NPC*      npc;
  char      buffer[512];

  npc = (NPC*)data;

  printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n" );

  printf( "<TABLE BORDER=0 WIDTH=\"100%\">\n" );
  for( skill = npc->skills; skill != 0; skill = skill->next ) {
    float total;
    NPCCOMPBREAKDOWN* breakdown;
    char skillName[100];

    strcpy( skillName, dndGetSkillName( skill->type ) );
    skillName[0] = toupper( skillName[0] );

    total = npcComputeActualSkillBonus( npc, skill->type, &breakdown );
    printf( "<TR><TD CLASS=\"NORMAL\">%s</TD><TD CLASS=\"NORMAL\"><B>%+g</B></TD><TD CLASS=\"NORMAL\">%s</TD></TR>\n",
              skillName,
              total,
              npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  }
  printf( "</TABLE>\n" );

  printf( "</TD></TR>\n" );

  return 0;
}


int displaySpells( void* data ) {
  NPC*    npc;
  NPCCLASS* cls;
  NPCSPELL** list;
  NPCSPELL*  spell;
  char buffer[100];
  int i;
  int count;
  int bonus;
  int hasPlus;
  int relevantAbility;
  int total;
  int maxLevel;

  npc = (NPC*)data;

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    list = 0;
    hasPlus = 0;
    switch( cls->type ) {
      case pcBARD:
        list = ((NPCBARDDATA*)cls->data)->spells;
        relevantAbility = npc->charisma;
        break;
      case pcCLERIC:
        hasPlus = 1;
        relevantAbility = npc->wisdom;

        printf( "<TR><TD CLASS=\"DIVIDER\" COLSPAN=2><IMG SRC=\"/img/blank.gif\" WIDTH=1 HEIGHT=1></TD></TR>" );
        printf( "<TR><TD CLASS=\"HEADER\" COLSPAN=2>Cleric Domains:</TD></TR>\n" );
        printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n<UL>\n" );
        for( i = 0; i < 2; i++ ) {
          strcpy( buffer, dndGetDomainName( ((NPCCLERICDATA*)cls->data)->domain[i] ) );
          buffer[ 0 ] = toupper( buffer[ 0 ] );
          printf( "<LI>%s\n", buffer );
        }
        printf( "</UL>\n</TD></TR>\n" );
        break;
      case pcDRUID:
        relevantAbility = npc->wisdom;
        break;
      case pcPALADIN:
        relevantAbility = npc->wisdom;
        break;
      case pcRANGER:
        relevantAbility = npc->wisdom;
        break;
      case pcSORCERER:
        list = ((NPCSORCERERDATA*)cls->data)->spells;
        relevantAbility = npc->charisma;
        break;
      case pcWIZARD:
        list = ((NPCWIZARDDATA*)cls->data)->spells;
        relevantAbility = npc->intelligence;
        break;
      case npcADEPT:
        relevantAbility = npc->wisdom;
        break;
      default:
        continue;
    }

    strcpy( buffer, dndGetClassName( cls->type ) );
    buffer[0] = toupper( buffer[0] );

    printf( "<TR><TD CLASS=\"DIVIDER\" COLSPAN=2><IMG SRC=\"/img/blank.gif\" WIDTH=1 HEIGHT=1></TD></TR>" );
    printf( "<TR><TD CLASS=\"HEADER\" COLSPAN=2>%s Spells Per Day:</TD></TR>\n", buffer );

    if( relevantAbility < 11 ) {
      printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>This character is unable to cast any %s spells, due to an unfortunately low relevant ability score.</TD></TR>\n",
                dndGetClassName( cls->type ) );
      continue;
    }

    total = 0;
    maxLevel = relevantAbility - 10;
    if( maxLevel > 9 ) {
      maxLevel = 9;
    }
    if( maxLevel < 0 ) {
      maxLevel = 0;
    }

    for( i = 0; i <= maxLevel; i++ ) {
      count = dndGetSpellsPerDay( cls->type, cls->level, i );
      if( count < 0 ) {
        continue;
      }

      count += dndGetBonusSpellsPerDay( relevantAbility, i );
      if( count < 1 ) {
        continue;
      }

      total += count;

      printf( "<TR><TD CLASS=\"NORMAL\" ALIGN=\"RIGHT\"><B>%d</B>%s-level:</TD><TD CLASS=\"NORMAL\"><B>%d%s</B> (DC %d where applicable)</TD></TR>\n",
                i, 
                getOrdinalSuffix( i ),
                count, 
                ( ( hasPlus && i > 0 ) ? "+1" : "" ),
                10 + i + dndGetAbilityBonus( relevantAbility ) );

      if( ( hasPlus ) && ( i > 0 ) ) {
        total++;
      }

      if( list != 0 ) {
        printf( "<TR><TD CLASS=\"NORMAL\">&nbsp;</TD><TD CLASS=\"NORMAL\">\n<UL>\n" );
        for( spell = list[ i ]; spell != 0; spell = spell->next ) {
          printf( "<LI>%s\n", dndGetSpellName( spell->spell ) );
        }
        printf( "</UL>\n</TD></TR>\n" );
      }
    }

    if( total > 0 ) {
      printf( "<TR><TD CLASS=\"DATA\" ALIGN=\"RIGHT\">Total:</TD><TD CLASS=\"DATA\">%d</TD></TR>\n", total );
    } else {
      printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>This character is unable to cast any %s spells until they attain a higher level.</TD></TR>\n",
                dndGetClassName( cls->type ) );
    }
  }

  return 0;
}


int displayAttacks( void* data ) {
  NPC* npc;
  int  baseAttack;
  int  attackCount;
  int  attack;
  int  i;
  int  bonus;
  NPCCOMPBREAKDOWN* breakdown;
  char buffer[512];

  npc = (NPC*)data;
  baseAttack = npcGetBaseAttack( npc );

  /* multiple attacks are based on the unmodified base attack */
  attackCount = dndGetClassAttacksPerRound( baseAttack );
  
  /* get the modified attack bonus */
  attack = npcComputeActualAttack( npc, attMELEE, &breakdown ); 

  printf( "<TR><TD CLASS=\"NORMAL\">Melee:</TD><TD CLASS=\"DATA\">" );
  for( i = 1; i <= attackCount; i++ ) {
    if( i > 1 ) {
      printf( "/" );
    }
    bonus = dndGetClassMultipleAttackBonus( attack, i );
    printf( "%+d", bonus );
  }
  printf( " %s</TD></TR>\n",
            npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  if( npcHasClass( npc, pcMONK ) ) {
    int monkAttackCount;
    int monkBaseAttack;
    NPCCLASS* cls;

    cls = npcGetClass( npc, pcMONK );

    monkBaseAttack = dndGetClassAttackBonus( pcMONK, cls->level ) +
                     dndGetRaceBonus( npc->race, npc->gender, rbtATTACK, 0 );
    monkAttackCount = dndGetMonkAttacksPerRound( monkBaseAttack );
    attack = npcComputeActualAttack( npc, attMELEE, &breakdown ); 

    printf( "<TR><TD CLASS=\"NORMAL\">Monk:</TD><TD CLASS=\"DATA\">" );
    for( i = 1; i <= monkAttackCount; i++ ) {
      if( i > 1 ) {
        printf( "/" );
      }

	    if (i <= (monkAttackCount - attackCount)) bonus = attack;
	    else bonus = dndGetClassMultipleAttackBonus( attack, i - (monkAttackCount - attackCount) );

      printf( "%+d", bonus - 2 );
    }
    printf( " %s</TD></TR>\n",
              npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
    npcDestroyComponentBreakdown( breakdown );
  }

  attack = npcComputeActualAttack( npc, attRANGED, &breakdown ); 
  printf( "<TR><TD CLASS=\"NORMAL\">Ranged:</TD><TD CLASS=\"DATA\">" );
  for( i = 1; i <= attackCount; i++ ) {
    if( i > 1 ) {
      printf( "/" );
    }
    bonus = dndGetClassMultipleAttackBonus( attack, i );
    printf( "%+d", bonus );
  }
  printf( " %s</TD></TR>\n",
            npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  return 0;
}


int displayNPC_StatBlock( NPC** npc ) {
  char    statBlock[4096];
  int     count;
  int     i;
  int     doBg;
  NPCSTATBLOCKOPTS opts;
  char    motivation1[ 1024 ];
  char    motivation2[ 1024 ];
  grGRAMMAR* grammar;

  doBg = ( *qValueDefault( "", "background" ) == 'Y' );
  if( doBg ) {
    grammar = openNPCMotivationGrammar( TEMPATH );
  }

  count = qiValue( "count" );
  if( count < 1 ) {
    count = 1;
  }
  if( count > 100 ) {
    count = 100;
  }

  printf( "<DIV CLASS=\"NORMAL\">\n" );

  if( !qiValue( "printable" ) ) {
    printf( "<A HREF=\"%s&printable=1\" TARGET=\"PRINT\">Open printable version</A>\n<P>", url );
  }

  memset( &opts, 0, sizeof( opts ) );

  opts.abilityBonuses = qiValue( "stats_ability_bonuses" );
  opts.acBreakdown = qiValue( "stats_ac_breakdown" );
  opts.initBreakdown = qiValue( "stats_init_breakdown" );
  opts.attackBreakdown = qiValue( "stats_attack_breakdown" );
  opts.saveBreakdown = qiValue( "stats_save_breakdown" );
  opts.skillBreakdown = qiValue( "stats_skill_breakdown" );
  opts.languages = qiValue( "stats_languages" );
  opts.skillsAndFeats = qiValue( "stats_skillsfeats" );
  opts.possessions = qiValue( "stats_possessions" );
  opts.spells = qiValue( "stats_spells" );
  opts.richFormatting = qiValue( "stats_formatting" );
  opts.skill_points = qiValue( "stats_skill_points" );

  for( i = 0; i < count; i++ ) {

    npcBuildStatBlock( npc[i], &opts, statBlock, sizeof( statBlock ) );

    strrepl( statBlock, "~B", "<B>" );
    strrepl( statBlock, "~b", "</B>" );
    strrepl( statBlock, "\n", "<BR>\n" );
    strrepl( statBlock, "~I", "<I>" );
    strrepl( statBlock, "~i", "</I>" );

    printf( statBlock );

    if( doBg ) {
      grammar->startSymbol = "[motivation]";
      getNPCMotivation( grammar, npc[i], motivation1, sizeof( motivation1 ) );
      do {      
        getNPCMotivation( grammar, npc[i], motivation2, sizeof( motivation1 ) );
      } while( strcmp( motivation1, motivation2 ) == 0 );
      printf( "<P><I>Primary motivation:</I> %s\n", motivation1 );
      printf( "<P><I>Secondary motivation:</I> %s\n", motivation2 );
      getNPCRecentPast( grammar, npc[i], motivation1, sizeof( motivation1 ) );
      printf( "<P><I>Recent Past:</I> %s\n", motivation1 );
    }

    if( i+1 < count ) {
      printf( "\n<HR SIZE=1>\n" );
    }
  }

  printf( "</DIV>\n<P>\n" );

  if( doBg ) {
    grDestroyGrammar( grammar );
  }

  return 0;
}


int displayLanguages( void* data ) {
  NPC* npc;
  NPCLANGUAGE* lang;
  char text[128];

  npc = (NPC*)data;

  printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n" );
  printf( "<UL>\n" );

  for( lang = npc->languages; lang != 0; lang = lang->next ) {
    strcpy( text, dndGetLanguageName( lang->language ) );
    text[0] = toupper( text[0] );
    printf( "<LI>%s\n", text );
  }

  printf( "</UL>\n" );
  printf( "</TD></TR>\n" );

  return 0;
}


int displayNPCBackground( void* data ) {
  grGRAMMAR* grammar;
  NPC*       npc;
  char       m1[1024];
  char       m2[1024];

  npc = (NPC*)data;

  if( *qValueDefault( "", "background" ) == 'Y' ) {
    grammar = openNPCMotivationGrammar( TEMPATH );

    grammar->startSymbol = "[motivation]";
    getNPCMotivation( grammar, npc, m1, sizeof( m1 ) );
    do {
      getNPCMotivation( grammar, npc, m2, sizeof( m2 ) );
    } while( strcmp( m1, m2 ) == 0 );

    printf( "<TR><TD CLASS=\"DIVIDER\" COLSPAN=2><IMG SRC=\"/img/blank.gif\" WIDTH=1 HEIGHT=1></TD></TR>\n" );
    printf( "<TR><TD CLASS=\"NORMAL\" COLSPAN=2>\n" );
    printf( "<B>Primary Motivation:</B> %s\n<P>\n", m1 );
    printf( "<B>Secondary Motivation:</B> %s\n<P>\n", m2 );

    getNPCRecentPast( grammar, npc, m1, sizeof( m1 ) );
    printf( "<B>Recent Past:</B> %s\n", m1 );

    printf( "</TD></TR>" );

    grDestroyGrammar( grammar );
  }

  return 0;
}


int displayNPC_Full( NPC* npc ) {
  char    alignment[20];
  char    race[40];
  char    gender[20];
  char    strbonus[5];
  char    dexbonus[5];
  char    conbonus[5];
  char    intbonus[5];
  char    wisbonus[5];
  char    chabonus[5];
  char    height[100];
  char    weight[100];
  int     armorClass;
  char    armorClassTxt[ 256 ];
  int     useWisdom;
  int     initiative;
  char    initiativeTxt[ 256 ];
  int     save;
  int     cr;
  int     speed;
  char    fortSave[100];
  char    refSave[100];
  char    willSave[100];
  char    gearValue[100];
  char    buffer[ 512 ];
  char    pcgen[ 1024 ];
  char    hpDesc[ 512 ];
  NPCCOMPBREAKDOWN* breakdown;
  NPCCLASS* cls;
  char* tem;
  t_ae_template_mgr mgr;

  strcpy( alignment, dndGetAlignmentText( npc->alignment ) );
  alignment[0] = toupper( alignment[0] );
  strcpy( race, (char*)dndGetRaceName( npc->race ) );
  race[0] = toupper( race[0] );

  strcpy( gender, ( npc->gender == gMALE ? "Male" : "Female" ) );

  /* compute the armor class, and build the text describing how we arrived
   * at this number */

  armorClass = npcComputeActualAC( npc, &breakdown );
  sprintf( armorClassTxt, "%d %s", armorClass,
           npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  /* compute the initiative score, and build the text describing how we
   * arrived at this number */
  initiative = npcComputeActualInitiative( npc, &breakdown );
  sprintf( initiativeTxt, "%+d %s", initiative,
           npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  /* calculate the ability bonuses */

  sprintf( strbonus, "%+d", dndGetAbilityBonus( npc->strength ) );
  sprintf( dexbonus, "%+d", dndGetAbilityBonus( npc->dexterity ) );
  sprintf( conbonus, "%+d", dndGetAbilityBonus( npc->constitution ) );
  sprintf( intbonus, "%+d", dndGetAbilityBonus( npc->intelligence ) );
  sprintf( wisbonus, "%+d", dndGetAbilityBonus( npc->wisdom ) );
  sprintf( chabonus, "%+d", dndGetAbilityBonus( npc->charisma ) );

  npcBuildHitDiceBreakdown( npc, hpDesc, sizeof( hpDesc ) );
  sprintf( &(hpDesc[strlen(hpDesc)]), " (%d)", npc->hp );

  cr = npcComputeCR( npc );

  speed = dndGetRaceSpeed( npc->race );
  if( npcHasClass( npc, pcBARBARIAN ) ) {
    speed += 10;
  }

  cls = npcGetClass( npc, pcMONK );
  if( cls != 0 ) {
    save = dndGetMonkSpeedForRace( npc->race, cls->level );
    if( save > speed ) {
      speed = save;
    }
  }

  /* set up the tags that will be written out */

  mgr = ae_init_html( 0, 0, 0 );

  ae_add_tag_ex( mgr, ae_escape_js_tag() );

  ae_add_tag( mgr, "ALIGNMENT", alignment );
  ae_add_tag( mgr, "RACE", race );
  ae_add_tag( mgr, "GENDER", gender );
  ae_add_tag_i( mgr, "STRENGTH", npc->strength );
  ae_add_tag_i( mgr, "DEXTERITY", npc->dexterity );
  ae_add_tag_i( mgr, "CONSTITUTION", npc->constitution );
  ae_add_tag_i( mgr, "INTELLIGENCE", npc->intelligence );
  ae_add_tag_i( mgr, "WISDOM", npc->wisdom );
  ae_add_tag_i( mgr, "CHARISMA", npc->charisma );
  ae_add_tag( mgr, "STRBONUS", strbonus );
  ae_add_tag( mgr, "DEXBONUS", dexbonus );
  ae_add_tag( mgr, "CONBONUS", conbonus );
  ae_add_tag( mgr, "INTBONUS", intbonus );
  ae_add_tag( mgr, "WISBONUS", wisbonus );
  ae_add_tag( mgr, "CHABONUS", chabonus );
  ae_add_tag( mgr, "HP", hpDesc );

  ae_add_tag_ex( mgr, ae_local_fn_tag( "SKILLS", displaySkills, npc ) );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "FEATS", displayFeats, npc->feats ) );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "CLASSES", displayClasses, npc->classes ) );
  ae_add_tag( mgr, "AC", armorClassTxt );
  ae_add_tag( mgr, "INIT", initiativeTxt );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "SPELLS", displaySpells, npc ) );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "ATTACK", displayAttacks, npc ) );

  /* compute and format the saving throws */

  save = npcComputeActualSave( npc, svFORTITUDE, &breakdown );
  sprintf( fortSave, "%+d %s", save,
           npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  save = npcComputeActualSave( npc, svREFLEX, &breakdown );
  sprintf( refSave, "%+d %s", save,
           npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  save = npcComputeActualSave( npc, svWILL, &breakdown );
  sprintf( willSave, "%+d %s", save,
           npcBuildComponentBreakdownDescription( breakdown, buffer, sizeof( buffer ) ) );
  npcDestroyComponentBreakdown( breakdown );

  /* compute and format the gp value of the NPC's gear */

  commify( gearValue, npcGearValue( npc ) );
  strcat( gearValue, " gp" );

  /* if we are not displaying the "printable" version, put up a link to
   * allow a printable version to be displayed. */

  if( !qiValue( "printable" ) ) {
    sprintf( buffer, "<A HREF=\"%s&printable=1\" TARGET=\"PRINT\">Open printable version</A>", url );
    sprintf( pcgen, "<A HREF=\"%s&action=pcgen\" TARGET=\"PCGEN\">Download in PCGen Format</a><br>"
                    "(<span style=\"color: red\">EXPERIMENTAL</span>) "
                    "[ <a href=\"javascript:pcgenHelp()\">What is this</a>? ]", url );
  } else {
    buffer[0] = 0;
    pcgen[0] = 0;
  }

  /* format the height and weight */

  if( npc->height_in > 0 ) {
    sprintf( height, "%d' %d\" ", npc->height_ft, npc->height_in );
  } else {
    sprintf( height, "%d'", npc->height_ft );
  }

  commify( weight, npc->weight );
  strcat( weight, " lbs" );

  ae_add_tag( mgr, "FORTSAVE", fortSave );
  ae_add_tag( mgr, "REFSAVE", refSave );
  ae_add_tag( mgr, "WILLSAVE", willSave );
  ae_add_tag( mgr, "GEARVALUE", gearValue );
  ae_add_tag( mgr, "DOPRINTABLE", buffer );
  ae_add_tag( mgr, "HEIGHT", height );
  ae_add_tag( mgr, "WEIGHT", weight );
  ae_add_tag_i( mgr, "CR", cr );
  ae_add_tag( mgr, "NAME", npc->name );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "LANGUAGES", displayLanguages, npc ) );
  ae_add_tag_i( mgr, "SPEED", speed );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "BACKGROUND", displayNPCBackground, npc ) );
  ae_add_tag( mgr, "DOPCGEN", pcgen );
  ae_add_tag_ex( mgr, ae_local_fn_tag( "SKILLPOINTS", displaySkillPoints, npc ) );

  tem = getLookAndFeel( qValueDefault( "standard", "look" ) )->longdesc;

  ae_done_html( mgr, tem );

  return 0;
}


int displayNPC( void* data ) {
  NPC**   npc;
  int     count;

  npc = (NPC**)data;
  count = qiValue( "count" );
  if( count < 1 ) {
    count = 1;
  }

  if( qiValue( "statblock" ) || ( count > 1 ) ) {
    displayNPC_StatBlock( npc );
  } else {
    displayNPC_Full( npc[0] );
  }

  return 0;
}


int doStatBlockConfigure() {
  t_ae_template_mgr mgr;
  int i;
  char* tem;

  mgr = ae_init_html( 0, 0, HTML_HEADER | HTML_NO_CACHE );

  ae_add_tag( mgr, "STATSABILITYBONUSES",  qValueDefault( "0", "stats_ability_bonuses" ) );
  ae_add_tag( mgr, "STATSACBREAKDOWN",     qValueDefault( "1", "stats_ac_breakdown" ) );
  ae_add_tag( mgr, "STATSINITBREAKDOWN",   qValueDefault( "1", "stats_init_breakdown" ) );
  ae_add_tag( mgr, "STATSATTACKBREAKDOWN", qValueDefault( "0", "stats_attack_breakdown" ) );
  ae_add_tag( mgr, "STATSSAVEBREAKDOWN",   qValueDefault( "0", "stats_save_breakdown" ) );
  ae_add_tag( mgr, "STATSSKILLBREAKDOWN",  qValueDefault( "0", "stats_skill_breakdown" ) );
  ae_add_tag( mgr, "STATSLANGUAGES",       qValueDefault( "1", "stats_languages" ) );
  ae_add_tag( mgr, "STATSSKILLSFEATS",     qValueDefault( "1", "stats_skillsfeats" ) );
  ae_add_tag( mgr, "STATSPOSSESSIONS",     qValueDefault( "1", "stats_possessions" ) );
  ae_add_tag( mgr, "STATSSPELLS",          qValueDefault( "1", "stats_spells" ) );
  ae_add_tag( mgr, "STATSFORMATTING",      qValueDefault( "1", "stats_formatting" ) );
  ae_add_tag( mgr, "STATSSKILLPOINTS",     qValueDefault( "1", "stats_skill_points" ) );
  ae_add_tag( mgr, "CGINAME",              CGINAME );
  ae_add_tag( mgr, "LOOK",                 qValueDefault( "standard", "look" ) );

  tem = getLookAndFeel( qValueDefault( "standard", "look" ) )->config;

  ae_done_html( mgr, tem );

  return 0;
}


int main( int argc, char* argv[] ) {
  NPC** npc;
  time_t seed;
  t_ae_template_mgr mgr;

  char buffer[100];
  char* tem;
  int i;
  int count;
  int istrat;
       
  char *alignment;
  char *gender;
  char *race;
  char *cls[3];
  char *level[3];
  char *stat;
  char *scnt;
  char *strat;
  char *background;
  char *look;
  char *action;

  char *defon;
  char* defoff;

  NPCGENERATOROPTS opts;
  NPCGENERATOROPTS tempOpts;

/*putenv( "REQUEST_METHOD=GET" );*/
/*putenv( "QUERY_STRING=look=standard&stats_ability_bonuses=0&stats_ac_breakdown=1&stats_init_breakdown=1&stats_attack_breakdown=0&stats_save_breakdown=0&stats_skill_breakdown=0&stats_languages=1&stats_skillsfeats=1&stats_possessions=1&stats_spells=1&stats_formatting=1&alignment=anyg&class=cleric&level=any&gender=male&class2=&level2=any&race=hillg&class3=&level3=any&count=1&scorestrategy=1&background=Y&seed=" );*/

  qDecoder();

  action = qValueDefault( "generate", "action" );

  if( qiValue( "configure" ) ) {
    doStatBlockConfigure();
    qFree();
    return 0;
  }
  
  mgr = ae_init_html( 0, 0, HTML_HEADER | HTML_NO_CACHE );
  ae_add_tag_ex( mgr, ae_escape_js_tag() );

  if( qiValue( "configsave" ) ) {
    defon = "0";
    defoff = "0";
  } else {
    defon = "1";
    defoff = "0";
  }
   
  seed = qiValue( "seed" );
  if( seed < 1 ) {
    seed = time( NULL );
    sprintf( url, "%s?%s%d", CGINAME, getenv( "QUERY_STRING" ), seed );
  } else {
    sprintf( url, "%s?%s", CGINAME, getenv( "QUERY_STRING" ) );
  }
  
  srand( seed );

  alignment = qValueDefault( "any", "alignment" );
  gender = qValueDefault( "any", "gender" );
  race = qValueDefault( "anycore", "race" );
  cls[0] = qValueDefault( "any", "class" );
  level[0] = qValueDefault( "any", "level" );
  cls[1] = qValueDefault( "", "class2" );
  level[1] = qValueDefault( "", "level2" );
  cls[2] = qValueDefault( "", "class3" );
  level[2] = qValueDefault( "", "level3" );
  stat = qValueDefault( "", "statblock" );
  scnt = qValueDefault( "1", "count" );
  count = qiValue( "count" );
  strat = qValueDefault( "1", "scorestrategy" );
  background = qValueDefault( "", "background" );
  look = qValueDefault( "standard", "look" );

  istrat = atoi( strat );

  if( count < 1 ) {
    count = 1;
  }
  if( count > 100 ) {
    count = 100;
  }

  commify( buffer, getCounter() );
  ae_add_tag( mgr, "COUNTER", buffer );
  ae_add_tag_i( mgr, "SEED", seed );
  ae_add_tag( mgr, "CGINAME", CGINAME );
  ae_add_tag( mgr, "ALIGNMENT", alignment );
  ae_add_tag( mgr, "GENDER", gender );
  ae_add_tag( mgr, "RACE", race );
  ae_add_tag( mgr, "CLASS", cls[0] );
  ae_add_tag( mgr, "LEVEL", level[0] );

  npc = 0;
     
  if( qValueDefault( (char*)NULL, "alignment" ) != (char*)NULL ) {
    logAccess( seed, alignment, gender, race, cls[0], level[0], cls[1], level[1], cls[2], level[2], count );

    npc = (NPC**)malloc( count * sizeof( NPC* ) );

    memset( &opts, 0, sizeof( opts ) );

    if( strcmp( alignment, "any" ) == 0 ) {
      opts.alignment = alANY;
    } else if( strcmp( alignment, "anyl" ) == 0 ) {
      opts.alignment = alANY_LAWFUL;
    } else if( strcmp( alignment, "anyN" ) == 0 ) {
      opts.alignment = alANY_LCNEUTRAL;
    } else if( strcmp( alignment, "anyc" ) == 0 ) {
      opts.alignment = alANY_CHAOTIC;
    } else if( strcmp( alignment, "anyg" ) == 0 ) {
      opts.alignment = alANY_GOOD;
    } else if( strcmp( alignment, "anyn" ) == 0 ) {
      opts.alignment = alANY_GENEUTRAL;
    } else if( strcmp( alignment, "anye" ) == 0 ) {
      opts.alignment = alANY_EVIL;
    } else if( strcmp( alignment, "nlaw" ) == 0 ) {
      opts.alignment = alANY_NONLAWFUL;
    } else if( strcmp( alignment, "ncha" ) == 0 ) {
      opts.alignment = alANY_NONCHAOTIC;
    } else if( strcmp( alignment, "ngoo" ) == 0 ) {
      opts.alignment = alANY_NONGOOD;
    } else if( strcmp( alignment, "nevi" ) == 0 ) {
      opts.alignment = alANY_NONEVIL;
    } else {
      opts.alignment = 0;
      switch( alignment[0] ) {
        case 'l': opts.alignment |= alLAWFUL; break;
        case 'n': opts.alignment |= alLCNEUTRAL; break;
        case 'c': opts.alignment |= alCHAOTIC; break;
      }
      switch( alignment[1] ) {
        case 'g': opts.alignment |= alGOOD; break;
        case 'n': opts.alignment |= alGENEUTRAL; break;
        case 'e': opts.alignment |= alEVIL; break;
      }
    }

    if( strcmp( gender, "any" ) == 0 ) {
      opts.gender = genderANY;
    } else if( strcmp( gender, "male" ) == 0 ) {
      opts.gender = gMALE;
    } else {
      opts.gender = gFEMALE;
    }

    for( i = 0; racetypes[ i ].name != 0; i++ ) {
      if( strcmp( racetypes[ i ].name, race ) == 0 ) {
        opts.raceType = racetypes[ i ].id;
        break;
      }
    }

    for( i = 0; i < 3; i++ ) {
      if( *(cls[i]) == 0 ) {
        continue;
      }
      if( strcmp( cls[i], "any" ) == 0 ) {
        opts.classType[i] = classANY;
      } else if( strcmp( cls[i], "anypc" ) == 0 ) {
        opts.classType[i] = classANY_PC;
      } else if( strcmp( cls[i], "anynpc" ) == 0 ) {
        opts.classType[i] = classANY_NPC;
      } else if( strcmp( cls[i], "barbarian" ) == 0 ) {
        opts.classType[i] = pcBARBARIAN;
      } else if( strcmp( cls[i], "bard" ) == 0 ) {
        opts.classType[i] = pcBARD;
      } else if( strcmp( cls[i], "cleric" ) == 0 ) {
        opts.classType[i] = pcCLERIC;
      } else if( strcmp( cls[i], "druid" ) == 0 ) {
        opts.classType[i] = pcDRUID;
      } else if( strcmp( cls[i], "fighter" ) == 0 ) {
        opts.classType[i] = pcFIGHTER;
      } else if( strcmp( cls[i], "monk" ) == 0 ) {
        opts.classType[i] = pcMONK;
      } else if( strcmp( cls[i], "paladin" ) == 0 ) {
        opts.classType[i] = pcPALADIN;
      } else if( strcmp( cls[i], "ranger" ) == 0 ) {
        opts.classType[i] = pcRANGER;
      } else if( strcmp( cls[i], "rogue" ) == 0 ) {
        opts.classType[i] = pcROGUE;
      } else if( strcmp( cls[i], "sorcerer" ) == 0 ) {
        opts.classType[i] = pcSORCERER;
      } else if( strcmp( cls[i], "wizard" ) == 0 ) {
        opts.classType[i] = pcWIZARD;
      } else if( strcmp( cls[i], "adept" ) == 0 ) {
        opts.classType[i] = npcADEPT;
      } else if( strcmp( cls[i], "aristocrat" ) == 0 ) {
        opts.classType[i] = npcARISTOCRAT;
      } else if( strcmp( cls[i], "commoner" ) == 0 ) {
        opts.classType[i] = npcCOMMONER;
      } else if( strcmp( cls[i], "expert" ) == 0 ) {
        opts.classType[i] = npcEXPERT;
      } else if( strcmp( cls[i], "warrior" ) == 0 ) {
        opts.classType[i] = npcWARRIOR;
      }

      if( strcmp( level[i], "any" ) == 0 ) {
        opts.level[i] = levelANY;
      } else if( strcmp( level[i], "lo" ) == 0 ) {
        opts.level[i] = levelLOW;
      } else if( strcmp( level[i], "mid" ) == 0 ) {
        opts.level[i] = levelMID;
      } else if( strcmp( level[i], "hi" ) == 0 ) {
        opts.level[i] = levelHI;
      } else {
        opts.level[i] = atoi( level[i] );
        if( opts.level[i] < 1 ) {
          opts.level[i] = 1;
        }
        if( opts.level[i] > 20 ) {
          opts.level[i] = 20;
        }
      }
    }

    opts.filePath = TEMPATH;
    opts.abilityScoreStrategy = strategies[ istrat ];

    for( i = 0; i < count; i++ ) {
      memcpy( &tempOpts, &opts, sizeof( tempOpts ) );
      npc[i] = npcGenerateNPC( &tempOpts );
    }

    ae_add_tag_ex( mgr, ae_local_fn_tag( "NPCDESC", displayNPC, npc ) );
  } else {
    logAccess( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
    ae_add_tag( mgr, "NPCDESC", "" );
  }

  ae_add_tag( mgr, "CLASS2", cls[1] );
  ae_add_tag( mgr, "CLASS3", cls[2] );
  ae_add_tag( mgr, "LEVEL2", level[1] );
  ae_add_tag( mgr, "LEVEL3", level[2] );
  ae_add_tag( mgr, "STATBLOCK", stat );
  ae_add_tag( mgr, "COUNT", scnt );

  ae_add_tag( mgr, "STATSABILITYBONUSES", qValueDefault( defoff, "stats_ability_bonuses" ) );
  ae_add_tag( mgr, "STATSACBREAKDOWN", qValueDefault( defon, "stats_ac_breakdown" ) );
  ae_add_tag( mgr, "STATSINITBREAKDOWN", qValueDefault( defon, "stats_init_breakdown" ) );
  ae_add_tag( mgr, "STATSATTACKBREAKDOWN", qValueDefault( defoff, "stats_attack_breakdown" ) );
  ae_add_tag( mgr, "STATSSAVEBREAKDOWN", qValueDefault( defoff, "stats_save_breakdown" ) );
  ae_add_tag( mgr, "STATSSKILLBREAKDOWN", qValueDefault( defoff, "stats_skill_breakdown" ) );
  ae_add_tag( mgr, "STATSLANGUAGES", qValueDefault( defon, "stats_languages" ) );
  ae_add_tag( mgr, "STATSSKILLSFEATS", qValueDefault( defon, "stats_skillsfeats" ) );
  ae_add_tag( mgr, "STATSPOSSESSIONS", qValueDefault( defon, "stats_possessions" ) );
  ae_add_tag( mgr, "STATSSPELLS", qValueDefault( defon, "stats_spells" ) );
  ae_add_tag( mgr, "STATSFORMATTING", qValueDefault( defon, "stats_formatting" ) );
  ae_add_tag( mgr, "STATSSKILLPOINTS", qValueDefault( defon, "stats_skill_points" ) );
  ae_add_tag( mgr, "SCORESTRATEGY", strat );
  ae_add_tag( mgr, "BACKGROUND", background );
  ae_add_tag( mgr, "LOOK", look );

  if( qiValue( "printable" ) ) {
    tem = getLookAndFeel( qValueDefault( "standard", "look" ) )->printable;
  } else {
    tem = getLookAndFeel( qValueDefault( "standard", "look" ) )->main;
  }
    
  if( strcmp( action, "pcgen" ) == 0 ) {
    convertToPCGen( npc[0], stdout );
  } else {
    ae_process_template( mgr, tem, stdout );
  }

  ae_done_html( mgr, 0 );

  if( npc != 0 ) {
    for( i = 0; i < count; i++ ) {
      npcDestroyNPC( npc[i] );
    }
    free( npc );
  }

  qFree();

  return 0;
}

