#define __VMSVERSION__ "1.6"

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Process.h"
#include "alignment.h"

void DisplayHelp ()
{
    fprintf (stdout, "VMSBackup Version " __VMSVERSION__ " " __DATE__ "\n");
    fprintf (stdout, "\n");
    fprintf (stdout, "VMSBackup [FILE] [-L:listoption] [-N] [-X:extractmode] [-M:mask] [-F] [-V] [-D] [-?]\n");
    fprintf (stdout, "\n");
    fprintf (stdout, "  FILE           Backup Data Set\n");
    fprintf (stdout, "  -L             Selects output list\n");
    fprintf (stdout, "  listoptions     S  Suppress Output             B  Brief Output (default)\n");
    fprintf (stdout, "                  F  Full Output                 C  CSV Output\n");
    fprintf (stdout, "  -N             Don't Extract File Contents\n");
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

int main (int argc, char* argv[])
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

    // Default Mode : Extraction
    sParameters.bExtract                        = true;

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
                else if (strcmp (argv[i], "-N") == 0)
                {
                    sParameters.bExtract                    = false;
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
