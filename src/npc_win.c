#include <windows.h>
#include <richedit.h>
#include <commctrl.h>
#include <shlobj.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "resource.h"

#include "npcEngine.h"
#include "grammar.h"
#include "gameutil.h"
#include "dndconst.h"
#include "pcgen_interface.h"
#include "wtstream.h"

typedef struct {
  char*  text;
  DWORD  data;
} DDDATA;


typedef struct {
  NPCSTATBLOCKOPTS displayOpts;
  HMENU            popMenu;
  NPC**            npcs;
  int              count;
  OPENFILENAME     export;
} WINDATA;

#define MAX_NPCS ( 40 )

static DDDATA alignments[] = {
  { "-- Any --", alANY },
  { "-- Any Lawful --", alANY_LAWFUL },
  { "-- Any Non-Lawful --", alANY_NONLAWFUL },
  { "-- Any Neutral --", alANY_LCNEUTRAL },
  { "-- Any Chaotic --", alANY_CHAOTIC },
  { "-- Any Non-Chaotic --", alANY_NONCHAOTIC },
  { "-- Any Good --", alANY_GOOD },
  { "-- Any Non-Good --", alANY_NONGOOD },
  { "-- Any Neutral --", alANY_GENEUTRAL },
  { "-- Any Evil --", alANY_EVIL },
  { "-- Any Non-Evil --", alANY_NONEVIL },
  { "Lawful Good", alLAWFUL | alGOOD },
  { "Lawful Neutral", alLAWFUL | alGENEUTRAL },
  { "Lawful Evil", alLAWFUL | alEVIL },
  { "Neutral Good", alLCNEUTRAL | alGOOD },
  { "True Neutral", alLCNEUTRAL | alGENEUTRAL },
  { "Neutral Evil", alLCNEUTRAL | alEVIL },
  { "Chaotic Good", alCHAOTIC | alGOOD },
  { "Chaotic Neutral", alCHAOTIC | alGENEUTRAL },
  { "Chaotic Evil", alCHAOTIC | alEVIL },
  { 0, 0 }
};

static DDDATA genders[] = {
  { "-- Any --", genderANY },
  { "Male", gMALE },
  { "Female", gFEMALE },
  { 0, 0 }
};

