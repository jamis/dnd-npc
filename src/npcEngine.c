/* ---------------------------------------------------------------------- *
 * npcEngine.c
 *
 * by Jamis Buck (jgb3@email.byu.edu)
 *
 * NPC generation functions for the Dungeons & Dragons(tm) API.  This file 
 * is in the public domain.
 * ---------------------------------------------------------------------- */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "dndconst.h"
#include "dndutil.h"
#include "npcEngine.h"
#include "grammar.h"
#include "gameutil.h"

#define PREFERRED_CLASS_CHANCE    ( 75 )

#define COMMON         ( 20 )
#define UNCOMMON       (  8 )
#define RARE           (  2 )
#define NEVER          (  0 )

typedef struct __raceabilities__ RACEABILITIES;
typedef struct __abilityscore__ ABILITYSCORE;
typedef struct __preferredfeats__ PREFERREDFEATS;
typedef struct __preferredweapons__ PREFERREDWEAPONS;
typedef struct __preferredspells__ PREFERREDSPELLS;

struct __raceabilities__ {
  int ability;
  int weight;
};


struct __abilityscore__ {
  int ability;
  int score;
};


struct __preferredfeats__ {
  int feat;
  int weight;
};


struct __preferredweapons__ {
  int weapon;
  int weight;
};


struct __preferredspells__ {
  int spell;
  int weight;
};


static const int allLanguages[] = { lnABYSSAL, lnAQUAN, lnAURAN, 
  lnCELESTIAL, lnCOMMON, lnDRACONIC, lnDWARVEN, lnELVEN, lnGNOME, 
  lnGOBLIN, lnGIANT, lnGNOLL, lnHALFLING, lnIGNAN, lnINFERNAL, lnORC, 
  lnSYLVAN, lnTERRAN, lnUNDERCOMMON, 0 };

static const int allSkills[] = { skALCHEMY, skANIMALEMPATHY, skAPPRAISE,
  skBALANCE, skBLUFF, skCLIMB, skCONCENTRATION, skCRAFT, skDECIPHERSCRIPT, 
  skDIPLOMACY, skDISABLEDEVICE, skDISGUISE, skESCAPEARTIST, skFORGERY, 
  skGATHERINFORMATION, skHANDLEANIMAL, skHEAL, skHIDE, skINNUENDO, 
  skINTIMIDATE, skINTUITDIRECTION, skJUMP, skKNOWLEDGE_ARCANA, 
  skKNOWLEDGE_RELIGION, skKNOWLEDGE_NATURE, skKNOWLEDGE, skLISTEN, 
  skMOVESILENTLY, skOPENLOCK, skPERFORM, skPICKPOCKET, skPROFESSION, 
  skREADLIPS, skRIDE, skSCRY, skSEARCH, skSENSEMOTIVE, skSPEAKLANGUAGE, 
  skSPELLCRAFT, skSPOT, skSWIM, skTUMBLE, skUSEMAGICDEVICE, skUSEROPE, 
  skWILDERNESSLORE, 0 };

static const int simpleWeapons[] = { wpGAUNTLET, wpUNARMED, wpDAGGER, 
  wpDAGGER_PUNCHING, wpGAUNTLET_SPIKED, wpMACE_LIGHT, wpSICKLE, wpCLUB, 
  wpHALFSPEAR, wpMACE_HEAVY, wpMORNINGSTAR, wpQUARTERSTAFF, wpSHORTSPEAR, 
  wpCROSSBOW_LIGHT, wpDART, wpSLING, wpCROSSBOW_HEAVY, wpJAVELIN, 0 };

static const int martialWeapons[] = { wpAXE_THROWING, wpHAMMER_LIGHT, 
  wpHANDAXE, wpLANCE_LIGHT, wpPICK_LIGHT, wpSAP, wpSWORD_SHORT, wpBATTLEAXE, 
  wpFLAIL_LIGHT, wpLANCE_HEAVY, wpLONGSWORD, wpPICK_HEAVY, wpRAPIER, 
  wpSCIMITAR, wpTRIDENT, wpWARHAMMER, wpFALCHION, wpFLAIL_HEAVY, wpGLAIVE, 
  wpGREATAXE, wpGREATCLUB, wpGREATSWORD, wpGUISARME, wpHALBERD, wpLONGSPEAR, 
  wpRANSEUR, wpSCYTHE, wpSHORTBOW, wpSHORTBOW_COMPOSITE, wpLONGBOW, 
  wpLONGBOW_COMPOSITE, 0 };

static const int exoticWeapons[] = { wpKUKRI, wpKAMA, wpNUNCHAKU, 
  wpSIANGHAM, wpSWORD_BASTARD, wpWARAXE_DWARVEN, wpHAMMER_GNOMEHOOKED, 
  wpAXE_ORCDOUBLE, wpCHAIN_SPIKED, wpFLAIL_DIRE, wpSWORD_TWOBLADED, 
  wpURGOSH_DWARVEN, wpCROSSBOW_HAND, wpSHURIKEN, wpWHIP, 
  wpCROSSBOW_REPEATING, wpNET, 0 };

static const int schoolsOfMagic[] = { ssUNIVERSAL, ssABJURATION, 
  ssCONJURATION, ssDIVINATION, ssENCHANTMENT, ssEVOCATION, ssILLUSION,
  ssNECROMANCY, ssTRANSMUTATION, 0 };

static const int requiredSkills[] = { skSPOT, skLISTEN, skHIDE, 
  skMOVESILENTLY, 0 };

/* preferred feat designations */

static PREFERREDFEATS barbarianFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON*2 },
  { ftARMORPROFICIENCY_HEAVY,     RARE },
  { ftBLINDFIGHT,                 COMMON },
  { ftCOMBATREFLEXES,             COMMON },
  { ftDODGE,                      UNCOMMON*2 },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  COMMON },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             UNCOMMON },
  { ftIMPROVEDCRITICAL,           UNCOMMON },
  { ftIMPROVEDINITIATIVE,         RARE*2 },
  { ftIMPROVEDUNARMEDSTRIKE,      RARE },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              RARE*2 },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE*2 },
  { ftRIDEBYATTACK,               RARE*2 },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             RARE },
  { ftFARSHOT,                    RARE },
  { ftPRECISESHOT,                RARE },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                COMMON*2 },
  { ftCLEAVE,                     COMMON },
  { ftIMPROVEDBULLRUSH,           UNCOMMON },
  { ftSUNDER,                     RARE*2 },
  { ftGREATCLEAVE,                COMMON },
  { ftQUICKDRAW,                  UNCOMMON },
  { ftRUN,                        COMMON },
  { ftSHIELDPROFICIENCY,          RARE },
  { ftSKILLFOCUS,                 RARE },
  { ftTOUGHNESS,                  RARE*2 },
  { ftTRACK,                      UNCOMMON },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                UNCOMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS barbarianWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE },
  { wpDAGGER,              RARE },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          RARE*2 },
  { wpSICKLE,              RARE },
  { wpCLUB,                UNCOMMON },
  { wpHALFSPEAR,           UNCOMMON },
  { wpMACE_HEAVY,          UNCOMMON },
  { wpMORNINGSTAR,         UNCOMMON },
  { wpQUARTERSTAFF,        RARE },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE*2 },
  { wpAXE_THROWING,        RARE },
  { wpHAMMER_LIGHT,        RARE },
  { wpHANDAXE,             RARE },
  { wpLANCE_LIGHT,         RARE },
  { wpPICK_LIGHT,          RARE },
  { wpSAP,                 RARE },
  { wpSWORD_SHORT,         UNCOMMON },
  { wpBATTLEAXE,           COMMON },
  { wpFLAIL_LIGHT,         UNCOMMON },
  { wpLANCE_HEAVY,         RARE },
  { wpLONGSWORD,           COMMON*2 },
  { wpPICK_HEAVY,          RARE },
  { wpRAPIER,              RARE },
  { wpSCIMITAR,            UNCOMMON },
  { wpTRIDENT,             RARE },
  { wpWARHAMMER,           COMMON },
  { wpFALCHION,            RARE },
  { wpFLAIL_HEAVY,         UNCOMMON },
  { wpGLAIVE,              RARE },
  { wpGREATAXE,            COMMON },
  { wpGREATCLUB,           COMMON },
  { wpGREATSWORD,          COMMON },
  { wpGUISARME,            RARE },
  { wpHALBERD,             RARE },
  { wpLONGSPEAR,           UNCOMMON },
  { wpRANSEUR,             RARE },
  { wpSCYTHE,              RARE },
  { wpSHORTBOW,            UNCOMMON },
  { wpSHORTBOW_COMPOSITE,  UNCOMMON },
  { wpLONGBOW,             UNCOMMON },
  { wpLONGBOW_COMPOSITE,   UNCOMMON },
  { 0,                     0 }
};

static PREFERREDFEATS bardFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftARMORPROFICIENCY_HEAVY,     RARE },
  { ftBLINDFIGHT,                 UNCOMMON },
  { ftCOMBATCASTING,              COMMON },
  { ftCOMBATREFLEXES,             UNCOMMON },
  { ftDODGE,                      COMMON },
  { ftMOBILITY,                   COMMON },
  { ftSPRINGATTACK,               UNCOMMON },
  { ftENDURANCE,                  RARE },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIRONWILL,                   RARE },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          RARE },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    RARE },
  { ftPRECISESHOT,                RARE },
  { ftRAPIDSHOT,                  UNCOMMON },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftSUNDER,                     RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        UNCOMMON },
  { ftSKILLFOCUS,                 COMMON },
  { ftSPELLFOCUS,                 UNCOMMON },
  { ftSPELLPENETRATION,           RARE },
  { ftTOUGHNESS,                  RARE*2 },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftWEAPONFINESSE,              UNCOMMON },
  { ftWEAPONFOCUS,                RARE },
  { ftBREWPOTION,                 UNCOMMON*2 },
  { ftCRAFTMAGICARMSANDARMOR,     UNCOMMON },
  { ftCRAFTROD,                   UNCOMMON },
  { ftCRAFTSTAFF,                 UNCOMMON },
  { ftCRAFTWAND,                  UNCOMMON },
  { ftCRAFTWONDROUSITEM,          UNCOMMON*2 },
  { ftFORGERING,                  UNCOMMON },
  { ftSCRIBESCROLL,               UNCOMMON*2 },
  { ftEMPOWERSPELL,               RARE },
  { ftENLARGESPELL,               RARE },
  { ftEXTENDSPELL,                RARE },
  { ftHEIGHTENSPELL,              RARE },
  { ftMAXIMIZESPELL,              RARE },
  { ftSILENTSPELL,                RARE },
  { ftSTILLSPELL,                 RARE },
  { 0,                            0 }
};

static PREFERREDWEAPONS bardWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE },
  { wpDAGGER,              COMMON },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          RARE*2 },
  { wpSICKLE,              RARE },
  { wpCLUB,                RARE },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          UNCOMMON },
  { wpMORNINGSTAR,         UNCOMMON },
  { wpQUARTERSTAFF,        UNCOMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { wpSAP,                 COMMON },
  { wpSWORD_SHORT,         UNCOMMON },
  { wpLONGSWORD,           COMMON*2 },
  { wpRAPIER,              UNCOMMON },
  { wpSHORTBOW,            UNCOMMON },
  { wpSHORTBOW_COMPOSITE,  UNCOMMON },
  { wpLONGBOW,             UNCOMMON },
  { wpLONGBOW_COMPOSITE,   UNCOMMON },
  { wpWHIP,                RARE },
  { 0,                     0 }
};


static PREFERREDSPELLS prefBardSpells[] = {
  { spBLINDNESSDEAFNESS,                     RARE },
  { spBLINK,                                 UNCOMMON },
  { spBLUR,                                  UNCOMMON },
  { spCHARMMONSTER,                          COMMON },
  { spCHARMPERSON,                           COMMON },
  { spCLAIRAUDIENCECLAIRVOYANCE,             UNCOMMON },
  { spCURECRITICALWOUNDS,                    COMMON },
  { spCURELIGHTWOUNDS,                       COMMON },
  { spCUREMODERATEWOUNDS,                    COMMON },
  { spCURESERIOUSWOUNDS,                     COMMON },
  { spDETECTMAGIC,                           COMMON },
  { spDIMENSIONDOOR,                         UNCOMMON },
  { spDISPELMAGIC,                           COMMON },
  { spDISPLACEMENT,                          UNCOMMON },
  { spDREAM,                                 UNCOMMON },
  { spEMOTION,                               UNCOMMON },
  { spEYEBITE,                               COMMON },
  { spFEAR,                                  UNCOMMON },
  { spGASEOUSFORM,                           UNCOMMON },
  { spGEASQUEST,                             UNCOMMON },
  { spHASTE,                                 UNCOMMON },
  { spHEALINGCIRCLE,                         UNCOMMON },
  { spHOLDMONSTER,                           COMMON },
  { spHOLDPERSON,                            COMMON },
  { spIDENTIFY,                              COMMON },
  { spIMPROVEDINVISIBILITY,                  COMMON },
  { spINVISIBILITY,                          COMMON },
  { spINVISIBILITYSPHERE,                    COMMON },
  { spLEGENDLORE,                            COMMON },
  { spLESSERGEAS,                            RARE },
  { spLEVITATE,                              UNCOMMON },
  { spLOCATECREATURE,                        UNCOMMON },
  { spLOCATEOBJECT,                          UNCOMMON },
  { spMAGEARMOR,                             UNCOMMON },
  { spMAJORIMAGE,                            UNCOMMON },
  { spMASSHASTE,                             UNCOMMON },
  { spMASSSUGGESTION,                        UNCOMMON },
  { spMIRAGEARCANA,                          UNCOMMON },
  { spMIRRORIMAGE,                           UNCOMMON },
  { spPERSISTENTIMAGE,                       UNCOMMON },
  { spSCULPTSOUND,                           UNCOMMON },
  { spSEEINVISIBILITY,                       UNCOMMON },
  { spSILENCE,                               UNCOMMON },
  { spSILENTIMAGE,                           UNCOMMON },
  { spSLEEP,                                 COMMON },
  { spSLOW,                                  UNCOMMON },
  { spSUMMONMONSTERI,                        UNCOMMON },
  { spSUMMONMONSTERII,                       UNCOMMON },
  { spSUMMONMONSTERIII,                      UNCOMMON },
  { spSUMMONMONSTERIV,                       UNCOMMON },
  { spSUMMONMONSTERV,                        UNCOMMON },
  { spSUMMONMONSTERVI,                       UNCOMMON },
  { spTONGUES,                               RARE },
  { spVENTRILOQUISM,                         UNCOMMON },
  { 0, 0 }
};


static PREFERREDFEATS clericFeats[] = {
  { ftALERTNESS,                  UNCOMMON },
  { ftAMBIDEXTERITY,              RARE },
  { ftBLINDFIGHT,                 RARE*2 },
  { ftCOMBATCASTING,              COMMON },
  { ftCOMBATREFLEXES,             UNCOMMON },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   RARE*2 },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftEXPERTISE,                  RARE*2 },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           UNCOMMON },
  { ftIMPROVEDINITIATIVE,         RARE*2 },
  { ftIMPROVEDUNARMEDSTRIKE,      RARE },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   RARE },
  { ftLEADERSHIP,                 COMMON },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             UNCOMMON },
  { ftTRAMPLE,                    UNCOMMON },
  { ftRIDEBYATTACK,               UNCOMMON },
  { ftSPIRITEDCHARGE,             RARE*2 },
  { ftPOINTBLANKSHOT,             RARE },
  { ftFARSHOT,                    RARE },
  { ftPRECISESHOT,                RARE },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE*2 },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        RARE },
  { ftSKILLFOCUS,                 COMMON },
  { ftSPELLFOCUS,                 RARE*2 },
  { ftSPELLPENETRATION,           RARE },
  { ftTOUGHNESS,                  RARE },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE*2 },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE*2 },
  { ftWEAPONFOCUS,                UNCOMMON },
  { ftBREWPOTION,                 UNCOMMON*2 },
  { ftCRAFTMAGICARMSANDARMOR,     UNCOMMON },
  { ftCRAFTROD,                   UNCOMMON },
  { ftCRAFTSTAFF,                 UNCOMMON*2 },
  { ftCRAFTWAND,                  UNCOMMON },
  { ftCRAFTWONDROUSITEM,          UNCOMMON },
  { ftFORGERING,                  UNCOMMON },
  { ftSCRIBESCROLL,               UNCOMMON*2 },
  { ftEMPOWERSPELL,               UNCOMMON },
  { ftENLARGESPELL,               UNCOMMON },
  { ftEXTENDSPELL,                UNCOMMON },
  { ftHEIGHTENSPELL,              UNCOMMON },
  { ftMAXIMIZESPELL,              UNCOMMON },
  { ftQUICKENSPELL,               UNCOMMON },
  { ftSILENTSPELL,                UNCOMMON },
  { ftSTILLSPELL,                 UNCOMMON },
  { ftEXTRATURNING,               COMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS clericWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE },
  { wpDAGGER,              RARE },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          COMMON },
  { wpSICKLE,              RARE },
  { wpCLUB,                RARE },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          COMMON },
  { wpMORNINGSTAR,         COMMON },
  { wpQUARTERSTAFF,        UNCOMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { 0,                     0 }
};

static PREFERREDFEATS druidFeats[] = {
  { ftALERTNESS,                   COMMON },
  { ftAMBIDEXTERITY,               UNCOMMON },
  { ftBLINDFIGHT,                  UNCOMMON },
  { ftCOMBATCASTING,               UNCOMMON*2 },
  { ftCOMBATREFLEXES,              UNCOMMON },
  { ftDODGE,                       UNCOMMON },
  { ftMOBILITY,                    UNCOMMON },
  { ftSPRINGATTACK,                RARE },
  { ftENDURANCE,                   RARE*2 },
  { ftEXPERTISE,                   RARE },
  { ftIMPROVEDDISARM,              RARE },
  { ftIMPROVEDTRIP,                RARE },
  { ftWHIRLWINDATTACK,             RARE },
  { ftGREATFORTITUDE,              UNCOMMON },
  { ftIMPROVEDCRITICAL,            RARE*2 },
  { ftIMPROVEDINITIATIVE,          UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,       UNCOMMON },
  { ftDEFLECTARROWS,               RARE*2 },
  { ftSTUNNINGFIST,                RARE },
  { ftIRONWILL,                    RARE },
  { ftLEADERSHIP,                  RARE },
  { ftLIGHTNINGREFLEXES,           RARE*2 },
  { ftMOUNTEDCOMBAT,               UNCOMMON },
  { ftMOUNTEDARCHERY,              RARE*2 },
  { ftTRAMPLE,                     RARE },
  { ftRIDEBYATTACK,                RARE },
  { ftSPIRITEDCHARGE,              RARE },
  { ftPOINTBLANKSHOT,              RARE },
  { ftFARSHOT,                     RARE },
  { ftPRECISESHOT,                 RARE },
  { ftRAPIDSHOT,                   RARE },
  { ftSHOTONTHERUN,                RARE },
  { ftPOWERATTACK,                 RARE },
  { ftCLEAVE,                      RARE },
  { ftIMPROVEDBULLRUSH,            RARE },
  { ftSUNDER,                      RARE },
  { ftGREATCLEAVE,                 RARE*2 },
  { ftQUICKDRAW,                   RARE },
  { ftRUN,                         UNCOMMON },
  { ftSKILLFOCUS,                  COMMON },
  { ftSPELLFOCUS,                  RARE*2 },
  { ftSPELLPENETRATION,            RARE },
  { ftTOUGHNESS,                   UNCOMMON },
  { ftTRACK,                       COMMON },
  { ftTWOWEAPONFIGHTING,           UNCOMMON },
  { ftIMPROVEDTWOWEAPONFIGHTING,   RARE },
  { ftWEAPONFINESSE,               RARE },
  { ftWEAPONFOCUS,                 RARE },
  { ftBREWPOTION,                  UNCOMMON*2 },
  { ftCRAFTMAGICARMSANDARMOR,      UNCOMMON },
  { ftCRAFTROD,                    UNCOMMON },
  { ftCRAFTSTAFF,                  UNCOMMON*2 },
  { ftCRAFTWAND,                   UNCOMMON },
  { ftCRAFTWONDROUSITEM,           UNCOMMON },
  { ftFORGERING,                   UNCOMMON },
  { ftSCRIBESCROLL,                UNCOMMON },
  { ftEMPOWERSPELL,                UNCOMMON },
  { ftENLARGESPELL,                UNCOMMON },
  { ftEXTENDSPELL,                 UNCOMMON },
  { ftHEIGHTENSPELL,               UNCOMMON },
  { ftMAXIMIZESPELL,               UNCOMMON },
  { ftQUICKENSPELL,                UNCOMMON },
  { ftSILENTSPELL,                 UNCOMMON*2 },
  { ftSTILLSPELL,                  UNCOMMON },
  { 0,                             0 }
};

static PREFERREDWEAPONS druidWeapons[] = {
  { wpCLUB,                UNCOMMON },
  { wpDAGGER,              RARE },
  { wpDART,                RARE },
  { wpLONGSPEAR,           RARE },
  { wpQUARTERSTAFF,        COMMON },
  { wpSCIMITAR,            RARE },
  { wpSICKLE,              RARE },
  { wpSHORTSPEAR,          RARE },
  { wpSLING,               COMMON },
  { 0,                     0 }
};

static PREFERREDFEATS fighterFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftBLINDFIGHT,                 COMMON },
  { ftCOMBATREFLEXES,             UNCOMMON },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               UNCOMMON },
  { ftENDURANCE,                  UNCOMMON },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE*2 },
  { ftEXPERTISE,                  UNCOMMON },
  { ftIMPROVEDDISARM,             UNCOMMON },
  { ftIMPROVEDTRIP,               UNCOMMON },
  { ftWHIRLWINDATTACK,            UNCOMMON },
  { ftGREATFORTITUDE,             RARE*2 },
  { ftIMPROVEDCRITICAL,           UNCOMMON },
  { ftIMPROVEDINITIATIVE,         COMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   RARE },
  { ftLEADERSHIP,                 COMMON },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             UNCOMMON },
  { ftTRAMPLE,                    COMMON },
  { ftRIDEBYATTACK,               COMMON },
  { ftSPIRITEDCHARGE,             UNCOMMON },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    RARE },
  { ftPRECISESHOT,                RARE },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                COMMON },
  { ftCLEAVE,                     COMMON },
  { ftIMPROVEDBULLRUSH,           UNCOMMON },
  { ftSUNDER,                     UNCOMMON },
  { ftGREATCLEAVE,                UNCOMMON },
  { ftQUICKDRAW,                  UNCOMMON },
  { ftRUN,                        RARE },
  { ftSKILLFOCUS,                 UNCOMMON },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftTRACK,                      RARE*2 },
  { ftTWOWEAPONFIGHTING,          UNCOMMON },
  { ftIMPROVEDTWOWEAPONFIGHTING,  UNCOMMON },
  { ftWEAPONFINESSE,              UNCOMMON*2 },
  { ftWEAPONFOCUS,                UNCOMMON*2 },
  { ftWEAPONSPECIALIZATION,       UNCOMMON*2 },
  { 0,                            0 }
};

