#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pwd.h>

#include "gameutil.h"
#include "npcEngine.h"
#include "dndconst.h"
#include "dndutil.h"
#include "grammar.h"
#include "npcHistory.h"
#include "pcgen_interface.h"

#include "wtstream.h"

#define APPNAME     "npc"

#define MODE_STATBLOCK  ( 0 )
#define MODE_PCGEN      ( 1 )

static void heroicAbilities( int* scores, void* data );
static void straight18s( int* scores, void* data );
static void averageAbilities( int* scores, void* data );


typedef struct tag_npcopts NPC_OPTS;
typedef struct tag_lookup  LOOKUP_TBL;

struct tag_npcopts {
  long seed;
  int  background;
  int  alignment;
  int  race;
  int  classes[ 3 ];
  int  levels[ 3 ];
  int  score_strategy;
  int  count;
  int  gender;

  int  mode;
  int  wrap;

  char path[ 512 ];
  char data_path[ 512 ];

  NPCSTATBLOCKOPTS opts;
};

struct tag_lookup {
  char* name;
  int   id;
};


static NPCABSCOREGENFUNC strategies[] = {
  0, 
  npcAbScoreStrategy_BestOf4d6, 
  npcAbScoreStrategy_Straight3d6,
  heroicAbilities,
  averageAbilities,
  straight18s
};


#define HELP_RACES      1
#define HELP_CLASSES    2
#define HELP_ALIGNMENTS 3
#define HELP_GENDERS    4
#define HELP_STRATEGIES 5

static LOOKUP_TBL help_lists[] = {
  { "alignments", HELP_ALIGNMENTS },
  { "classes",    HELP_CLASSES },
  { "genders",    HELP_GENDERS },
  { "races",      HELP_RACES },
  { "strategies", HELP_STRATEGIES },
  { 0,            0 }
};


static LOOKUP_TBL strategies_lookup[] = {
  { "18s",      5 },
  { "3d6",      2 },
  { "4d6",      1 },
  { "average",  4 },
  { "heroic",   3 },
  { 0,          0 }
};


static LOOKUP_TBL levels[] = {
  { "any",  levelANY },
  { "high", levelHI },
  { "low",  levelLOW },
  { "mid",  levelMID },
  { 0,      0 }
};


static LOOKUP_TBL genders[] = {
  { "any",       gANY },
  { "female",    gFEMALE },
  { "male",      gMALE },
  { 0,           0 }
};


static LOOKUP_TBL alignments[] = {
  { "any",       alANY },
  { "ce",        alCHAOTIC | alEVIL },
  { "cg",        alCHAOTIC | alGOOD },
  { "chaotic",   alANY_CHAOTIC },
  { "cn",        alCHAOTIC | alGENEUTRAL },
  { "evil",      alANY_EVIL },
  { "geneutral", alANY_GENEUTRAL },
  { "good",      alANY_GOOD },
  { "lawful",    alANY_LAWFUL },
  { "lcneutral", alANY_LCNEUTRAL },
  { "le",        alLAWFUL | alEVIL },
  { "lg",        alLAWFUL | alGOOD },
  { "ln",        alLAWFUL | alGENEUTRAL },
  { "nchaotic",  alANY_NONCHAOTIC },
  { "ne",        alLCNEUTRAL | alEVIL },
  { "nevil",     alANY_NONEVIL },
  { "ng",        alLCNEUTRAL | alGOOD },
  { "ngood",     alANY_NONGOOD },
  { "nlawful",   alANY_NONLAWFUL },
  { "nn",        alLCNEUTRAL | alGENEUTRAL },
  { 0,           0 }
};


static LOOKUP_TBL classtypes[] = {
  { "adept",      npcADEPT },
  { "any",        classANY },
  { "anynpc",     classANY_NPC },
  { "anypc",      classANY_PC },
  { "aristocrat", npcARISTOCRAT },
  { "barbarian",  pcBARBARIAN },
  { "bard",       pcBARD },
  { "cleric",     pcCLERIC },
  { "commoner",   npcCOMMONER },
  { "druid",      pcDRUID },
  { "expert",     npcEXPERT },
  { "fighter",    pcFIGHTER },
  { "monk",       pcMONK },
  { "paladin",    pcPALADIN },
  { "ranger",     pcRANGER },
  { "rogue",      pcROGUE },
  { "sorcerer",   pcSORCERER },
  { "warrior",    npcWARRIOR },
  { "wizard",     pcWIZARD },
  { 0,            0 }
};