static DDDATA races[] = {
  { "-- Any Core Race --", raceANY },
  { "-- Any PHB Race --", raceANY_CORE },
  { "  Dwarf, Hill", rcDWARF_HILL },
  { "  Elf, High", rcELF_HIGH },
  { "  Gnome, Rock", rcGNOME_ROCK },
  { "  Half-elf", rcHALFELF },
  { "  Halfling, Lightfoot", rcHALFLING_LIGHTFOOT },
  { "  Half-orc", rcHALFORC },
  { "  Human", rcHUMAN },
  { "-- Any DMG Race --", raceANY_DMG },
  { "  Aasimar", rcAASIMAR },
  { "  Bugbear", rcBUGBEAR },
  { "  Doppleganger", rcDOPPLEGANGER },
  { "  Dwarf, Deep", rcDWARF_DEEP },
  { "  Dwarf, Derro", rcDWARF_DERRO },
  { "  Dwarf, Duergar", rcDWARF_DUERGAR },
  { "  Dwarf, Mountain", rcDWARF_MOUNTAIN },
  { "  Elf, Aquatic", rcELF_AQUATIC },
  { "  Elf, Drow", rcELF_DROW },
  { "  Elf, Gray", rcELF_GRAY },
  { "  Elf, Wild", rcELF_WILD },
  { "  Elf, Wood", rcELF_WOOD },
  { "  Gnoll", rcGNOLL },
  { "  Gnome, Forest", rcGNOME_FOREST },
  { "  Gnome, Svirfneblin", rcGNOME_SVIRFNEBLIN },
  { "  Goblin", rcGOBLIN },
  { "  Halfling, Deep", rcHALFLING_DEEP },
  { "  Halfling, Tallfellow", rcHALFLING_TALLFELLOW },
  { "  Hobgoblin", rcHOBGOBLIN },
  { "  Kobold", rcKOBOLD },
  { "  Lizardfolk", rcLIZARDFOLK },
  { "  Mind Flayer", rcMINDFLAYER },
  { "  Minotaur", rcMINOTAUR },
  { "  Ogre", rcOGRE },
  { "  Ogre Mage", rcOGREMAGE },
  { "  Orc", rcORC },
  { "  Tiefling", rcTIEFLING },
  { "  Troglodyte", rcTROGLODYTE },
  { "-- Any MM Race --", raceANY_MM },
  { "  Aranea", rcARANEA },
  { "  Azer", rcAZER },
  { "  Centaur", rcCENTAUR },
  { "  Drider", rcDRIDER },
  { "  Ettin", rcETTIN },
  { "  Giant, Cloud", rcGIANT_CLOUD },
  { "  Giant, Fire", rcGIANT_FIRE },
  { "  Giant, Frost", rcGIANT_FROST },
  { "  Giant, Hill", rcGIANT_HILL },
  { "  Giant, Stone", rcGIANT_STONE },
  { "  Giant, Storm", rcGIANT_STORM },
  { "  Grimlock", rcGRIMLOCK },
  { "  Hag, Annis", rcHAG_ANNIS },
  { "  Hag, Green", rcHAG_GREEN },
  { "  Hag, Sea", rcHAG_SEA },
  { "  Harpy", rcHARPY },
  { "  Kuo-toa", rcKUOTOA },
  { "  Locathah", rcLOCATHAH },
  { "  Medusa", rcMEDUSA },
  { "  Sahuagin", rcSAHUAGIN },
  { "  Troll", rcTROLL },
  { "  Yuan-ti, Abomination", rcYUANTI_ABOMINATION },
  { "  Yuan-ti, Halfblood", rcYUANTI_HALFBLOOD },
  { "  Yuan-ti, Pureblood", rcYUANTI_PUREBLOOD },
  { "-- Any Crea. Col. Race --", raceANY_CC },
  { "  Abandoned", rcCC_ABANDONED },
  { "  Asaath", rcCC_ASAATH },
  { "  Bat Devil", rcCC_BATDEVIL },
  { "  C. Krewe, Plague Wretch", rcCC_PLAGUEWRETCH },
  { "  Charduni", rcCC_CHARDUNI },
  { "  Coal Goblin", rcCC_COALGOBLIN },
  { "  Dwarf, Forsaken", rcCC_DWARF_FORSAKEN },
  { "  Elf, Forsaken", rcCC_ELF_FORSAKEN },
  { "  Ice Ghoul", rcCC_ICE_GHOUL },
  { "  Proud", rcCC_PROUD },
  { "  Manticora", rcCC_MANTICORA },
  { "  Morgaunt", rcCC_MORGAUNT },
  { "  Ratman", rcCC_RATMAN },
  { "  Ratman, Brown Gorger", rcCC_RATMAN_BROWNGORGER },
  { "  Ratman, The Diseased", rcCC_RATMAN_DISEASED },
  { "  Ratman, Foamer", rcCC_RATMAN_FOAMER },
  { "  Ratman, Red Witch", rcCC_RATMAN_REDWITCH },
  { "  Skin Devil", rcCC_SKINDEVIL },
  { "  Spider-eye Goblin", rcCC_SPIDEREYEGOBLIN },
  { "  Steppe Troll", rcCC_STEPPETROLL },
  { "  Strife Elemental", rcCC_STRIFEELEMENTAL },
  { "  Tokal Tribesman", rcCC_TOKALTRIBESMAN },
  { "  Ubantu Tribesman", rcCC_UBANTUTRIBESMAN },
  { 0, 0 }
};

