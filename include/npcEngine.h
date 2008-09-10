/* ---------------------------------------------------------------------- *
 * dndutil.h
 *
 * by Jamis Buck (jgb3@email.byu.edu)
 *
 * NPC generation functions for the Dungeons & Dragons(tm) API.
 *
 * Function summary:
 *
 * void      npcAbScoreStrategy_BestOf4d6( int* scores, void* data );
 * void      npcAbScoreStrategy_Straight3d6( int* scores, void* data );
 * char*     npcBuildComponentBreakdownDescription( NPCCOMPBREAKDOWN* breakdown, char* buffer, int len )
 * char*     npcBuildHitDiceBreakdown( NPC* npc, char* buffer, int len )
 * char*     npcBuildStatBlock( NPC* npc, char* buffer, int len )
 * int       npcClassCount( NPC* npc )
 * int       npcComputeActualAC( NPC* npc, NPCCOMPBREAKDOWN** breakdown )
 * int       npcComputeActualAttack( NPC* npc, int type, NPCCOMPBREAKDOWN** breakdown )
 * int       npcComputeActualInitiative( NPC* npc, NPCCOMPBREAKDOWN** breakdown )
 * int       npcComputeActualSave( NPC* npc, int save, NPCCOMPBREAKDOWN** breakdown )
 * float     npcComputeActualSkillBonus( NPC* npc, int type, NPCCOMPBREAKDOWN** breakdown )
 * int       npcComputeCR( NPC* npc )
 * void      npcDestroyComponentBreakdown( NPCCOMPBREAKDOWN* breakdown )
 * void      npcDestroyNPC( NPC* npc )
 * int       npcGearValue( NPC* npc )
 * NPC*      npcGenerateNPC( NPCGENERATOROPTS* opts )
 * int       npcGetBaseAttack( NPC* npc )
 * int       npcGetBaseClassAttack( NPC* npc )
 * int       npcGetBaseFortitudeSave( NPC* npc )
 * int       npcGetBaseReflexSave( NPC* npc )
 * int       npcGetBaseWillSave( NPC* npc )
 * int       npcGetCharacterLevel( NPC* npc )
 * NPCCLASS* npcGetClass( NPC* npc, int classType )
 * int       npcGetRandomAlignment( int alType );
 * int       npcGetRandomRace( int raceType )
 * NPCSKILL* npcGetSkill( NPC* npc, int skillType )
 * int       npcHasClass( NPC* npc, int classType )
 * int       npcHasFeat( NPC* npc, int featType )
 * int       npcHasSkill( NPC* npc, int skillType )
 * int       npcHasSkillFocus( NPC* npc, int skillType )
 * int       npcKnowsLanguage( NPC* npc, int language )
 * char*     npcRandomName( int race, int gender, char* filePath, char* name, int buflen )
 *
 * ---------------------------------------------------------------------- */

#ifndef __NPCENGINE_H__
#define __NPCENGINE_H__

/* ---------------------------------------------------------------------- *
 * Constant definitions
 * ---------------------------------------------------------------------- */

#define classNONE             (  0 )
#define classANY              ( -1 )
#define classANY_PC           ( -2 )
#define classANY_NPC          ( -3 )
#define raceANY               (  0 )
#define raceANY_CORE          ( -1 )
#define raceANY_DMG           ( -2 )
#define raceANY_MM            ( -3 )
#define raceANY_CC            ( -4 )
#define genderANY             (  0 )
#define alANY                 (  0 )
#define alANY_GOOD            ( -1 )
#define alANY_EVIL            ( -2 )
#define alANY_GENEUTRAL       ( -3 )
#define alANY_LAWFUL          ( -4 )
#define alANY_CHAOTIC         ( -5 )
#define alANY_LCNEUTRAL       ( -6 )
#define alANY_NONGOOD         ( -7 )
#define alANY_NONEVIL         ( -8 )
#define alANY_NONLAWFUL       ( -9 )
#define alANY_NONCHAOTIC      ( -10 )
#define levelANY              (  0 )
#define levelLOW              ( -1 )
#define levelMID              ( -2 )
#define levelHI               ( -3 )

/* component breakdown types */