static LOOKUP_TBL racetypes[] = {
  { "aasimar",    rcAASIMAR },
  { "abandoned",  rcCC_ABANDONED },
  { "any",        raceANY },
  { "anycc",      raceANY_CC },
  { "anycore",    raceANY_CORE },
  { "anydmg",     raceANY_DMG },
  { "anymm",      raceANY_MM },
  { "aqelf",      rcELF_AQUATIC },
  { "aranea",     rcARANEA },
  { "asaath",     rcCC_ASAATH },
  { "azer",       rcAZER },
  { "batdevil",   rcCC_BATDEVIL },
  { "bgratman",   rcCC_RATMAN_BROWNGORGER },
  { "bugbear",    rcBUGBEAR },
  { "centaur",    rcCENTAUR },
  { "cgoblin",    rcCC_COALGOBLIN },
  { "charduni",   rcCC_CHARDUNI },
  { "cloudg",     rcGIANT_CLOUD },
  { "deepdwarf",  rcDWARF_DEEP },
  { "deephalf",   rcHALFLING_DEEP },
  { "derro",      rcDWARF_DERRO },
  { "dopple",     rcDOPPLEGANGER },
  { "dratman",    rcCC_RATMAN_DISEASED },
  { "drider",     rcDRIDER },
  { "drow",       rcELF_DROW },
  { "duergar",    rcDWARF_DUERGAR },
  { "dwarf",      rcDWARF_HILL },
  { "elf",        rcELF_HIGH },
  { "ettin",      rcETTIN },
  { "fdwarf",     rcCC_DWARF_FORSAKEN },
  { "felf",       rcCC_ELF_FORSAKEN },
  { "fireg",      rcGIANT_FIRE },
  { "fratman",    rcCC_RATMAN_FOAMER },
  { "frostg",     rcGIANT_FROST },
  { "frstgnome",  rcGNOME_FOREST },
  { "gnoll",      rcGNOLL },
  { "gnome",      rcGNOME_ROCK },
  { "goblin",     rcGOBLIN },
  { "grayelf",    rcELF_GRAY },
  { "grimlock",   rcGRIMLOCK },
  { "haga",       rcHAG_ANNIS },
  { "hagg",       rcHAG_GREEN },
  { "hags",       rcHAG_SEA },
  { "halfelf",    rcHALFELF },
  { "halfling",   rcHALFLING_LIGHTFOOT },
  { "halforc",    rcHALFORC },
  { "harpy",      rcHARPY },
  { "hillg",      rcGIANT_HILL },
  { "hobgob",     rcHOBGOBLIN },
  { "human",      rcHUMAN },
  { "iceghoul",   rcCC_ICE_GHOUL },
  { "kobold",     rcKOBOLD },
  { "kuotoa",     rcKUOTOA },
  { "lizard",     rcLIZARDFOLK },
  { "locathah",   rcLOCATHAH },
  { "mcora",      rcCC_MANTICORA },
  { "medusa",     rcMEDUSA },
  { "mindflayer", rcMINDFLAYER },
  { "minotaur",   rcMINOTAUR },
  { "morgaunt",   rcCC_MORGAUNT },
  { "mtndwarf",   rcDWARF_MOUNTAIN },
  { "ogre",       rcOGRE },
  { "ogremage",   rcOGREMAGE },
  { "orc",        rcORC },
  { "proud",      rcCC_PROUD },
  { "pwretch",    rcCC_PLAGUEWRETCH },
  { "ratman",     rcCC_RATMAN },
  { "rwratman",   rcCC_RATMAN_REDWITCH },
  { "sahuagin",   rcSAHUAGIN },
  { "segoblin",   rcCC_SPIDEREYEGOBLIN },
  { "selement",   rcCC_STRIFEELEMENTAL },
  { "skindev",    rcCC_SKINDEVIL },
  { "stoneg",     rcGIANT_STONE },
  { "stormg",     rcGIANT_STORM },
  { "stroll",     rcCC_STEPPETROLL },
  { "svirf",      rcGNOME_SVIRFNEBLIN },
  { "tallflw",    rcHALFLING_TALLFELLOW },
  { "tiefling",   rcTIEFLING },
  { "tokal",      rcCC_TOKALTRIBESMAN },
  { "troglodyte", rcTROGLODYTE },
  { "troll",      rcTROLL },
  { "ubantu",     rcCC_UBANTUTRIBESMAN },
  { "wildelf",    rcELF_WILD },
  { "woodelf",    rcELF_WOOD },
  { "yuantia",    rcYUANTI_ABOMINATION },
  { "yuantih",    rcYUANTI_HALFBLOOD },
  { "yuantip",    rcYUANTI_PUREBLOOD },
  { 0,            0 }
};