static DDDATA classes[] = {
  { "-- None --", classNONE },
  { "-- Any Class --", classANY },
  { "-- Any PC Class", classANY_PC },
  { "  Barbarian", pcBARBARIAN },
  { "  Bard", pcBARD },
  { "  Cleric", pcCLERIC },
  { "  Druid", pcDRUID },
  { "  Fighter", pcFIGHTER },
  { "  Monk", pcMONK },
  { "  Paladin", pcPALADIN },
  { "  Ranger", pcRANGER },
  { "  Rogue", pcROGUE },
  { "  Sorcerer", pcSORCERER },
  { "  Wizard", pcWIZARD },
  { "-- Any NPC Class", classANY_NPC },
  { "  Adept", npcADEPT },
  { "  Aristocrat", npcARISTOCRAT },
  { "  Commoner", npcCOMMONER },
  { "  Expert", npcEXPERT },
  { "  Warrior", npcWARRIOR },
  { 0, 0 }
};

static DDDATA levels[] = {
  { "-- Any (1-20) --", levelANY },
  { "-- Low (1-5) --", levelLOW },
  { "-- Mid (6-12) --", levelMID },
  { "-- Hi (13-20) --", levelHI },
  { "  1", 1 },
  { "  2", 2 },
  { "  3", 3 },
  { "  4", 4 },
  { "  5", 5 },
  { "  6", 6 },
  { "  7", 7 },
  { "  8", 8 },
  { "  9", 9 },
  { "  10", 10 },
  { "  11", 11 },
  { "  12", 12 },
  { "  13", 13 },
  { "  14", 14 },
  { "  15", 15 },
  { "  16", 16 },
  { "  17", 17 },
  { "  18", 18 },
  { "  19", 19 },
  { "  20", 20 },
  { 0, 0 }
};

void heroicAbilities( int* scores, void* data );
void straight18s( int* scores, void* data );
void averageAbilities( int* scores, void* data );

static DDDATA strategies[] = {
  { "Best 3 of 4d6", (DWORD)npcAbScoreStrategy_BestOf4d6 },
  { "Straight 3d6", (DWORD)npcAbScoreStrategy_Straight3d6 },
  { "Average", (DWORD)averageAbilities },
  { "Heroic", (DWORD)heroicAbilities },
  { "Straight 18's", (DWORD)straight18s },
  { 0, 0 }
};


int           MessageBoxf( HWND hWnd, LPSTR title, UINT flags, LPSTR format, ... );
int WINAPI    WinMain( HINSTANCE, HINSTANCE, LPSTR, INT );
int           InitApplication( HINSTANCE );
int           InitInstance( LPSTR, UINT );
BOOL CALLBACK MainNPCProc( HWND, UINT, WPARAM, LPARAM );
int           InitMainWindow( HWND );
void          CloseMainWindow( HWND );
void          PopulateDropDown( HWND, DDDATA*, int start );
void          GenerateNPCs( HWND );
DWORD         GetSelectedData( HWND );
BOOL CALLBACK AboutNPCProc( HWND, UINT, WPARAM, LPARAM );
int           InitAboutWindow( HWND );
void          CloseAboutWindow( HWND );
BOOL CALLBACK ConfigNPCProc( HWND, UINT, WPARAM, LPARAM );
int           InitConfigWindow( HWND );
void          CloseConfigWindow( HWND, int );
void          doTestForContextMenu( HWND, WPARAM, LPARAM );
void          DestroyNPCs( HWND );
void          ExportNPCs( HWND );

HINSTANCE ghInst;


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


int MessageBoxf( HWND hWnd, LPSTR title, UINT flags, LPSTR format, ... ) {
  char buffer[ 4096 ];
  va_list args;

  va_start( args, format );
  vsprintf( buffer, format, args );
  va_end( args );

  return MessageBox( hWnd, buffer, title, flags );
}


int
WINAPI
WinMain( HINSTANCE hInstance,
         HINSTANCE hPrevInstance,
         LPSTR lpCmdLine,
         INT nCmdShow )
{
  if( InitApplication( hInstance ) == 0 )
  {
    MessageBox( NULL, "Could not initialize application.", "Error!", MB_OK | MB_ICONSTOP );
    return FALSE;
  }

  if( InitInstance( lpCmdLine, nCmdShow ) == 0 )
  {
	  MessageBox( NULL, "Could not initialize instance.", "Error!", MB_OK | MB_ICONSTOP );
	  return FALSE;
	}

  return FALSE;
}