#define bdtABILITYSCORE       (  1 )
#define bdtSIZE               (  2 )
#define bdtRACIAL             (  3 )
#define bdtARMOR              (  4 )
#define bdtFEAT               (  5 )
#define bdtBASE               (  6 )
#define bdtRANK               (  7 )
#define bdtHALFRANK           (  8 )
#define bdtFOCUS              (  9 )
#define bdtNATURAL            ( 10 )
#define bdtCLASS              ( 11 )

/* attack types */

#define attMELEE              (  1 )
#define attRANGED             (  2 )

/* ---------------------------------------------------------------------- *
 * Type definitions
 * ---------------------------------------------------------------------- */

typedef struct __npc__             NPC;
typedef struct __npcclass__        NPCCLASS;
typedef struct __npcfeat__         NPCFEAT;
typedef struct __npcskill__        NPCSKILL;
typedef struct __npcspell__        NPCSPELL;
typedef struct __npclanguage__     NPCLANGUAGE;

typedef struct __npcbarddata__     NPCBARDDATA;
typedef struct __npcclericdata__   NPCCLERICDATA;
typedef struct __npcdruiddata__    NPCDRUIDDATA;
typedef struct __npcpaladindata__  NPCPALADINDATA;
typedef struct __npcrangerdata__   NPCRANGERDATA;
typedef struct __npcsorcererdata__ NPCSORCERERDATA;
typedef struct __npcwizarddata__   NPCWIZARDDATA;
typedef struct __npcexpertdata__   NPCEXPERTDATA;

typedef struct __npcfeatweapon__   NPCFEATWEAPON;
typedef struct __npcfeatskill__    NPCFEATSKILL;
typedef struct __npcfeatschool__   NPCFEATSCHOOL;
typedef struct __npcfeatspells___  NPCFEATSPELLS;

typedef struct __npcgeneratoroptions__ NPCGENERATOROPTS;
typedef struct __npcstatblockoptions__ NPCSTATBLOCKOPTS;

typedef struct __npccompbreakdown__ NPCCOMPBREAKDOWN;

/* first parameter - 6 element array of integers, second parameter - user
 * specified data. */

typedef void (*NPCABSCOREGENFUNC)( int*, void* );

/* ---------------------------------------------------------------------- *
 * Structure definitions
 * ---------------------------------------------------------------------- */

struct __npclanguage__ {
  int language;
  NPCLANGUAGE* next;
};


struct __npcspell__ {
  int spell;
  NPCSPELL* next;
};


struct __npcbarddata__ {
  NPCSPELL* spells[10];
};


struct __npcclericdata__ {
  int domain[2];
};


struct __npcdruiddata__ {
  int      companion;
};


struct __npcrangerdata__ {
  int favoredEnemies[5];
};


struct __npcpaladindata__ {
  int mount;
};


struct __npcsorcererdata__ {
  int familiar;
  NPCSPELL* spells[10];
};


struct __npcexpertdata__ {
  int classSkills[10];
};


struct __npcwizarddata__ {
  int familiar;
  int favoredSchool;
  int opposedSchools[3];
  NPCSPELL* spells[10];
};


struct __npcfeatweapon__ {
  int weapon;
};


struct __npcfeatskill__ {
  int skill;
};


struct __npcfeatschool__ {
  int school;
};


struct __npcfeatspells___ {
  int spells[3];
};


struct __npcclass__ {
  int       type;
  int       level;
  int       hp;
  int       skill_points;
  void*     data;
  NPCCLASS* next;
};


struct __npcfeat__ {
  int   type;
  int   autoAdd;
  void* data;
  NPCFEAT* next;
};


struct __npcskill__ {
  int   type;
  float rank;
  void* data;
  NPCSKILL* next;
};

        
struct __npc__ {
  int   strength;
  int   dexterity;
  int   constitution;
  int   intelligence;
  int   wisdom;
  int   charisma;

  int   hp;
  int   age;
  int   weight;
  int   height_ft; 
  int   height_in;
  int   race;
  int   gender;
  int   alignment;

  char* name;

  NPCCLASS*    classes;
  NPCFEAT*     feats;
  NPCSKILL*    skills;
  NPCLANGUAGE* languages;
};


