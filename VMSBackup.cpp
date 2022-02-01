#define __VMSVERSION__ "1.3"

// VMS Backup Utiliy
//
// Description
//
// This tool is designed to parse and extract Open VMS Backup data sets.  This
// has been primarily reverse engineered based on backup sets generated using
// Open VMS 5.5.
//
// Any areas whereby publically available code has been used will be delimited
// by "==========" and display whichever copyright notice is applicable.
//
// Any remaining code that is not delimited shall be considered available for
// any commercial/non-commercial usage and is not subject to any usage
// limitations.
//
// History
//
// 1.3 - Added header dumping.
// 1.2 - Fixed the following issues:
//       1/ The single-pass selection criteria was still broken.  A two pass
//          mechanism is required if one of the following is in effect:
//          a) Smart Extraction is enabled.
//          b) Version selection is in effect.
//       2/ Broke the mask handling for file extensions.
//          for example -M:*.ADA;* didn't work, but *.ADA*;* did!
//       3/ Enhanced the outputs for file format and file attributes based
//          upon comparison to VAX directory data.  What isn't completely
//          known is how much is hard coded, and how much is specific to the
//          VAX setup I'm using.
//       4/ Cleaned up invalid records at the EOF.  I've suppressed outputting
//          errors on the last block unless debug or enhanced debug is
//          selected.  Processing *will* still abort though.
//       5/ Fixed an issue whereby the tool would seemingly randomly choose to
//          place an EOL at the end of the file or ignore it.  This was the
//          result of some overly complex rules regarding records that span
//          blocks.  There's now a much simpler rule in place, if we're at the
//          end of a record and an LRCF isn't detected (and this is a file
//          requiring ASCII conversion), then an LRCF is added.
//       6/ Left some placemarker code in the DumpFullFileHeader to allow
//          alternating between a dir /full output and a backup view output.
//          Ideally this sort of thing could be configurable i.e.
//          -LB:[Options] for Backup View and -LD:[Options] for a Dir View
//          which could support the whole plethora of VAX options... but I
//          doubt I'll ever get to that.
//       7/ Fixed a very nasty bug whereby a variable record header that spans
//          a block wasn't correctly handled resulting in the file itself
//          being incorrectly handled in either a binary or ascii mode.
// 1.1 - Fixed two issues:
//       1/ Broke the single-pass mechanism for data output.
//       2/ Fixed an issue with VBN Raw records whereby the file pointer wasn't
//          being incremented for the last file chunk.
//       There's still the invalid record at the EOF, but thus-far I've no clue
//       about what that actually represents but the issue seems harmless enough
//       for now.
// 1.0 - First version under configuration control.
// 0.4 - Removed the -A parameter in favour of improving wild card support to
//       handle VAX versioning, including relative versions (0, -1 etc).
//       So a mask of *.*;* is equivalent to -A, and a mask of *.*;0 is
//       equivalent to unsetting the flag.  This should improve useability
//       by some margin.  Now it's possible to get all versions of a file without
//       having to set -V!
// 0.3 - Fixed issue in ProcessVBNVar whereby record entry headers that span
//       Blocks resulted in the tool locking up.  These are now handled
//       with no apparent issues.
//
//       Fixed issue in ProcessVBNNonVar whereby the LR/CF detection fell over
//       if LR/CF spanned two records.  Additionally improved detection logic
//       designed to ungarble various mangled LR/CF combinations.
//
//       Unified the majority of the file writing to use a new routine called
//       VMSWriteFile.  This gives a single point of entry for handling ASCII
//       data output.
//
//       Improved EOL handling in ProcessVBNNonVar which will hopefully
//       EOL's occuring if the record already ends with an EOL.  I'm hesitant
//       to suppress them if any EOL's are present until I've further data.
//       This should fix a certain mismatch between ASCII data transferred via
//       FTP and data decoded from the BCK archive.
//
//       Moved most of the code into the file Process.cpp.  This will
//       eventually be further broken down, but for now, it helps to allow
//       this file to focus on front-end matters.  The main file is now only
//       responsible for determining whether the input data set can be
//       physically opened.
//
//       Removed basictypes.h in favour of using stdint.h.  In the event that
//       this library isn't available (Visual Studio), you can grab a version
//       of the library from:
//
//       http://code.google.com/p/msinttypes/downloads/list
//
//       However it's worth noting that I had to modify line 50 to:
//
//       extern "C++" {
//
//       in order to get it to compile with Visual Studio 6.  Also, be sure to
//       add the following pre-processor definition __STDC_CONSTANT_MACROS.
//
//       This last update increases portability, and hopefully removes the
//       need for me to constantly re-jig types to increase compatability.
//       Anyone know of something similar for type alignment?
//
//       Note: All enhancements will still be documented in a single place for
//       simplicity.
//
//       Added an enhanced diagnostics mode.  This is only a start, but the
//       idea is to basically give an indication as to *exactly* what the
//       application is currently doing, in order to diagnose crashes,
//       incorrect decodes etc.  All these features will probably eventually
//       be reduced to conditional compile flags, but for now, given the
//       relative immaturity of the application, it's nicer to do this way.
//
// 0.2 - Fixed file pointer tracking in ProcessVBNNonVar to prevent garbage
//       data being appended to a file.
//
//       Modified ProcessVBNVar to handle VFC types (appear to be variable
//       records with 8 bit checksum data).  UDF has been mapped to just
//       output raw data.
//
//       Made default behaviour of invalid data or non-VBN/Volume/NULL records
//       to close any open file handles.
//
//       Minor compiler fix for Solaris when testing size/alignment data.
//
//       Now verified as correct on over 1 Gigs worth of backup sets
//       (> 20,000 files).  Auto mode actually corrected some issues found
//       when FTP'ing data which contained records and LR/CF data!
//
// 0.1 - Initial version

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Process.h"
#include "alignment.h"