static PREFERREDFEATS fighterBonusFeats[] = {
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftBLINDFIGHT,                 COMMON },
  { ftCOMBATREFLEXES,             COMMON },
  { ftDODGE,                      COMMON },
  { ftMOBILITY,                   COMMON },
  { ftSPRINGATTACK,               UNCOMMON },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftEXPERTISE,                  UNCOMMON },
  { ftIMPROVEDDISARM,             COMMON },
  { ftIMPROVEDTRIP,               UNCOMMON },
  { ftWHIRLWINDATTACK,            COMMON },
  { ftIMPROVEDCRITICAL,           COMMON },
  { ftIMPROVEDINITIATIVE,         COMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             UNCOMMON },
  { ftTRAMPLE,                    COMMON },
  { ftRIDEBYATTACK,               COMMON },
  { ftSPIRITEDCHARGE,             COMMON },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  COMMON },
  { ftSHOTONTHERUN,               COMMON },
  { ftPOWERATTACK,                COMMON },
  { ftCLEAVE,                     COMMON },
  { ftIMPROVEDBULLRUSH,           UNCOMMON },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                UNCOMMON },
  { ftQUICKDRAW,                  UNCOMMON },
  { ftTWOWEAPONFIGHTING,          UNCOMMON },
  { ftIMPROVEDTWOWEAPONFIGHTING,  UNCOMMON },
  { ftWEAPONFINESSE,              UNCOMMON*2 },
  { ftWEAPONFOCUS,                UNCOMMON*2 },
  { ftWEAPONSPECIALIZATION,       UNCOMMON*2 },
  { 0,                            0 }
};

static PREFERREDWEAPONS fighterWeapons[] = {
  { wpGAUNTLET,            RARE*2 },
  { wpUNARMED,             RARE*2 },
  { wpDAGGER,              RARE },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE*2 },
  { wpMACE_LIGHT,          RARE },
  { wpSICKLE,              RARE },
  { wpCLUB,                RARE },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          RARE*2 },
  { wpMORNINGSTAR,         RARE*2 },
  { wpQUARTERSTAFF,        RARE },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE*2 },
  { wpAXE_THROWING,        RARE },
  { wpHAMMER_LIGHT,        RARE },
  { wpHANDAXE,             RARE },
  { wpLANCE_LIGHT,         UNCOMMON },
  { wpPICK_LIGHT,          RARE },
  { wpSAP,                 RARE },
  { wpSWORD_SHORT,         UNCOMMON },
  { wpBATTLEAXE,           UNCOMMON },
  { wpFLAIL_LIGHT,         UNCOMMON },
  { wpLANCE_HEAVY,         UNCOMMON },
  { wpLONGSWORD,           COMMON*2 },
  { wpPICK_HEAVY,          RARE },
  { wpRAPIER,              UNCOMMON },
  { wpSCIMITAR,            UNCOMMON },
  { wpTRIDENT,             RARE },
  { wpWARHAMMER,           UNCOMMON },
  { wpFALCHION,            RARE },
  { wpFLAIL_HEAVY,         UNCOMMON },
  { wpGLAIVE,              RARE },
  { wpGREATAXE,            UNCOMMON },
  { wpGREATCLUB,           UNCOMMON },
  { wpGREATSWORD,          COMMON },
  { wpGUISARME,            RARE },
  { wpHALBERD,             UNCOMMON },
  { wpLONGSPEAR,           UNCOMMON },
  { wpRANSEUR,             RARE },
  { wpSCYTHE,              RARE },
  { wpSHORTBOW,            COMMON },
  { wpSHORTBOW_COMPOSITE,  UNCOMMON },
  { wpLONGBOW,             UNCOMMON },
  { wpLONGBOW_COMPOSITE,   UNCOMMON },
  { 0,                     0 }
};

static PREFERREDFEATS monkFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftBLINDFIGHT,                 COMMON },
  { ftCOMBATREFLEXES,             UNCOMMON },
  { ftDODGE,                      COMMON },
  { ftMOBILITY,                   COMMON },
  { ftSPRINGATTACK,               UNCOMMON },
  { ftENDURANCE,                  RARE*2 },
  { ftEXPERTISE,                  COMMON },
  { ftIMPROVEDDISARM,             UNCOMMON },
  { ftWHIRLWINDATTACK,            UNCOMMON },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             RARE },
  { ftFARSHOT,                    RARE },
  { ftPRECISESHOT,                RARE },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        UNCOMMON },
  { ftSKILLFOCUS,                 UNCOMMON },
  { ftTOUGHNESS,                  RARE },
  { ftTRACK,                      RARE*2 },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              UNCOMMON },
  { ftWEAPONFOCUS,                UNCOMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS monkWeapons[] = {
  { wpCLUB,                UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpDAGGER,              COMMON },
  { wpHANDAXE,             RARE },
  { wpJAVELIN,             RARE },
  { wpKAMA,                COMMON },
  { wpNUNCHAKU,            COMMON },
  { wpQUARTERSTAFF,        COMMON },
  { wpSHURIKEN,            UNCOMMON },
  { wpSIANGHAM,            UNCOMMON },
  { wpSLING,               UNCOMMON },
  { 0,                     0 }
};

static PREFERREDFEATS paladinFeats[] = {
  { ftALERTNESS,                  UNCOMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftBLINDFIGHT,                 UNCOMMON },
  { ftCOMBATCASTING,              UNCOMMON },
  { ftCOMBATREFLEXES,             COMMON },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               UNCOMMON },
  { ftENDURANCE,                  UNCOMMON },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftEXPERTISE,                  UNCOMMON },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           UNCOMMON },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      RARE },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 COMMON*2 },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             COMMON },
  { ftTRAMPLE,                    COMMON },
  { ftRIDEBYATTACK,               COMMON },
  { ftSPIRITEDCHARGE,             COMMON },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                COMMON },
  { ftCLEAVE,                     UNCOMMON },
  { ftIMPROVEDBULLRUSH,           UNCOMMON },
  { ftSUNDER,                     UNCOMMON },
  { ftGREATCLEAVE,                UNCOMMON },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        RARE },
  { ftSKILLFOCUS,                 UNCOMMON },
  { ftSPELLFOCUS,                 RARE },
  { ftSPELLPENETRATION,           RARE },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              UNCOMMON*2 },
  { ftWEAPONFOCUS,                UNCOMMON*2 },
  { ftBREWPOTION,                 RARE },
  { ftCRAFTMAGICARMSANDARMOR,     RARE },
  { ftCRAFTROD,                   RARE },
  { ftCRAFTSTAFF,                 RARE },
  { ftCRAFTWAND,                  RARE },
  { ftCRAFTWONDROUSITEM,          RARE },
  { ftFORGERING,                  RARE },
  { ftSCRIBESCROLL,               RARE },
  { ftEMPOWERSPELL,               RARE },
  { ftENLARGESPELL,               RARE },
  { ftEXTENDSPELL,                RARE },
  { ftHEIGHTENSPELL,              RARE },
  { ftMAXIMIZESPELL,              RARE },
  { ftQUICKENSPELL,               RARE },
  { ftSILENTSPELL,                RARE },
  { ftSTILLSPELL,                 RARE },
  { ftEXTRATURNING,               UNCOMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS paladinWeapons[] = {
  { wpGAUNTLET,            RARE*2 },
  { wpUNARMED,             RARE*2 },
  { wpDAGGER,              RARE },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE*2 },
  { wpMACE_LIGHT,          RARE },
  { wpSICKLE,              RARE },
  { wpCLUB,                RARE },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          RARE*2 },
  { wpMORNINGSTAR,         RARE*2 },
  { wpQUARTERSTAFF,        RARE },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE*2 },
  { wpAXE_THROWING,        RARE },
  { wpHAMMER_LIGHT,        RARE },
  { wpHANDAXE,             RARE },
  { wpLANCE_LIGHT,         UNCOMMON },
  { wpPICK_LIGHT,          RARE },
  { wpSAP,                 RARE },
  { wpSWORD_SHORT,         UNCOMMON },
  { wpBATTLEAXE,           UNCOMMON },
  { wpFLAIL_LIGHT,         UNCOMMON },
  { wpLANCE_HEAVY,         UNCOMMON },
  { wpLONGSWORD,           COMMON*2 },
  { wpPICK_HEAVY,          RARE },
  { wpRAPIER,              UNCOMMON },
  { wpSCIMITAR,            UNCOMMON },
  { wpTRIDENT,             RARE },
  { wpWARHAMMER,           UNCOMMON },
  { wpFALCHION,            RARE },
  { wpFLAIL_HEAVY,         UNCOMMON },
  { wpGLAIVE,              RARE },
  { wpGREATAXE,            UNCOMMON },
  { wpGREATCLUB,           UNCOMMON },
  { wpGREATSWORD,          COMMON },
  { wpGUISARME,            RARE },
  { wpHALBERD,             UNCOMMON },
  { wpLONGSPEAR,           UNCOMMON },
  { wpRANSEUR,             RARE },
  { wpSCYTHE,              RARE },
  { wpSHORTBOW,            COMMON },
  { wpSHORTBOW_COMPOSITE,  UNCOMMON },
  { wpLONGBOW,             UNCOMMON },
  { wpLONGBOW_COMPOSITE,   UNCOMMON },
  { 0,                     0 }
};

static PREFERREDFEATS rangerFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftARMORPROFICIENCY_HEAVY,     RARE },
  { ftBLINDFIGHT,                 UNCOMMON },
  { ftCOMBATCASTING,              RARE },
  { ftCOMBATREFLEXES,             COMMON },
  { ftDODGE,                      COMMON },
  { ftMOBILITY,                   COMMON },
  { ftSPRINGATTACK,               COMMON },
  { ftENDURANCE,                  UNCOMMON },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftEXPERTISE,                  COMMON },
  { ftIMPROVEDDISARM,             UNCOMMON },
  { ftIMPROVEDTRIP,               UNCOMMON },
  { ftWHIRLWINDATTACK,            UNCOMMON },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           UNCOMMON },
  { ftIMPROVEDINITIATIVE,         COMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             COMMON },
  { ftTRAMPLE,                    UNCOMMON },
  { ftRIDEBYATTACK,               UNCOMMON },
  { ftSPIRITEDCHARGE,             UNCOMMON },
  { ftPOINTBLANKSHOT,             COMMON },
  { ftFARSHOT,                    COMMON },
  { ftPRECISESHOT,                COMMON },
  { ftRAPIDSHOT,                  UNCOMMON },
  { ftSHOTONTHERUN,               UNCOMMON },
  { ftPOWERATTACK,                UNCOMMON },
  { ftCLEAVE,                     UNCOMMON },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  UNCOMMON },
  { ftRUN,                        UNCOMMON },
  { ftSKILLFOCUS,                 COMMON },
  { ftSPELLFOCUS,                 RARE },
  { ftSPELLPENETRATION,           RARE },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftIMPROVEDTWOWEAPONFIGHTING,  COMMON },
  { ftWEAPONFINESSE,              UNCOMMON*2 },
  { ftWEAPONFOCUS,                UNCOMMON*2 },
  { ftBREWPOTION,                 RARE*2 },
  { ftCRAFTMAGICARMSANDARMOR,     RARE },
  { ftCRAFTROD,                   RARE },
  { ftCRAFTSTAFF,                 RARE },
  { ftCRAFTWAND,                  RARE },
  { ftCRAFTWONDROUSITEM,          RARE },
  { ftFORGERING,                  RARE },
  { ftSCRIBESCROLL,               RARE },
  { ftEMPOWERSPELL,               RARE },
  { ftENLARGESPELL,               RARE },
  { ftEXTENDSPELL,                RARE },
  { ftHEIGHTENSPELL,              RARE },
  { ftMAXIMIZESPELL,              RARE },
  { ftQUICKENSPELL,               RARE },
  { ftSILENTSPELL,                RARE },
  { ftSTILLSPELL,                 RARE },
  { 0,                            0 }
};

static PREFERREDWEAPONS rangerWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE*2 },
  { wpDAGGER,              RARE*2 },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          RARE },
  { wpSICKLE,              RARE },
  { wpCLUB,                RARE*2 },
  { wpHALFSPEAR,           RARE*2 },
  { wpMACE_HEAVY,          RARE },
  { wpMORNINGSTAR,         RARE*2 },
  { wpQUARTERSTAFF,        COMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { wpAXE_THROWING,        RARE },
  { wpHAMMER_LIGHT,        RARE },
  { wpHANDAXE,             RARE },
  { wpLANCE_LIGHT,         RARE },
  { wpPICK_LIGHT,          RARE },
  { wpSAP,                 RARE },
  { wpSWORD_SHORT,         UNCOMMON },
  { wpBATTLEAXE,           RARE },
  { wpFLAIL_LIGHT,         RARE },
  { wpLANCE_HEAVY,         RARE },
  { wpLONGSWORD,           COMMON*2 },
  { wpPICK_HEAVY,          RARE },
  { wpRAPIER,              RARE },
  { wpSCIMITAR,            RARE },
  { wpTRIDENT,             RARE },
  { wpWARHAMMER,           RARE },
  { wpFALCHION,            RARE },
  { wpFLAIL_HEAVY,         RARE },
  { wpGLAIVE,              RARE },
  { wpGREATAXE,            RARE },
  { wpGREATCLUB,           RARE },
  { wpGREATSWORD,          RARE },
  { wpGUISARME,            RARE },
  { wpHALBERD,             RARE },
  { wpLONGSPEAR,           UNCOMMON },
  { wpRANSEUR,             RARE },
  { wpSCYTHE,              RARE },
  { wpSHORTBOW,            COMMON },
  { wpSHORTBOW_COMPOSITE,  COMMON },
  { wpLONGBOW,             COMMON },
  { wpLONGBOW_COMPOSITE,   COMMON },
  { 0,                     0 }
};

static PREFERREDFEATS rogueFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftARMORPROFICIENCY_MEDIUM,    RARE },
  { ftARMORPROFICIENCY_HEAVY,     RARE },
  { ftBLINDFIGHT,                 UNCOMMON },
  { ftCOMBATREFLEXES,             UNCOMMON },
  { ftDODGE,                      COMMON },
  { ftMOBILITY,                   COMMON },
  { ftSPRINGATTACK,               COMMON },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftWEAPONPROFICIENCY_MARTIAL,  RARE*2 },
  { ftEXPERTISE,                  UNCOMMON },
  { ftIMPROVEDDISARM,             COMMON },
  { ftIMPROVEDTRIP,               UNCOMMON },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           COMMON },
  { ftIMPROVEDINITIATIVE,         COMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              UNCOMMON },
  { ftSTUNNINGFIST,               UNCOMMON },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              UNCOMMON },
  { ftMOUNTEDARCHERY,             UNCOMMON },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             COMMON },
  { ftFARSHOT,                    COMMON },
  { ftPRECISESHOT,                COMMON },
  { ftRAPIDSHOT,                  COMMON },
  { ftSHOTONTHERUN,               COMMON },
  { ftPOWERATTACK,                UNCOMMON },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     UNCOMMON },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  UNCOMMON },
  { ftRUN,                        UNCOMMON },
  { ftSHIELDPROFICIENCY,          UNCOMMON },
  { ftSKILLFOCUS,                 COMMON },
  { ftTOUGHNESS,                  RARE },
  { ftTRACK,                      UNCOMMON },
  { ftTWOWEAPONFIGHTING,          UNCOMMON },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              UNCOMMON },
  { ftWEAPONFOCUS,                UNCOMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS rogueWeapons[] = {
  { wpCROSSBOW_HAND,       RARE },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDAGGER,              UNCOMMON },
  { wpDAGGER_PUNCHING,     RARE },
  { wpDART,                RARE*2 },
  { wpMACE_LIGHT,          RARE },
  { wpSAP,                 UNCOMMON },
  { wpSHORTBOW,            UNCOMMON },
  { wpSHORTBOW_COMPOSITE,  UNCOMMON },
  { wpSWORD_SHORT,         COMMON },
  { wpCLUB,                RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpMACE_HEAVY,          RARE },
  { wpMORNINGSTAR,         RARE },
  { wpQUARTERSTAFF,        RARE },
  { wpRAPIER,              RARE },
  { 0,                     0 }
};

static PREFERREDFEATS sorcererFeats[] = {
  { ftALERTNESS,                  UNCOMMON },
  { ftAMBIDEXTERITY,              RARE },
  { ftARMORPROFICIENCY_LIGHT,     RARE },
  { ftBLINDFIGHT,                 RARE },
  { ftCOMBATCASTING,              COMMON },
  { ftCOMBATREFLEXES,             RARE },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftWEAPONPROFICIENCY_MARTIAL,  RARE },
  { ftWEAPONPROFICIENCY_SIMPLE,   RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE*2 },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 UNCOMMON },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  UNCOMMON },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        UNCOMMON },
  { ftSHIELDPROFICIENCY,          RARE },
  { ftSKILLFOCUS,                 COMMON },
  { ftSPELLFOCUS,                 COMMON },
  { ftSPELLPENETRATION,           COMMON },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                RARE },
  { ftBREWPOTION,                 COMMON },
  { ftCRAFTMAGICARMSANDARMOR,     COMMON },
  { ftCRAFTROD,                   COMMON },
  { ftCRAFTSTAFF,                 COMMON },
  { ftCRAFTWAND,                  COMMON },
  { ftCRAFTWONDROUSITEM,          COMMON },
  { ftFORGERING,                  COMMON },
  { ftSCRIBESCROLL,               COMMON*2 },
  { ftEMPOWERSPELL,               COMMON },
  { ftENLARGESPELL,               COMMON },
  { ftEXTENDSPELL,                COMMON },
  { ftHEIGHTENSPELL,              COMMON },
  { ftMAXIMIZESPELL,              COMMON },
  { ftSILENTSPELL,                COMMON },
  { ftSTILLSPELL,                 COMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS sorcererWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE },
  { wpDAGGER,              COMMON },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          UNCOMMON },
  { wpSICKLE,              RARE },
  { wpCLUB,                UNCOMMON },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          COMMON },
  { wpMORNINGSTAR,         UNCOMMON },
  { wpQUARTERSTAFF,        COMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      UNCOMMON },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { 0,                     0 }
};

static PREFERREDSPELLS prefSorcererSpells[] = {
  { spACIDFOG,                               UNCOMMON },
  { spALTERSELF,                             UNCOMMON },
  { spANALYZEDWEOMER,                        COMMON },
  { spANIMATEDEAD,                           UNCOMMON },
  { spANTIMAGICFIELD,                        UNCOMMON },
  { spARCANEEYE,                             UNCOMMON },
  { spBESTOWCURSE,                           UNCOMMON },
  { spBIGBYSCRUSHINGHAND,                    UNCOMMON },
  { spBLINDNESSDEAFNESS,                     UNCOMMON },
  { spBLINK,                                 UNCOMMON },
  { spBLUR,                                  COMMON },
  { spBURNINGHANDS,                          UNCOMMON },
  { spCHAINLIGHTNING,                        COMMON },
  { spCHANGESELF,                            UNCOMMON },
  { spCHARMMONSTER,                          COMMON },
  { spCHARMPERSON,                           COMMON },
  { spCHILLTOUCH,                            UNCOMMON },
  { spCLOUDKILL,                             COMMON },
  { spCONEOFCOLD,                            COMMON },
  { spCONTINGENCY,                           UNCOMMON },
  { spDANCINGLIGHTS,                         COMMON },
  { spDELAYEDBLASTFIREBALL,                  COMMON },
  { spDETECTMAGIC,                           COMMON },
  { spDIMENSIONDOOR,                         UNCOMMON },
  { spDISINTEGRATE,                          COMMON },
  { spDISPELMAGIC,                           COMMON },
  { spDOMINATEMONSTER,                       UNCOMMON },
  { spDOMINATEPERSON,                        UNCOMMON },
  { spENERGYDRAIN,                           UNCOMMON },
  { spEYEBITE,                               COMMON },
  { spFEATHERFALL,                           UNCOMMON },
  { spFINGEROFDEATH,                         UNCOMMON },
  { spFIRESHIELD,                            UNCOMMON },
  { spFIRETRAP,                              UNCOMMON },
  { spFIREBALL,                              COMMON },
  { spFLAMEARROW,                            UNCOMMON },
  { spFLESHTOSTONE,                          COMMON },
  { spFLY,                                   COMMON },
  { spGASEOUSFORM,                           RARE },
  { spGEASQUEST,                             RARE },
  { spGHOSTSOUND,                            COMMON },
  { spGHOULTOUCH,                            UNCOMMON },
  { spHASTE,                                 COMMON },
  { spHOLDMONSTER,                           COMMON },
  { spHOLDPERSON,                            COMMON },
  { spHORRIDWILTING,                         COMMON },
  { spIDENTIFY,                              COMMON },
  { spIMPROVEDINVISIBILITY,                  COMMON },
  { spINCENDIARYCLOUD,                       COMMON },
  { spINVISIBILITY,                          COMMON },
  { spINVISIBILITYSPHERE,                    UNCOMMON },
  { spKNOCK,                                 COMMON },
  { spLESSERGEAS,                            UNCOMMON },
  { spLEVITATE,                              UNCOMMON },
  { spLIGHT,                                 COMMON },
  { spLIGHTNINGBOLT,                         COMMON },
  { spLIMITEDWISH,                           UNCOMMON },
  { spMAGEARMOR,                             COMMON },
  { spMAGEHAND,                              COMMON },
  { spMAGICMISSILE,                          COMMON*2 },
  { spMASSCHARM,                             COMMON },
  { spMASSHASTE,                             UNCOMMON },
  { spMASSINVISIBILITY,                      COMMON },
  { spMASSSUGGESTION,                        UNCOMMON },
  { spMELFSACIDARROW,                        COMMON },
  { spMETEORSWARM,                           COMMON },
  { spMINORGLOBEOFINVULNERABILITY,           UNCOMMON },
  { spMIRRORIMAGE,                           COMMON },
  { spMORDENKAINENSDISJUNCTION,              UNCOMMON },
  { spPERMANENCY,                            UNCOMMON },
  { spPOLYMORPHANYOBJECT,                    COMMON },
  { spPOLYMORPHOTHER,                        COMMON },
  { spPOLYMORPHSELF,                         COMMON },
  { spPOWERWORDBLIND,                        UNCOMMON },
  { spPOWERWORDKILL,                         UNCOMMON },
  { spPOWERWORDSTUN,                         UNCOMMON },
  { spPRESTIDIGITATION,                      UNCOMMON },
  { spPROTECTIONFROMSPELLS,                  UNCOMMON },
  { spRAYOFFROST,                            UNCOMMON },
  { spREADMAGIC,                             COMMON },
  { spSEEINVISIBILITY,                       UNCOMMON },
  { spSHAPECHANGE,                           COMMON },
  { spSHIELD,                                COMMON },
  { spSHOCKINGGRASP,                         UNCOMMON },
  { spSILENTIMAGE,                           UNCOMMON },
  { spSLEEP,                                 COMMON },
  { spSLOW,                                  COMMON },
  { spSPIDERCLIMB,                           UNCOMMON },
  { spSUMMONMONSTERI,                        UNCOMMON },
  { spSUMMONMONSTERII,                       UNCOMMON },
  { spSUMMONMONSTERIII,                      UNCOMMON },
  { spSUMMONMONSTERIV,                       UNCOMMON },
  { spSUMMONMONSTERIX,                       UNCOMMON },
  { spSUMMONMONSTERV,                        UNCOMMON },
  { spSUMMONMONSTERVI,                       UNCOMMON },
  { spSUMMONMONSTERVII,                      UNCOMMON },
  { spSUMMONMONSTERVIII,                     UNCOMMON },
  { spSYMBOL,                                UNCOMMON },
  { spTELEPORT,                              COMMON },
  { spTELEPORTWITHOUTERROR,                  COMMON },
  { spTELEPORTATIONCIRCLE,                   UNCOMMON },
  { spTIMESTOP,                              UNCOMMON },
  { spTRUESEEING,                            UNCOMMON },
  { spWALLOFFIRE,                            UNCOMMON },
  { spWALLOFFORCE,                           UNCOMMON },
  { spWALLOFICE,                             UNCOMMON },
  { spWALLOFIRON,                            UNCOMMON },
  { spWALLOFSTONE,                           UNCOMMON },
  { spWEB,                                   COMMON },
  { spWISH,                                  RARE*2 },
  { 0, 0 }
};