int
InitApplication( HINSTANCE hInstance )
{
  ghInst = hInstance;

  srand( time( NULL ) );

  return TRUE;
}


int
InitInstance( LPSTR lpCmdLine, UINT nCmdShow )
{
//  HMODULE hRTFMod20;
  HMODULE hRTFMod10;

//  hRTFMod20 = LoadLibrary( "RICHED20.DLL" ); /* uses RICHEDIT_CLASS */
  hRTFMod10 = LoadLibrary( "RICHED32.DLL" ); /* uses RICHEDIT_CLASS10A */

  DialogBoxParam( ghInst, MAKEINTRESOURCE( IDD_NPC ), NULL, MainNPCProc, 0 );

  FreeLibrary( hRTFMod10 );
//  FreeLibrary( hRTFMod20 );

  return TRUE;
}


void CloseMainWindow( HWND hWnd ) {
  WINDATA* data;

  data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );
  DestroyNPCs( hWnd );
  free( data );

  EndDialog( hWnd, 0 );
}


BOOL 
CALLBACK 
MainNPCProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  switch( msg )
  {
    case WM_INITDIALOG:
      return InitMainWindow( hWnd );

    case WM_CLOSE:
      CloseMainWindow( hWnd );
      return 1;

    case WM_COMMAND:
      switch( LOWORD( wParam ) ) {
        case IDC_GENERATE:
          GenerateNPCs( hWnd );
          return 1;
        case IDC_ABOUT:
          DialogBoxParam( ghInst, MAKEINTRESOURCE( IDD_ABOUT ), NULL, AboutNPCProc, 0 );
          return 1;
        case IDC_PCGENEXPORT:
          ExportNPCs( hWnd );
          return 1;
        case IDC_CONFIG:
          DialogBoxParam( ghInst, MAKEINTRESOURCE( IDD_CONFIG ), NULL, ConfigNPCProc, GetWindowLong( hWnd, GWL_USERDATA ) );
          return 1;
        case IDC_EDIT_SELECT_ALL:
          {
            HWND ctrl;
            long size;
            CHARRANGE cr;

            ctrl = GetDlgItem( hWnd, IDC_NPCS );
            size = SendMessage( ctrl, WM_GETTEXTLENGTH, 0, 0 );
            cr.cpMin = 0;
            cr.cpMax = size;

            SendMessage( ctrl, EM_EXSETSEL, 0, (LPARAM)&cr );
          }
          break;
        case IDC_EDIT_COPY:
          {
            HANDLE mem;
            long   size;
            HWND ctrl;
            CHARRANGE cr;
            LPVOID buf;

            OpenClipboard( hWnd );

            ctrl = GetDlgItem( hWnd, IDC_NPCS );
            SendMessage( ctrl, EM_EXGETSEL, 0, (LPARAM)&cr );
            size = cr.cpMax - cr.cpMin;
            if( size > 0 ) {
              size++;
              mem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, size );
              buf = GlobalLock( mem );
              SendMessage( ctrl, EM_GETSELTEXT, 0, (LPARAM)buf );
              GlobalUnlock( mem );
              SetClipboardData( CF_TEXT, mem );
            }

            CloseClipboard();
          }
          break;
        default:
          break;
      }
      break;

    case WM_CONTEXTMENU:
      doTestForContextMenu( hWnd, wParam, lParam );
      break;
	}

	return 0;
}