struct __npcgeneratoroptions__ {
  int   raceType;
  int   gender;
  int   alignment;
  int   level[3];
  int   classType[3];
  char* filePath;
  
  NPCABSCOREGENFUNC abilityScoreStrategy;

  void* userData;
};


struct __npcstatblockoptions__ {
  int   abilityBonuses;
  int   acBreakdown;
  int   initBreakdown;
  int   attackBreakdown;
  int   saveBreakdown;
  int   skillBreakdown;
  int   languages;
  int   skillsAndFeats;
  int   possessions;
  int   spells;
  int   richFormatting;
  int   skill_points;
};


struct __npccompbreakdown__ {
  int   component;
  int   data1;
  int   data2;
  NPCCOMPBREAKDOWN* next;
};

/* ---------------------------------------------------------------------- *
 * Functions
 * ---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- *
 * npcGenerateNPC
 *
 * Returns a pointer to a new NPC, generated according to the options
 * given in the parameter.
 * ---------------------------------------------------------------------- */
NPC* npcGenerateNPC( NPCGENERATOROPTS* opts );

/* ---------------------------------------------------------------------- *
 * npcDestroyNPC
 *
 * Destroys (deallocates) the given NPC.
 * ---------------------------------------------------------------------- */
void npcDestroyNPC( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcHasClass
 *
 * Returns non-zero if the given NPC has at least one level of the given
 * class.
 * ---------------------------------------------------------------------- */
int npcHasClass( NPC* npc, int classType );

/* ---------------------------------------------------------------------- *
 * npcHasFeat
 *
 * Returns non-zero if the given NPC has taken the given feat at least
 * once.
 * ---------------------------------------------------------------------- */
int npcHasFeat( NPC* npc, int featType );

/* ---------------------------------------------------------------------- *
 * npcHasSkill
 *
 * Returns non-zero if the given NPC has taken the given skill at least
 * once.
 * ---------------------------------------------------------------------- */
int npcHasSkill( NPC* npc, int skillType );

/* ---------------------------------------------------------------------- *
 * npcHasSkillFocus
 *
 * Returns non-zero if the given NPC has taken the "skill focus" feat at
 * least once for the given skill.
 * ---------------------------------------------------------------------- */
int npcHasSkillFocus( NPC* npc, int skillType );

/* ---------------------------------------------------------------------- *
 * npcHasKnowsLanguage
 *
 * Returns non-zero if the given NPC knows the given language.
 * ---------------------------------------------------------------------- */
int npcKnowsLanguage( NPC* npc, int language );

/* ---------------------------------------------------------------------- *
 * npcGearValue
 *
 * Returns the gold piece value of the given NPC's gear.
 * ---------------------------------------------------------------------- */
int npcGearValue( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcClassCount
 *
 * Returns the number of classes the NPC has.
 * ---------------------------------------------------------------------- */
int npcClassCount( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcGetClass
 *
 * Returns NULL if the NPC does not have any levels in the given class,
 * or a pointer to the given class if the NPC has at least one level in it.
 * ---------------------------------------------------------------------- */
NPCCLASS* npcGetClass( NPC* npc, int classType );

/* ---------------------------------------------------------------------- *
 * npcGetSkill
 *
 * Returns NULL if the NPC does not have the given skill, or a pointer to
 * the given skill.
 * ---------------------------------------------------------------------- */
NPCSKILL* npcGetSkill( NPC* npc, int skillType );

/* ---------------------------------------------------------------------- *
 * npcGetBaseFortitudeSave
 *
 * Computes the base fortitude saving throw for the NPC by summing the
 * base fortitude saves for each of the NPC's classes.
 * ---------------------------------------------------------------------- */
int npcGetBaseFortitudeSave( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcGetBaseReflexSave
 *
 * Computes the base reflex saving throw for the NPC by summing the
 * base reflex saves for each of the NPC's classes.
 * ---------------------------------------------------------------------- */
int npcGetBaseReflexSave( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcGetBaseWillSave
 *
 * Computes the base will saving throw for the NPC by summing the
 * base will saves for each of the NPC's classes.
 * ---------------------------------------------------------------------- */
int npcGetBaseWillSave( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcGetBaseAttack
 *
 * Computes the base attack bonus for the NPC by calling 
 * npcGetBaseClassAttack to get the base attack bonus based on the NPC's
 * classes, and adds any racial bonus to attack that the NPC has.
 * ---------------------------------------------------------------------- */
int npcGetBaseAttack( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcGetBaseClassAttack
 *
 * Computes the base attack bonus for the NPC by summing the base attack
 * bonuses for each of the NPC's classes.
 * ---------------------------------------------------------------------- */
int npcGetBaseClassAttack( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcGetCharacterLevel
 *
 * Computes the character level of the NPC by summing the levels of each
 * class the NPC has taken.
 * ---------------------------------------------------------------------- */
int npcGetCharacterLevel( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcComputeActualAC
 *
 * Computes the actual armor class of the NPC, taking into account ability
 * score bonuses, racial bonuses, class bonuses, etc.
 *
 * If the 'breakdown' parameter is non-null, it will contain a list of
 * the various components that comprised the AC.  (See
 * npcBuildComponentBreakdownDescription and npcDestroyComponentBreakdown.)
 * ---------------------------------------------------------------------- */
int npcComputeActualAC( NPC* npc, NPCCOMPBREAKDOWN** breakdown );

/* ---------------------------------------------------------------------- *
 * npcComputeActualInitiative
 *
 * Computes the initiative bonus of the NPC, taking into account ability
 * score bonuses, racial bonuses, class bonuses, etc.
 *
 * If the 'breakdown' parameter is non-null, it will contain a list of
 * the various components that comprised the AC.  (See
 * npcBuildComponentBreakdownDescription and npcDestroyComponentBreakdown.)
 * ---------------------------------------------------------------------- */
int npcComputeActualInitiative( NPC* npc, NPCCOMPBREAKDOWN** breakdown );

/* ---------------------------------------------------------------------- *
 * npcComputeActualSave
 *
 * Computes the given saving throw of the NPC, taking into account ability
 * score bonuses, racial bonuses, class bonuses, etc.
 *
 * 'save' must be one of svFORTITUDE, svREFLEX, or svWILL.
 *
 * If the 'breakdown' parameter is non-null, it will contain a list of
 * the various components that comprised the AC.  (See
 * npcBuildComponentBreakdownDescription and npcDestroyComponentBreakdown.)
 * ---------------------------------------------------------------------- */
int npcComputeActualSave( NPC* npc, int save, NPCCOMPBREAKDOWN** breakdown );

/* ---------------------------------------------------------------------- *
 * npcComputeActualAttack
 *
 * Computes the given attack bonus of the NPC, taking into account ability
 * score bonuses, racial bonuses, class bonuses, etc.
 *
 * 'save' must be one of attMELEE or attRANGED.
 *
 * If the 'breakdown' parameter is non-null, it will contain a list of
 * the various components that comprised the AC.  (See
 * npcBuildComponentBreakdownDescription and npcDestroyComponentBreakdown.)
 * ---------------------------------------------------------------------- */
int npcComputeActualAttack( NPC* npc, int type, NPCCOMPBREAKDOWN** breakdown );

/* ---------------------------------------------------------------------- *
 * npcComputeActualSkill
 *
 * Computes the given skill bonus of the NPC, taking into account ability
 * score bonuses, racial bonuses, class bonuses, skill ranks taken, etc.
 *
 * 'type' must be one of the sk--- constants.
 *
 * If the 'breakdown' parameter is non-null, it will contain a list of
 * the various components that comprised the AC.  (See
 * npcBuildComponentBreakdownDescription and npcDestroyComponentBreakdown.)
 * ---------------------------------------------------------------------- */
float npcComputeActualSkillBonus( NPC* npc, int type, NPCCOMPBREAKDOWN** breakdown );

/* ---------------------------------------------------------------------- *
 * npcBuildComponentBreakdownDescription
 *
 * Builds a description of the given component breakdown.  The description
 * is either an empty string (if there were no components to describe), or
 * a parenthetical list of comma delimited items of the format
 * <bonus> <type>, where <bonus> is a numeric bonus or penalty and <type>
 * is Racial, Natural, Size, Base, Str, Dex, Con, Int, Wis, Cha, etc.  If
 * the description is longer than the indicated buffer length, the
 * description will be truncated.
 * ---------------------------------------------------------------------- */
char* npcBuildComponentBreakdownDescription( NPCCOMPBREAKDOWN* breakdown, 
                                             char* buffer, 
                                             int len );

/* ---------------------------------------------------------------------- *
 * npcBuildHitDiceBreakdown
 *
 * Builds a description of the hit dice of the given NPC, including the
 * hit dice of the base creature (where applicable) and the hit dice of the
 * component classes.  Constitution bonuses are also indicated.  If the
 * description is longer than the indicated buffer length, it will be 
 * truncated.
 * ---------------------------------------------------------------------- */
char* npcBuildHitDiceBreakdown( NPC* npc, char* buffer, int len );

/* ---------------------------------------------------------------------- *
 * npcBuildStatBlock
 *
 * Builds a brief stat-block description of the NPC into the indicated
 * buffer.  If the buffer is not long enough to contain the entire stat-
 * block, the stat-block will be truncated.  The stat-block will contain
 * formatting features as well, which need to be interpreted correctly for
 * clean display:
 *
 * ~I<text>~i - italics, where <text> is the text to italicize.
 * ~B<text>~b - bold, where <text> is the text to bold.
 * <newline> - new-line (\n) characters, indicating line breaks.
 * ---------------------------------------------------------------------- */
char* npcBuildStatBlock( NPC* npc, NPCSTATBLOCKOPTS* opts, char* buffer, int len );

/* ---------------------------------------------------------------------- *
 * npcDestroyComponentBreakdown
 *
 * Destroys (deallocates) the given component breakdown.
 * ---------------------------------------------------------------------- */
void npcDestroyComponentBreakdown( NPCCOMPBREAKDOWN* breakdown );

/* ---------------------------------------------------------------------- *
 * npcRandomName
 *
 * Copies a randomly generated name appropriate to the given race and
 * gender into the given buffer.  If the buffer is not long enough, it will
 * be truncated.  The buffer is also returned as the return value.
 *
 * 'filePath' represents an absolute or relative path from the current
 * application execution path to the location of the grammar files that
 * define the names.  These files are:
 *
 * - human_names.txt
 * - dwarf_names.txt
 * - elf_names.txt
 * - gnome_names.txt
 * - halfling_names.txt
 * - monstrous_names.txt
 * ---------------------------------------------------------------------- */
char* npcRandomName( int race, int gender, char* filePath, char* name, int buflen );

/* ---------------------------------------------------------------------- *
 * npcComputeCR
 *
 * Computes the challenge rating (CR) of the given NPC.
 * ---------------------------------------------------------------------- */
int npcComputeCR( NPC* npc );

/* ---------------------------------------------------------------------- *
 * npcAbScoreStrategy_Best3Of4D6
 *
 * Computes the ability scores by taking the best 3 of 4d6, rolled 6
 * times.
 * ---------------------------------------------------------------------- */
void npcAbScoreStrategy_BestOf4d6( int* scores, void* data );

/* ---------------------------------------------------------------------- *
 * npcAbScoreStrategy_Straight3d6
 *
 * Computes the ability scores by rolling 3d6 6 times.
 * ---------------------------------------------------------------------- */
void npcAbScoreStrategy_Straight3d6( int* scores, void* data );

/* ---------------------------------------------------------------------- *
 * int npcGetRandomRace( int raceType )
 *
 * Gets a random race based on the value of raceType.  If raceType is one
 * of the rc-- constants, that value is returned.  If raceType is one of
 * the race-- constants, a race is randomly selected appropriate to the
 * constant that was specified.
 * ---------------------------------------------------------------------- */
int npcGetRandomRace( int raceType );

/* ---------------------------------------------------------------------- *
 * int npcGetRandomAlignment( int alType )
 *
 * Gets a random alignment based on the value of alType, which must be
 * one of the al-- constants.
 * ---------------------------------------------------------------------- */
int npcGetRandomAlignment( int alType );

#ifdef __cplusplus
}
#endif

#endif /* __NPCENGINE_H__ */