static PREFERREDFEATS wizardFeats[] = {
  { ftALERTNESS,                  UNCOMMON },
  { ftAMBIDEXTERITY,              RARE },
  { ftARMORPROFICIENCY_LIGHT,     RARE },
  { ftBLINDFIGHT,                 RARE },
  { ftCOMBATCASTING,              COMMON },
  { ftCOMBATREFLEXES,             RARE },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftWEAPONPROFICIENCY_MARTIAL,  RARE },
  { ftWEAPONPROFICIENCY_SIMPLE,   RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE*2 },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 UNCOMMON },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  UNCOMMON },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        UNCOMMON },
  { ftSHIELDPROFICIENCY,          RARE },
  { ftSKILLFOCUS,                 UNCOMMON },
  { ftSPELLFOCUS,                 COMMON },
  { ftSPELLPENETRATION,           COMMON },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                RARE },
  { ftBREWPOTION,                 COMMON },
  { ftCRAFTMAGICARMSANDARMOR,     COMMON },
  { ftCRAFTROD,                   COMMON },
  { ftCRAFTSTAFF,                 COMMON },
  { ftCRAFTWAND,                  COMMON },
  { ftCRAFTWONDROUSITEM,          COMMON },
  { ftFORGERING,                  COMMON },
  { ftSCRIBESCROLL,               COMMON*2 },
  { ftEMPOWERSPELL,               COMMON },
  { ftENLARGESPELL,               COMMON },
  { ftEXTENDSPELL,                COMMON },
  { ftHEIGHTENSPELL,              COMMON },
  { ftMAXIMIZESPELL,              COMMON },
  { ftQUICKENSPELL,               COMMON },
  { ftSILENTSPELL,                COMMON },
  { ftSTILLSPELL,                 COMMON },
  { ftSPELLMASTERY,               UNCOMMON },
  { 0,                            0 }
};

static PREFERREDFEATS wizardBonusFeats[] = {
  { ftBREWPOTION,                 COMMON },
  { ftCRAFTMAGICARMSANDARMOR,     UNCOMMON },
  { ftCRAFTROD,                   RARE },
  { ftCRAFTSTAFF,                 UNCOMMON },
  { ftCRAFTWAND,                  COMMON },
  { ftCRAFTWONDROUSITEM,          UNCOMMON },
  { ftFORGERING,                  COMMON },
  { ftSCRIBESCROLL,               COMMON*2 },
  { ftEMPOWERSPELL,               COMMON },
  { ftENLARGESPELL,               COMMON },
  { ftEXTENDSPELL,                COMMON },
  { ftHEIGHTENSPELL,              COMMON },
  { ftMAXIMIZESPELL,              COMMON },
  { ftQUICKENSPELL,               COMMON },
  { ftSILENTSPELL,                COMMON },
  { ftSTILLSPELL,                 COMMON },
  { ftSPELLMASTERY,               UNCOMMON },
  { 0,                            0 }
};

static PREFERREDWEAPONS wizardWeapons[] = {
  { wpCLUB,                RARE },
  { wpDAGGER,              COMMON },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpCROSSBOW_LIGHT,      UNCOMMON },
  { wpQUARTERSTAFF,        COMMON },
  { 0,                     0 }
};

static PREFERREDSPELLS prefWizardSpells[] = {
  { spACIDFOG,                               UNCOMMON },
  { spALTERSELF,                             UNCOMMON },
  { spANALYZEDWEOMER,                        COMMON },
  { spANIMATEDEAD,                           UNCOMMON },
  { spANTIMAGICFIELD,                        UNCOMMON },
  { spARCANEEYE,                             UNCOMMON },
  { spBESTOWCURSE,                           UNCOMMON },
  { spBIGBYSCRUSHINGHAND,                    UNCOMMON },
  { spBLINDNESSDEAFNESS,                     UNCOMMON },
  { spBLINK,                                 UNCOMMON },
  { spBLUR,                                  COMMON },
  { spBURNINGHANDS,                          UNCOMMON },
  { spCHAINLIGHTNING,                        COMMON },
  { spCHANGESELF,                            UNCOMMON },
  { spCHARMMONSTER,                          COMMON },
  { spCHARMPERSON,                           COMMON },
  { spCHILLTOUCH,                            UNCOMMON },
  { spCLOUDKILL,                             COMMON },
  { spCONEOFCOLD,                            COMMON },
  { spCONTINGENCY,                           UNCOMMON },
  { spDANCINGLIGHTS,                         COMMON },
  { spDELAYEDBLASTFIREBALL,                  COMMON },
  { spDETECTMAGIC,                           COMMON },
  { spDIMENSIONDOOR,                         UNCOMMON },
  { spDISINTEGRATE,                          COMMON },
  { spDISPELMAGIC,                           COMMON },
  { spDOMINATEMONSTER,                       UNCOMMON },
  { spDOMINATEPERSON,                        UNCOMMON },
  { spENERGYDRAIN,                           UNCOMMON },
  { spEYEBITE,                               COMMON },
  { spFEATHERFALL,                           UNCOMMON },
  { spFINGEROFDEATH,                         UNCOMMON },
  { spFIRESHIELD,                            UNCOMMON },
  { spFIRETRAP,                              UNCOMMON },
  { spFIREBALL,                              COMMON },
  { spFLAMEARROW,                            UNCOMMON },
  { spFLESHTOSTONE,                          COMMON },
  { spFLY,                                   COMMON },
  { spGASEOUSFORM,                           RARE },
  { spGEASQUEST,                             RARE },
  { spGHOSTSOUND,                            COMMON },
  { spGHOULTOUCH,                            UNCOMMON },
  { spHASTE,                                 COMMON },
  { spHOLDMONSTER,                           COMMON },
  { spHOLDPERSON,                            COMMON },
  { spHORRIDWILTING,                         COMMON },
  { spIDENTIFY,                              COMMON },
  { spIMPROVEDINVISIBILITY,                  COMMON },
  { spINCENDIARYCLOUD,                       COMMON },
  { spINVISIBILITY,                          COMMON },
  { spINVISIBILITYSPHERE,                    UNCOMMON },
  { spKNOCK,                                 COMMON },
  { spLESSERGEAS,                            UNCOMMON },
  { spLEVITATE,                              UNCOMMON },
  { spLIGHT,                                 COMMON },
  { spLIGHTNINGBOLT,                         COMMON },
  { spLIMITEDWISH,                           UNCOMMON },
  { spMAGEARMOR,                             COMMON },
  { spMAGEHAND,                              COMMON },
  { spMAGICMISSILE,                          COMMON*2 },
  { spMASSCHARM,                             COMMON },
  { spMASSHASTE,                             UNCOMMON },
  { spMASSINVISIBILITY,                      COMMON },
  { spMASSSUGGESTION,                        UNCOMMON },
  { spMELFSACIDARROW,                        COMMON },
  { spMETEORSWARM,                           COMMON },
  { spMINORGLOBEOFINVULNERABILITY,           UNCOMMON },
  { spMIRRORIMAGE,                           COMMON },
  { spMORDENKAINENSDISJUNCTION,              UNCOMMON },
  { spPERMANENCY,                            UNCOMMON },
  { spPOLYMORPHANYOBJECT,                    COMMON },
  { spPOLYMORPHOTHER,                        COMMON },
  { spPOLYMORPHSELF,                         COMMON },
  { spPOWERWORDBLIND,                        UNCOMMON },
  { spPOWERWORDKILL,                         UNCOMMON },
  { spPOWERWORDSTUN,                         UNCOMMON },
  { spPRESTIDIGITATION,                      UNCOMMON },
  { spPROTECTIONFROMSPELLS,                  UNCOMMON },
  { spRAYOFFROST,                            UNCOMMON },
  { spREADMAGIC,                             COMMON },
  { spSEEINVISIBILITY,                       UNCOMMON },
  { spSHAPECHANGE,                           COMMON },
  { spSHIELD,                                COMMON },
  { spSHOCKINGGRASP,                         UNCOMMON },
  { spSILENTIMAGE,                           UNCOMMON },
  { spSLEEP,                                 COMMON },
  { spSLOW,                                  COMMON },
  { spSPIDERCLIMB,                           UNCOMMON },
  { spSUMMONMONSTERI,                        UNCOMMON },
  { spSUMMONMONSTERII,                       UNCOMMON },
  { spSUMMONMONSTERIII,                      UNCOMMON },
  { spSUMMONMONSTERIV,                       UNCOMMON },
  { spSUMMONMONSTERIX,                       UNCOMMON },
  { spSUMMONMONSTERV,                        UNCOMMON },
  { spSUMMONMONSTERVI,                       UNCOMMON },
  { spSUMMONMONSTERVII,                      UNCOMMON },
  { spSUMMONMONSTERVIII,                     UNCOMMON },
  { spSYMBOL,                                UNCOMMON },
  { spTELEPORT,                              COMMON },
  { spTELEPORTWITHOUTERROR,                  COMMON },
  { spTELEPORTATIONCIRCLE,                   UNCOMMON },
  { spTIMESTOP,                              UNCOMMON },
  { spTRUESEEING,                            UNCOMMON },
  { spWALLOFFIRE,                            UNCOMMON },
  { spWALLOFFORCE,                           UNCOMMON },
  { spWALLOFICE,                             UNCOMMON },
  { spWALLOFIRON,                            UNCOMMON },
  { spWALLOFSTONE,                           UNCOMMON },
  { spWEB,                                   COMMON },
  { spWISH,                                  RARE*2 },
  { 0, 0 }
};

static PREFERREDFEATS adeptFeats[] = {
  { ftALERTNESS,                  UNCOMMON },
  { ftAMBIDEXTERITY,              RARE },
  { ftARMORPROFICIENCY_LIGHT,     RARE },
  { ftBLINDFIGHT,                 RARE },
  { ftCOMBATCASTING,              COMMON },
  { ftCOMBATREFLEXES,             RARE },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftWEAPONPROFICIENCY_MARTIAL,  RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE*2 },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 UNCOMMON },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  UNCOMMON },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        UNCOMMON },
  { ftSHIELDPROFICIENCY,          RARE },
  { ftSKILLFOCUS,                 UNCOMMON },
  { ftSPELLFOCUS,                 COMMON },
  { ftSPELLPENETRATION,           COMMON },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                RARE },
  { ftBREWPOTION,                 COMMON },
  { ftCRAFTMAGICARMSANDARMOR,     COMMON },
  { ftCRAFTROD,                   COMMON },
  { ftCRAFTSTAFF,                 COMMON },
  { ftCRAFTWAND,                  COMMON },
  { ftCRAFTWONDROUSITEM,          COMMON },
  { ftFORGERING,                  COMMON },
  { ftSCRIBESCROLL,               COMMON*2 },
  { ftEMPOWERSPELL,               COMMON },
  { ftENLARGESPELL,               COMMON },
  { ftEXTENDSPELL,                COMMON },
  { ftHEIGHTENSPELL,              COMMON },
  { ftMAXIMIZESPELL,              COMMON },
  { ftQUICKENSPELL,               COMMON },
  { ftSILENTSPELL,                COMMON },
  { ftSTILLSPELL,                 COMMON },
  { 0,                            0 }
};

static PREFERREDFEATS commonFeats[] = {
  { ftALERTNESS,                  UNCOMMON },
  { ftAMBIDEXTERITY,              RARE },
  { ftARMORPROFICIENCY_LIGHT,     RARE },
  { ftARMORPROFICIENCY_MEDIUM,    RARE },
  { ftARMORPROFICIENCY_HEAVY,     RARE },
  { ftBLINDFIGHT,                 RARE },
  { ftCOMBATREFLEXES,             RARE },
  { ftDODGE,                      RARE },
  { ftMOBILITY,                   RARE },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftWEAPONPROFICIENCY_MARTIAL,  RARE },
  { ftWEAPONPROFICIENCY_SIMPLE,   RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             UNCOMMON },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         RARE },
  { ftIMPROVEDUNARMEDSTRIKE,      RARE },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        UNCOMMON },
  { ftSHIELDPROFICIENCY,          RARE },
  { ftSKILLFOCUS,                 COMMON*3 },
  { ftTOUGHNESS,                  RARE },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                RARE },
  { 0,                            0 }
};

static PREFERREDWEAPONS adeptWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE },
  { wpDAGGER,              COMMON },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          UNCOMMON },
  { wpSICKLE,              RARE },
  { wpCLUB,                UNCOMMON },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          RARE },
  { wpMORNINGSTAR,         RARE },
  { wpQUARTERSTAFF,        COMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      UNCOMMON },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { 0,                     0 }
};

static PREFERREDFEATS nobleFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              RARE },
  { ftBLINDFIGHT,                 RARE },
  { ftCOMBATREFLEXES,             RARE },
  { ftDODGE,                      RARE },
  { ftMOBILITY,                   RARE },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      RARE },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   UNCOMMON },
  { ftLEADERSHIP,                 COMMON*2 },
  { ftLIGHTNINGREFLEXES,          RARE },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             COMMON },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             COMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        RARE },
  { ftSKILLFOCUS,                 COMMON*3 },
  { ftTOUGHNESS,                  RARE },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                RARE },
  { 0,                            0 }
};

static PREFERREDWEAPONS nobleWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE*2 },
  { wpDAGGER,              RARE*2 },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          RARE },
  { wpSICKLE,              RARE },
  { wpCLUB,                RARE*2 },
  { wpHALFSPEAR,           RARE*2 },
  { wpMACE_HEAVY,          RARE },
  { wpMORNINGSTAR,         RARE*2 },
  { wpQUARTERSTAFF,        UNCOMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      RARE },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { wpAXE_THROWING,        RARE },
  { wpHAMMER_LIGHT,        RARE },
  { wpHANDAXE,             RARE },
  { wpLANCE_LIGHT,         RARE },
  { wpPICK_LIGHT,          RARE },
  { wpSAP,                 RARE },
  { wpSWORD_SHORT,         COMMON },
  { wpBATTLEAXE,           RARE },
  { wpFLAIL_LIGHT,         RARE },
  { wpLANCE_HEAVY,         RARE },
  { wpLONGSWORD,           COMMON*2 },
  { wpPICK_HEAVY,          RARE },
  { wpRAPIER,              UNCOMMON },
  { wpSCIMITAR,            RARE },
  { wpTRIDENT,             RARE },
  { wpWARHAMMER,           RARE },
  { wpFALCHION,            RARE },
  { wpFLAIL_HEAVY,         RARE },
  { wpGLAIVE,              RARE },
  { wpGREATAXE,            RARE },
  { wpGREATCLUB,           RARE },
  { wpGREATSWORD,          RARE },
  { wpGUISARME,            RARE },
  { wpHALBERD,             RARE },
  { wpLONGSPEAR,           UNCOMMON },
  { wpRANSEUR,             RARE },
  { wpSCYTHE,              RARE },
  { wpSHORTBOW,            COMMON },
  { wpSHORTBOW_COMPOSITE,  COMMON },
  { wpLONGBOW,             COMMON },
  { wpLONGBOW_COMPOSITE,   COMMON },
  { 0,                     0 }
};

static PREFERREDFEATS expertFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftARMORPROFICIENCY_MEDIUM,    RARE },
  { ftARMORPROFICIENCY_HEAVY,     RARE },
  { ftBLINDFIGHT,                 RARE },
  { ftCOMBATREFLEXES,             RARE },
  { ftDODGE,                      RARE },
  { ftMOBILITY,                   RARE },
  { ftSPRINGATTACK,               RARE },
  { ftENDURANCE,                  RARE },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE },
  { ftWEAPONPROFICIENCY_MARTIAL,  RARE },
  { ftEXPERTISE,                  RARE },
  { ftIMPROVEDDISARM,             RARE },
  { ftIMPROVEDTRIP,               RARE },
  { ftWHIRLWINDATTACK,            RARE },
  { ftGREATFORTITUDE,             RARE },
  { ftIMPROVEDCRITICAL,           RARE },
  { ftIMPROVEDINITIATIVE,         UNCOMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      RARE },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   RARE },
  { ftLEADERSHIP,                 RARE },
  { ftLIGHTNINGREFLEXES,          RARE },
  { ftMOUNTEDCOMBAT,              RARE },
  { ftMOUNTEDARCHERY,             RARE },
  { ftTRAMPLE,                    RARE },
  { ftRIDEBYATTACK,               RARE },
  { ftSPIRITEDCHARGE,             RARE },
  { ftPOINTBLANKSHOT,             COMMON },
  { ftFARSHOT,                    UNCOMMON },
  { ftPRECISESHOT,                UNCOMMON },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                RARE },
  { ftCLEAVE,                     RARE },
  { ftIMPROVEDBULLRUSH,           RARE },
  { ftSUNDER,                     RARE },
  { ftGREATCLEAVE,                RARE },
  { ftQUICKDRAW,                  RARE },
  { ftRUN,                        RARE },
  { ftSKILLFOCUS,                 COMMON*4 },
  { ftTOUGHNESS,                  RARE },
  { ftTRACK,                      RARE },
  { ftTWOWEAPONFIGHTING,          RARE },
  { ftIMPROVEDTWOWEAPONFIGHTING,  RARE },
  { ftWEAPONFINESSE,              RARE },
  { ftWEAPONFOCUS,                RARE },
  { 0,                            0 }
};

static PREFERREDWEAPONS expertWeapons[] = {
  { wpGAUNTLET,            RARE },
  { wpUNARMED,             RARE },
  { wpDAGGER,              COMMON },
  { wpDAGGER_PUNCHING,     RARE },
  { wpGAUNTLET_SPIKED,     RARE },
  { wpMACE_LIGHT,          UNCOMMON },
  { wpSICKLE,              RARE },
  { wpCLUB,                UNCOMMON },
  { wpHALFSPEAR,           RARE },
  { wpMACE_HEAVY,          RARE },
  { wpMORNINGSTAR,         RARE },
  { wpQUARTERSTAFF,        COMMON },
  { wpSHORTSPEAR,          UNCOMMON },
  { wpCROSSBOW_LIGHT,      UNCOMMON },
  { wpDART,                RARE },
  { wpSLING,               RARE },
  { wpCROSSBOW_HEAVY,      RARE },
  { wpJAVELIN,             RARE },
  { 0,                     0 }
};

static PREFERREDWEAPONS commonWeapons[] = {
  { 0,                     0 }
};

static PREFERREDFEATS warriorFeats[] = {
  { ftALERTNESS,                  COMMON },
  { ftAMBIDEXTERITY,              UNCOMMON },
  { ftBLINDFIGHT,                 COMMON },
  { ftCOMBATREFLEXES,             UNCOMMON },
  { ftDODGE,                      UNCOMMON },
  { ftMOBILITY,                   UNCOMMON },
  { ftSPRINGATTACK,               UNCOMMON },
  { ftENDURANCE,                  UNCOMMON },
  { ftWEAPONPROFICIENCY_EXOTIC,   RARE*2 },
  { ftEXPERTISE,                  UNCOMMON },
  { ftIMPROVEDDISARM,             UNCOMMON },
  { ftIMPROVEDTRIP,               UNCOMMON },
  { ftWHIRLWINDATTACK,            UNCOMMON },
  { ftGREATFORTITUDE,             RARE*2 },
  { ftIMPROVEDCRITICAL,           UNCOMMON },
  { ftIMPROVEDINITIATIVE,         COMMON },
  { ftIMPROVEDUNARMEDSTRIKE,      UNCOMMON },
  { ftDEFLECTARROWS,              RARE },
  { ftSTUNNINGFIST,               RARE },
  { ftIRONWILL,                   RARE },
  { ftLEADERSHIP,                 COMMON },
  { ftLIGHTNINGREFLEXES,          UNCOMMON },
  { ftMOUNTEDCOMBAT,              COMMON },
  { ftMOUNTEDARCHERY,             UNCOMMON },
  { ftTRAMPLE,                    COMMON },
  { ftRIDEBYATTACK,               COMMON },
  { ftSPIRITEDCHARGE,             UNCOMMON },
  { ftPOINTBLANKSHOT,             UNCOMMON },
  { ftFARSHOT,                    RARE },
  { ftPRECISESHOT,                RARE },
  { ftRAPIDSHOT,                  RARE },
  { ftSHOTONTHERUN,               RARE },
  { ftPOWERATTACK,                COMMON },
  { ftCLEAVE,                     COMMON },
  { ftIMPROVEDBULLRUSH,           UNCOMMON },
  { ftSUNDER,                     UNCOMMON },
  { ftGREATCLEAVE,                UNCOMMON },
  { ftQUICKDRAW,                  UNCOMMON },
  { ftRUN,                        RARE },
  { ftSKILLFOCUS,                 UNCOMMON },
  { ftTOUGHNESS,                  UNCOMMON },
  { ftTRACK,                      RARE*2 },
  { ftTWOWEAPONFIGHTING,          UNCOMMON },
  { ftIMPROVEDTWOWEAPONFIGHTING,  UNCOMMON },
  { ftWEAPONFINESSE,              UNCOMMON*2 },
  { ftWEAPONFOCUS,                UNCOMMON*2 },
  { 0,                            0 }
};

struct {
  int               dtop_PC;
  int               dtop_NPC;
  int               dtop_All;
  int               type;
  PREFERREDFEATS*   feats;
  PREFERREDFEATS*   bonusFeats;
  PREFERREDWEAPONS* weapons;
  int               abilities[6];
} classes[] = {
  {   9,   0,   4, pcBARBARIAN,   barbarianFeats, 0,                 barbarianWeapons, { abSTRENGTH, abCONSTITUTION, abDEXTERITY, abWISDOM, abCHARISMA, abINTELLIGENCE } },
  {  18,   0,   8, pcBARD,        bardFeats,      0,                 bardWeapons,      { abCHARISMA, abDEXTERITY, abINTELLIGENCE, abCONSTITUTION, abSTRENGTH, abWISDOM } },
  {  27,   0,  12, pcCLERIC,      clericFeats,    0,                 clericWeapons,    { abWISDOM, abSTRENGTH, abCHARISMA, abCONSTITUTION, abDEXTERITY, abINTELLIGENCE } },
  {  36,   0,  16, pcDRUID,       druidFeats,     0,                 druidWeapons,     { abWISDOM, abDEXTERITY, abSTRENGTH, abINTELLIGENCE, abCONSTITUTION, abCHARISMA } },
  {  46,   0,  20, pcFIGHTER,     fighterFeats,   fighterBonusFeats, fighterWeapons,   { abSTRENGTH, abCONSTITUTION, abDEXTERITY, abINTELLIGENCE, abCHARISMA, abWISDOM } },
  {  55,   0,  24, pcMONK,        monkFeats,      0,                 monkWeapons,      { abWISDOM, abDEXTERITY, abSTRENGTH, abINTELLIGENCE, abCONSTITUTION, abCHARISMA } },
  {  64,   0,  28, pcPALADIN,     paladinFeats,   0,                 paladinWeapons,   { abCHARISMA, abWISDOM, abSTRENGTH, abCONSTITUTION, abINTELLIGENCE, abDEXTERITY } },
  {  73,   0,  32, pcRANGER,      rangerFeats,    0,                 rangerWeapons,    { abDEXTERITY, abSTRENGTH, abWISDOM, abINTELLIGENCE, abCONSTITUTION, abCHARISMA } },
  {  82,   0,  36, pcROGUE,       rogueFeats,     0,                 rogueWeapons,     { abDEXTERITY, abWISDOM, abINTELLIGENCE, abCONSTITUTION, abSTRENGTH, abCHARISMA } },
  {  91,   0,  40, pcSORCERER,    sorcererFeats,  0,                 sorcererWeapons,  { abCHARISMA, abINTELLIGENCE, abCONSTITUTION, abDEXTERITY, abWISDOM, abSTRENGTH } },
  { 100,   0,  44, pcWIZARD,      wizardFeats,    wizardBonusFeats,  wizardWeapons,    { abINTELLIGENCE, abCONSTITUTION, abDEXTERITY, abWISDOM, abCHARISMA, abSTRENGTH } },
  {   0,  20,  48, npcADEPT,      adeptFeats,     0,                 adeptWeapons,     { abWISDOM, abINTELLIGENCE, abCHARISMA, abSTRENGTH, abCONSTITUTION, abDEXTERITY } },
  {   0,  40,  52, npcARISTOCRAT, nobleFeats,     0,                 nobleWeapons,     { abCHARISMA, abWISDOM, abDEXTERITY, abINTELLIGENCE, abSTRENGTH, abCONSTITUTION } },
  {   0,  60,  92, npcCOMMONER,   commonFeats,    0,                 commonWeapons,    { abSTRENGTH, abWISDOM, abDEXTERITY, abINTELLIGENCE, abCHARISMA, abCONSTITUTION } },
  {   0,  80,  96, npcEXPERT,     expertFeats,    0,                 expertWeapons,    { abDEXTERITY, abINTELLIGENCE, abWISDOM, abCONSTITUTION, abCHARISMA, abSTRENGTH } },
  {   0, 100, 100, npcWARRIOR,    warriorFeats,   0,                 fighterWeapons,   { abSTRENGTH, abCONSTITUTION, abDEXTERITY, abINTELLIGENCE, abCHARISMA, abWISDOM } },
  {   0,   0,   0, 0,             0,              0,                 0,                { 0, 0, 0, 0, 0, 0 } }
};