int
InitMainWindow( HWND hWnd )
{
  HICON icon;
  WINDATA* data;

  data = (WINDATA*)malloc( sizeof( WINDATA ) );
  memset( data, 0, sizeof( *data ) );

  data->export.lStructSize = sizeof( data->export );
  data->export.hwndOwner = hWnd;
  data->export.hInstance = ghInst;

  icon = LoadIcon( ghInst, MAKEINTRESOURCE( IDI_NPC ) );
  SetClassLong( hWnd, GCL_HICON, (LONG)icon );

  data->popMenu = LoadMenu( ghInst, MAKEINTRESOURCE( IDR_EDITMENU ) );

  data->displayOpts.acBreakdown = 1;
  data->displayOpts.initBreakdown = 1;
  data->displayOpts.languages = 1;
  data->displayOpts.possessions = 1;
  data->displayOpts.skillsAndFeats = 1;
  data->displayOpts.spells = 1;

  SetWindowLong( hWnd, GWL_USERDATA, (LONG)data );

  SendMessage( GetDlgItem( hWnd, IDC_NPCS ), EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS );

  PopulateDropDown( GetDlgItem( hWnd, IDC_ALIGNMENT ), alignments, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_GENDER ), genders, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_RACE ), races, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_CLASS1 ), classes, 1 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_LEVEL1 ), levels, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_CLASS2 ), classes, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_LEVEL2 ), levels, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_CLASS3 ), classes, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_LEVEL3 ), levels, 0 );
  PopulateDropDown( GetDlgItem( hWnd, IDC_ABSTRATEGY ), strategies, 0 );

  SetWindowText( GetDlgItem( hWnd, IDC_COUNT ), "1" );
  SetWindowText( GetDlgItem( hWnd, IDC_SEED ), "" );
  SetWindowText( GetDlgItem( hWnd, IDC_CURRENTSEED ), "" );
  SetWindowText( GetDlgItem( hWnd, IDC_NPCS ), "" );
 
  return TRUE;
}


void PopulateDropDown( HWND dropdown, DDDATA* data, int start ) {
  int i;
  int idx;

  for( i = start; data[ i ].text != 0; i++ ) {
    idx = SendMessage( dropdown, CB_ADDSTRING, 0, (LPARAM)data[i].text );
    SendMessage( dropdown, CB_SETITEMDATA, idx, (LPARAM)data[i].data );
  }

  SendMessage( dropdown, CB_SETCURSEL, 0, 0 );
}


#define SEPARATOR "\r\n-----------------------------\r\n"