void DisplayHelp ()
{
    fprintf (stdout, "VMSBackup Version " __VMSVERSION__ " " __DATE__ "\n");
    fprintf (stdout, "\n");
    fprintf (stdout, "VMSBackup [FILE] [-L:listoption] [-E] [-X:extractmode] [-M:mask] [-F] [-V] [-D] [-?]\n");
    fprintf (stdout, "\n");
    fprintf (stdout, "  FILE           Backup Data Set\n");
    fprintf (stdout, "  -L             Selects output list\n");
    fprintf (stdout, "  listoptions     S  Suppress Output             B  Brief Output (default)\n");
    fprintf (stdout, "                  F  Full Output                 C  CSV Output\n");
    fprintf (stdout, "  -E             Extract File Contents (default off)\n");
    fprintf (stdout, "  -X             Selects Extraction Mode\n");
    fprintf (stdout, "  extractmode     S  Smart/Auto Mode (default)   A  ASCII Mode\n");
    fprintf (stdout, "                  B  Binary Mode                 R  Raw Mode\n");
    fprintf (stdout, "  -M             File Extraction Mask\n");
    fprintf (stdout, "                  e.g. *.*, *.bin;*, *a*.*;-1 etc.\n");
    fprintf (stdout, "                  Default is *.*;0.\n");
    fprintf (stdout, "  -F             Extract with full path (default off)\n");
    fprintf (stdout, "  -V             Extract with version numbers in the filename (default off)\n");
    fprintf (stdout, "  -D             Debug Mode (default off)\n");
    fprintf (stdout, "  -DD            Enhanced Debug Mode (default off)\n");
    fprintf (stdout, "  -?             Display this help\n");
}

bool SelfTestSize (char* szType, int iExpectedSize, int iActualSize)
{
    bool                bValid                  = iExpectedSize == iActualSize;

    if (!bValid)
    {
        fprintf (stderr, "%s type is incorrect.  Expected Size = %i bytes, Actual Size = %i bytes\n", szType, iExpectedSize, iActualSize);
    }

    // Return the inverse validity
    return !bValid;
}

bool SelfTestAlignment (char* szType, uint16_t iExpectedValue, uint16_t iActualValue)
{
    bool                bValid                  = iExpectedValue == iActualValue;

    if (!bValid)
    {
        fprintf (stderr, "%s alignment is incorrect.  Expected Data = 0x%04X, Actual Data = 0x%04X\n", szType, iExpectedValue, iActualValue);
    }

    // Return the inverse validity
    return !bValid;
}