struct {
  int dtop_Core;
  int dtop_DMG;
  int dtop_MM;
  int dtop_All;
  int dtop_CC;
  int type;
} races[] = {
  {  30,   0,   0,   2,   0, rcHUMAN },              
  {  40,   0,   0,   4,   0, rcHALFELF },            
  {  55,   0,   0,   6,   0, rcELF_HIGH },           
  {  70,   0,   0,   8,   0, rcDWARF_HILL },         
  {  85,   0,   0,  10,   0, rcHALFLING_LIGHTFOOT },
  {  95,   0,   0,  12,   0, rcGNOME_ROCK },         
  { 100,   0,   0,  14,   0, rcHALFORC },            
  {   0,   3,   0,  15,   0, rcAASIMAR },            
  {   0,   6,   0,  17,   0, rcDWARF_DEEP },         
  {   0,  11,   0,  19,   0, rcDWARF_MOUNTAIN },
  {   0,  14,   0,  21,   0, rcELF_GRAY },           
  {   0,  17,   0,  23,   0, rcELF_WILD },           
  {   0,  20,   0,  25,   0, rcELF_WOOD },           
  {   0,  25,   0,  27,   0, rcGNOME_FOREST },       
  {   0,  28,   0,  29,   0, rcHALFLING_DEEP },
  {   0,  33,   0,  31,   0, rcHALFLING_TALLFELLOW },
  {   0,  36,   0,  33,   0, rcGNOME_SVIRFNEBLIN },  
  {   0,  39,   0,  35,   0, rcLIZARDFOLK },         
  {   0,  42,   0,  37,   0, rcDOPPLEGANGER },       
  {   0,  48,   0,  39,   0, rcGOBLIN },             
  {   0,  53,   0,  41,   0, rcHOBGOBLIN },          
  {   0,  59,   0,  43,   0, rcKOBOLD },             
  {   0,  64,   0,  45,   0, rcORC },                
  {   0,  67,   0,  46,   0, rcTIEFLING },           
  {   0,  70,   0,  47,   0, rcELF_DROW },
  {   0,  71,   0,  48,   0, rcELF_AQUATIC },
  {   0,  73,   0,  49,   0, rcDWARF_DUERGAR },      
  {   0,  74,   0,  50,   0, rcDWARF_DERRO },        
  {   0,  77,   0,  51,   0, rcGNOLL },              
  {   0,  80,   0,  53,   0, rcTROGLODYTE },         
  {   0,  85,   0,  55,   0, rcBUGBEAR },            
  {   0,  91,   0,  57,   0, rcOGRE },               
  {   0,  94,   0,  58,   0, rcMINOTAUR },           
  {   0,  97,   0,  59,   0, rcMINDFLAYER },         
  {   0, 100,   0,  60,   0, rcOGREMAGE },           
  {   0,   0,   4,  62,   0, rcARANEA },             
  {   0,   0,   8,  63,   0, rcAZER },               
  {   0,   0,  13,  65,   0, rcCENTAUR },            
  {   0,   0,  17,  66,   0, rcDRIDER },             
  {   0,   0,  21,  68,   0, rcETTIN },              
  {   0,   0,  26,  70,   0, rcGIANT_HILL },         
  {   0,   0,  30,  72,   0, rcGIANT_STONE },        
  {   0,   0,  34,  74,   0, rcGIANT_FROST },        
  {   0,   0,  38,  76,   0, rcGIANT_FIRE },         
  {   0,   0,  42,  78,   0, rcGIANT_CLOUD },        
  {   0,   0,  46,  79,   0, rcGIANT_STORM },        
  {   0,   0,  50,  81,   0, rcGRIMLOCK },           
  {   0,   0,  54,  83,   0, rcHAG_SEA },            
  {   0,   0,  58,  85,   0, rcHAG_ANNIS },          
  {   0,   0,  62,  86,   0, rcHAG_GREEN },          
  {   0,   0,  67,  88,   0, rcHARPY },              
  {   0,   0,  71,  90,   0, rcKUOTOA },             
  {   0,   0,  75,  92,   0, rcLOCATHAH },           
  {   0,   0,  79,  93,   0, rcMEDUSA },             
  {   0,   0,  83,  95,   0, rcSAHUAGIN },           
  {   0,   0,  88,  97,   0, rcTROLL },
  {   0,   0,  92,  98,   0, rcYUANTI_PUREBLOOD },   
  {   0,   0,  96,  99,   0, rcYUANTI_HALFBLOOD },   
  {   0,   0, 100, 100,   0, rcYUANTI_ABOMINATION }, 
  {   0,   0,   0,   0,   4, rcCC_ABANDONED },       
  {   0,   0,   0,   0,   9, rcCC_ASAATH },          
  {   0,   0,   0,   0,  13, rcCC_BATDEVIL },        
  {   0,   0,   0,   0,  18, rcCC_PLAGUEWRETCH },    
  {   0,   0,   0,   0,  22, rcCC_CHARDUNI },        
  {   0,   0,   0,   0,  27, rcCC_COALGOBLIN },      
  {   0,   0,   0,   0,  31, rcCC_DWARF_FORSAKEN },  
  {   0,   0,   0,   0,  37, rcCC_ELF_FORSAKEN },    
  {   0,   0,   0,   0,  41, rcCC_ICE_GHOUL },        
  {   0,   0,   0,   0,  46, rcCC_MANTICORA },       
  {   0,   0,   0,   0,  50, rcCC_MORGAUNT },        
  {   0,   0,   0,   0,  53, rcCC_PROUD },
  {   0,   0,   0,   0,  57, rcCC_RATMAN },          
  {   0,   0,   0,   0,  62, rcCC_RATMAN_BROWNGORGER },
  {   0,   0,   0,   0,  66, rcCC_RATMAN_DISEASED },
  {   0,   0,   0,   0,  71, rcCC_RATMAN_FOAMER,  },
  {   0,   0,   0,   0,  75, rcCC_RATMAN_REDWITCH },
  {   0,   0,   0,   0,  79, rcCC_SKINDEVIL },
  {   0,   0,   0,   0,  83, rcCC_SPIDEREYEGOBLIN },
  {   0,   0,   0,   0,  87, rcCC_STEPPETROLL },
  {   0,   0,   0,   0,  91, rcCC_STRIFEELEMENTAL },
  {   0,   0,   0,   0,  95, rcCC_TOKALTRIBESMAN },
  {   0,   0,   0,   0, 100, rcCC_UBANTUTRIBESMAN },
  {   0,   0,   0,   0,   0, 0 }
};


static int getRandomGEAlignment() {
  int i;

  i = rollDice( 1, 100 );
  if( i <= 20 ) {
    return alGOOD;
  } else if( i <= 50 ) {
    return alGENEUTRAL;
  } else {
    return alEVIL;
  }

  return 0;
}

static int getRandomLCAlignment() {
  switch( rollDice( 1, 3 ) ) {
    case 1: return alLAWFUL;
    case 2: return alLCNEUTRAL;
    case 3: return alCHAOTIC;
  }
  return 0;
}


static int icomp( const void* x, const void* y ) {
  int ix = *(int*)x;
  int iy = *(int*)y;

  if( ix < iy ) return 1;
  if( ix > iy ) return -1;
  return 0;
}


static int abscorecomp( const void* x, const void* y ) {
  ABILITYSCORE* ax = (ABILITYSCORE*)x;
  ABILITYSCORE* ay = (ABILITYSCORE*)y;

  if( ax->score < ay->score ) return 1;
  if( ax->score > ay->score ) return -1;
  return 0;
}


static int getSkillType( NPC* npc, int classType, int skill ) {
  NPCCLASS* cls;
  NPCEXPERTDATA* data;
  int i;

  if( classType == npcEXPERT ) {
    cls = npcGetClass( npc, classType );
    if( cls == 0 ) {
      return sktEXCLUSIVE;
    }
    data = (NPCEXPERTDATA*)cls->data;
    for( i = 0; i < 10; i++ ) {
      if( data->classSkills[ i ] == skill ) {
        return sktCLASS;
      }
    }
  }

  return dndGetSkillType( classType, skill );
}


static int getNamedAbility( NPC* npc, int ability ) {
  switch( ability ) {
    case abSTRENGTH: return npc->strength;
    case abDEXTERITY: return npc->dexterity;
    case abCONSTITUTION: return npc->constitution;
    case abINTELLIGENCE: return npc->intelligence;
    case abWISDOM: return npc->wisdom;
    case abCHARISMA: return npc->charisma;
  }

  return 0;
}


static void addLanguage( NPC* npc, int language ) {
  NPCLANGUAGE* l;
  NPCLANGUAGE* i;
  NPCLANGUAGE* p;
  char*        lname;
  int          rc;

  l = (NPCLANGUAGE*)malloc( sizeof( NPCLANGUAGE ) );
  memset( l, 0, sizeof( *l ) );

  lname = dndGetLanguageName( language );

  l->language = language;

  if( npc->languages == 0 ) {
    npc->languages = l;
  } else {
    i = npc->languages;
    p = 0;
    while( i != 0 ) {
      rc = strcmp( lname, dndGetLanguageName( i->language ) );
      if( rc < 0 ) {
        l->next = i;
        if( p == 0 ) {
          npc->languages = l;
        } else {
          p->next = l;
        }
        return;
      }
      p = i;
      i = i->next;
    }
    p->next = l;
  }
}


static int getWeightingForSpell( int classType, int spell ) {
  int i;
  PREFERREDSPELLS* spells;

  switch( classType ) {
    case pcBARD:
      spells = prefBardSpells;
      break;
    case pcSORCERER:
      spells = prefSorcererSpells;
      break;
    case pcWIZARD:
      spells = prefWizardSpells;
      break;
    default:
      return 1;
  }

  for( i = 0; spells[ i ].spell != 0; i++ ) {
    if( spells[ i ].spell == spell ) {
      return spells[ i ].weight;
    }
  }

  return 1;
}

static void addSpellToRepertoire( NPCCLASS* cls, int spell, int level ) {
  NPCSPELL** list;
  NPCSPELL*  i;
  NPCSPELL*  p;
  NPCSPELL*  n;
  char*      name;
  int        rc;

  /* verify that the spell level is valid */

  if( ( level < 0 ) || ( level > 9 ) ) {
    return;
  }

  /* grab the spell list from the appropriate class data structure, and
   * verify the spell level further */

  switch( cls->type ) {
    case pcBARD:
      if( level > 6 ) {
        return;
      }
      list = ((NPCBARDDATA*)(cls->data))->spells;
      break;
    case pcSORCERER:
      list = ((NPCSORCERERDATA*)(cls->data))->spells;
      break;
    case pcWIZARD:
      list = ((NPCWIZARDDATA*)(cls->data))->spells;
      break;
    default:
      return;
  }

  /* allocate a new spell structure */

  n = (NPCSPELL*)malloc( sizeof( NPCSPELL ) );
  memset( n, 0, sizeof( *n ) );
  n->spell = spell;

  /* put the spell in the repertoire at the indicated level */

  i = list[ level ];
  if( i == 0 ) {
    list[ level ] = n;
  } else {
    /* insert the spell in sorted order, by name */

    name = dndGetSpellName( spell );
    p = 0;
    do {
      rc = strcmp( name, dndGetSpellName( i->spell ) );
      
      if( rc < 0 ) {
        n->next = i;
        if( p == 0 ) {
          list[ level ] = n;
        } else {
          p->next = n;
        }
        return;
      } else if( rc == 0 ) { /* spell already exists in repertoire */
        free( n );
        return;
      }

      p = i;
      i = i->next;
    } while( i != 0 );
    p->next = n;    
  }
}


static void selectDomainsForNPC( NPC* npc, NPCCLERICDATA* data ) {
  WEIGHTEDLIST* wlist;
  int           total;
  int           i;
  int           forbidden;

  wlist = 0;
  total = 0;

  i = 1;
  for( i = dmAIR; dndGetDomainName( i ) != 0; i++ ) {
    forbidden = dndGetForbiddenAlignmentsForDomain( i );
    if( ( npc->alignment & forbidden ) == 0 ) {
      total += addToWeightedList( &wlist, i, 1 );
    }
  }

  for( i = 0; i < 2; i++ ) {
    data->domain[ i ] = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  }

  destroyWeightedList( &wlist );
}


static int selectSpellMasterySpells( NPCSPELL** list, NPCFEAT* feats, NPCFEATSPELLS* featSpells ) {
  WEIGHTEDLIST*  wlist;
  int            total;
  int            i;
  int            j;
  int            found;
  NPCSPELL*      spell;
  NPCFEAT*       feat;
  NPCFEATSPELLS* nfs;

  wlist = 0;
  total = 0;

  for( i = 1; i < 10; i++ ) {
    for( spell = list[ i ]; spell != 0; spell = spell->next ) {

      found = 0;
      for( feat = feats; feat != 0; feat = feat->next ) {
        if( feat->type == ftSPELLMASTERY ) {
          nfs = (NPCFEATSPELLS*)feat->data;
          for( j = 0; j < 3; j++ ) {
            if( nfs->spells[j] == spell->spell ) {
              found = 1;
              break;
            }
          } /* 1 ... 3 */
        } /* if feat->type == ftSPELLMASTERY */
        if( found ) {
          break;
        }
      } /* feats */

      if( found ) {
        continue;
      }

      total += addToWeightedList( &wlist, spell->spell, getWeightingForSpell( pcWIZARD, spell->spell ) );
    } /* list[ i ] */
  } /* 1 ... 9 */

  if( wlist == 0 ) {
    return 0;
  }

  for( i = 0; i < 3; i++ ) {
    featSpells->spells[i] = getWeightedItem( &wlist, rollDice( 1, total ), &total );
    if( ( wlist == 0 ) && ( i < 2 ) ) {
      return 0;
    }    
  }

  destroyWeightedList( &wlist );

  return 1;
}


static int findWeaponWeighting( int weapon, PREFERREDWEAPONS* weapons ) {
  int i;

  for( i = 0; weapons[ i ].weapon != 0; i++ ) {
    if( weapons[ i ].weapon == weapon ) {
      return weapons[ i ].weight;
    }
  }

  return 1;
}


static int countFeatsWithWeapon( int weapon, NPCFEAT* feats ) {
  int count;
  NPCFEAT* f;
  NPCFEATWEAPON* nfw;

  count = 0;
  for( f = feats; f != 0; f = f->next ) {
    switch( f->type ) {
      case ftWEAPONPROFICIENCY_SIMPLE:
      case ftWEAPONPROFICIENCY_MARTIAL:
      case ftWEAPONPROFICIENCY_EXOTIC:
      case ftIMPROVEDCRITICAL:
      case ftWEAPONFINESSE:
      case ftWEAPONFOCUS:
      case ftWEAPONSPECIALIZATION:
        nfw = f->data;
        if( nfw->weapon == weapon ) {
          count++;
        }
        break;
    }
  }

  return count;
}


static int selectAppropriateSchoolOfMagic( NPC* npc ) {
  NPCFEAT* f;
  WEIGHTEDLIST* wlist;
  int total;
  int i;
  int found;
  NPCFEATSCHOOL* nfsch;

  wlist = 0;
  total = 0;

  for( i = 0; schoolsOfMagic[ i ] != 0; i++ ) {
    if( schoolsOfMagic[ i ] == ssUNIVERSAL ) {
      continue;
    }

    found = 0;
    for( f = npc->feats; f != 0; f = f->next ) {
      if( f->type == ftSPELLFOCUS ) {
        nfsch = f->data;
        if( nfsch->school == schoolsOfMagic[ i ] ) {
          found = 1;
          break;
        }
      }
    }
    if( found ) {
      continue;
    }
    total += addToWeightedList( &wlist, schoolsOfMagic[i], 1 );
  }

  if( wlist == 0 ) {
    return 0;
  }

  i = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  destroyWeightedList( &wlist );

  return i;
}


static int selectExistingSkill( int feat, int classType, NPCSKILL* skills, NPCFEAT* feats ) {
  WEIGHTEDLIST* wlist;
  int           total;
  NPCSKILL*     s;
  NPCFEAT*      f;
  int           i;
  int           found;
  NPCFEATSKILL* nfs;

  wlist = 0;
  total = 0;

  for( s = skills; s != 0; s = s->next ) {
    found = 0;

    for( f = feats; f != 0; f = f->next ) {
      if( f->type == feat ) {
        nfs = (NPCFEATSKILL*)f->data;
        if( nfs->skill == s->type ) {
          found = 1;
          break;
        }
      }
    }

    if( found ) {
      continue;
    }

    i = dndGetSkillType( classType, s->type );
    if( i == sktCLASS ) {
      total += addToWeightedList( &wlist, s->type, 10 );
    } else {
      total += addToWeightedList( &wlist, s->type, 1 );
    }
  }

  if( wlist == 0 ){
    return 0;
  }

  i = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  destroyWeightedList( &wlist );

  return i;
}


/* a character cannot use a weapon that is more than one size large than
 * the character -- ie, halflings cannot use greatswords */

int npcCanUseWeapon( NPC* npc, int weapon ) {
  int wsize;

  wsize = dndGetWeaponSize( weapon );
  if( wsize - dndGetRaceSize( npc->race ) > 1 ) {
    return 0;
  }

  return 1;
}


/* selectProficientWeapon
 *   - selects one weapon among those in the PREFERREDWEAPONs array, and 
 *     among those specified in the feats list as being proficient.
 */

static int selectProficientWeapon( NPC* npc, int feat, PREFERREDWEAPONS* weapons, int type ) {
  WEIGHTEDLIST* wlist;
  int           total;
  int           i;
  int           found;
  NPCFEAT*      f;
  NPCFEAT*      f2;
  int           size;

  size = ( type & ( wtLIGHT | wtONEHANDED | wtTWOHANDED ) );
  type &= ~( wtLIGHT | wtONEHANDED | wtTWOHANDED );
  if( type == 0 ) {
    type = 0xFFFF;
  }

  wlist = 0;
  total = 0;
  for( i = 0; weapons[ i ].weapon != 0; i++ ) {
    found = 0;

    if( !npcCanUseWeapon( npc, weapons[ i ].weapon ) ) {
      continue;
    }
        
    for( f = npc->feats; f != 0; f = f->next ) {
      if( f->type == feat ) {
        NPCFEATWEAPON* nfw = (NPCFEATWEAPON*)f->data;
        if( nfw->weapon == weapons[ i ].weapon ) {
          found = 1;
          break;
        }
      }
    }

    if( !found && ( dndGetWeaponType( weapons[ i ].weapon ) & type ) == 0 ) {
      found = 1;
    }

    if( !found && ( size > 0 ) ) {
      if( ( dndGetRelativeWeaponSize( npc->race, weapons[ i ].weapon ) & size ) == 0 ) {
        found = 1;
      }
    }

    if( found ) {
      continue;
    }

    found = countFeatsWithWeapon( weapons[ i ].weapon, npc->feats );
    total += addToWeightedList( &wlist, weapons[ i ].weapon, findWeaponWeighting( weapons[ i ].weapon, weapons ) * ( 2 << ( found + 1 ) ) * 10 );
  }

  for( f = npc->feats; f != 0; f = f->next ) {
    NPCFEATWEAPON* nfw1 = (NPCFEATWEAPON*)f->data;
    found = 1;
    switch( f->type ) {
      case ftWEAPONPROFICIENCY_SIMPLE:
      case ftWEAPONPROFICIENCY_MARTIAL:
      case ftWEAPONPROFICIENCY_EXOTIC:
        found = 0;
        for( f2 = npc->feats; f2 != 0; f2 = f2->next ) {
          if( f2->type == feat ) {
            NPCFEATWEAPON* nfw2 = (NPCFEATWEAPON*)f2->data;
            if( nfw2->weapon == nfw1->weapon ) {
              found = 1;
              break;
            }
          }
        }
        break;
    }
    if( found ) {
      continue;
    }
    found = countFeatsWithWeapon( nfw1->weapon, npc->feats );
    total += addToWeightedList( &wlist, nfw1->weapon, findWeaponWeighting( nfw1->weapon, weapons ) * ( 2 << ( found + 1 ) ) * 10 );
  }

  if( wlist == 0 ) {
    return 0;
  }

  i = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  destroyWeightedList( &wlist );

  return i;
}


/* selectWeaponForProficiency
 *   selects one weapon from the appropriate list (simple, martial, exotic)
 *   that does not already exist in either the weapons list or the feats
 *   list. */

static int selectWeaponForProficiency( NPC* npc, int feat, PREFERREDWEAPONS* weapons ) {
  WEIGHTEDLIST* wlist;
  int           total;
  int           i;
  int           j;
  int           found;
  NPCFEAT*      f;
  int*          weaponList;

  switch( feat ) {
    case ftWEAPONPROFICIENCY_SIMPLE: weaponList = (int*)simpleWeapons; break;
    case ftWEAPONPROFICIENCY_MARTIAL: weaponList = (int*)martialWeapons; break;
    case ftWEAPONPROFICIENCY_EXOTIC: weaponList = (int*)exoticWeapons; break;
    default:
      return 0;
  }

  wlist = 0;
  total = 0;
  for( i = 0; weaponList[ i ] != 0; i++ ) {
    found = 0;
    
    if( !npcCanUseWeapon( npc, weaponList[ i ] ) ) {
      continue;
    }

    for( j = 0; weapons[ j ].weapon != 0; j++ ) {
      if( weaponList[ i ] == weapons[ j ].weapon ) {
        found = 1;
        break;
      }
    }
    
    if( found ) {
      continue;
    }

    for( f = npc->feats; f != 0; f = f->next ) {
      if( f->type == feat ) {
        NPCFEATWEAPON* nfw = (NPCFEATWEAPON*)f->data;
        if( nfw->weapon == weaponList[ i ] ) {
          found = 1;
          break;
        }
      }
    }

    if( found ) {
      continue;
    }

    total += addToWeightedList( &wlist, weaponList[ i ], findWeaponWeighting( weaponList[ i ], weapons ) );
  }

  if( wlist == 0 ) {
    return 0;
  }

  i = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  destroyWeightedList( &wlist );

  return i;
}


static int selectWeaponForSpecialization( NPC* npc ) {
  WEIGHTEDLIST *wlist;
  int            total;
  NPCFEAT*       f;
  NPCFEAT*       f2;
  int            found;

  wlist = 0;
  total = 0;  

  for( f = npc->feats; f != 0; f = f->next ) {
    if( f->type == ftWEAPONFOCUS ) {
      NPCFEATWEAPON* nfw1 = (NPCFEATWEAPON*)f->data;

      found = 0;
      for( f2 = npc->feats; f2 != 0; f2 = f2->next ) {
        if( f2->type == ftWEAPONSPECIALIZATION ) {
          NPCFEATWEAPON* nfw2 = (NPCFEATWEAPON*)f2->data;

          if( nfw1->weapon == nfw2->weapon ) {
            found = 1;
          }
        }
      }

      if( found ) {
        continue;
      }

      total += addToWeightedList( &wlist, nfw1->weapon, ( 2 << countFeatsWithWeapon( nfw1->weapon, npc->feats ) ) * 10 );
    }
  }

  if( wlist == 0 ) {
    return 0;
  }

  found = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  destroyWeightedList( &wlist );

  return found;
}