void GenerateNPCs( HWND hWnd ) {
  NPCGENERATOROPTS opts;
  NPCGENERATOROPTS temp_opts;
  char fbuf[1024];
  char temp[100];
  int i;
  int count;
  long seed;
  char* bigbuf;
  int   bigbuflen;
  char* tempbuf;
  int   tempbuflen;
  char* tmp;
  int   seplen;
  WINDATA* data;

  data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );
  if( data->count > 0 ) {
    DestroyNPCs( hWnd );
  }

  memset( &opts, 0, sizeof( opts ) );
  opts.raceType = GetSelectedData( GetDlgItem( hWnd, IDC_RACE ) );
  opts.alignment = GetSelectedData( GetDlgItem( hWnd, IDC_ALIGNMENT ) );
  opts.classType[0] = GetSelectedData( GetDlgItem( hWnd, IDC_CLASS1 ) );
  opts.level[0] = GetSelectedData( GetDlgItem( hWnd, IDC_LEVEL1 ) );
  opts.classType[1] = GetSelectedData( GetDlgItem( hWnd, IDC_CLASS2 ) );
  opts.level[1] = GetSelectedData( GetDlgItem( hWnd, IDC_LEVEL2 ) );
  opts.classType[2] = GetSelectedData( GetDlgItem( hWnd, IDC_CLASS3 ) );
  opts.level[2] = GetSelectedData( GetDlgItem( hWnd, IDC_LEVEL3 ) );
  opts.gender = GetSelectedData( GetDlgItem( hWnd, IDC_GENDER ) );
  opts.abilityScoreStrategy = (NPCABSCOREGENFUNC)GetSelectedData( GetDlgItem( hWnd, IDC_ABSTRATEGY ) );

  fbuf[ 0 ] = 0;
  GetModuleFileName( ghInst, fbuf, sizeof( fbuf ) );
  i = strlen( fbuf ) - 1;
  while( i >= 0 ) {
    if( fbuf[i] == '\\' ) {
      fbuf[i+1] = 0;
      break;
    }
    i--;
  }

  opts.filePath = fbuf;

  GetWindowText( GetDlgItem( hWnd, IDC_COUNT ), temp, sizeof( temp ) );
  count = atoi( temp );
  if( count < 1 ) {
    count = 1;
  }
  if( count > MAX_NPCS ) {
    count = MAX_NPCS;
  }
  sprintf( temp, "%d", count );
  SetWindowText( GetDlgItem( hWnd, IDC_COUNT ), temp );

  GetWindowText( GetDlgItem( hWnd, IDC_SEED ), temp, sizeof( temp ) );
  seed = atol( temp );
  if( seed < 1 ) {
    seed = time(NULL);
  }
  sprintf( temp, "%ld", seed );
  SetWindowText( GetDlgItem( hWnd, IDC_CURRENTSEED ), temp );

  srand( seed );

  data->count = count;
  data->npcs = (NPC**)malloc( sizeof( NPC* ) * count );

  for( i = 0; i < count; i++ ) {
    memcpy( &temp_opts, &opts, sizeof( temp_opts ) );
    data->npcs[i] = npcGenerateNPC( &temp_opts );
  }

  bigbuf = 0;
  bigbuflen = 0;
  tempbuflen = 32 * 1024;
  tempbuf = (char*)malloc( tempbuflen );
  seplen = strlen( SEPARATOR );

  for( i = 0; i < count; i++ ) {
    npcBuildStatBlock( data->npcs[i], &(data->displayOpts), tempbuf, tempbuflen );

    strrepl( tempbuf, "~I", "" );
    strrepl( tempbuf, "~i", "" );
    strrepl( tempbuf, "~B", "" );
    strrepl( tempbuf, "~b", "" );
    strrepl( tempbuf, "\n", "\r\n" );

    bigbuflen += strlen( tempbuf ) + seplen;

    tmp = (char*)malloc( bigbuflen + 1 );
    *tmp = 0;

    if( bigbuf != 0 ) {
      strcpy( tmp, bigbuf );
      free( bigbuf );
    }

    strcat( tmp, tempbuf );

    if( i+1 < count ) {
      strcat( tmp, SEPARATOR );
    }

    bigbuf = tmp;
  }

  i = SetWindowText( GetDlgItem( hWnd, IDC_NPCS ), bigbuf );

  free( bigbuf );
  free( tempbuf );

  EnableWindow( GetDlgItem( hWnd, IDC_PCGENEXPORT ), TRUE );
}


DWORD GetSelectedData( HWND dropdown ) {
  int idx;

  idx = SendMessage( dropdown, CB_GETCURSEL, 0, 0 );
  return SendMessage( dropdown, CB_GETITEMDATA, idx, 0 );
}


BOOL CALLBACK AboutNPCProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch( msg )
  {
    case WM_INITDIALOG:
      return InitAboutWindow( hWnd );

    case WM_CLOSE:
      CloseAboutWindow( hWnd );
      return 1;

    case WM_COMMAND:
      switch( LOWORD( wParam ) ) {
        case IDOK:
          CloseAboutWindow( hWnd );
          return 1;
      }
      break;
	}

	return 0;
}


int InitAboutWindow( HWND hWnd ) {
  SetWindowText( GetDlgItem( hWnd, IDC_ABOUTTEXT ),
     "This NPC Generator was created by Jamis Buck (jgb3@email.byu.edu) and is "
     "also available online at http://www.aarg.net/~minam/npc.cgi.  Other generators "
     "may be accessed from Jamis Buck's RPG Generators, at http://www.enworld.org/generators.\r\n\r\n"
     "This generator took a great deal of time and effort to create, so if you find it particularly enjoyable "
     "or useful, the author would appreciate any donation you could make, even as little as $1.  The proceeds "
     "will go to creating more and better role playing tools, both on- and off-line.\r\n\r\n"
     "Please send your donation (check or money-order) to:\r\n\r\n"
     "Jamis Buck\r\n"
     "2847 W 1060 N\r\n"
     "Provo, UT  84601\r\n\r\n"
     "Other options for donations (including PayPal) may be found at "
     "Jamis Buck's RPG Generators (at the URL indicated above).\r\n\r\n"
     "Thank-you for using the NPC generator!" );

  return 0;
}