static int icomp( const void* x, const void* y ) {
  int ix = *(int*)x;
  int iy = *(int*)y;

  if( ix < iy ) return 1;
  if( ix > iy ) return -1;
  return 0;
}


static void heroicAbilities( int* scores, void* data ) {
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


static void straight18s( int* scores, void* data ) {
  int i;

  for( i = 0; i < 6; i++ ) {
    scores[ i ] = 18;
  }
}


static void averageAbilities( int* scores, void* data ) {
  int i;

  for( i = 0; i < 6; i++ ) {
    scores[i] = 10 + ( rand() % 2 );
  }
}


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


static int lookup( char* string, LOOKUP_TBL* tbl, int* found ) {
  int rc;
  int idx;
  int len;

  len = strlen( string );
  *found = 1;
  for( idx = 0; tbl[ idx ].name != 0; idx++ ) {
    rc = strncasecmp( string, tbl[ idx ].name, len );
    if( rc == 0 ) {
      return tbl[ idx ].id;
    } else if( rc < 0 ) {
      break;
    }
  }

  *found = 0;
  return -1;
}


#define USAGE APPNAME ", by Jamis Buck (jgb3@email.byu.edu)\n\
\n\
  Options:\n\
    -r <race>        -S <seed>      -c <class>     -g <gender>\n\
    -l <level>       -h (race|class|strategy|alignment|gender)\n\
    -s <strategy>    -n <count>     -a <alignment> -b\n\
    -o (a|c|i|b|s|k|l|f|p|S)        -w (p|s)       -p <path>\n\
    -W <wrap-length> -d <data-path>\n\
\n\
  For <race>, <level>, <strategy>, <class>, and <alignment>, you only\n\
  need to type part of the full name.  To see what possible options exist\n\
  for those values, use the -h option, followed by race, level, strategy,\n\
  class, alignment, or gender. (No space, ie: -hrace, or -hgender).\n\
\n\
  If -b is given, background information is generated.  The -W option will\n\
  cause the lines to wrap at the given line length, at the first white space\n\
  found.  The -d option species the path to the NPC data files (for names and\n\
  background files) -- it defaults to '~/.npc/'.\n\
\n\
  The -o options are for the stat-block output.  They mean:\n\
    a: ability bonuses\n\
    c: AC breakdown\n\
    i: initiative breakdown\n\
    b: attack breakdown\n\
    s: saving throw breakdown\n\
    k: skill breakdown\n\
    l: languages\n\
    f: skills and feats\n\
    p: posessions\n\
    S: spells\n\
    P: skill point count\n\
\n\
  The -w option is for determining the output format.  It means:\n\
    p: output in PCGen format.\n\
    s: output in stat-block format.\n\
\n\
  <path>, when doing stat-block output (-ws), is the name of a file, in\n\
  which case the output is all redirected to that file.  When doing\n\
  PCGen output (-wp), <path> is the name of a directory, in which case\n\
  each NPC is created in a separate file in the given directory.\n\
"

static int usage() {
  puts( USAGE );
  exit(0);
}


static int getLookup( char* parmName, char* arg, LOOKUP_TBL* list ) {
  int i;
  int found;

  i = lookup( arg, list, &found );
  if( !found ) {
    printf( "Invalid %s: '%s'\n", parmName, arg );
    usage();
  }

  return i;
}


static void getHomeDirectory( char* home ) {
  uid_t uid;
  struct passwd *pwd;

  uid = geteuid();
  pwd = getpwuid( uid );

  strcpy( home, pwd->pw_dir );
  if( home[ strlen( home ) - 1 ] != '/' ) {
    strcat( home, "/" );
  }
}


static int parseCommandLine( int argc, char* argv[], NPC_OPTS* opts ) {
  int c;
  int i;
  int found;
  int class_count;
  int level_count;
  LOOKUP_TBL* list;

  memset( opts, 0, sizeof( *opts ) );
  opts->race = raceANY_CORE;
  opts->classes[0] = classANY;
  opts->classes[1] = classNONE;
  opts->classes[2] = classNONE;
  opts->levels[0] = levelANY;
  opts->levels[1] = levelANY;
  opts->levels[2] = levelANY;
  opts->score_strategy = 1;
  opts->count = 1;
  opts->gender = gANY;
  opts->wrap = 0;

  getHomeDirectory( opts->data_path );
  strcat( opts->data_path, ".npc/" );

  class_count = 0;
  level_count = 0;

  srand( time( NULL ) );

  while( ( c = getopt( argc, argv, "r:c:l:s:a:S:g:n:h::bo:p:w:W:d:" ) ) != -1 ) {
    switch( c ) {
      case 'r':
        opts->race = getLookup( "race", optarg, racetypes );
        break;
      case 'c':
        opts->classes[ class_count++ ] = getLookup( "class", optarg, classtypes );
        break;
      case 'l':
        opts->levels[ level_count ] = lookup( optarg, levels, &found );
        if( !found ) {
          opts->levels[ level_count ] = atoi( optarg );
          if( opts->levels[ level_count ] < 1 ) {
            printf( "Invalid level specification: %s\n", optarg );
            usage();
          }
        }
        level_count++;
        break;
      case 's':
        opts->score_strategy = getLookup( "strategy", optarg, strategies_lookup );
        break;
      case 'a':
        opts->alignment = getLookup( "alignment", optarg, alignments );
        break;
      case 'S':
        opts->seed = atoi( optarg );
        srand( opts->seed );
        break;
      case 'g':
        opts->gender = getLookup( "gender", optarg, genders );
        break;
      case 'h':
        if( optarg == 0 ) {
          usage();
        } else {
          i = lookup( optarg, help_lists, &found );
          switch( i ) {
            case HELP_RACES:      list = racetypes; break;
            case HELP_CLASSES:    list = classtypes; break;
            case HELP_ALIGNMENTS: list = alignments; break;
            case HELP_GENDERS:    list = genders; break;
            case HELP_STRATEGIES: list = strategies_lookup; break;
            default:
              usage();
          }
          for( i = 0; list[ i ].name != 0; i++ ) {
            printf( "  %s\n", list[ i ].name );
          }
          printf( "\n" );
        }
        exit( 0 );
      case 'n':
        opts->count = atoi( optarg );
        break;
      case 'b':
        opts->background = !opts->background;
        break;
      case 'o':
        for( i = 0; optarg[i]; i++ ) {
          switch( optarg[i] ) {
            case 'a': opts->opts.abilityBonuses = !opts->opts.abilityBonuses; break;
            case 'c': opts->opts.acBreakdown = !opts->opts.acBreakdown; break;
            case 'i': opts->opts.initBreakdown = !opts->opts.initBreakdown; break;
            case 'b': opts->opts.attackBreakdown = !opts->opts.attackBreakdown; break;
            case 's': opts->opts.saveBreakdown = !opts->opts.saveBreakdown; break;
            case 'k': opts->opts.skillBreakdown = !opts->opts.skillBreakdown; break;
            case 'l': opts->opts.languages = !opts->opts.languages; break;
            case 'f': opts->opts.skillsAndFeats = !opts->opts.skillsAndFeats; break;
            case 'p': opts->opts.possessions = !opts->opts.possessions; break;
            case 'S': opts->opts.spells = !opts->opts.spells; break;
            case 'P': opts->opts.skill_points = !opts->opts.skill_points; break;
            default:
              printf( "Invalid stat-block format specifier: %c\n", optarg[i] );
              usage();
          }
        }
        break;
      case 'w':
        for( i = 0; optarg[i]; i++ ) {
          switch( optarg[i] ) {
            case 's': opts->mode = MODE_STATBLOCK; break;
            case 'p': opts->mode = MODE_PCGEN; break;
            default:
              printf( "Invalid output mode specifier: %c\n", optarg[i] );
              usage();
          }
        }
        break;
      case 'p':
        strcpy( opts->path, optarg );
        break;
      case 'W':
        opts->wrap = atoi( optarg );
        break;
      case 'd':
        strcpy( opts->data_path, optarg );
        if( opts->data_path[ strlen( opts->data_path )-1 ] != '/' ) {
          strcat( opts->data_path, "/" );
        }
        break;
      default:
        usage();
    }
  }

  return 0;
}


static void wrapLines( char* string, int width ) {
  int     c;
  int     count;
  int     i;
  int     last;

  if( width > 0 ) {
    count = width+1;
    last = 0;
    while( count < strlen( string ) ) {
      for( i = last; i < count; i++ ) {
        if( string[i] == '\n' )
          break;
      }

      if( string[i] != '\n' ) {
        for( i = count; i > 0; i-- ) {
          c = string[i];
          if( isspace( c ) ) {
            string[i] = '\n';
            break;
          }
        }
      }

      last = i+1;
      count = i + width+1;
    }
  }
}


static int displayNPC_StatBlock( NPC_OPTS* opts, wtSTREAM_t *stream, NPC* npc ) {
  char    statBlock[4096];
  int     count;
  int     i;
  int     doBg;
  char    motivation1[ 1024 ];
  char    motivation2[ 1024 ];
  grGRAMMAR* grammar;

  if( opts->background ) {
    grammar = openNPCMotivationGrammar( opts->data_path );
  }

  npcBuildStatBlock( npc, &opts->opts, statBlock, sizeof( statBlock ) );

  strrepl( statBlock, "~B", "" );
  strrepl( statBlock, "~b", "" );
  strrepl( statBlock, "~I", "" );
  strrepl( statBlock, "~i", "" );

  wrapLines( statBlock, opts->wrap );
  wtPrint( stream, statBlock );

  if( opts->background ) {
    grammar->startSymbol = "[motivation]";
    getNPCMotivation( grammar, npc, motivation1, sizeof( motivation1 ) );
    do {      
      getNPCMotivation( grammar, npc, motivation2, sizeof( motivation1 ) );
    } while( strcmp( motivation1, motivation2 ) == 0 );

    strcpy( statBlock, "Primary motivation: " );
    strcat( statBlock, motivation1 );
    wrapLines( statBlock, opts->wrap );
    wtPrintf( stream, "\n\n%s\n", statBlock );

    strcpy( statBlock, "Secondary motivation: " );
    strcat( statBlock, motivation2 );
    wrapLines( statBlock, opts->wrap );
    wtPrintf( stream, "\n%s\n", statBlock );

    getNPCRecentPast( grammar, npc, motivation1, sizeof( motivation1 ) );

    strcpy( statBlock, "Recent Past: " );
    strcat( statBlock, motivation1 );
    wrapLines( statBlock, opts->wrap );
    wtPrintf( stream, "\n%s\n", statBlock );
  }

  if( opts->background ) {
    grDestroyGrammar( grammar );
  }

  wtPrintf( stream, "\n" );

  return 0;
}


int main( int argc, char* argv[] ) {
  char buffer[512];
  int i;
  wtSTREAM_t* stream;
       
  NPC* npc;
  NPCGENERATOROPTS opts;
  NPCGENERATOROPTS tempOpts;
  NPC_OPTS cmd_opts;

  if( parseCommandLine( argc, argv, &cmd_opts ) != 0 ) {
    return 0;
  }

  opts.raceType  = cmd_opts.race;
  opts.gender    = cmd_opts.gender;
  opts.alignment = cmd_opts.alignment;
  opts.level[0]  = cmd_opts.levels[0];
  opts.level[1]  = cmd_opts.levels[1];
  opts.level[2]  = cmd_opts.levels[2];
  opts.classType[0] = cmd_opts.classes[0];
  opts.classType[1] = cmd_opts.classes[1];
  opts.classType[2] = cmd_opts.classes[2];
  opts.abilityScoreStrategy = strategies[ cmd_opts.score_strategy ];

  opts.filePath = cmd_opts.data_path;

  if( cmd_opts.mode == MODE_STATBLOCK && cmd_opts.path[0] != 0 ) {
    stream = wtOpenFileStream( cmd_opts.path, "w" );
  } else {
    stream = wtOpenFileStreamFromHandle( stdout );
  }

  for( i = 0; i < cmd_opts.count; i++ ) {
    memcpy( &tempOpts, &opts, sizeof( tempOpts ) );
    npc = npcGenerateNPC( &tempOpts );
    if( cmd_opts.mode == MODE_STATBLOCK ) {
      displayNPC_StatBlock( &cmd_opts, stream, npc );
      if( i+1 < cmd_opts.count ) {
        wtPrint( stream, "---------------------------\n" );
      }
    } else if( cmd_opts.mode == MODE_PCGEN ) {
      if( cmd_opts.path[0] != 0 ) {
        wtCloseStream( stream );
        strcpy( buffer, cmd_opts.path );
        strcat( buffer, "/" );
        strcat( buffer, npc->name );
        strcat( buffer, ".pcg" );
        stream = wtOpenFileStream( buffer, "w" );
      }
      convertToPCGen( npc, stream );
      wtPrint( stream, "\n" );
    }
    npcDestroyNPC( npc );
  }

  wtCloseStream( stream );

  return 0;
}