void computeExpertSkillSet( NPCEXPERTDATA* data ) {
  int i;
  WEIGHTEDLIST* wlist;
  int total;

  wlist = 0;
  total = 0;
  for( i = 0; allSkills[ i ] != 0; i++ ) {
    total += addToWeightedList( &wlist, allSkills[ i ], 1 );
  }

  for( i = 0; i < 10; i++ ) {
    data->classSkills[ i ] = getWeightedItem( &wlist, rollDice( 1, total ), &total );
  }

  destroyWeightedList( &wlist );
}


void addClass( NPC* npc, int type, int level ) {
  NPCCLASS* c;
  NPCCLASS* i;

  c = (NPCCLASS*)malloc( sizeof( NPCCLASS ) );
  memset( c, 0, sizeof( *c ) );

  c->type = type;
  c->level = level;

  switch( type ) {
    case pcBARD:
      if( npc->charisma < 12 ) {
        npc->charisma = 12;
      }
      c->data = (void*)malloc( sizeof( NPCBARDDATA ) );
      memset( c->data, 0, sizeof( NPCBARDDATA ) );
      break;
    case pcCLERIC:
      if( npc->wisdom < 12 ) {
        npc->wisdom = 12;
      }
      c->data = (void*)malloc( sizeof( NPCCLERICDATA ) );
      memset( c->data, 0, sizeof( NPCCLERICDATA ) );
      selectDomainsForNPC( npc, (NPCCLERICDATA*)c->data );
      break;
    case pcDRUID:
      if( npc->wisdom < 12 ) {
        npc->wisdom = 12;
      }
      c->data = (void*)malloc( sizeof( NPCDRUIDDATA ) );
      memset( c->data, 0, sizeof( NPCDRUIDDATA ) );
      break;
    case pcPALADIN:
      if( npc->wisdom < 12 ) {
        npc->wisdom = 12;
      }
      c->data = (void*)malloc( sizeof( NPCPALADINDATA ) );
      memset( c->data, 0, sizeof( NPCPALADINDATA ) );
      break;
    case pcRANGER:
      if( npc->wisdom < 12 ) {
        npc->wisdom = 12;
      }
      c->data = (void*)malloc( sizeof( NPCRANGERDATA ) );
      memset( c->data, 0, sizeof( NPCRANGERDATA ) );
      break;
    case pcSORCERER:
      if( npc->charisma < 12 ) {
        npc->charisma = 12;
      }
      c->data = (void*)malloc( sizeof( NPCSORCERERDATA ) );
      memset( c->data, 0, sizeof( NPCSORCERERDATA ) );
      break;
    case pcWIZARD:
      if( npc->intelligence < 12 ) {
        npc->intelligence = 12;
      }
      c->data = (void*)malloc( sizeof( NPCWIZARDDATA ) );
      memset( c->data, 0, sizeof( NPCWIZARDDATA ) );
      break;
    case npcADEPT:
      if( npc->wisdom < 12 ) {
        npc->wisdom = 12;
      }
      break;
    case npcEXPERT:
      c->data = (void*)malloc( sizeof( NPCEXPERTDATA ) );
      computeExpertSkillSet( (NPCEXPERTDATA*)c->data );
      break;
  }

  i = npc->classes;
  if( i == 0 ) {
    npc->classes = c;
  } else {
    while( i->next != 0 ) {
      i = i->next;
    }
    i->next = c;
  }
}


int lookupAbilityScore( ABILITYSCORE* scores, int ability ) {
  int i;
  for( i = 0; i < 6; i++ ) {
    if( scores[i].ability == ability ) {
      return scores[i].score;
    }
  }
  return 0;
}


int lookupAbility( int classType, int which ) {
  int i;
  for( i = 0; classes[i].type != 0; i++ ) {
    if( classes[i].type == classType ) {
      return classes[i].abilities[which];
    }
  }
  return 0;
}


void addSkill( NPC* npc, int skillType, float ranks ) {
  NPCSKILL* skill;
  NPCSKILL* i;
  NPCSKILL* p;
  char* skillTxt;
  int rc;

  skill = (NPCSKILL*)malloc( sizeof( NPCSKILL ) );
  memset( skill, 0, sizeof( *skill ) );

  skillTxt = dndGetSkillName( skillType );

  skill->type = skillType;
  skill->rank = ranks;

  if( npc->skills == 0 ) {
    npc->skills = skill;
  } else {
    i = npc->skills;
    p = 0;
    while( i != 0 ) {
      rc = strcmp( skillTxt, dndGetSkillName( i->type ) );
      if( rc < 0 ) {
        skill->next = i;
        if( p == 0 ) {
          npc->skills = skill;
        } else {
          p->next = skill;
        }
        return;
      }
      p = i;
      i = i->next;
    }
    p->next = skill;
  }
}


void addFeat( NPC* npc, int featType, int autoAdd, void* data ) {
  NPCFEAT* feat;
  NPCFEAT* i;
  NPCFEAT* p;
  int rc;
  char* featTxt;

  feat = (NPCFEAT*)malloc( sizeof( NPCFEAT ) );
  memset( feat, 0, sizeof( *feat ) );

  featTxt = dndGetFeatName( featType );

  feat->type = featType;
  feat->autoAdd = autoAdd;
  feat->data = data;

  if( npc->feats == 0 ) {
    npc->feats = feat;
  } else {
    i = npc->feats;
    p = 0;
    while( i != 0 ) {
      rc = strcmp( featTxt, dndGetFeatName( i->type ) );
      if( rc < 0 ) {
        feat->next = i;
        if( p == 0 ) {
          npc->feats = feat;
        } else {
          p->next = feat;
        }
        return;
      }
      p = i;
      i = i->next;
    }
    p->next = feat;
  }
}


void addFeatIfNotExist( NPC* npc, int featType, int autoAdd, void* data ) {
  if( npcHasFeat( npc, featType ) ) {
    return;
  }

  addFeat( npc, featType, autoAdd, data );
}


int addNewLanguages( NPC* npc, int count ) {
  WEIGHTEDLIST* llist;
  int ltotal;
  char* next;
  int k;
  int n;

  /* pick count languages to learn */

  llist = 0;
  ltotal = 0;
  n = npc->race;
  while( ( k = dndGetBonusLanguages( n, &next ) ) != 0 ) {
    n = 0;
    if( !npcKnowsLanguage( npc, k ) ) {
      ltotal += addToWeightedList( &llist, k, 1 );
    }
  }

  while( count > 0 ) {
    if( llist == 0 ) {
      ltotal = 0;
      for( k = 0; allLanguages[ k ] != 0; k++ ) {
        if( !npcKnowsLanguage( npc, allLanguages[ k ] ) ) {
          ltotal += addToWeightedList( &llist, allLanguages[ k ], 1 );
        }
      }
    }
    if( llist == 0 ) {
      break;
    }
    addLanguage( npc,
                 getWeightedItem( &llist, rollDice( 1, ltotal ), &ltotal ) );
    count--;
  }

  destroyWeightedList( &llist );

  return count;
}


float computeMaxSkillRank( NPC* npc, int excludeClass, int skill ) {
  NPCCLASS* cls;
  float     total;
  int       i;

  total = 0;
  for( cls = 0; cls != 0; cls = cls->next ) {
    if( cls->type != excludeClass ) {
      i = getSkillType( npc, cls->type, skill );

      if( i == sktCLASS ) {
        total += cls->level + 3;
      } else if( i == sktCROSSCLASS ) {
        total += ( cls->level + 3 ) / 2;
      }
    }
  }

  return total;
}


void computeSkills( NPC* npc, int classType, int tlevel, int clevel ) {
  int points;
  WEIGHTEDLIST* wlist;
  int total;
  int weight;
  int i;
  int j;
  int k;
  float maxClassRank;
  int diff;
  float ranks;
  float rankIncrease;
  NPCSKILL *skill;
  NPCCLASS *class;

  class = npcGetClass( npc, classType );

  points = dndGetSkillBonusForClass( classType ) +
           dndGetAbilityBonus( npc->intelligence );
  if( points < 1 ) {
    points = 1;
  }

  /* humans get an additional skill point at each level */

  if( npc->race == rcHUMAN ) {
    points++;
  }

  /* at first level, the number of skill points is quadrupled */

  if( tlevel == 1 ) {
    points *= 4;
  }

  class->skill_points += points;

  while( points > 0 ) {

    /* make a weighted list of all existing skills so that the ones at the
     * end of the list don't be shorted. */

    wlist = 0;
    total = 0;
    for( skill = npc->skills; skill != 0; skill = skill->next ) {
      i = getSkillType( npc, classType, skill->type );
      total += addToWeightedList( &wlist, (int)skill, ( i == sktCLASS ? 20 : 1 ) );
    }

    /* fist, go through all existing skills.  If the existing skill is a
     * class skill, there is an 60% chance that the skill will be raised
     * to it's maximum rank (or as high as possible given remaining skill
     * points) and a 20% chance that the skill will be raised by 1 point.
     * if the skill is cross-class, there is a 30% chance that it will be
     * raised to its maximum rank (or as high as possible given remaining
     * skill points) and a 10% chance that it will be raised by 0.5 point.
     */

    while( wlist != 0 ) {
      skill = (NPCSKILL*)getWeightedItem( &wlist, rollDice( 1, total ), &total );
      i = getSkillType( npc, classType, skill->type );
      
      maxClassRank = computeMaxSkillRank( npc, classType, skill->type );
      rankIncrease = 0.0;

      if( i == sktCLASS ) {
        maxClassRank += clevel + 3;
        j = rollDice( 1, 100 );
        if( j <= 60 ) {
          diff = (int)( maxClassRank - skill->rank );
          if( diff > points ) {
            diff = points;
          }
          rankIncrease = (float)diff;
        } else if( j <= 80 ) {
          rankIncrease = 1.0;
        }
      } else {
        maxClassRank += (float)( ( clevel + 3 ) / 2.0 );
        j = rollDice( 1, 100 );
        if( j <= 30 ) {
          diff = (int)( maxClassRank - skill->rank );
          if( diff > points ) {
            diff = points;
          }
          rankIncrease = (float)diff;
        } else if( j <= 40 ) {
          rankIncrease = 0.5;
        }
      }

      if( rankIncrease < 0.5 ) {
        continue;
      }

      if( skill->type == skSPEAKLANGUAGE ) {
        if( rankIncrease - (int)rankIncrease > 0 ) {
          if( rankIncrease*2 + 1 > points ) {
            if( rankIncrease*2 - 1 < 1 ) {
              continue;
            }
            rankIncrease -= 0.5;
          } else {
            rankIncrease += 0.5;
          }
        }

        k = addNewLanguages( npc, (int)rankIncrease );
        rankIncrease -= k;
        if( rankIncrease < 1 ) {
          continue;
        }
      }

      skill->rank += rankIncrease;
      diff = (int)( i == sktCLASS ? rankIncrease : rankIncrease*2 );
      points -= diff;
            
      if( points < 1 ) {
        break;
      }
    }

    destroyWeightedList( &wlist );

    if( points < 1 ) {
      break;
    }

    /* build a weighted list of all skills that the given character can
     * possibly have */

    wlist = 0;
    total = 0;

    for( i = 0; allSkills[ i ] != 0; i++ ) {
    
      /* don't put the same skill in more than once */
      if( npcHasSkill( npc, allSkills[ i ] ) ) {
        continue;
      }

      j = getSkillType( npc, classType, allSkills[ i ] );

      if( j == sktEXCLUSIVE ) {
        continue;
      } else if( j == sktCLASS ) {
        /* changed from 10 to 20 by Jamis Buck, 13 Dec 2001 */
        weight = 20;
      } else {
        weight = 1;
      }

      k = dndGetSkillAbility( allSkills[ j ] );
      if( k != abNONE ) {
        weight *= dndGetAbilityBonus( getNamedAbility( npc, k ) );
        if( j == sktCLASS ) {
          k = 10;
        } else {
          k = 1;
        }
        if( weight < k ) {
          weight = k;
        }
      }

      if( ( ( classType == pcBARD ) && ( allSkills[ i ] == skPERFORM ) ) ||
          ( ( npcHasFeat( npc, ftTRACK ) && ( allSkills[ i ] == skWILDERNESSLORE ) ) ) )
      {
        weight *= 20;
      }

      total += addToWeightedList( &wlist, allSkills[ i ], weight );
    }

    /* if wlist is 0, then there are no more skills available to be taken */

    if( wlist == 0 ) {
      break;
    }

    while( wlist != 0 ) {
      j = getWeightedItem( &wlist, rollDice( 1, total ), &total );

      /* if this is the first character level, the character is 80% likely
       * to put the skill at the maximum possible rank, and 20% likely to
       * put it at half the maximum rank (or, at least, at the minimum
       * possible -- 1, or 0.5). If this is not the first character level,
       * they have a 50% chance of putting 1 (or 0.5) rank, and a 50% chance
       * of putting 2 (or 1) ranks in. */

      i = getSkillType( npc, classType, j );
      maxClassRank = (float)( i == sktCLASS ? ( clevel + 3 ) : ( ( clevel + 3 ) / 2.0 ) );

      k = rollDice( 1, 100 );
      if( tlevel == 1 ) {
        if( k <= 80 ) {
          ranks = maxClassRank;
        } else {
          if( maxClassRank - (int)maxClassRank > 0 ) {
            ranks = (float)( ( maxClassRank + 0.5 ) / 2.0 );
          } else {
            ranks = (float)( maxClassRank / 2.0 );
          }
        }
        if( i == sktCLASS ) {
          if( ranks > points ) {
            ranks = (float)points;
          }
        } else if( ranks*2 > points ) {
          ranks = (float)( points / 2.0 );
        }
      } else {
        if( k <= 50 ) {
          ranks = (float)( i == sktCLASS ? 1 : 0.5 );
        } else {
          ranks = (float)( i == sktCLASS ? 2 : 1 );
        }
      }

      if( j == skSPEAKLANGUAGE ) {
        if( i != sktCLASS ) {
          if( ranks - (int)ranks > 0 ) {
            if( ranks*2 + 1 > points ) {
              if( ranks*2 - 1 < 1 ) {
                continue;
              }
              ranks -= 0.5;                
            } else {
              ranks += 0.5;
            }
          }
        }
        
        k = addNewLanguages( npc, (int)ranks );

        ranks -= (float)( i == sktCLASS ? k : k / 2.0 );

        if( ranks < 0.5 ) {
          continue;
        }
      }

      addSkill( npc, j, ranks );
      diff = (int)( i == sktCLASS ? ranks : ranks*2 );
      points -= diff;

      if( points < 1 ) {
        break;
      }
    }

    destroyWeightedList( &wlist );
  }
}


int calculateCharacterLevel( NPC* npc ) {
  NPCCLASS* cls;
  int       total;

  total = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total += cls->level;
  }
  
  return total; 
}


int calculateSpellcasterLevel( NPC* npc ) {
  NPCCLASS* cls;
  int       total;

  total = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    switch( cls->type ) {
      case pcCLERIC:
      case pcDRUID:
      case npcADEPT:
      case pcSORCERER:
      case pcWIZARD:
      case pcBARD:
      case prcLOREMASTER:
      case prcBLACKGUARD:
        if( total < cls->level ) {
          total = cls->level;
        }
        break;
      case pcPALADIN:
      case pcRANGER:
        if( cls->level >= 4 ) {
          if( total < ( cls->level >> 1 ) ) {
            total = cls->level >> 1;
          }
        }
        break;
    }
  }
  
  return total; 
}


int meetsFeatPreReqs( NPC* npc, int feat, int classType, int level ) {
  int   type;
  int   data1;
  int   data2;
  int   rc;
  char* next;

  rc = dndGetFeatPrerequisite( feat, &type, &data1, &data2, &next );
  while( rc != 0 ) {
    switch( type ) {
      case fprMINIMUMABILITYSCORE:
        data1 = getNamedAbility( npc, data1 );
        if( data1 < data2 ) {
          return 0;
        }
        break;
      case fprFEAT:
        if( !npcHasFeat( npc, data1 ) ) {
          return 0;
        }
        break;
      case fprMINIMUMBASEATTACKBONUS:
        if( npcGetBaseAttack( npc ) < data1 ) {
          return 0;
        }
        break;
      case fprMINIMUMCHARACTERLEVEL:
        if( calculateCharacterLevel( npc ) < data1 ) {
          return 0;
        }
        break;
      case fprSKILL:
        if( !npcHasSkill( npc, data1 ) ) {
          return 0;
        }
        break;
      case fprPROFICIENTWITHWEAPON:
        /* this is handled when the calling routine tries to identify a
         * weapon with which the NPC is already proficient.  If there are
         * none, then the feat is skipped. */
        break;
      case fprMINIMUMSPELLCASTERLEVEL:
        if( calculateSpellcasterLevel( npc ) < data1 ) {
          return 0;
        }
        break;
      case fprCLERICORPALADIN:
        if( ( classType != pcCLERIC ) && ( classType != pcPALADIN ) ) {
          return 0;
        }
        break;
      case fprWIZARD:
        if( classType != pcWIZARD ) {
          return 0;
        }
        break;
      case fprMINIMUMFIGHTERLEVEL:
        if( classType != pcFIGHTER ) {
          return 0;
        }
        if( level < data1 ) {
          return 0;
        }
        break;
    }

    rc = dndGetFeatPrerequisite( 0, &type, &data1, &data2, &next );
  }

  return 1;
}


PREFERREDWEAPONS* getPreferredWeapons( int classType ) {
  int i;

  for( i = 0; classes[ i ].type != 0; i++ ) {
    if( classes[ i ].type == classType ) {
      return classes[ i ].weapons;
    }
  }

  return 0;
}


void addFeatsFromList( NPC* npc, PREFERREDFEATS* feats, int count, int classType, int level ) {
  WEIGHTEDLIST* wlist;
  int           total;
  int           i;
  int           feat;
  int           rc;
  NPCFEATWEAPON* nfw;
  NPCFEATSKILL*  nfs;
  NPCFEATSCHOOL* nfsch;
  void*         data;

  wlist = 0;
  total = 0;

  for( i = 0; feats[i].feat != 0; i++ ) {
    if( !npcHasFeat( npc, feats[i].feat ) || dndFeatIsReusable( feats[i].feat ) ) {
      total += addToWeightedList( &wlist, feats[i].feat, feats[i].weight );
    }
  }

  for( i = 0; i < count; i++ ) {
    feat = getWeightedItem( &wlist, rollDice( 1, total ), &total );
    if( meetsFeatPreReqs( npc, feat, classType, level ) ) {

      data = 0;

      switch( feat ) {
        case ftSPELLMASTERY:
          {
            NPCCLASS *c;
            NPCWIZARDDATA *d;
            NPCFEATSPELLS* nfspell;

            c = npcGetClass( npc, classType );
            d = ((NPCWIZARDDATA*)c->data);

            nfspell = (NPCFEATSPELLS*)malloc( sizeof( NPCFEATSPELLS ) );
            memset( nfspell, 0, sizeof( *nfspell ) );

            rc = selectSpellMasterySpells( d->spells, npc->feats, nfspell );

            if( rc == 0 ) {
              free( nfspell );
              continue;
            }
            data = nfspell;
          }
          break;
        case ftSPELLFOCUS:
          rc = selectAppropriateSchoolOfMagic( npc );
          if( rc == 0 ) {
            continue;
          }
          nfsch = (NPCFEATSCHOOL*)malloc( sizeof( NPCFEATSCHOOL ) );
          nfsch->school = rc;
          data = nfsch;
          break;
        case ftWEAPONPROFICIENCY_EXOTIC:
        case ftWEAPONPROFICIENCY_MARTIAL:
        case ftWEAPONPROFICIENCY_SIMPLE:
          rc = selectWeaponForProficiency( npc, feat, getPreferredWeapons( classType ) );
          if( rc == 0 ) {
            continue;
          }
          nfw = (NPCFEATWEAPON*)malloc( sizeof( NPCFEATWEAPON ) );
          nfw->weapon = rc;
          data = nfw;
          break;
        case ftIMPROVEDCRITICAL:
        case ftWEAPONFINESSE:
        case ftWEAPONFOCUS:
          if( feat == ftWEAPONFINESSE ) {
            if( npc->dexterity <= npc->strength ) {
              feat = ftWEAPONFOCUS;
            } else if( npc->dexterity < 12 ) {
              continue;
            }              
          }
          
          /* weapon finesse cannot be used with ranged weapons, so we need to
           * be sure we don't accidentally select one. */

          if( feat == ftWEAPONFINESSE ) {
            rc = wtMELEE | wtLIGHT;
          } else {
            rc = wtMELEE | wtRANGED;
          }

          rc = selectProficientWeapon( npc, feat, getPreferredWeapons( classType ), rc );
          if( rc == 0 ) {
            continue;
          }
          nfw = (NPCFEATWEAPON*)malloc( sizeof( NPCFEATWEAPON ) );
          nfw->weapon = rc;
          data = nfw;
          break;
        case ftWEAPONSPECIALIZATION:
          rc = selectWeaponForSpecialization( npc );
          if( rc == 0 ) {
            continue;
          }
          nfw = (NPCFEATWEAPON*)malloc( sizeof( NPCFEATWEAPON ) );
          nfw->weapon = rc;
          data = nfw;
          break;
        case ftTOUGHNESS:
          npc->hp += 3;
          break;
        case ftSKILLFOCUS:
          rc = selectExistingSkill( feat, classType, npc->skills, npc->feats );
          if( rc == 0 ) {
            continue;
          }
          if( dndGetSkillAbility( rc ) == abNONE ) {
            continue;
          }
          nfs = (NPCFEATSKILL*)malloc( sizeof( NPCFEATSKILL ) );
          nfs->skill = rc;
          data = nfs;
          break;
      }

      addFeat( npc, feat, 0, data );
      if( dndFeatIsReusable( feat ) ) {
        total += addToWeightedList( &wlist, feat, 1 );
      }
    } else {
      i--;
    }
    if( wlist == 0 ) {
      break;
    }
  }

  if( i < count ) {
    printf( "[BUG] %d feats are left over (unused)!\n", ( count - i ) );
  }

  destroyWeightedList( &wlist );
}