void CloseAboutWindow( HWND hWnd ) {
  EndDialog( hWnd, 0 );
}


BOOL CALLBACK ConfigNPCProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
  switch( msg )
  {
    case WM_INITDIALOG:
      SetWindowLong( hWnd, GWL_USERDATA, lParam );
      return InitConfigWindow( hWnd );

    case WM_CLOSE:
      CloseConfigWindow( hWnd, 0 );
      return 1;

    case WM_COMMAND:
      switch( LOWORD( wParam ) ) {
        case IDOK:
          CloseConfigWindow( hWnd, 1 );
          return 1;
        case IDCANCEL:
          CloseConfigWindow( hWnd, 0 );
          return 1;
      }
      break;
	}

	return 0;
}


void doSetCheck( HWND hWnd, int item, int set ) {
  SendMessage( GetDlgItem( hWnd, item ), BM_SETCHECK, set, 0 );
}

int doGetCheck( HWND hWnd, int item ) {
  return ( SendMessage( GetDlgItem( hWnd, item ), BM_GETCHECK, 0, 0 ) == BST_CHECKED );
}


int InitConfigWindow( HWND hWnd ) {
  WINDATA* data;

  data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );

  if( data->displayOpts.abilityBonuses ) {
    doSetCheck( hWnd, IDC_STATSABILITYBONUSES, 1 );
  }
  if( data->displayOpts.acBreakdown ) {
    doSetCheck( hWnd, IDC_STATSACBREAKDOWN, 1 );
  }
  if( data->displayOpts.attackBreakdown ) {
    doSetCheck( hWnd, IDC_STATSATTACKBREAKDOWN, 1 );
  }
  if( data->displayOpts.initBreakdown ) {
    doSetCheck( hWnd, IDC_STATSINITBREAKDOWN, 1 );
  }
  if( data->displayOpts.languages ) {
    doSetCheck( hWnd, IDC_STATSLANGUAGES, 1 );
  }
  if( data->displayOpts.possessions ) {
    doSetCheck( hWnd, IDC_STATSPOSSESSIONS, 1 );
  }
  if( data->displayOpts.saveBreakdown ) {
    doSetCheck( hWnd, IDC_STATSSAVEBREAKDOWN, 1 );
  }
  if( data->displayOpts.skillBreakdown ) {
    doSetCheck( hWnd, IDC_STATSSKILLBREAKDOWN, 1 );
  }
  if( data->displayOpts.skillsAndFeats ) {
    doSetCheck( hWnd, IDC_STATSSKILLSFEATS, 1 );
  }
  if( data->displayOpts.spells ) {
    doSetCheck( hWnd, IDC_STATSSPELLS, 1 );
  }

  return 0;
}


void CloseConfigWindow( HWND hWnd, int doSave ) {
  if( doSave ) {
    WINDATA* data;
    data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );

    data->displayOpts.abilityBonuses = doGetCheck( hWnd, IDC_STATSABILITYBONUSES );
    data->displayOpts.acBreakdown = doGetCheck( hWnd, IDC_STATSACBREAKDOWN );
    data->displayOpts.attackBreakdown = doGetCheck( hWnd, IDC_STATSATTACKBREAKDOWN );
    data->displayOpts.initBreakdown = doGetCheck( hWnd, IDC_STATSINITBREAKDOWN );
    data->displayOpts.languages = doGetCheck( hWnd, IDC_STATSLANGUAGES );
    data->displayOpts.possessions = doGetCheck( hWnd, IDC_STATSPOSSESSIONS );
    data->displayOpts.saveBreakdown = doGetCheck( hWnd, IDC_STATSSAVEBREAKDOWN );
    data->displayOpts.skillBreakdown = doGetCheck( hWnd, IDC_STATSSKILLBREAKDOWN );
    data->displayOpts.skillsAndFeats = doGetCheck( hWnd, IDC_STATSSKILLSFEATS );
    data->displayOpts.spells = doGetCheck( hWnd, IDC_STATSSPELLS );
  }
  EndDialog( hWnd, doSave );
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