bool SelfTestAlignment (char* szType, int16_t iExpectedValue, int16_t iActualValue)
{
    bool                bValid                  = iExpectedValue == iActualValue;

    if (!bValid)
    {
        fprintf (stderr, "%s alignment is incorrect.  Expected Data = 0x%04X, Actual Data = 0x%04X\n", szType, iExpectedValue, iActualValue);
    }

    // Return the inverse validity
    return !bValid;
}

bool SelfTestAlignment (char* szType, uint32_t iExpectedValue, uint32_t iActualValue)
{
    bool                bValid                  = iExpectedValue == iActualValue;

    if (!bValid)
    {
        fprintf (stderr, "%s alignment is incorrect.  Expected Data = 0x%08X, Actual Data = 0x%08X\n", szType, iExpectedValue, iActualValue);
    }

    // Return the inverse validity
    return !bValid;
}

bool SelfTestAlignment (char* szType, int32_t iExpectedValue, int32_t iActualValue)
{
    bool                bValid                  = iExpectedValue == iActualValue;

    if (!bValid)
    {
        fprintf (stderr, "%s alignment is incorrect.  Expected Data = 0x%08X, Actual Data = 0x%08X\n", szType, iExpectedValue, iActualValue);
    }

    // Return the inverse validity
    return !bValid;
}

bool SelfTestAlignment (char* szType, uint64_t iExpectedValue, uint64_t iActualValue)
{
    bool                bValid                  = iExpectedValue == iActualValue;

    if (!bValid)
    {
        fprintf (stderr, "%s alignment is incorrect.  Expected Data = 0x%08X%08X, Actual Data = 0x%08X%08X\n", szType, (uint32_t) ((uint64_t) (iExpectedValue & UINT64_C(0xFFFFFFFF00000000)) >> 32), (uint32_t) (iExpectedValue & UINT64_C(0xFFFFFFFF)),
                                                                                                                       (uint32_t) ((uint64_t) (iActualValue & UINT64_C(0xFFFFFFFF00000000))   >> 32), (uint32_t) (iActualValue & UINT64_C(0xFFFFFFFF)) );
    }

    // Return the inverse validity
    return !bValid;
}

bool SelfTestAlignment (char* szType, int64_t iExpectedValue, int64_t iActualValue)
{
    bool                bValid                  = iExpectedValue == iActualValue;

    if (!bValid)
    {
        fprintf (stderr, "%s alignment is incorrect.  Expected Data = 0x%08X%08X, Actual Data = 0x%08X%08X\n", szType, (uint32_t) ((uint64_t) (iExpectedValue & UINT64_C(0xFFFFFFFF00000000)) >> 32), (uint32_t) (iExpectedValue & UINT64_C(0xFFFFFFFF)),
                                                                                                                       (uint32_t) ((uint64_t) (iActualValue & UINT64_C(0xFFFFFFFF00000000))   >> 32), (uint32_t) (iActualValue & UINT64_C(0xFFFFFFFF)) );
    }

    // Return the inverse validity
    return !bValid;
}