void computeFeats( NPC* npc, int classType, int tlevel, int clevel ) {
  int i;
  int featCount;
  int bonusFeatCount;
  PREFERREDFEATS* feats;
  PREFERREDFEATS* bonusFeats;

  /* many classes have level dependent abilities that either mimic or
   * actually are feats.  we'll insert those automatically here. */

  switch( classType ) {
    case pcBARBARIAN:
      break;
    case pcBARD:
      break;      
    case pcCLERIC:
      break;
    case pcDRUID:
      break;
    case pcFIGHTER:
      break;
    case pcMONK:
      switch( clevel ) {
        case 1:
          addFeatIfNotExist( npc, ftIMPROVEDUNARMEDSTRIKE, 1, 0 );
          addFeatIfNotExist( npc, ftSTUNNINGFIST, 1, 0 );
          break;
        case 2:
          addFeatIfNotExist( npc, ftDEFLECTARROWS, 1, 0 );
          break;
        case 6:
          addFeatIfNotExist( npc, ftIMPROVEDTRIP, 1, 0 );
          break;
      }
      break;
    case pcPALADIN:
      break;
    case pcRANGER:
      switch( clevel ) {
        case 1:
          addFeatIfNotExist( npc, ftTRACK, 1, 0 );
          break;
      }
      break;
    case pcROGUE:
      break;
    case pcSORCERER:
      break;
    case pcWIZARD:
      switch( clevel ) {
        case 1:
          addFeatIfNotExist( npc, ftSCRIBESCROLL, 1, 0 );
          break;
      }
      break;
  }

  /* first, we count how many feats the character is entitled to at the
   * current level. */

  featCount = bonusFeatCount = 0;

  /* at first level, all classes get at least one feat.  Some classes 
   * (such as fighters) also gain a bonus feat at first level.  Humans
   * get an additional feat at first level. */

  if( tlevel == 1 ) {
    featCount++;
    if( npc->race == rcHUMAN ) {
      featCount++;
    }
  }

  if( ( clevel == 1 ) && dndClassHasBonusFeatAtFirstLevel( classType ) ) {
    bonusFeatCount++;
  }

  /* every three character levels the character gets another feat */

  if( tlevel % 3 == 0 ) {
    featCount++;
  }

  /* some classes (like fighters and wizards) get bonus feats every so
   * many levels */

  i = dndClassBonusFeatEveryXLevels( classType );
  if( i != 0 ) {
    if( clevel % i == 0 ) {
      bonusFeatCount++;
    }
  }

  if( featCount + bonusFeatCount < 1 ) {
    return;
  }

  feats = bonusFeats = 0;

  for( i = 0; classes[ i ].type != 0; i++ ) {
    if( classes[ i ].type == classType ) {
      feats = classes[ i ].feats;
      bonusFeats = classes[ i ].bonusFeats;
      break;      
    }
  }

  if( featCount > 0 ) {
    if( feats != 0 ) {
      addFeatsFromList( npc, feats, featCount, classType, clevel );
    }
  }

  if( bonusFeatCount > 0 ) {
    if( bonusFeats != 0 ) {
      addFeatsFromList( npc, bonusFeats, bonusFeatCount, classType, clevel );
    }
  }
}


NPCSPELL** getSpellListForClass( NPC* npc, int classType ) {
  NPCCLASS* c;

  for( c = npc->classes; c != 0; c = c->next ) {
    if( c->type == classType ) {
      switch( c->type ) {
        case pcWIZARD:
          return ((NPCWIZARDDATA*)c->data)->spells;
        case pcSORCERER:
          return ((NPCSORCERERDATA*)c->data)->spells;
        case pcBARD:
          return ((NPCBARDDATA*)c->data)->spells;
      }
    }
  }

  return 0;
}

int npcHasSpell( NPCSPELL** list, int spell, int level ) {
  NPCSPELL* c;

  for( c = list[ level ]; c != 0; c = c->next ) {
    if( c->spell == spell ) {
      return 1;
    }
  }

  return 0;
}


void addAllSpellsOfLevelXToRepertoire( NPCCLASS* cls, int classType, int level ) {
  char* next;
  int   spell;

  spell = dndGetSpellOfLevel( classType, level, &next );
  while( spell != 0 ) {
    addSpellToRepertoire( cls, spell, level );
    spell = dndGetSpellOfLevel( 0, level, &next );
  }
}


/* puts two new spells into the caster's repertoire */

void computeSpells( NPC* npc, int classType, int tlevel, int clevel ) {
  int relevantAbility;
  int slevel;
  int count;
  int knowable;
  int currentCount;
  int slotsAvail[10];
  NPCSPELL** list;
  NPCSPELL*  s;
  NPCCLASS*  c;
  WEIGHTEDLIST* wlist;
  int total;
  int spellsLeft;
  char* next;
  int spell;
  int bonusSpells;

  /* only wizards, sorcerers, and bards need to worry about repertoires */

  if( classType != pcWIZARD && classType != pcSORCERER && classType != pcBARD ) {
    return;
  }

  switch( classType ) {
    case pcWIZARD:
      relevantAbility = npc->intelligence;
      break;
    case pcSORCERER:
    case pcBARD:
      relevantAbility = npc->charisma;
      break;
  }

  /* get the spellcaster's current repertoire of spells */

  list = getSpellListForClass( npc, classType );
  c = npcGetClass( npc, classType );

  if( ( classType == pcSORCERER ) || ( classType == pcBARD ) ) {
    /* determine how many "slots" are available in the caster's repertoire,
     * so that we can fill two of them */
       
    memset( slotsAvail, 0, sizeof( slotsAvail ) );
    slevel = 0;
    do {
      count = dndGetSpellsPerDay( classType, clevel, slevel );
      if( count == -1 ) {
        break;
      }

      count += dndGetBonusSpellsPerDay( relevantAbility, slevel );
      if( count < 1 ) {
        break;
      }

      knowable = dndGetSpellsKnown( classType, clevel, slevel );
      if( knowable < 0 ) {
        break;
      }

      currentCount = 0;
      for( s = list[ slevel ]; s != 0; s = s->next ) {
        currentCount++;
      }

      if( currentCount < knowable ) {
        slotsAvail[ slevel ] = knowable - currentCount;
      }

      slevel++;

      if( slevel > 10 + relevantAbility ) {
        break;
      }
    } while( slevel < 10 );

    /* build the weighted list of spells to choose from */

    for( slevel = 0; slevel < 10; slevel++ ) {
      if( slotsAvail[ slevel ] > 0 ) {
        wlist = 0;
        total = 0;
        spell = dndGetSpellOfLevel( classType, slevel, &next );
        while( spell != 0 ) {
          if( !npcHasSpell( list, spell, slevel ) ) {
            total += addToWeightedList( &wlist, spell, getWeightingForSpell( classType, spell ) );
          }
          spell = dndGetSpellOfLevel( 0, slevel, &next );
        }
        if( wlist == 0 ) {
          continue;
        }
        while( slotsAvail[ slevel ] > 0 ) {
          spell = getWeightedItem( &wlist, rollDice( 1, total ), &total );
          addSpellToRepertoire( c, spell, slevel ); 
          slotsAvail[ slevel ]--;
        }
        destroyWeightedList( &wlist );
      }
    }
  } else { /* if classType == pcWIZARD */
    const int odds[30] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
                           1, 1, 1, 2, 2, 2, 2, 2, 3, 3,
                           3, 3, 3, 3, 4, 4, 4, 4, 5, 5 };
    if( clevel == 1 ) {
      addAllSpellsOfLevelXToRepertoire( c, classType, 0 );
      spellsLeft = 4;
      bonusSpells = 0;
    } else {
      spellsLeft = 2;
      bonusSpells = odds[ clevel + ( rand() % 11 ) - 1 ];
    }

    knowable = relevantAbility - 10;
    if( knowable > 9 ) {
      knowable = 9;
    }

    if( knowable < 1 ) {
      return;
    }

    while( spellsLeft > 0 ) {
      for( slevel = knowable; slevel > 0; slevel-- ) {
        count = dndGetSpellsPerDay( classType, clevel, slevel );
        if( count < 0 ) {
          continue;
        }
        count += dndGetBonusSpellsPerDay( relevantAbility, slevel );
        if( count < 1 ) {
          continue;
        }

        wlist = 0;
        total = 0;
        spell = dndGetSpellOfLevel( classType, slevel, &next );
        while( spell != 0 ) {
          if( !npcHasSpell( list, spell, slevel ) ) {
            total += addToWeightedList( &wlist, spell, getWeightingForSpell( classType, spell ) );
          }
          spell = dndGetSpellOfLevel( 0, slevel, &next );
        }
        if( wlist == 0 ) {
          continue;
        }
        while( spellsLeft > 0 ) {
          spell = getWeightedItem( &wlist, rollDice( 1, total ), &total );
          addSpellToRepertoire( c, spell, slevel ); 
          spellsLeft--;
          if( wlist == 0 ) {
            break;
          }
          if( ( slevel > 1 ) && ( rollDice( 1, 100 ) <= 10 ) ) {
            break;
          }
        }
        destroyWeightedList( &wlist );
      }
    }
    while( bonusSpells > 0 ) {
      for( slevel = rollDice( 1, knowable ); slevel <= knowable; slevel++ ) {
        count = dndGetSpellsPerDay( classType, clevel, slevel );
        if( count < 0 ) {
          continue;
        }
        count += dndGetBonusSpellsPerDay( relevantAbility, slevel );
        if( count < 1 ) {
          continue;
        }

        wlist = 0;
        total = 0;
        spell = dndGetSpellOfLevel( classType, slevel, &next );
        while( spell != 0 ) {
          if( !npcHasSpell( list, spell, slevel ) ) {
            total += addToWeightedList( &wlist, spell, getWeightingForSpell( classType, spell ) );
          }
          spell = dndGetSpellOfLevel( 0, slevel, &next );
        }
        if( wlist == 0 ) {
          continue;
        }
        while( bonusSpells > 0 ) {
          spell = getWeightedItem( &wlist, rollDice( 1, total ), &total );
          addSpellToRepertoire( c, spell, slevel ); 
          bonusSpells--;
          if( wlist == 0 ) {
            break;
          }
          if( rollDice( 1, 100 ) <= 50 ) {
            break;
          }
        }
        destroyWeightedList( &wlist );
      }
    }
  }
}


void computeUpdatedAbilities( NPC* npc, int classType, int tlevel, int clevel ) {
  int odds[10] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 2 };
  int i;

  if( tlevel % 4 == 0 ) {
    i = lookupAbility( classType, odds[ rand() % 10 ] );
    switch( i ) {
      case abSTRENGTH: npc->strength++; break;
      case abDEXTERITY: npc->dexterity++; break;
      case abCONSTITUTION: npc->constitution++; break;
      case abINTELLIGENCE: npc->intelligence++; break;
      case abWISDOM: npc->wisdom++; break;
      case abCHARISMA: npc->charisma++; break;
    }
  }

  computeSkills( npc, classType, tlevel, clevel );
  computeFeats( npc, classType, tlevel, clevel );
  computeSpells( npc, classType, tlevel, clevel );
}


static void generateNPCHeightWeight( NPC* npc ) {
  DNDRACIALHEIGHTWEIGHT data;
  int rc;
  int hmod;
  int wmod;

  rc = dndGetRacialHeightWeight( npc->race, npc->gender, &data );
  if( !rc ) {
    return;
  }

  npc->height_ft = data.baseHeight_Feet;
  npc->height_in = data.baseHeight_Inches;
  npc->weight = data.baseWeight;

  hmod = rollDice( data.heightModDieCount, data.heightModDie );
  for( rc = 0; rc < hmod; rc++ ) {
    if( npc->height_in == 11 ) {
      npc->height_ft++;
      npc->height_in = 0;
    } else {
      npc->height_in++;
    }
  }

  wmod = rollDice( data.weightModDieCount, data.weightModDie ) * hmod;
  npc->weight += wmod;
}


static int stepsRemoved( int alignment, int from ) {
  const int lc = alLAWFUL | alLCNEUTRAL | alCHAOTIC;
  const int ge = alGOOD | alGENEUTRAL | alEVIL;
  int lc_align;
  int ge_align;
  int lc_alignfrom;
  int ge_alignfrom;
  int steps;

  steps = 0;

  if( ( alignment == 0 ) || ( from == 0 ) ) {
    return steps;
  }

  lc_align = alignment & lc;
  lc_alignfrom = from & lc;

  ge_align = alignment & ge;
  ge_alignfrom = from & ge;

  if( ( lc_align != 0 ) && ( lc_alignfrom != 0 ) ) {
    switch( lc_align ) {
      case alLAWFUL:
        switch( lc_alignfrom ) {
          case alLAWFUL: break;
          case alLCNEUTRAL: steps += 1; break;
          case alCHAOTIC: steps += 2; break;            
        }
        break;
      case alLCNEUTRAL:
        switch( lc_alignfrom ) {
          case alLAWFUL: steps += 1; break;
          case alLCNEUTRAL: break;
          case alCHAOTIC: steps += 1; break;            
        }
        break;
      case alCHAOTIC:
        switch( lc_alignfrom ) {
          case alLAWFUL: steps += 2; break;
          case alLCNEUTRAL: steps += 1; break;
          case alCHAOTIC: break;            
        }
        break;
    }
  }

  if( ( ge_align != 0 ) && ( ge_alignfrom != 0 ) ) {
    switch( ge_align ) {
      case alGOOD:
        switch( ge_alignfrom ) {
          case alGOOD: break;
          case alGENEUTRAL: steps += 2; break;
          case alEVIL: steps += 4; break;            
        }
        break;
      case alGENEUTRAL:
        switch( ge_alignfrom ) {
          case alGOOD: steps += 2; break;
          case alGENEUTRAL: break;
          case alEVIL: steps += 2; break;            
        }
        break;
      case alEVIL:
        switch( ge_alignfrom ) {
          case alGOOD: steps += 4; break;
          case alGENEUTRAL: steps += 2; break;
          case alEVIL: break;            
        }
        break;
    }
  }

  return steps;
}


static void freeClassData( NPCCLASS* c ) {
  NPCSPELL** list;
  NPCSPELL*  i;
  NPCSPELL*  n;
  int        levels;
  int        j;

  list = 0;

  switch( c->type ) {
    case pcBARD:
      list = ((NPCBARDDATA*)(c->data))->spells;
      levels = 7;
      break;
    case pcSORCERER:
      list = ((NPCSORCERERDATA*)(c->data))->spells;
      levels = 10;
      break;
    case pcWIZARD:
      list = ((NPCWIZARDDATA*)(c->data))->spells;
      levels = 10;
      break;
  }

  if( list != 0 ) {
    for( j = 0; j < levels; j++ ) {
      i = list[ j ];
      while( i != 0 ) {
        n = i->next;
        free( i );
        i = n;
      }
    }
  }
}


void npcDestroyNPC( NPC* npc ) {
  NPCCLASS* cli;
  NPCFEAT*  fti;
  NPCSKILL* ski;
  NPCCLASS* cln;
  NPCFEAT*  ftn;
  NPCSKILL* skn;

  free( npc->name );

  cli = npc->classes;
  while( cli != 0 ) {
    cln = cli->next;
    freeClassData( cli );
    free( cli );
    cli = cln;
  }

  fti = npc->feats;
  while( fti != 0 ) {
    ftn = fti->next;
    if( fti->data != 0 ) {
      free( fti->data );
    }
    free( fti );
    fti = ftn;
  }

  ski = npc->skills;
  while( ski != 0 ) {
    skn = ski->next;
    if( ski->data != 0 ) {
      free( ski->data );
    }
    free( ski );
    ski = skn;
  }

  free( npc );
}


int npcGetBaseClassAttack( NPC* npc ) {
  NPCCLASS* cls;
  int       total;

  total = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total += dndGetClassAttackBonus( cls->type, cls->level );
  }

  return total; 
}


int npcGetBaseAttack( NPC* npc ) {
  int       total;

  total  = npcGetBaseClassAttack( npc );  
  total += dndGetRaceBonus( npc->race, npc->gender, rbtATTACK, 0 );

  return total; 
}


int npcHasClass( NPC* npc, int classType ) {
  NPCCLASS* cls;

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    if( cls->type == classType ) {
      return 1;
    }
  }

  return 0;
}


NPCCLASS* npcGetClass( NPC* npc, int classType ) {
  NPCCLASS* c;

  for( c = npc->classes; c != 0; c = c->next ) {
    if( c->type == classType ) {
      return c;
    }
  }

  return 0;
}


int npcGetBaseFortitudeSave( NPC* npc ) {
  NPCCLASS* cls;
  int       total;

  total = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total += dndGetFortitudeSave( cls->type, cls->level );
  }
  
  return total; 
}


int npcGetBaseReflexSave( NPC* npc ) {
  NPCCLASS* cls;
  int       total;

  total = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total += dndGetReflexSave( cls->type, cls->level );
  }
  
  return total; 
}


int npcGetBaseWillSave( NPC* npc ) {
  NPCCLASS* cls;
  int       total;

  total = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total += dndGetWillSave( cls->type, cls->level );
  }
  
  return total; 
}


int npcClassCount( NPC* npc ) {
  NPCCLASS* cls;
  int total;

  total = 0;

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total++;
  }

  return total;
}


int npcGetCharacterLevel( NPC* npc ) {
  NPCCLASS* cls;
  int total;

  total = 0;

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    total += cls->level;
  }

  return total;
}


void npcDestroyComponentBreakdown( NPCCOMPBREAKDOWN* breakdown ) {
  NPCCOMPBREAKDOWN* n;
  NPCCOMPBREAKDOWN* i;

  i = breakdown;
  while( i != 0 ) {
    n = i->next;
    free( i );
    i = n;
  }
}


static NPCCOMPBREAKDOWN* s_newBreakdownItem( int comp, int data1, int data2 ) {
  NPCCOMPBREAKDOWN* bd;

  bd = (NPCCOMPBREAKDOWN*)malloc( sizeof( NPCCOMPBREAKDOWN ) );
  memset( bd, 0, sizeof( *bd ) );

  bd->component = comp;
  bd->data1 = data1;
  bd->data2 = data2;

  return bd;
}


int npcComputeActualAC( NPC* npc, NPCCOMPBREAKDOWN** breakdown ) {
  NPCCOMPBREAKDOWN** b;
  int ac;
  int mod;

  if( breakdown != 0 ) {
    b = breakdown;
    *b = 0;
  }

  ac = 10;
  
  mod = dndGetAbilityBonus( npc->dexterity );
  if( mod != 0 ) {
    ac += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtABILITYSCORE, abDEXTERITY, mod );
      b = &((*b)->next);
    }
  }

  if( npcHasClass( npc, pcMONK ) ) {
    NPCCLASS *cls;

    mod = dndGetAbilityBonus( npc->wisdom );
    if( mod != 0 ) {
      ac += mod;
      if( breakdown != 0 ) {
        *b = s_newBreakdownItem( bdtABILITYSCORE, abWISDOM, mod );
        b = &((*b)->next);
      }
    }

    cls = npcGetClass( npc, pcMONK );
    mod = cls->level / 5;
    if( mod > 0 ) {
      ac += mod;
      if( breakdown != 0 ) {
        *b = s_newBreakdownItem( bdtCLASS, pcMONK, mod );
        b = &((*b)->next);
      }
    }
  }

  mod = dndGetSizeACMod( dndGetRaceSize( npc->race ) );
  if( mod != 0 ) {
    ac += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtSIZE, mod, 0 );
      b = &((*b)->next);
    }
  }

  mod = dndGetRaceBonus( npc->race, npc->gender, rbtARMORCLASS, 0 );
  if( mod != 0 ) {
    ac += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtNATURAL, mod, 0 );
      b = &((*b)->next);
    }
  }

  return ac;
}


int npcHasFeat( NPC* npc, int featType ) {
  NPCFEAT *feat;

  for( feat = npc->feats; feat != 0; feat = feat->next ) {
    if( feat->type == featType ) {
      return 1;
    }
  }

  return 0;
}


int npcComputeActualInitiative( NPC* npc, NPCCOMPBREAKDOWN** breakdown ) {
  NPCCOMPBREAKDOWN** b;
  int init;
  int mod;

  if( breakdown != 0 ) {
    b = breakdown;
    *b = 0;
  }

  init = 0;
  
  mod = dndGetAbilityBonus( npc->dexterity );
  if( mod != 0 ) {
    init += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtABILITYSCORE, abDEXTERITY, mod );
      b = &((*b)->next);
    }
  }

  if( npcHasFeat( npc, ftIMPROVEDINITIATIVE ) ) {
    init += 4;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtFEAT, ftIMPROVEDINITIATIVE, 4 );
      b = &((*b)->next);
    }
  }

  return init;
}


char* npcBuildComponentBreakdownDescription( NPCCOMPBREAKDOWN* breakdown, 
                                             char* buffer, 
                                             int len )
{
  char temp1[ 100 ];
  char temp2[ 100 ];
  NPCCOMPBREAKDOWN* i;
  int parens;
  int pos;

  *buffer = 0;
  pos = 0;
  parens = 0;

  for( i = breakdown; i != 0; i = i->next ) {
    if( pos + 2 >= len ) {
      break;
    }

    if( !parens ) {
      strcat( buffer, "(" );
      pos++;
      parens = 1;
    } else {
      strcat( buffer, ", " );
      pos += 2;
    }

    switch( i->component ) {
      case bdtABILITYSCORE:
        strcpy( temp1, dndGetAbilityName( i->data1 ) );
        temp1[0] = toupper( temp1[0] );
        temp1[3] = 0;
        sprintf( temp2, "%+d %s", i->data2, temp1 );
        strncpy( &(buffer[pos]), temp2, len - pos );
        pos += strlen( temp2 );
        break;
      case bdtBASE:
        sprintf( temp1, "%+d Base", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtSIZE:
        sprintf( temp1, "%+d Size", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtRANK:
        sprintf( temp1, "%+d Rank", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtHALFRANK:
        sprintf( temp1, "%+g Rank", i->data1 / 2.0 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtFOCUS:
        sprintf( temp1, "%+d Focus", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtRACIAL:
        sprintf( temp1, "%+d Racial", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtNATURAL:
        sprintf( temp1, "%+d Natural", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtARMOR:
        sprintf( temp1, "%+d Armor", i->data1 );
        strncpy( &(buffer[pos]), temp1, len - pos );
        pos += strlen( temp1 );
        break;
      case bdtFEAT:
        strcpy( temp1, dndGetFeatName( i->data1 ) );
        temp1[0] = toupper( temp1[0] );
        sprintf( temp2, "%+d %s", 
                 i->data2,
                 temp1 );
        strncpy( &(buffer[pos]), temp2, len - pos );
        pos += strlen( temp2 );
        break;
      case bdtCLASS:
        strcpy( temp1, dndGetClassAbbr( i->data1 ) );
        temp1[0] = toupper( temp1[0] );
        sprintf( temp2, "%+d %s", 
                 i->data2,
                 temp1 );
        strncpy( &(buffer[pos]), temp2, len - pos );
        pos += strlen( temp2 );
        break;
    }
  }

  if( ( pos < len - 1 ) && ( parens ) ) {
    strcat( buffer, ")" );
  }    

  return buffer;
}


int npcComputeActualSave( NPC* npc, int save, NPCCOMPBREAKDOWN** breakdown ) {
  NPCCOMPBREAKDOWN** b;
  int base;
  int mod;
  int ability;

  if( breakdown != 0 ) {
    b = breakdown;
    *b = 0;
  }

  base = 0;
  switch( save ) {
    case svFORTITUDE:
      mod = npcGetBaseFortitudeSave( npc );
      break;
    case svREFLEX:
      mod = npcGetBaseReflexSave( npc );
      break;
    case svWILL:
      mod = npcGetBaseWillSave( npc );
      break;
  }
  
  base += mod;
  if( breakdown != 0 ) {
    *b = s_newBreakdownItem( bdtBASE, mod, 0 );
    b = &((*b)->next);
  }

  switch( save ) {
    case svFORTITUDE:
      ability = abCONSTITUTION;
      mod = dndGetAbilityBonus( npc->constitution );
      break;
    case svREFLEX:
      ability = abDEXTERITY;
      mod = dndGetAbilityBonus( npc->dexterity );
      break;
    case svWILL:
      ability = abWISDOM;
      mod = dndGetAbilityBonus( npc->wisdom );
      break;
  }

  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtABILITYSCORE, ability, mod );
      b = &((*b)->next);
    }
  }

  if( npcHasClass( npc, pcPALADIN ) ) {
    mod = dndGetAbilityBonus( npc->charisma );
    if( mod > 0 ) {
      base += mod;
      if( breakdown != 0 ) {
        *b = s_newBreakdownItem( bdtABILITYSCORE, abCHARISMA, mod );
        b = &((*b)->next);
      }
    }
  }
    
  switch( save ) {
    case svFORTITUDE:
      ability = ftGREATFORTITUDE;
      break;
    case svREFLEX:
      ability = ftLIGHTNINGREFLEXES;
      break;
    case svWILL:
      ability = ftIRONWILL;
      break;
  }

  if( npcHasFeat( npc, ability ) ) {
    base += 2;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtFEAT, ability, 2 );
      b = &((*b)->next);
    }
  }

  mod = dndGetRaceBonus( npc->race, npc->gender, rbtSAVINGTHROW, save );
  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtRACIAL, mod, 0 );
      b = &((*b)->next);
    }
  }

  return base;
}