void setMenuItemState( HMENU menu, int item, int state ) {
  MENUITEMINFO mii;

  mii.cbSize = sizeof( mii );
  mii.fMask = MIIM_STATE;
  mii.fState = state;

  SetMenuItemInfo( menu, item, FALSE, &mii );
}


void doTestForContextMenu( HWND hWnd, WPARAM wParam, LPARAM lParam ) {
  HWND ctrl;
  RECT rect;
  RECT rect2;
  int  xpos;
  int  ypos;
  WINDATA *data;

  data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );

  ctrl = GetDlgItem( hWnd, IDC_NPCS );
  GetWindowRect( ctrl, &rect );
  GetWindowRect( hWnd, &rect2 );

  xpos = LOWORD( lParam );
  ypos = HIWORD( lParam );

  if( ( xpos >= rect.left ) && ( ypos >= rect.top ) &&
      ( xpos <= rect.right ) && ( ypos <= rect.bottom ) )
  {
    CHARRANGE cr;
    int       hasSelection;
    HMENU     menu;

    SendMessage( ctrl, EM_EXGETSEL, 0, (LPARAM)&cr );
    hasSelection = ( cr.cpMin < cr.cpMax );

    menu = GetSubMenu( data->popMenu, 0 );
    setMenuItemState( menu, IDC_EDIT_SELECT_ALL, ( hasSelection ? MFS_DISABLED | MFS_GRAYED : MFS_ENABLED ) );
    setMenuItemState( menu, IDC_EDIT_COPY, ( !hasSelection ? MFS_DISABLED | MFS_GRAYED : MFS_ENABLED ) );

    TrackPopupMenu( menu, TPM_LEFTALIGN, xpos, ypos, 0, hWnd, 0 );
  }
}


void DestroyNPCs( HWND hWnd ) {
  WINDATA *data;
  int i;

  data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );
  if( data->count > 0 ) {
    for( i = 0; i < data->count; i++ ) {
      npcDestroyNPC( data->npcs[ i ] );
    }
    free( data->npcs );
    data->count = 0;
    data->npcs = NULL;
  }
}


void ExportNPCs( HWND hWnd ) {
  WINDATA *data;
  int i;
  char fname[ 512 ];
  char path[ 512 ];
  wtSTREAM_t* stream;
  BROWSEINFO bi = { 0 };
  LPITEMIDLIST pidl;
  IMalloc* imalloc = 0;

  data = (WINDATA*)GetWindowLong( hWnd, GWL_USERDATA );
  if( data->count > 0 ) {
    bi.lpszTitle = "Please choose where the PCG files will be saved:";
    pidl = SHBrowseForFolder( &bi );

    if( pidl == 0 ) {
      return;
    }

    if( !SHGetPathFromIDList( pidl, path ) ) {
      MessageBoxf( hWnd, "Error", MB_OK, "An error occurred getting the path" );
    } else {
      for( i = 0; i < data->count; i++ ) {
        sprintf( fname, "%s\\%s.pcg", path, data->npcs[i]->name );
        stream = wtOpenFileStream( fname, "w" );
        if( stream == 0 ) {
          MessageBoxf( hWnd, "Error", MB_OK, "Could not open file '%s'", fname );
          return;
        }
        convertToPCGen( data->npcs[ i ], stream );
        wtCloseStream( stream );
      }

      MessageBoxf( hWnd, "Done", MB_OK, "Exported %d NPCs", data->count );
    }

    CoTaskMemFree( pidl );
  }
}