int main (int argc, char** argv)
{
    //////////////////////////////////////////////////////////////////////////
    // Constants

    // Valid Version Mask Characters
    const char*         cszValidVersionMask     = "*-1234567890";

    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Backup File Set
    FILE*               pFile                   = NULL;

    // Pointer to Extract Mask
    char*               pszExtractMask;
    bool                bVersionMaskValid;
    bool                bVersionMaskMatch;

    // File Argument Pointer
    unsigned            uiFileArgumentPointer   = 1;
    bool                bFileArgumentFound      = false;

    // Display Help
    bool                bDisplayHelp            = false;

    // Command Line Parameters
    SBackupParameters   sParameters;

    // Alignment Variables
    uint8_t             uiRaw[8];
    int16_t*            pRawS16;
    uint16_t*           pRawUS16;
    int32_t*            pRawS32;
    uint32_t*           pRawUS32;
    int64_t*            pRawS64;
    uint64_t*           pRawUS64;

    //////////////////////////////////////////////////////////////////////////
    // Self Test the Data Types

    // Sizes
    if (        SelfTestSize ((char*) "char",     1, sizeof(char))     ||
                SelfTestSize ((char*) "uint8_t",  1, sizeof(uint8_t))  ||
                SelfTestSize ((char*) "int8_t",   1, sizeof(int8_t))   ||
                SelfTestSize ((char*) "uint16_t", 2, sizeof(uint16_t)) ||
                SelfTestSize ((char*) "int16_t",  2, sizeof(int16_t))  ||
                SelfTestSize ((char*) "uint32_t", 4, sizeof(uint32_t)) ||
                SelfTestSize ((char*) "int32_t",  4, sizeof(int32_t))  ||
                SelfTestSize ((char*) "uint64_t", 8, sizeof(uint64_t)) ||
                SelfTestSize ((char*) "int64_t",  8, sizeof(int64_t))   )
    {
        return 0;
    }

    // Alignments
    uiRaw[0]  = 0x12;
    uiRaw[1]  = 0x34;
    uiRaw[2]  = 0x56;
    uiRaw[3]  = 0x78;
    uiRaw[4]  = 0x9a;
    uiRaw[5]  = 0xbc;
    uiRaw[6]  = 0xde;
    uiRaw[7]  = 0xf0;
    pRawS16   = (int16_t*)  uiRaw;
    pRawUS16  = (uint16_t*) uiRaw;
    pRawS32   = (int32_t*)  uiRaw;
    pRawUS32  = (uint32_t*) uiRaw;
    pRawS64   = (int64_t*)  uiRaw;
    pRawUS64  = (uint64_t*) uiRaw;

    if (        SelfTestAlignment ((char*) "uint16_t", (uint16_t) 0x3412,                       (uint16_t) SWAPSHORT(*pRawUS16))     ||
                SelfTestAlignment ((char*) "int16_t",  (int16_t)  0x3412,                       (int16_t)  SWAPSHORT(*pRawS16))      ||
                SelfTestAlignment ((char*) "uint32_t", (uint32_t) 0x78563412,                   (uint32_t) SWAPLONG(*pRawUS32))      ||
                SelfTestAlignment ((char*) "int32_t",  (int32_t)  0x78563412,                   (int32_t)  SWAPLONG(*pRawS32))       ||
                SelfTestAlignment ((char*) "uint64_t", (uint64_t) UINT64_C(0xf0debc9a78563412), (uint64_t) SWAPLONGLONG(*pRawUS64))  ||
                SelfTestAlignment ((char*) "int64_t",  (int64_t)  INT64_C(0xf0debc9a78563412),  (int64_t)  SWAPLONGLONG(*pRawS64))     )
    {
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    // Default the File Parameters

    // Default Mode : Brief File List
    sParameters.bOutputSuppress                 = false;
    sParameters.bOutputBrief                    = true;
    sParameters.bOutputFull                     = false;
    sParameters.bOutputCSV                      = false;

    // Default Mode : No extraction
    sParameters.bExtract                        = false;

    // Default Mode : Smart Extraction
    sParameters.bExtractModeSmart               = true;
    sParameters.bExtractModeASCII               = false;
    sParameters.bExtractModeBinary              = false;
    sParameters.bExtractModeRaw                 = false;

    // Default Mode : Single Pass Mode
    sParameters.bExtractFirstPass               = false;
    sParameters.bIgnoreVBN                      = false;

    // Default Mode : No buffered file data
    sParameters.bLFDetected                     = false;
    sParameters.uiFilePointer                   = 0;
    sParameters.uiFileSize                      = 0;
    sParameters.cFormat                         = 0;
    sParameters.uiRemainingStartPos             = 0;
    sParameters.uiRemainingRecordLength         = 0;

    // Default Mode : *.*;0
    sParameters.szExtractMask               = new char[strlen ((char*) "*.*") + 1];
    strcpy (sParameters.szExtractMask, (char*) "*.*");
    sParameters.szExtractMaskVersion        = new char[strlen ((char*) "0") + 1];
    strcpy (sParameters.szExtractMaskVersion, (char*) "0");
    sParameters.iExtractVersion             = 0;

    // Default Mode : Don't extract with paths
    sParameters.bExtractFolder              = false;

    // Default Mode : Don't extract with file versions in the file name
    sParameters.bExtractWithVersion         = false;

    // Default Mode : No Debugging
    sParameters.bExtractDebug               = false;
    sParameters.bExtractDebugEnhanced       = false;

    //////////////////////////////////////////////////////////////////////////
    // Read the Command Line Parameters

    if (argc <= 1)
    {
        // No Parameters, so display the Help
        DisplayHelp ();

        // Remove all alocations
        delete (sParameters.szExtractMask);
        delete (sParameters.szExtractMaskVersion);

        return 0;
    }
    else
    {
        // Parse through the parameter list

        for (int i = 1; (i < argc); i++)
        {
            if ((argv[i][0] == '-') || bFileArgumentFound)
            {
                if (!bFileArgumentFound)
                {
                    // Not the File, so increment the File Pointer
                    uiFileArgumentPointer++;
                }

                if ( (strcmp (argv[i], "-LS") == 0) || (strcmp (argv[i], "-L:S") == 0) )
                {
                    sParameters.bOutputSuppress             = true;
                    sParameters.bOutputBrief                = false;
                    sParameters.bOutputFull                 = false;
                    sParameters.bOutputCSV                  = false;
                }
                else if ( (strcmp (argv[i], "-LB") == 0) || (strcmp (argv[i], "-L:B") == 0) )
                {
                    sParameters.bOutputSuppress             = false;
                    sParameters.bOutputBrief                = true;
                    sParameters.bOutputFull                 = false;
                    sParameters.bOutputCSV                  = false;
                }
                else if ( (strcmp (argv[i], "-LF") == 0) || (strcmp (argv[i], "-L:F") == 0) )
                {
                    sParameters.bOutputSuppress             = false;
                    sParameters.bOutputBrief                = false;
                    sParameters.bOutputFull                 = true;
                    sParameters.bOutputCSV                  = false;
                }
                else if ( (strcmp (argv[i], "-LC") == 0) || (strcmp (argv[i], "-L:C") == 0) )
                {
                    sParameters.bOutputSuppress             = false;
                    sParameters.bOutputBrief                = false;
                    sParameters.bOutputFull                 = false;
                    sParameters.bOutputCSV                  = true;
                }
                else if (strcmp (argv[i], "-E") == 0)
                {
                    sParameters.bExtract                    = true;
                }
                else if ( (strcmp (argv[i], "-XS") == 0) || (strcmp (argv[i], "-X:S") == 0) )
                {
                    sParameters.bExtractModeSmart           = true;
                    sParameters.bExtractModeASCII           = false;
                    sParameters.bExtractModeBinary          = false;
                    sParameters.bExtractModeRaw             = false;
                }
                else if ( (strcmp (argv[i], "-XA") == 0) || (strcmp (argv[i], "-X:A") == 0) )
                {
                    sParameters.bExtractModeSmart           = false;
                    sParameters.bExtractModeASCII           = true;
                    sParameters.bExtractModeBinary          = false;
                    sParameters.bExtractModeRaw             = false;
                }
                else if ( (strcmp (argv[i], "-XB") == 0) || (strcmp (argv[i], "-X:B") == 0) )
                {
                    sParameters.bExtractModeSmart           = false;
                    sParameters.bExtractModeASCII           = false;
                    sParameters.bExtractModeBinary          = true;
                    sParameters.bExtractModeRaw             = false;
                }
                else if ( (strcmp (argv[i], "-XR") == 0) || (strcmp (argv[i], "-X:R") == 0) )
                {
                    sParameters.bExtractModeSmart           = false;
                    sParameters.bExtractModeASCII           = false;
                    sParameters.bExtractModeBinary          = false;
                    sParameters.bExtractModeRaw             = true;
                }
                else if ( (strncmp (argv[i], "-M", 2) == 0) || (strncmp (argv[i], "-M:", 3) == 0) )
                {
                    if (strncmp (argv[i], "-M:", 3) == 0)
                    {
                        pszExtractMask = &argv[i][3];
                    }
                    else
                    {
                        pszExtractMask = &argv[i][2];
                    }

                    // Parse the Extracted Mask
                    if (strstr (pszExtractMask, ";"))
                    {
                        // A semi-colon is present, so copy the Mask data prior to the semi-colon
                        // as the Mask, and the data after as the version.

                        // Remove all alocations
                        delete (sParameters.szExtractMask);
                        delete (sParameters.szExtractMaskVersion);

                        // Copy the Extraction Mask, and default the version
                        sParameters.szExtractMask               = new char[strlen (pszExtractMask) + 1];
                        strcpy (sParameters.szExtractMask, pszExtractMask);

                        // A bit wasteful, but simpler than tokenizing
                        if (strstr (sParameters.szExtractMask, ";"))
                        {
                            *strstr (sParameters.szExtractMask, ";")    = '\0';
                        }

                        // Move the Version Pointer
                        pszExtractMask  = strstr (pszExtractMask, ";") + sizeof(char);

                        // Validate the contents of the Version Mask
                        bVersionMaskValid   = strlen (pszExtractMask) > 0;
                        for (int p = 0; (p < strlen (pszExtractMask)) && bVersionMaskValid; p++)
                        {
                            // The current character must be equal to one of the parameters
                            bVersionMaskMatch   = false;
                            for (int q = 0; (q < strlen (cszValidVersionMask)) && !bVersionMaskMatch; q++)
                            {
                                bVersionMaskMatch   =   bVersionMaskMatch ||
                                                        (pszExtractMask[p] == cszValidVersionMask[q]);
                            }

                            bVersionMaskValid = bVersionMaskValid && bVersionMaskMatch;
                        }

                        if (!bVersionMaskValid)
                        {
                            // Bad Version Mask, so default it
                            sParameters.szExtractMaskVersion        = new char[strlen ((char*) "0") + 1];
                            strcpy (sParameters.szExtractMaskVersion, (char*) "0");
                            sParameters.iExtractVersion             = 0;
                        }
                        else
                        {
                            sParameters.szExtractMaskVersion        = new char[strlen (pszExtractMask) + 1];
                            strcpy (sParameters.szExtractMaskVersion, pszExtractMask);
                            sParameters.iExtractVersion             = strtol (sParameters.szExtractMaskVersion, NULL, 10);
                        }
                    }
                    else
                    {
                        // No semi-colon, so copy the Mask as-is, and default the extract version
                        // to latest only

                        // Remove all alocations
                        delete (sParameters.szExtractMask);
                        delete (sParameters.szExtractMaskVersion);

                        // Copy the Extraction Mask, and default the version
                        sParameters.szExtractMask               = new char[strlen (pszExtractMask) + 1];
                        strcpy (sParameters.szExtractMask, pszExtractMask);
                        sParameters.szExtractMaskVersion        = new char[strlen ((char*) "0") + 1];
                        strcpy (sParameters.szExtractMaskVersion, (char*) "0");
                        sParameters.iExtractVersion             = 0;
                    }
                }
                else if (strcmp (argv[i], "-F") == 0)
                {
                    sParameters.bExtractFolder              = true;
                }
                else if (strcmp (argv[i], "-V") == 0)
                {
                    sParameters.bExtractWithVersion         = true;
                }
                else if (strcmp (argv[i], "-DD") == 0)
                {
                    sParameters.bExtractDebugEnhanced       = true;
                    sParameters.bExtractDebug               = true;
                }
                else if (strcmp (argv[i], "-D") == 0)
                {
                    sParameters.bExtractDebug               = true;
                }
                else if (strcmp (argv[i], "-?") == 0)
                {
                    // Display the Help
                    bDisplayHelp                            = true;
                }
                else
                {
                    fprintf (stderr, "WARNING : Unknown parameter %s.\n", argv[i]);
                }
            }
            else
            {
                bFileArgumentFound  = true;
            }
        }

        // Determine whether 2 passes are needed
        sParameters.bExtractFirstPass   =   (sParameters.bExtract && sParameters.bExtractModeSmart) ||
                                            (strcmp (sParameters.szExtractMaskVersion, "*") != 0);

        //////////////////////////////////////////////////////////////////////
        // DEBUG
        if (sParameters.bExtractDebug)
        {
            // Output the Chosen Parameters
            fprintf (stdout, "*** START DEBUG ***\n");

            fprintf (stdout, "VMS Backup Options\n");

            fprintf (stdout, "Output Mode             = ");
            if (sParameters.bOutputSuppress)
            {
                fprintf (stdout, "SUPPRESS\n");
            }
            else if (sParameters.bOutputBrief)
            {
                fprintf (stdout, "BRIEF\n");
            }
            else if (sParameters.bOutputFull)
            {
                fprintf (stdout, "FULL\n");
            }
            else if (sParameters.bOutputCSV)
            {
                fprintf (stdout, "CSV\n");
            }
            else
            {
                fprintf (stdout, "*** ERROR ***\n");
            }

            fprintf (stdout, "Extract                 = ");
            if (sParameters.bExtract)
            {
                fprintf (stdout, "ON\n");
            }
            else
            {
                fprintf (stdout, "OFF\n");
            }

            fprintf (stdout, "Extract Mode            = ");
            if (sParameters.bExtractModeSmart)
            {
                fprintf (stdout, "SMART\n");
            }
            else if (sParameters.bExtractModeASCII)
            {
                fprintf (stdout, "ASCII\n");
            }
            else if (sParameters.bExtractModeBinary)
            {
                fprintf (stdout, "BINARY\n");
            }
            else if (sParameters.bExtractModeRaw)
            {
                fprintf (stdout, "RAW\n");
            }
            else
            {
                fprintf (stdout, "*** ERROR ***\n");
            }

            fprintf (stdout, "Extract Mask            = ");
            fprintf (stdout, "%s\n", sParameters.szExtractMask);

            fprintf (stdout, "Extract Version         = ");
            fprintf (stdout, "%s\n", sParameters.szExtractMaskVersion);

            fprintf (stdout, "Extract Full Paths      = ");
            if (sParameters.bExtractFolder)
            {
                fprintf (stdout, "ON\n");
            }
            else
            {
                fprintf (stdout, "OFF\n");
            }

            fprintf (stdout, "Extract with Versions   = ");
            if (sParameters.bExtractWithVersion)
            {
                fprintf (stdout, "ON\n");
            }
            else
            {
                fprintf (stdout, "OFF\n");
            }

            fprintf (stdout, "Debug Mode              = ");
            if (sParameters.bExtractDebugEnhanced)
            {
                fprintf (stdout, "ON (ENHANCED)\n");
            }
            else if (sParameters.bExtractDebug)
            {
                fprintf (stdout, "ON (NORMAL)\n");
            }
            else
            {
                fprintf (stdout, "OFF\n");
            }

            fprintf (stdout, "Number of Passes        = ");
            if (sParameters.bExtractFirstPass)
            {
                fprintf (stdout, "2\n");
            }
            else
            {
                fprintf (stdout, "1\n");
            }

            fprintf (stdout, "*** END DEBUG ***\n");
        }
        //////////////////////////////////////////////////////////////////////
        // END DEBUG


        if (bDisplayHelp)
        {
            fprintf (stdout, "\n");
            DisplayHelp ();

            // Remove all alocations
            delete (sParameters.szExtractMask);
            delete (sParameters.szExtractMaskVersion);

            return 0;
        }
        else if (bFileArgumentFound)
        {
            // Try to open the Backup Set
            pFile   = fopen (argv[uiFileArgumentPointer], "rb");
        }
        else
        {
            fprintf (stderr, "ERROR   : No file specified.\n");

            // Remove all alocations
            delete (sParameters.szExtractMask);
            delete (sParameters.szExtractMaskVersion);

            return 1;
        }

        if (pFile)
        {
            //////////////////////////////////////////////////////////////////////
            // Process the Backup Set
            ProcessBackup (pFile, &sParameters);
        }
        else
        {
            fprintf (stderr, "ERROR   : Unable to open %s for reading\n", argv[1]);

            // Remove all alocations
            delete (sParameters.szExtractMask);
            delete (sParameters.szExtractMaskVersion);

            return 1;
        }

    }

    // Close the Backup Set
    fclose (pFile);

    return 0;
}