int npcHasSkill( NPC* npc, int skillType ) {
  NPCSKILL *skill;

  for( skill = npc->skills; skill != 0; skill = skill->next ) {
    if( skill->type == skillType ) {
      return 1;
    }
  }

  return 0;
}


int npcComputeActualAttack( NPC* npc, int type, NPCCOMPBREAKDOWN** breakdown ) {
  NPCCOMPBREAKDOWN** b;
  int base;
  int mod;
  int ability;

  if( breakdown != 0 ) {
    b = breakdown;
    *b = 0;
  }

  base = 0;

  mod = npcGetBaseClassAttack( npc );
  base += mod;
  if( breakdown != 0 ) {
    *b = s_newBreakdownItem( bdtBASE, mod, 0 );
    b = &((*b)->next);
  }

  switch( type ) {
    case attMELEE:
      ability = abSTRENGTH;
      mod = dndGetAbilityBonus( npc->strength );
      break;
    case attRANGED:
      ability = abDEXTERITY;
      mod = dndGetAbilityBonus( npc->dexterity );
      break;
  }

  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtABILITYSCORE, ability, mod );
      b = &((*b)->next);
    }
  }

  mod = dndGetSizeACMod( dndGetRaceSize( npc->race ) );
  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtSIZE, mod, 0 );
      b = &((*b)->next);
    }
  }

  mod = dndGetRaceBonus( npc->race, npc->gender, rbtATTACK, 0 );
  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtRACIAL, mod, 0 );
      b = &((*b)->next);
    }
  }
    
  return base;
}


NPCSKILL* npcGetSkill( NPC* npc, int skillType ) {
  NPCSKILL* c;

  for( c = npc->skills; c != 0; c = c->next ) {
    if( c->type == skillType ) {
      return c;
    }
  }

  return 0;
}


float npcComputeActualSkillBonus( NPC* npc, int type, NPCCOMPBREAKDOWN** breakdown ) {
  NPCCOMPBREAKDOWN** b;
  NPCSKILL* skill;
  float base;
  int mod;
  int ability;
  int data;

  if( breakdown != 0 ) {
    b = breakdown;
    *b = 0;
  }

  base = 0;

  skill = npcGetSkill( npc, type );
  if( skill != 0 ) {
    if( skill->rank - (int)skill->rank < 0.5 ) {
      mod = (int)skill->rank;
      data = bdtRANK;
    } else {
      mod = (int)( skill->rank * 2 );
      data = bdtHALFRANK;
    }
    if( mod != 0 ) {
      base += skill->rank;
      if( breakdown != 0 ) {
        *b = s_newBreakdownItem( data, mod, 0 );
        b = &((*b)->next);
      }
    }
  }

  ability = dndGetSkillAbility( type );
  switch( ability ) {
    case abSTRENGTH:
      mod = dndGetAbilityBonus( npc->strength );
      break;
    case abDEXTERITY:
      mod = dndGetAbilityBonus( npc->dexterity );
      break;
    case abCONSTITUTION:
      mod = dndGetAbilityBonus( npc->constitution );
      break;
    case abINTELLIGENCE:
      mod = dndGetAbilityBonus( npc->intelligence );
      break;
    case abWISDOM:
      mod = dndGetAbilityBonus( npc->wisdom );
      break;
    case abCHARISMA:
      mod = dndGetAbilityBonus( npc->charisma );
      break;
    case abNONE:
      mod = 0;
      break;
  }

  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtABILITYSCORE, ability, mod );
      b = &((*b)->next);
    }
  }

  mod = dndGetRaceBonus( npc->race, npc->gender, rbtSKILL, type );
  if( mod != 0 ) {
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtRACIAL, mod, 0 );
      b = &((*b)->next);
    }
  }
    
  if( npcHasSkillFocus( npc, type ) ) {
    mod = 2;
    base += mod;
    if( breakdown != 0 ) {
      *b = s_newBreakdownItem( bdtFOCUS, mod, 0 );
      b = &((*b)->next);
    }
  }

  if( type == skHIDE ) {
    mod = dndGetSizeHideMod( dndGetRaceSize( npc->race ) );
    if( mod != 0 ) {
      base += mod;
      if( breakdown != 0 ) {
        *b = s_newBreakdownItem( bdtSIZE, mod, 0 );
        b = &((*b)->next);
      }
    }
  }

  if( ( type == skSPOT ) || ( type == skLISTEN ) ) {
    if( npcHasFeat( npc, ftALERTNESS ) ) {
      mod = 2;
      base += mod;
      if( breakdown != 0 ) {
        *b = s_newBreakdownItem( bdtFEAT, ftALERTNESS, mod );
        b = &((*b)->next);
      }
    }
  }

  return base;
}


int npcHasSkillFocus( NPC* npc, int skillType ) {
  NPCFEAT* feat;

  for( feat = npc->feats; feat != 0; feat = feat->next ) {
    if( feat->type == ftSKILLFOCUS ) {
      if( ((NPCFEATSKILL*)feat->data)->skill == skillType ) {
        return 1;
      }
    }
  }

  return 0;
}


char* npcBuildHitDiceBreakdown( NPC* npc, char* buffer, int len ) {
  NPCCLASS* cls;
  int       count;
  int       die;
  char      temp[ 100 ];
  int       con;
  int       length;

  *buffer = 0;
  length = 0;

  con = dndGetAbilityBonus( npc->constitution );

  if( dndGetRaceExtraHitDice( npc->race, &count, &die ) ) {
    if( con != 0 ) {
      sprintf( temp, "%dd%d%+d", count, die, count*con );
    } else {
      sprintf( temp, "%dd%d", count, die );
    }
    strncpy( buffer, temp, len - length - strlen( temp ) + 1 );
    length = strlen( buffer );
  }

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    if( length >= len ) {
      break;
    }

    die = dndGetClassHitDie( cls->type );
    if( con != 0 ) {
      sprintf( temp, "%dd%d%+d", cls->level, die, cls->level*con );
    } else {
      sprintf( temp, "%dd%d", cls->level, die );
    }
    if( *buffer != 0 ) {
      strcat( buffer, " + " );
      length += 3;
    }
    strncpy( &(buffer[length]), temp, len - length - strlen( temp ) + 1 );
    length = strlen( buffer );
  }

  return buffer;
}


char* npcBuildStatBlock( NPC* npc, NPCSTATBLOCKOPTS* opts, char* buffer, int len ) {
  char* temp;
  char  temp2[100];
  char  temp3[100];
  char  temp4[100];
  NPCCLASS* cls;
  int pos;
  int count;
  int n;
  int i;
  int j;
  NPCCOMPBREAKDOWN* breakdown;
  NPCSKILL* skill;
  NPCFEAT* feat;
  NPCFEATSPELLS* nfsp;
  NPCFEATWEAPON* nfw;
  NPCFEATSKILL* nfs;
  NPCFEATSCHOOL* nfsch;
  NPCSPELL** list;
  NPCSPELL*  spell;
  NPCLANGUAGE* lang;
  int        hasPlus;
  int        relevantAbility;
  int        cr;
  char*      italBegin;
  char*      italEnd;
  char*      boldBegin;
  char*      boldEnd;

  if( opts->richFormatting ) {
    italBegin = "~I";
    italEnd = "~i";
    boldBegin = "~B";
    boldEnd = "~b";
  } else {
    italBegin = italEnd = boldBegin = boldEnd = "";
  }

  /* name, gender race cls#/cls#/cls#:  CR ?; Size ?; HD ?; hp ?; Init ? (why);
   * Spd ?; AC ? (why); Atk ? melee, or ? ranged; SV Fort ?, Ref ?, Will ?;
   * AL ?; Str ?, Dex ?, Con ?, Int ?, Wis ?, Cha ?.
   *
   * Skills and feats: skill ?, skill ?; feat, feat.
   *
   * Possessions: possession, possession.
   */

  temp = (char*)malloc( 32 * 1024 );

  sprintf( temp, "%s%s, %s %s ", 
           boldBegin,
           npc->name,
           ( npc->gender == gMALE ? "male" : "female" ),
           dndGetRaceName( npc->race ) );
  pos = strlen( temp );

  count = 0;
  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    if( count > 0 ) {
      strcat( temp, "/" );
      pos++;
    }

    strcpy( temp2, dndGetClassAbbr( cls->type ) );
    temp2[0] = toupper( temp2[0] );

    sprintf( &(temp[pos]), "%s%d",
             temp2,
             cls->level );

    pos = strlen( temp );
    count++;
  }

  strcat( temp, ":" );
  strcat( temp, boldEnd );
  strcat( temp, "  " );
  pos = strlen( temp );

  cr = npcComputeCR( npc );
  sprintf( &(temp[pos]), "CR %d; ", cr );
  pos = strlen( temp );

  strcpy( temp2, dndGetSizeName( dndGetRaceSize( npc->race ) ) );
  temp2[0] = toupper( temp2[0] );
  temp2[1] = 0;

  sprintf( &(temp[pos]), "Size %s (%d ft., %d in. tall); ", temp2, npc->height_ft, npc->height_in );
  pos = strlen( temp );

  npcBuildHitDiceBreakdown( npc, temp2, sizeof( temp2 ) );
  sprintf( &(temp[pos]), "HD %s; hp %d; ", temp2, npc->hp );
  pos = strlen( temp );

  temp2[0] = 0;
  n = npcComputeActualInitiative( npc, &breakdown );
  if( opts->initBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
  }
  npcDestroyComponentBreakdown( breakdown );

  if( temp2[0] != 0 ) {
    sprintf( &(temp[pos]), "Init %+d %s; ", n, temp2 );
  } else {
    sprintf( &(temp[pos]), "Init %+d; ", n );
  }
  pos = strlen( temp );

  cls = npcGetClass( npc, pcMONK );
  if( cls != 0 ) {
    n = dndGetMonkSpeedForRace( npc->race, cls->level );
  } else {
    n = dndGetRaceSpeed( npc->race );
    if( npcHasClass( npc, pcBARBARIAN ) ) {
      n += 10;
    }
  }

  sprintf( &(temp[pos]), "Spd %d ft.; ", n );
  pos = strlen( temp );

  temp2[0] = 0;
  n = npcComputeActualAC( npc, &breakdown );
  if( opts->acBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
  }
  npcDestroyComponentBreakdown( breakdown );

  if( temp2[0] != 0 ) {
    sprintf( &(temp[pos]), "AC %d %s; ", n, temp2 );
  } else {
    sprintf( &(temp[pos]), "AC %d; ", n );
  }
  pos = strlen( temp );

  n = npcGetBaseAttack( npc );
  count = dndGetClassAttacksPerRound( n );

  n = npcComputeActualAttack( npc, attMELEE, &breakdown );
  strcat( temp, "Attack " );
  pos = strlen( temp );
  for( i = 1; i <= count; i++ ) {
    if( i > 1 ) {
      strcat( temp, "/" );
      pos++;
    }
    j = dndGetClassMultipleAttackBonus( n, i );
    sprintf( &(temp[pos]), "%+d", j );
    pos = strlen( temp );
  }
  if( opts->attackBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
    if( temp2[0] != 0 ) {
      strcat( temp, " " );
      strcat( temp, temp2 );
    }
  }
  npcDestroyComponentBreakdown( breakdown );
  strcat( temp, " melee, or " );
  pos = strlen( temp );

  if( npcHasClass( npc, pcMONK ) ) {
    int monkAttackCount;
    int monkBaseAttack;
    NPCCLASS* cls;

    cls = npcGetClass( npc, pcMONK );

    monkBaseAttack = dndGetClassAttackBonus( pcMONK, cls->level ) +
                     dndGetRaceBonus( npc->race, npc->gender, rbtATTACK, 0 );
    monkAttackCount = dndGetMonkAttacksPerRound( monkBaseAttack );
    n = npcComputeActualAttack( npc, attMELEE, &breakdown ); 

    for( i = 1; i <= monkAttackCount; i++ ) {
      if( i > 1 ) {
        strcat( temp, "/" );
        pos++;
      }
      j = dndGetMonkMultipleAttackBonus( n, i );
      sprintf( &(temp[pos]), "%+d", j );
      pos = strlen( temp );
    }
    if( opts->attackBreakdown ) {
      npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
      if( temp2[0] != 0 ) {
        strcat( temp, " " );
        strcat( temp, temp2 );
      }
    }
    npcDestroyComponentBreakdown( breakdown );
    strcat( temp, " monk, or " );
    pos = strlen( temp );
  }

  n = npcComputeActualAttack( npc, attRANGED, &breakdown );
  for( i = 1; i <= count; i++ ) {
    if( i > 1 ) {
      strcat( temp, "/" );
      pos++;
    }
    j = dndGetClassMultipleAttackBonus( n, i );
    sprintf( &(temp[pos]), "%+d", j );
    pos = strlen( temp );
  }
  if( opts->attackBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
    if( temp2[0] != 0 ) {
      strcat( temp, " " );
      strcat( temp, temp2 );
    }
  }
  npcDestroyComponentBreakdown( breakdown );
  strcat( temp, " ranged; " );
  pos = strlen( temp );

  temp2[0] = temp3[0] = temp4[0] = 0;
  
  n = npcComputeActualSave( npc, svFORTITUDE, &breakdown );
  if( opts->saveBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
    if( temp2[0] != 0 ) {
      memmove( &(temp2[1]), temp2, strlen( temp2 ) + 1 );
      temp2[0] = ' ';
    }
  }
  npcDestroyComponentBreakdown( breakdown );

  i = npcComputeActualSave( npc, svREFLEX, &breakdown );
  if( opts->saveBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp3, sizeof( temp3 ) );
    if( temp3[0] != 0 ) {
      memmove( &(temp3[1]), temp3, strlen( temp3 ) + 1 );
      temp3[0] = ' ';
    }
  }
  npcDestroyComponentBreakdown( breakdown );

  j = npcComputeActualSave( npc, svWILL, &breakdown );
  if( opts->saveBreakdown ) {
    npcBuildComponentBreakdownDescription( breakdown, temp4, sizeof( temp4 ) );
    if( temp4[0] != 0 ) {
      memmove( &(temp4[1]), temp4, strlen( temp4 ) + 1 );
      temp4[0] = ' ';
    }
  }
  npcDestroyComponentBreakdown( breakdown );

  sprintf( &(temp[pos]), "SV Fort %+d%s, Ref %+d%s, Will %+d%s; ", 
           n, temp2, i, temp3, j, temp4 );
  pos = strlen( temp );

  sprintf( &(temp[pos]), "AL %s; ", dndGetAlignmentAbbr( npc->alignment ) );
  pos = strlen( temp );

  if( opts->abilityBonuses ) {
    sprintf( &(temp[pos]), "Str %d (%+d), Dex %d (%+d), Con %d (%+d), Int %d (%+d), Wis %d (%+d), Cha %d (%+d).",
             npc->strength, dndGetAbilityBonus( npc->strength ),
             npc->dexterity, dndGetAbilityBonus( npc->dexterity ),
             npc->constitution, dndGetAbilityBonus( npc->constitution ),
             npc->intelligence, dndGetAbilityBonus( npc->intelligence ),
             npc->wisdom, dndGetAbilityBonus( npc->wisdom ),
             npc->charisma, dndGetAbilityBonus( npc->charisma ) );
  } else {
    sprintf( &(temp[pos]), "Str %d, Dex %d, Con %d, Int %d, Wis %d, Cha %d.",
             npc->strength,
             npc->dexterity,
             npc->constitution,
             npc->intelligence,
             npc->wisdom,
             npc->charisma );
  }

  if( opts->languages ) {
    strcat( temp, "\n\n" );
    strcat( temp, italBegin );
    strcat( temp, "Languages Spoken:" );
    strcat( temp, italEnd );
    strcat( temp, "  " );
    pos = strlen( temp );
    
    n = 0;
    for( lang = npc->languages; lang != 0; lang = lang->next ) {
      if( n > 0 ) {
        strcat( temp, ", " );
      }
      strcpy( temp2, dndGetLanguageName( lang->language ) );
      temp2[ 0 ] = toupper( temp2[ 0 ] );
      strcat( temp, temp2 );
      n++;
    }
    strcat( temp, "." );
    pos = strlen( temp );
  }

  if( opts->skillsAndFeats || opts->skill_points ) {
    strcat( temp, "\n" );
  }

  if( opts->skill_points ) {
    strcat( temp, "\n" );
    strcat( temp, italBegin );
    strcat( temp, "Skill points:" );
    strcat( temp, italEnd );
    strcat( temp, "  " );
    for( cls = npc->classes; cls != 0; cls = cls->next ) {
      strcpy( temp3, dndGetClassAbbr( cls->type ) );
      temp3[0] = toupper( temp3[0] );
      sprintf( temp2, "%s %d", temp3, cls->skill_points );
      strcat( temp, temp2 );
      if( cls->next != NULL ) {
        strcat( temp, ", " );
      }
    }
  }

  if( opts->skillsAndFeats ) {
    strcat( temp, "\n" );
    strcat( temp, italBegin );
    strcat( temp, "Skills and feats:" );
    strcat( temp, italEnd );
    strcat( temp, "  " );
    pos = strlen( temp );

    n = 0;
    for( skill = npc->skills; skill != 0; skill = skill->next ) {
      if( n > 0 ) {
        strcat( temp, ", " );
        pos += 2;
      }
      strcpy( temp2, dndGetSkillName( skill->type ) );
      temp2[0] = toupper( temp2[0] );
      sprintf( &(temp[pos]), "%s %+g",
               temp2,
               npcComputeActualSkillBonus( npc, skill->type, &breakdown ) );
      if( opts->skillBreakdown ) {
        npcBuildComponentBreakdownDescription( breakdown, temp2, sizeof( temp2 ) );
        if( temp2[0] != 0 ) {
          strcat( temp, " " );
          strcat( temp, temp2 );
        }
      }
      npcDestroyComponentBreakdown( breakdown );
      pos = strlen( temp );
      n++;
    }
    strcat( temp, "; " );
    pos = strlen( temp );

    n = 0;
    for( feat = npc->feats; feat != 0; feat = feat->next ) {
      if( n > 0 ) {
        strcat( temp, ", " );
        pos += 2;
      }
      strcpy( buffer, dndGetFeatName( feat->type ) );
      buffer[0] = toupper( buffer[0] );

      if( feat->autoAdd ) {
        strcat( temp, "[" );
        pos++;
      }

      strcat( temp, buffer );
      pos += strlen( buffer );

      switch( feat->type ) {
        case ftSPELLMASTERY:
          nfsp = feat->data;
          strcat( temp, " (" );
          pos += 2;
          for( i = 0; i < 3; i++ ) {
            sprintf( &(temp[pos]), "%s%s", 
                      ( i > 0 ? ", " : "" ),
                      dndGetSpellName( nfsp->spells[ i ] ) );
            pos = strlen( temp );
          }
          strcat( temp, ")" );
          pos++;
          break;
        case ftSPELLFOCUS:
          nfsch = feat->data;
          sprintf( &(temp[pos]), " (%s)", dndGetSchoolOfMagicName( nfsch->school ) );
          pos = strlen( temp );
          break;
        case ftWEAPONPROFICIENCY_SIMPLE:
        case ftWEAPONPROFICIENCY_MARTIAL:
        case ftWEAPONPROFICIENCY_EXOTIC:
        case ftIMPROVEDCRITICAL:
        case ftWEAPONFOCUS:
        case ftWEAPONFINESSE:
        case ftWEAPONSPECIALIZATION:
          nfw = feat->data;
          sprintf( &(temp[pos]), " (%s)", dndGetWeaponName( nfw->weapon ) );
          pos = strlen( temp );
          break;
        case ftSKILLFOCUS:
          nfs = feat->data;
          sprintf( &(temp[pos]), " (%s)", dndGetSkillName( nfs->skill ) );
          pos = strlen( temp );
          break;
      }

      if( feat->autoAdd ) {
        strcat( temp, "]" );
        pos++;
      }
      n++;
    }
    strcat( temp, "." );
    pos = strlen( temp );
  }

  /* FIXME: add posessions here */
  if( opts->possessions ) {
    strcat( temp, "\n\n" );
    strcat( temp, italBegin );
    strcat( temp, "Possessions:" );
    strcat( temp, italEnd );
    strcat( temp, "  " );

    commify( temp2, npcGearValue( npc ) );
    strcat( temp2, " gp " );
    strcat( temp, temp2 );
    strcat( temp, " in gear." );
  }

  /* spells */

  if( opts->spells ) {
    for( cls = npc->classes; cls != 0; cls = cls->next ) {
      list = 0;
      hasPlus = 0;

      switch( cls->type ) {
        case pcBARD:
          list = ((NPCBARDDATA*)(cls->data))->spells;
          relevantAbility = npc->charisma;
          break;
        case pcCLERIC:
          relevantAbility = npc->wisdom;
          hasPlus = 1;
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
          list = ((NPCSORCERERDATA*)(cls->data))->spells;
          relevantAbility = npc->charisma;
          break;
        case pcWIZARD:
          list = ((NPCWIZARDDATA*)(cls->data))->spells;
          relevantAbility = npc->intelligence;
          break;
        case npcADEPT:
          relevantAbility = npc->wisdom;
          break;
        default:
          continue;
      }

      strcat( temp, "\n\n" );
      pos += 2;

      if( cls->type == pcCLERIC ) {
        strcat( temp, italBegin );
        strcat( temp, "Cleric Domains:" );
        strcat( temp, italEnd );
        strcat( temp, "  " );
        for( i = 0; i < 2; i++ ) {
          strcpy( temp2, dndGetDomainName( ((NPCCLERICDATA*)(cls->data))->domain[i] ) );
          temp2[ 0 ] = toupper( temp2[ 0 ] );
          if( i > 0 ) {
            strcat( temp, ", " );
          }
          strcat( temp, temp2 );
        }
        strcat( temp, ".\n" );
        pos = strlen( temp );
      }

      strcpy( temp2, dndGetClassName( cls->type ) );
      temp2[ 0 ] = toupper( temp2[ 0 ] );
      strcat( temp, italBegin );
      strcat( temp, temp2 );
      if( list == 0 ) {
        strcat( temp, " Spells Per Day:" );
        strcat( temp, italEnd );
        strcat( temp, "  " );
      } else {
        strcat( temp, " Spells Known (" );
      }
      pos = strlen( temp );

      n = relevantAbility - 10;
      if( n > 9 ) {
        n = 9;
      }
      if( n < 0 ) {
        n = 0;
      }

      if( ( cls->type == pcRANGER ) || ( cls->type == pcPALADIN ) ) {
        if( cls->level < 4 ) {
          strcat( temp, "None until 4th level" );
          pos = strlen( temp );
        }
      }

      j = 0;
      for( i = 0; i <= n; i++ ) {
        count = dndGetSpellsPerDay( cls->type, cls->level, i );
        if( count < 0 ) {
          continue;
        }

        count += dndGetBonusSpellsPerDay( relevantAbility, i );
        if( count < 1 ) {
          continue;
        }

        if( j ) {
          strcat( temp, "/" );
          pos++;
        }

        j = 1;
        sprintf( &(temp[pos]), "%d%s", count, ( ( hasPlus && ( i > 0 ) ) ? "+1" : "" ) );
        pos = strlen( temp );
      }

      if( list == 0 ) {
        strcat( temp, "." );
      } else {
        strcat( temp, "):" );
        strcat( temp, italEnd );
        strcat( temp, "  " );
      }

      pos = strlen( temp );
      if( list == 0 ) {
        continue;
      }

      for( i = 0; i <= n; i++ ) {
        if( list[ i ] != 0 ) {
          sprintf( &(temp[pos]), "%d%s -- ", i, getOrdinalSuffix( i ) );
          strcat( temp, italBegin );
          pos = strlen( temp );
          for( spell = list[i]; spell != 0; spell = spell->next ) {
            strcpy( temp2, dndGetSpellName( spell->spell ) );
            strcat( temp, temp2 );
            if( spell->next != 0 ) {
              strcat( temp, ", " );
            }
          }
          strcat( temp, "." );
          strcat( temp, italEnd );
          strcat( temp, "  " );
          pos = strlen( temp );
        }
      }
    }
  }

  len = strlen( temp ) + 1;
  strncpy( buffer, temp, len );
  free( temp );

  return buffer;
}


char* npcRandomName( int race, int gender, char* filePath, char* name, int buflen ) {
  char       grammarPath[ 1024 ];
  grGRAMMAR* grammar;

  switch( race ) {
    case rcHALFELF:
      if( rand() % 2 ) {
        race = rcELF_HIGH;
      } else {
        race = rcHUMAN;
      }
      break;
    case rcHALFORC:
      if( rand() % 2 ) {
        race = rcORC;
      } else {
        race = rcHUMAN;
      }
      break;
  }

  strcpy( grammarPath, filePath );

  switch( race ) {
    case rcHUMAN:
      strcat( grammarPath, "human_names.txt" );
      break;
    case rcDWARF_HILL:
    case rcDWARF_MOUNTAIN:
    case rcDWARF_DEEP:
    case rcDWARF_DUERGAR:
    case rcDWARF_DERRO:
    case rcHALFLING_DEEP:
    case rcCC_CHARDUNI:
    case rcCC_DWARF_FORSAKEN:
      strcat( grammarPath, "dwarf_names.txt" );
      break;
    case rcELF_HIGH:
    case rcELF_GRAY:
    case rcELF_WILD:
    case rcELF_WOOD:
    case rcELF_DROW:
    case rcCC_ELF_FORSAKEN:
      strcat( grammarPath, "elf_names.txt" );
      break;
    case rcHALFLING_LIGHTFOOT:
    case rcHALFLING_TALLFELLOW:
      strcat( grammarPath, "halfling_names.txt" );
      break;
    case rcGNOME_ROCK:
    case rcGNOME_FOREST:
    case rcGNOME_SVIRFNEBLIN:
      strcat( grammarPath, "gnome_names.txt" );
      break;
    default:
      strcat( grammarPath, "monstrous_names.txt" );
      break;
  }

  *name = 0;

  grammar = grLoadGrammar( grammarPath );
  if( grammar == NULL ) {
    return NULL;
  }

  if( gender == gMALE ) {
    grammar->startSymbol = "[M]";
  } else if( gender == gFEMALE ) {
    grammar->startSymbol = "[F]";
  }

  grBuildPhrase( grammar, name, buflen );

  grDestroyGrammar( grammar );

  return name;
}


int npcKnowsLanguage( NPC* npc, int language ) {
  NPCLANGUAGE* lang;

  for( lang = npc->languages; lang != 0; lang = lang->next ) {
    if( lang->language == language ) {
      return 1;
    }
  }

  return 0;
}


int classMatchesRequest( int classType, int request ) {
  int i;

  if( request == classANY ) {
    return 1;
  }

  for( i = 0; classes[ i ].type != 0; i++ ) {
    switch( request ) {
      case classANY_PC:
        if( ( classes[ i ].dtop_PC != 0 ) && ( classes[ i ].type == classType ) ) {
          return 1;
        }
        break;
      case classANY_NPC:
        if( ( classes[ i ].dtop_NPC != 0 ) && ( classes[ i ].type == classType ) ) {
          return 1;
        }
        break;
    }
  }

  return 0;
}


int conflictDetected( NPC* npc, NPCGENERATOROPTS* opts, int classType ) {
  /* first, check for alignment conflicts */
  switch( classType ) {
    case pcBARBARIAN:
      if( ( npc->alignment & alLAWFUL ) != 0 ) {
        return 1;
      }
      break;
    case pcBARD:
      if( ( npc->alignment & alLAWFUL ) != 0 ) {
        return 1;
      }
      break;
    case pcDRUID:
      if( ( npc->alignment & ( alLCNEUTRAL | alGENEUTRAL ) ) == 0 ) {
        return 1;
      }
      break;
    case pcMONK:
      if( ( npc->alignment & alLAWFUL ) == 0 ) {
        return 1;
      }
      break;
    case pcPALADIN:
      if( npc->alignment != ( alLAWFUL | alGOOD ) ) {
        return 1;
      }
      break;
  }

  return 0;
}


int enforceAlignment( NPC* npc, NPCGENERATOROPTS* opts ) {
  int i;
  int constrained;
  int doC, doL, doLCN, doG, doE, doGEN;

  constrained = 0;

  doG = ( opts->alignment == alANY_GOOD );
  doE = ( opts->alignment == alANY_EVIL );
  doGEN = ( opts->alignment == alANY_GENEUTRAL );
  doL = ( opts->alignment == alANY_LAWFUL );
  doC = ( opts->alignment == alANY_CHAOTIC );
  doLCN = ( opts->alignment == alANY_LCNEUTRAL );

  if( opts->alignment < 0 ) {
    constrained = 1;
  }

  for( i = 0; i < 3; i++ ) {
    if( opts->classType[ i ] == pcBARD ) {
      if( ( npc->alignment & alLAWFUL ) != 0 ) {
        constrained = 1;

        /* added 4 Jun 2001 to fix problem with wrong alignments being selected */
        if( doL ) continue;

        npc->alignment &= ~alLAWFUL;
        if( rand() % 2 == 0 ) {
          npc->alignment |= alLCNEUTRAL;
        } else {
          npc->alignment |= alCHAOTIC;
        }
        break;
      }
    } else if( opts->classType[ i ] == pcBARBARIAN ) {
      if( ( npc->alignment & alLAWFUL ) != 0 ) {
        constrained = 1;

        /* added 4 Jun 2001 to fix problem with wrong alignments being selected */
        if( doL ) continue;

        npc->alignment &= ~alLAWFUL;
        if( rand() % 2 == 0 ) {
          npc->alignment |= alLCNEUTRAL;
        } else {
          npc->alignment |= alCHAOTIC;
        }
        break;
      }
    } else if( opts->classType[ i ] == pcDRUID ) {
      if( ( npc->alignment & ( alLCNEUTRAL | alGENEUTRAL ) ) == 0 ) {
        constrained = 1;
        /* changed 4 Jun 2001 to fix problem with wrong alignments being selected */
        if( !( doL || doC ) && ( doG || doE || ( rand() % 2 == 0 ) ) ) {
          npc->alignment &= ~( alLAWFUL | alCHAOTIC );
          npc->alignment |= alLCNEUTRAL;
        } else {
          npc->alignment &= ~( alGOOD | alEVIL );
          npc->alignment |= alGENEUTRAL;
        }
        break;
      }
    } else if( opts->classType[ i ] == pcMONK ) {
      if( ( npc->alignment & alLAWFUL ) == 0 ) {
        constrained = 1;

        /* added 4 Jun 2001 to fix problem with wrong alignments being selected */
        if( doC || doLCN ) continue;

        npc->alignment &= ~( alCHAOTIC | alLCNEUTRAL );
        npc->alignment |= alLAWFUL;
        break;
      }
    } else if( opts->classType[ i ] == pcPALADIN ) {
      if( npc->alignment != ( alLAWFUL | alGOOD ) ) {
        constrained = 1;

        /* added 4 Jun 2001 to fix problem with wrong alignments being selected */
        if( doG || doGEN ) {
          npc->alignment = alLAWFUL | ( doG ? alGOOD : alGENEUTRAL );
          continue;
        }

        if( doC || doLCN ) {
          npc->alignment = ( doC ? alCHAOTIC : alLCNEUTRAL ) | alGOOD;
          continue;
        }

        npc->alignment = alLAWFUL | alGOOD;
        break;
      }
    }
  }

  return constrained;
}


void generateRandomClasses( NPC* npc, NPCGENERATOROPTS* opts ) {
  int  i;
  int  d;
  int  j;
  int  v;
  int  k;
  int  found;
  int  preferredClass;

  /* first, check for the odds for the primary class to be the race's
   * preferred class */

  if( opts->classType[ 0 ] <= 0 ) {
    preferredClass = dndGetRacePreferredClass( npc->race );
    if( preferredClass != 0 ) {
      if( classMatchesRequest( preferredClass, opts->classType[0] ) ) {
        if( rollDice( 1, 100 ) <= PREFERRED_CLASS_CHANCE ) {
          opts->classType[ 0 ] = preferredClass;
        }
      }
    }
  }

  /* go through each of the "multiclass" slots in the opts structure.  if
   * any of them have a 'classANY*' request, generate a random class and
   * make sure it does not already exist among the classes generated. */

  for( i = 0; i < 3; i++ ) {
    if( ( opts->classType[ i ] != classANY ) &&
        ( opts->classType[ i ] != classANY_PC ) &&
        ( opts->classType[ i ] != classANY_NPC ) )
    {
      for( k = 0; k < i; k++ ) {
        if( opts->classType[ k ] == opts->classType[ i ] ) {
          opts->classType[ k ] = classNONE;
          if( opts->level[ k ] > 0 ) {
            opts->level[ i ] += opts->level[ k ];
          }
          enforceAlignment( npc, opts );
          break;
        }
      }
      continue;
    }

    do {
      found = 0;
      d = rollDice( 1, 100 );
      for( j = 0; classes[ j ].type != 0; j++ ) {
        switch( opts->classType[ i ] ) {
          case classANY: v = classes[ j ].dtop_All; break;
          case classANY_PC: v = classes[ j ].dtop_PC; break;
          case classANY_NPC: v = classes[ j ].dtop_NPC; break;
        }
        if( d <= v ) {
          for( k = 0; k < i; k++ ) {
            if( opts->classType[ k ] == classes[ j ].type ) {
              found = 1;
              break;
            }
          }
          if( !found ) {
            if( conflictDetected( npc, opts, classes[ j ].type ) ) {
              found = 1;
            } else {
              opts->classType[ i ] = classes[ j ].type;
            }
          }
          break;
        }
      }
    } while( found );
  }
}


void generateAbilityScores( NPC* npc, NPCGENERATOROPTS* opts ) {
  int i;
  int j;
  int rolledScores[6];
  WEIGHTEDLIST* wlist;
  int total;
  int weights[7];
  int weightMods[6] = { 64, 16, 8, 4, 2, 1 };

  if( opts->abilityScoreStrategy == 0 ) {
    npcAbScoreStrategy_Straight3d6( rolledScores, opts->userData );
  } else {
    opts->abilityScoreStrategy( rolledScores, opts->userData );
  }

  /* sort the 6 scores from highest to lowest */

  qsort( rolledScores, 6, sizeof( int ), icomp );

  /* initialize the weights */

  for( i = 0; i < 7; i++ ) {
    weights[ i ] = 1;
  }

  /* for each ability of each requested class, weight that ability by it's
   * importance to that class. */

  for( i = 0; i < 6; i++ ) {
    for( j = 0; j < 3; j++ ) {
      if( opts->classType[ j ] <= 0 ) {
        continue;
      }
      total = weightMods[ i ] - j * 8;
      if( total <= 0 ) {
        total = 1;
      }

      weights[ lookupAbility( opts->classType[ j ], i ) ] += total;
    }
  }

  /* build a weighted list based on the weights calculated above */
  
  wlist = 0;
  total = 0;
  for( i = 1; i < 7; i++ ) { /* abSTRENGTH to abCHARISMA */
    total += addToWeightedList( &wlist, i, weights[ i ] );
  }

  /* now, randomly select the weighted items out and assign them to the
   * indicated ability score */

  i = 0;
  while( wlist != 0 ) {
    j = getWeightedItem( &wlist, rollDice( 1, total ), &total );
    switch( j ) {
      case abSTRENGTH: npc->strength = rolledScores[ i ]; break;
      case abDEXTERITY: npc->dexterity = rolledScores[ i ]; break;
      case abCONSTITUTION: npc->constitution = rolledScores[ i ]; break;
      case abINTELLIGENCE: npc->intelligence = rolledScores[ i ]; break;
      case abWISDOM: npc->wisdom = rolledScores[ i ]; break;
      case abCHARISMA: npc->charisma = rolledScores[ i ]; break;
    }
    i++;
  }

  /* modify the ability scores based on the character's race */

  npc->strength     += dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abSTRENGTH );
  npc->dexterity    += dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abDEXTERITY );
  npc->constitution += dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abCONSTITUTION );
  npc->intelligence += dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abINTELLIGENCE );
  npc->wisdom       += dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abWISDOM );
  npc->charisma     += dndGetRaceBonus( npc->race, npc->gender, rbtABILITYSCORE, abCHARISMA );

  /* intelligence cannot be less than 3 */

  if( npc->intelligence < 3 ) {
    npc->intelligence = 3;
  }

  /* the others should not be less than 1 */

  if( npc->strength < 1 ) {
    npc->strength = 1;
  }
  if( npc->dexterity < 1 ) {
    npc->dexterity = 1;
  }
  if( npc->constitution < 1 ) {
    npc->constitution = 1;
  }
  if( npc->wisdom < 1 ) {
    npc->wisdom = 1;
  }
  if( npc->charisma < 1 ) {
    npc->charisma = 1;
  }
}


int npcGetRandomRace( int raceType ) {
  int i;
  int j;
  int k;

  if( raceType > 0 ) {
    return raceType;
  }

  j = rollDice( 1, 100 );
  for( i = 0; races[ i ].type != 0; i++ ) {
    if( raceType == raceANY ) {
      k = races[ i ].dtop_All;
    } else if( raceType == raceANY_CORE ) {
      k = races[ i ].dtop_Core;
    } else if( raceType == raceANY_DMG ) {
      k = races[ i ].dtop_DMG;
    } else if( raceType == raceANY_MM ) {
      k = races[ i ].dtop_MM;
    } else if( raceType == raceANY_CC ) {
      k = races[ i ].dtop_CC;
    }
    if( j <= k ) {
      return races[ i ].type;
    }
  }

  return 0;
}


int npcGetRandomAlignment( int alType ) {
  if( alType > 0 ) {
    return alType;
  }

  switch( alType ) {
    case alANY:
      return getRandomLCAlignment() | getRandomGEAlignment();
    case alANY_GOOD:
      return getRandomLCAlignment() | alGOOD;
    case alANY_GENEUTRAL:
      return getRandomLCAlignment() | alGENEUTRAL;
    case alANY_EVIL:
      return getRandomLCAlignment() | alEVIL;
    case alANY_LAWFUL:
      return alLAWFUL | getRandomGEAlignment();
    case alANY_LCNEUTRAL:
      return alLCNEUTRAL | getRandomGEAlignment();
    case alANY_CHAOTIC:
      return alCHAOTIC | getRandomGEAlignment();
    case alANY_NONGOOD:
      if( rand() % 2 == 0 ) {
        return getRandomLCAlignment() | alGENEUTRAL;
      } else {
        return getRandomLCAlignment() | alEVIL;
      }
    case alANY_NONEVIL:
      if( rand() % 2 == 0 ) {
        return getRandomLCAlignment() | alGENEUTRAL;
      } else {
        return getRandomLCAlignment() | alGOOD;
      }
    case alANY_NONLAWFUL:
      if( rand() % 2 == 0 ) {
        return alLCNEUTRAL | getRandomGEAlignment();
      } else {
        return alCHAOTIC | getRandomGEAlignment();
      }
    case alANY_NONCHAOTIC:
      if( rand() % 2 == 0 ) {
        return alLCNEUTRAL | getRandomGEAlignment();
      } else {
        return alLAWFUL | getRandomGEAlignment();
      }
  }

  return 0;
}


NPC* npcGenerateNPC( NPCGENERATOROPTS* opts )
{
  NPC* npc;
  int  clevel;
  int  tlevel;
  int i;
  int j;
  int k;
  int hp;
  int constrained;
  int levelsLeft;
  char* next;
  char temp[ 100 ];
  NPCCLASS* cls;
  WEIGHTEDLIST* wlist;

  npc = (NPC*)malloc( sizeof( NPC ) );
  memset( npc, 0, sizeof( *npc ) );

  if( opts->gender == gANY ) {
    if( rand() % 3 < 1 ) {
      npc->gender = gFEMALE;
    } else {
      npc->gender = gMALE;
    }
  } else {
    npc->gender = opts->gender;
  }

  /* if the alignment was not specified, choose one (from the tables in the
   * DMG, chapter 2, pg. 47 */

  constrained = 0;

  if( opts->alignment <= alANY ) {
    do {
      npc->alignment = npcGetRandomAlignment( opts->alignment );

      constrained = enforceAlignment( npc, opts );

      if( constrained ) {
        i = 0;
      } else {
        if( opts->raceType > 0 ) {
          k = dndGetRaceAlignment( opts->raceType );
        } else {
          k = 0;
        }
        i = stepsRemoved( k, npc->alignment );
      }
    } while( rollDice( 1, 4 ) + i > 4 );
  } else {
    npc->alignment = opts->alignment;
  }

  /* if the race was not specified, determine it randomly */

  if( opts->raceType <= 0 ) {
    do {
      npc->race = npcGetRandomRace( opts->raceType );

      /* make sure the alignment is kept within what the race usually is */
      k = dndGetRaceAlignment( npc->race );
      i = stepsRemoved( k, npc->alignment );
    } while( rollDice( 1, 4 ) + i > 4 );
  } else {
    npc->race = opts->raceType;
  }

  /* determine the height and weight of the npc */
  generateNPCHeightWeight( npc );

  /* generate any classes that were requested to be random */
  generateRandomClasses( npc, opts );

  /* generate the ability scores */
  generateAbilityScores( npc, opts );

  /* add all automatic languages that the race is entitled to */
  j = npc->race;
  while( ( i = dndGetGivenLanguages( j, &next ) ) != 0 ) {
    j = 0;
    addLanguage( npc, i );
  }

  k = dndGetAbilityBonus( npc->intelligence );
  if( k > 0 ) {
    /* build a list of bonus languages that the NPC can take */
    wlist = 0;
    clevel = 0;
    j = npc->race;
    while( ( i = dndGetBonusLanguages( j, &next ) ) != 0 ) {
      j = 0;
      if( !npcKnowsLanguage( npc, i ) ) {
        clevel += addToWeightedList( &wlist, i, 1 );
      }
    }

    while( k > 0 ) {
      if( wlist == 0 ) {
        /* add all remaining languages */
        clevel = 0;
        for( i = 0; allLanguages[ i ] != 0; i++ ) {
          if( !npcKnowsLanguage( npc, allLanguages[ i ] ) ) {
            clevel += addToWeightedList( &wlist, allLanguages[ i ], 1 );
          }
        }
      }

      if( wlist == 0 ) {
        break;
      }

      i = getWeightedItem( &wlist, rollDice( 1, clevel ), &clevel );
      addLanguage( npc, i );

      k--;
    }

    destroyWeightedList( &wlist );
  }

  /* if the race grants any feat to beginning level characters, add them to
   * the NPC's feat list */

  k = npc->race;
  while( dndGetRaceBonusOfType( k, npc->gender, rbtFEAT, &i, &j, &next ) ) {
    addFeat( npc, i, 1, 0 );
    k = 0;
  }

  /* determine the character level */

  levelsLeft = 20;
  tlevel = 0;

  for( j = 0; j < 3; j++ ) {
    if( opts->classType[j] == classNONE ) {
      continue;
    }

    switch( opts->level[j] ) {
      case levelANY:
        clevel = selectBetween( 1, 20 );
        break;
      case levelLOW:
        clevel = selectBetween( 1, 5 );
        break;
      case levelMID:
        clevel = selectBetween( 6, 12 );
        break;
      case levelHI:
        clevel = selectBetween( 13, 20 );
        break;
      default:
        clevel = opts->level[j];
    }

    if( clevel > levelsLeft ) {
      clevel = levelsLeft;
    }

    if( opts->classType[j] == pcDRUID ) {
      addLanguage( npc, lnDRUIDIC );
    }

    addClass( npc, opts->classType[j], clevel );
    for( i = 1; i <= clevel; i++ ) {
      computeUpdatedAbilities( npc, opts->classType[j], i+tlevel, i );
    }

    levelsLeft -= clevel;
    tlevel += clevel;

    if( levelsLeft < 1 ) {
      break;
    }
  }

  /* compute hit points */

  i = dndGetAbilityBonus( npc->constitution );
  if( dndGetRaceExtraHitDice( npc->race, &j, &k ) ) {
    while( j > 0 ) {
      hp = rollDice( 1, k ) + i;
      if( hp < 1 ) {
        hp = 1;
      }
      npc->hp += hp;
      j--;
    }
  }

  cls = npc->classes;
  cls->hp = 0;
  clevel = 0;
  while( cls != 0 ) {
    j = dndGetClassHitDie( cls->type );

    for( k = 0; k < cls->level; k++ ) {
      
      /* we have to do this one die at a time (instead of all at once)
       * because if a die roll modified by a constitution penalty is less
       * than 1, it must be modified upwards to 1.  Every level gained
       * brings in at least one hit point! */

      if( ( k == 0 ) && ( clevel < 1 ) ) { /* maximum hp at first level */
        hp = j + i;
      } else {
        hp = rollDice( 1, j ) + i;
      }

      if( hp < 1 ) {
        hp = 1;
      }
      npc->hp += hp;
      cls->hp += hp;
    }

    clevel += cls->level;
    cls = cls->next;
  }

  /* if the race of the npc grants a bonus to any skill, and the npc has
   * not taken any ranks in that skill, add it to their skill list with
   * 0 ranks. */

  i = npc->race;
  while( dndGetRaceBonusOfType( i, npc->gender, rbtSKILL, &j, &k, &next ) ) {
    if( !npcHasSkill( npc, j ) ) {
      addSkill( npc, j, 0 );
    }
    i = 0;
  }

  /* lastly, if the npc does not have any of the following skills, add them
   * at 0 ranks, because they are used often (even untrained). */

  for( i = 0; requiredSkills[ i ] != 0; i++ ) {
    if( !npcHasSkill( npc, requiredSkills[ i ] ) ) {
      addSkill( npc, requiredSkills[ i ], 0 );
    }
  }

  npcRandomName( npc->race, npc->gender, opts->filePath, temp, sizeof( temp ) );
  npc->name = strdup( temp );
  npc->name[0] = toupper( npc->name[0] );

  return npc;
}


int npcGearValue( NPC* npc ) {
  NPCCLASS* cls;
  int       i;

  if( npcHasClass( npc, npcCOMMONER ) ) {
    cls = npcGetClass( npc, npcCOMMONER );
    i = npcGetCharacterLevel( npc ) - cls->level;
    i = dndGetNPCGearValue( cls->level ) / 200 +
           ( i > 0 ? dndGetNPCGearValue( i ) : 0 );
  } else {
    i = dndGetNPCGearValue( npcGetCharacterLevel( npc ) );
  }
  
  return i;
}


int npcComputeCR( NPC* npc ) {
  int       cr;
  int       level;
  NPCCLASS* cls;

  cr = dndGetRaceCR( npc->race );

  for( cls = npc->classes; cls != 0; cls = cls->next ) {
    level = cls->level;
    if( dndGetClassType( cls->type ) == cctNONPLAYER ) {
      level--;
    }
    cr += level;
  }

  if( cr < 1 ) {
    cr = 1;
  }

  return cr;
}


void npcAbScoreStrategy_BestOf4d6( int* scores, void* data ) {
  int rolls[6];
  int i;
  int j;

  /* roll the 6 abilities by rolling 4 dice for each, and keeping only the
   * highest 3. */

  for( i = 0; i < 6; i++ ) {
    for( j = 0; j < 4; j++ ) {
      rolls[j] = rollDice( 1, 6 );
    }
    qsort( rolls, 4, sizeof( int ), icomp );
    scores[i] = rolls[0] + rolls[1] + rolls[2];
  }
}


void npcAbScoreStrategy_Straight3d6( int* scores, void* data ) {
  int i;

  for( i = 0; i < 6; i++ ) {
    scores[ i ] = rollDice( 3, 6 );
  }
}
