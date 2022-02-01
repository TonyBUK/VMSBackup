#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "Process.h"
#include "BBHeader.h"
#include "BBHeader.h"
#include "BRHeader.h"
#include "BSFileHeader.h"
#include "VMSBackupTypes.h"
#include "alignment.h"

//============================================================================
#include "wildcards.h"
#include "timevmstounix.h"
//============================================================================

void VMSWriteFile (SBackupParameters* psParameters, unsigned long uiDataLength, uint8_t* cBuffer, FILE** ppCurrentOutputFile, bool* pbLastElementWasLFCR, bool* pbContainsLFCR)
{
    const char      szEndOfLine[2]      = {'\r','\n'};

    unsigned long   uiBufferPointer;

    bool            bLastElementWasLFCR = false;
    bool            bContainsLFCR       = false;

    if (psParameters == NULL)
    {
        // Assume Binary
        fwrite (cBuffer, uiDataLength, sizeof (uint8_t), *ppCurrentOutputFile);
    }
    else if (psParameters->bExtractModeASCII)
    {
        // ASCII Mode
        for (uiBufferPointer = 0; uiBufferPointer < uiDataLength; uiBufferPointer++)
        {
            if ((cBuffer[uiBufferPointer] == '\r') && !psParameters->bLFDetected)
            {
                // Do nothing whilst the EOL is assessed
                psParameters->bLFDetected   = true;

                // Indicate this data package contains an LF/CR entry
                bContainsLFCR               = true;
                bLastElementWasLFCR         = (uiBufferPointer+1) == uiDataLength;
            }
            else if (psParameters->bLFDetected && (cBuffer[uiBufferPointer] == '\n'))
            {
                // This file already contains standard EOL conventions
                fwrite (szEndOfLine, 2, sizeof (char), *ppCurrentOutputFile);
                psParameters->bLFDetected   = false;

                // Indicate this data package contains an LF/CR entry
                bContainsLFCR               = true;
                bLastElementWasLFCR         = (uiBufferPointer+1) == uiDataLength;
            }
            else if (!psParameters->bLFDetected && (cBuffer[uiBufferPointer] == '\n'))
            {
                // This is not seen as a valid EOL, so normalise it to a PC standard
                fwrite (szEndOfLine, 2, sizeof (char), *ppCurrentOutputFile);
                psParameters->bLFDetected   = false;

                // Indicate this data package contains an LF/CR entry
                bContainsLFCR               = true;
                bLastElementWasLFCR         = (uiBufferPointer+1) == uiDataLength;
            }
            else if (psParameters->bLFDetected && (cBuffer[uiBufferPointer] != '\n'))
            {
                // This is not seen as a valid EOL, so normalise it to a PC standard
                fwrite (szEndOfLine, 2, sizeof (char), *ppCurrentOutputFile);
                psParameters->bLFDetected   = false;

                // Indicate this data package contains an LF/CR entry
                bContainsLFCR               = true;
                //! This indicates the *previous* byte was an LF/CR therefore the current
                //! element isn't, hence no check to see if the Last Element is an LF/CR

                // Output the current byte since it contained non-EOL data
                fwrite (&cBuffer[uiBufferPointer] , 1, sizeof (char), *ppCurrentOutputFile);
            }
            else
            {
                fwrite (&cBuffer[uiBufferPointer] , 1, sizeof (char), *ppCurrentOutputFile);
                psParameters->bLFDetected   = false;
            }
        }
    }
    else
    {
        // Binary/Raw Mode
        fwrite (cBuffer, uiDataLength, sizeof (uint8_t), *ppCurrentOutputFile);
    }

    if (pbLastElementWasLFCR != NULL)
    {
        *pbLastElementWasLFCR   = bLastElementWasLFCR;
    }

    if (pbContainsLFCR != NULL)
    {
        *pbContainsLFCR         = bContainsLFCR;
    }
}

void VMSWriteEOL (SBackupParameters* psParameters, FILE** ppCurrentOutputFile, bool bForceEOL)
{
    const char      szEndOfLine[2]      = {'\r','\n'};

    if (psParameters->bExtractModeASCII)
    {
        // If the last byte was \ '\r' it won't have been written
        if (((psParameters->uiFilePointer >= psParameters->uiFileSize) && psParameters->bLFDetected))
        {
            fwrite (szEndOfLine, 1, sizeof (char), *ppCurrentOutputFile);
            psParameters->bLFDetected   = false;
        }
        else if (bForceEOL)
        {
            fwrite (szEndOfLine, 2, sizeof (char), *ppCurrentOutputFile);
        }
    }
}

void DecodeTimeAndDate (uint16_t uiRaw[4], uint32_t uiSubSecondResolution)
{
    //////////////////////////////////////////////////////////////////////////
    // Constants

    // Month Lookup Table
    const char*     cszMonth[]          = { "JAN", "FEB", "MAR", "APR",
                                            "MAY", "JUN", "JUL", "AUG",
                                            "SEP", "OCT", "NOV", "DEC"  };

    //////////////////////////////////////////////////////////////////////////
    // Variables

    // VMS Raw Time encoded as a 64 Bit Integer
    int64_t*        uiVMS               = (int64_t*) uiRaw;

    // Subsecond Time
    uint32_t        uiTimeSubSeconds    = 0;

    // Time Structure
    time_t          sDate;

    // Time Stored in GMT
    struct tm*      sGMTTime;

    // Open VMS Time is a 64 Bit value representing 100ns tics since 00:00 November 17, 1858
    // (Modified Julian Day Zero)
    // http://h71000.www7.hp.com/wizard/wiz_2315.html

    // C Time Stamp is a 32 Bit value representing seconds since January 1 1970

    if (    (uiRaw[0] == 0) &&
            (uiRaw[1] == 0) &&
            (uiRaw[2] == 0) &&
            (uiRaw[3] == 0) )
    {
        fprintf (stdout, "<None specified>");
    }
    else
    {
        // Convert the VMS Format timestamp to a "UNIX" (i.e., C format) timestamp
        *uiVMS   = SWAPLONGLONG (*uiVMS);
        timevmstounix (uiVMS, &sDate);

        // Convert time to GM Time
        sGMTTime = gmtime(&sDate);

        if (uiSubSecondResolution == 0)
        {
            // Output the Date/Time to match the Open VMS directory listing
            fprintf (stdout, "%i-%s-%i %02i:%02i:%02i", sGMTTime->tm_mday,
                                                        cszMonth[sGMTTime->tm_mon],
                                                        sGMTTime->tm_year + 1900,
                                                        sGMTTime->tm_hour,
                                                        sGMTTime->tm_min,
                                                        sGMTTime->tm_sec    );
        }
        else
        {
            // The above algorithm will only have a resolution of seconds, therefore
            // extra processing is required in order to get milliseconds

            // VMS Time has an LSB of 100ns
            // Convert to Hundredths of a Second
            uiTimeSubSeconds    = (uint32_t) (*uiVMS % (int64_t) 10000000);

            uiTimeSubSeconds    = uiTimeSubSeconds / (uint32_t) pow ((float) 10, (float) (7-uiSubSecondResolution));

            // Output the Date/Time to match the Open VMS directory listing
            fprintf (stdout, "%i-%s-%i %02i:%02i:%02i.%02i",    sGMTTime->tm_mday,
                                                                cszMonth[sGMTTime->tm_mon],
                                                                sGMTTime->tm_year + 1900,
                                                                sGMTTime->tm_hour,
                                                                sGMTTime->tm_min,
                                                                sGMTTime->tm_sec,
                                                                uiTimeSubSeconds );
        }
    }
}

void DecodeFileProtection (uint8_t cValue)
{
    if ((cValue & 0x1) == 0)
    {
        fprintf (stdout, "R");
    }
    if ((cValue & 0x2) == 0)
    {
        fprintf (stdout, "W");
    }
    if ((cValue & 0x4) == 0)
    {
        fprintf (stdout, "E");
    }
    if ((cValue & 0x8) == 0)
    {
        fprintf (stdout, "D");
    }
}

void DecodeRecordFormat (uint8_t cValue, uint16_t uiSize)
{
    switch (cValue)
    {
        case RECORD_FORMAT_UDF:
        {
            break;
        }

        case RECORD_FORMAT_FIX:
        {
            fprintf (stdout, "Fixed length 512 byte records");
            break;
        }

        case RECORD_FORMAT_VAR:
        {
            fprintf (stdout, "Variable length");
            if (uiSize != 0)
            {
                fprintf (stdout, ", maximum %i bytes", uiSize);
            }
            break;
        }


        case RECORD_FORMAT_VFC:
        {
            fprintf (stdout, "VFC, 2 byte header");
            if (uiSize != 0)
            {
                fprintf (stdout, ", maximum %i bytes", uiSize);
            }
            break;
        }


        case RECORD_FORMAT_STM:
        {
            fprintf (stdout, "Stream");
            if (uiSize != 0)
            {
                fprintf (stdout, ", maximum %i bytes", uiSize);
            }
            break;
        }


        case RECORD_FORMAT_STMLF:
        {
            fprintf (stdout, "Stream_LF");
            if (uiSize != 0)
            {
                fprintf (stdout, ", maximum %i bytes", uiSize);
            }
            break;
        }


        case RECORD_FORMAT_STMCR:
        {
            if (uiSize != 0)
            {
                fprintf (stdout, ", maximum %i bytes", uiSize);
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

void DecodeRecordAttributes (uint8_t cValue, SBackupParameters* psParameters)
{
    switch (cValue)
    {
        case RECORD_ATTRIBUTE_FTN:
        {
            fprintf (stdout, "None");
            break;
        }

        case RECORD_ATTRIBUTE_CR:
        {
            break;
        }

        case RECORD_ATTRIBUTE_CRN:
        {
            fprintf (stdout, "Carriage return carriage control");
            break;
        }

        case RECORD_ATTRIBUTE_BLK:
        {
            break;
        }

        case RECORD_ATTRIBUTE_PRN:
        {
            fprintf (stdout, "Print file carriage control");
            break;
        }

        default:
        {
            if (!psParameters->bExtractFirstPass)
            {
                fprintf (stderr, "WARNING : Unknown attribute %i\n", cValue);
            }
            break;
        }
    }
}

void DumpHeader (BBHeader* pHeader, SBackupParameters* psParameters)
{
        fprintf (stdout, "Save set:          %s\n", pHeader->T_SSNAME ());
//        fprintf (stdout, "Written by:        \n");
//        fprintf (stdout, "UIC:               \n");
//        fprintf (stdout, "Date:              \n");
//        fprintf (stdout, "Command:           \n");
//        fprintf (stdout, "Operating system:  %04X\n", pHeader->W_OPSYS ());
//        fprintf (stdout, "BACKUP version:    \n");
//        fprintf (stdout, "CPU ID register:   \n");
//        fprintf (stdout, "Node name:         \n");
//        fprintf (stdout, "Written on:        \n");
        fprintf (stdout, "Block size:        %i\n", pHeader->L_BLOCKSIZE ());
//        fprintf (stdout, "Group size:        \n");
//        fprintf (stdout, "Buffer count:      \n");
        fprintf (stdout, "\n");
}

void DumpBriefFileHeader (BSFileHeader* pcFileHeader, uint32_t uiSubSecondResolution)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Output the File Name
    fprintf (stdout, "%s\n", pcFileHeader->FILENAME ());

    // Output the File Size
    if (SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[6]) == 0)
    {
        fprintf (stdout, "                      Size: %7i/%-7i",
        SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[5]) - 1,
        SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));
    }
    else
    {
        fprintf (stdout, "                      Size: %7i/%-7i",
        SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[5]) - 0,
        SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));
    }

    // Output the Creation Date
    fprintf (stdout, "   Created: ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->CREDATE (), uiSubSecondResolution);
    fprintf (stdout, "\n");
}

void DumpFullFileHeader (BSFileHeader* pcFileHeader, BRHeader* pcHeader, SBackupParameters* psParameters)
{
    // TO BE CLEANED
    // Change 0 to 1 to have the output format match a directory /full syntax, otherwise it will
    // match a backup set view.

#if 1
    //////////////////////////////////////////////////////////////////////////
    // Variables

    //////////////////////////////////////////////////////////////////////////
    // Dump the Full File Header (Directory Format)

    // Output the File Name
    fprintf (stdout, "%s", pcFileHeader->FILENAME ());

    // Output the File Id
    fprintf (stdout, "                  File ID:  (%i,%i,%i)\n", SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[0]),
                                                                 SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[1]),
                                                                 SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[2])-1);

    // Output the File Size
    if (SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[6]) == 0)
    {
        fprintf (stdout, "Size: %12i/%-12i",
        SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[5]) - 1,
        SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));
    }
    else
    {
        fprintf (stdout, "Size: %12i/%-12i",
        SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[5]) - 0,
        SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));
    }

    // Output the Owner
    fprintf (stdout, "Owner: [%06o,%06o]\n", SWAPSHORT (((uint16_t*) pcFileHeader->UIC ())[1]),
                                             SWAPSHORT (((uint16_t*) pcFileHeader->UIC ())[0])   );

    // Output the Creation Date
    fprintf (stdout, "Created:  ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->CREDATE (), 2);
    fprintf (stdout, "\n");

    // Output the Revised Date
    fprintf (stdout, "Revised:  ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->REVDATE (), 2);
    fprintf (stdout, " (%i)\n", SWAPSHORT (*((uint16_t*) pcFileHeader->REVISION ())));

    // Output the Expiry Date
    fprintf (stdout, "Expires:  ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->EXPDATE (), 2);
    fprintf (stdout, "\n");

    // Output the Backup Date
    fprintf (stdout, "Backup:  ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->BAKDATE (), 2);
    fprintf (stdout, "\n");

    // Output File Organisation
    //! Not enough data collated to determine other enumerations
    fprintf (stdout, "File organization:  ");
    if (!pcHeader->V_NONSEQUENTIAL ())
    {
        fprintf (stdout, "Sequential\n");
    }
    else
    {
        fprintf (stdout, "????\n");
    }

    // File Attributes
    fprintf (stdout, "File attributes:    ");

    // File Attributes - Allocation
    fprintf (stdout, "Allocation: %i", SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));

    // File Attributes - Extend
    //! No Idea, always 0
    fprintf (stdout, ", Extend: %i", 0);

    // File Attributes - Global Buffer Count
    //! No Idea, always 0
    fprintf (stdout, ", Global Buffer Count: %i", 0);

    // File Attributes - , Version Limit
    fprintf (stdout, ", Version limit: %i", SWAPSHORT (((uint16_t*) pcFileHeader->VERLIMIT ())[0]));

    // File Attributes - ????
    //! No Idea, sometimes ", Contiguous-best-try", the check below seems to
    //! work for test data
    if (SWAPSHORT (((uint16_t*) pcFileHeader->UCHAR ())[0]) == 32)
    {
        fprintf (stdout, ", Contiguous best try\n");
    }
    else
    {
        fprintf (stdout, "\n");
    }

    // Record Format
    fprintf (stdout, "  Record format:      ");
    DecodeRecordFormat (pcFileHeader->RECATTR ()[0], SWAPSHORT (pcFileHeader->RECSIZE ()[0]));
    fprintf (stdout, "\n");

    // Record Attributes
    fprintf (stdout, "  Record attributes:  ");
    DecodeRecordAttributes (pcFileHeader->RECATTR ()[1], psParameters);
    fprintf (stdout, "\n");

    // Output File Protection
    fprintf (stdout, "  File protection:    ");
    fprintf (stdout, "System:");
    DecodeFileProtection (pcFileHeader->FPRO ()[0] & 0xF);
    fprintf (stdout, ", Owner:");
    DecodeFileProtection (pcFileHeader->FPRO ()[0] >> 4);
    fprintf (stdout, ", Group:");
    DecodeFileProtection (pcFileHeader->FPRO ()[1] & 0xF);
    fprintf (stdout, ", World:");
    DecodeFileProtection (pcFileHeader->FPRO ()[1] >> 4);
    fprintf (stdout, "\n");

    // Seperate the File Entries
    fprintf (stdout, "\n");

#else
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Temporary Strings
    char*           szTempString1   = new char[32];
    char*           szTempString2   = new char[32];

    // First dump the brief header
    DumpBriefFileHeader (pcFileHeader, 0);

    // Output the Owner
    fprintf (stdout, "                      Owner: [%06o,%06o]  ",  SWAPSHORT (((uint16_t*) pcFileHeader->UIC ())[1]),
                                                                    SWAPSHORT (((uint16_t*) pcFileHeader->UIC ())[0])   );

    // Output the Revised Date
    fprintf (stdout, "Revised: ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->REVDATE (), 0);
    fprintf (stdout, " (%i)\n", SWAPSHORT (*((uint16_t*) pcFileHeader->REVISION ())));

    // Output the File Id
    //! Note : As a quirk, we actually cut the string if it's too wide
    sprintf (szTempString1, "(%i,%i,%i)              ",
                                            SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[0]),
                                            SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[1]),
                                            SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[2])   );
    strncpy (szTempString2, szTempString1, 14);
    szTempString2[14]   = '\0';
    fprintf (stdout, "                      File ID: %s", szTempString2);

    // Output the Expiry Date
    fprintf (stdout, " Expires: ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->EXPDATE (), 0);
    fprintf (stdout, "\n");

    // Output the Backup Date
    fprintf (stdout, "                                              Backup:  ");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->BAKDATE (), 0);
    fprintf (stdout, "\n");

    // Output File Protection
    fprintf (stdout, "  File protection:    ");
    fprintf (stdout, "System:");
    DecodeFileProtection (pcFileHeader->FPRO ()[0] & 0xF);
    fprintf (stdout, ", Owner:");
    DecodeFileProtection (pcFileHeader->FPRO ()[0] >> 4);
    fprintf (stdout, ", Group:");
    DecodeFileProtection (pcFileHeader->FPRO ()[1] & 0xF);
    fprintf (stdout, ", World:");
    DecodeFileProtection (pcFileHeader->FPRO ()[1] >> 4);
    fprintf (stdout, "\n");

    // Output File Organisation
    //! Not enough data collated to determine other enumerations
    fprintf (stdout, "  File organization:  ");
    if (!pcHeader->V_NONSEQUENTIAL ())
    {
        fprintf (stdout, "Sequential\n");
    }
    else
    {
        fprintf (stdout, "????\n");
    }

    // File Attributes
    fprintf (stdout, "  File attributes:    ");

    // File Attributes - Allocation
    fprintf (stdout, "Allocation = %i", SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));

    // File Attributes - Extend
    //! No Idea, always 0
    fprintf (stdout, ", Extend = %i\n", 0);

    // File Attributes - Global Buffer Count
    //! No Idea, always 0
    fprintf (stdout, "                      Global Buffer Count = %i", 0);

    // File Attributes - ????
    //! No Idea, sometimes ", Contiguous-best-try", the check below seems to
    //! work for test data
    if (SWAPSHORT (((uint16_t*) pcFileHeader->UCHAR ())[0]) == 32)
    {
        fprintf (stdout, ", Contiguous-best-try\n");
    }
    else
    {
        fprintf (stdout, "\n");
    }

    // Record Format
    fprintf (stdout, "  Record format:      ");
    DecodeRecordFormat (pcFileHeader->RECATTR ()[0], SWAPSHORT (pcFileHeader->RECSIZE ()[0]));
    fprintf (stdout, "\n");

    // Record Attributes
    fprintf (stdout, "  Record attributes:  ");
    DecodeRecordAttributes (pcFileHeader->RECATTR ()[1], psParameters);
    fprintf (stdout, "\n");

    // Seperate the File Entries
    fprintf (stdout, "\n");

    // Remove all allocations
    delete (szTempString1);
    delete (szTempString2);
#endif
}

void DumpCSVFileHeader (BSFileHeader* pcFileHeader, BRHeader* pcHeader, SBackupParameters* psParameters)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Output the File Name
    fprintf (stdout, "\"%s\",", pcFileHeader->FILENAME ());

    // Output the File Size
    if (SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[6]) == 0)
    {
        fprintf (stdout, "%i,%i,",
        SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[5]) - 1,
        SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));
    }
    else
    {
        fprintf (stdout, "%i,%i,",
        SWAPSHORT (((uint16_t*) pcFileHeader->RECATTR ())[5]) - 0,
        SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));
    }

    // Output the Creation Date
    fprintf (stdout, "\"");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->CREDATE (), 7);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");

    // Output the Owner
    fprintf (stdout, ",%06o,%06o,", SWAPSHORT (((uint16_t*) pcFileHeader->UIC ())[1]),
                                    SWAPSHORT (((uint16_t*) pcFileHeader->UIC ())[0])   );

    // Output the Revised Date
    fprintf (stdout, "\"");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->REVDATE (), 7);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");
    fprintf (stdout, "%i,", SWAPSHORT (*((uint16_t*) pcFileHeader->REVISION ())));

    // Output the File Id
    fprintf (stdout, "%i,%i,%i,",   SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[0]),
                                    SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[1]),
                                    SWAPSHORT (((uint16_t*) pcFileHeader->FID ())[2])   );

    // Output the Expiry Date
    fprintf (stdout, "\"");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->EXPDATE (), 7);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");

    // Output the Backup Date
    fprintf (stdout, "\"");
    DecodeTimeAndDate ((uint16_t*) pcFileHeader->BAKDATE (), 7);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");

    // Output File Protection
    fprintf (stdout, "\"");
    DecodeFileProtection (pcFileHeader->FPRO ()[0] & 0xF);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");
    fprintf (stdout, "\"");
    DecodeFileProtection (pcFileHeader->FPRO ()[0] >> 4);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");
    fprintf (stdout, "\"");
    DecodeFileProtection (pcFileHeader->FPRO ()[1] & 0xF);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");
    fprintf (stdout, "\"");
    DecodeFileProtection (pcFileHeader->FPRO ()[1] >> 4);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");

    // Output File Organisation
    //! Not enough data collated to determine other enumerations
    if (!pcHeader->V_NONSEQUENTIAL ())
    {
        fprintf (stdout, "Sequential,");
    }
    else
    {
        fprintf (stdout, "????,");
    }

    // File Attributes

    // File Attributes - Allocation
    fprintf (stdout, "%i,", SWAPLONG (((uint32_t*) pcFileHeader->FILESIZE ())[0]));

    // File Attributes - Extend
    //! No Idea, always 0
    fprintf (stdout, "%i,", 0);

    // File Attributes - Global Buffer Count
    //! No Idea, always 0
    fprintf (stdout, "%i,", 0);

    // File Attributes - ????
    //! No Idea, sometimes ", Contiguous-best-try", the check below seems to
    //! work for test data
    if (SWAPSHORT (((uint16_t*) pcFileHeader->UCHAR ())[0]) == 32)
    {
        fprintf (stdout, "Contiguous-best-try,");
    }
    else
    {
        fprintf (stdout, ",");
    }

    // Record Format
    fprintf (stdout, "\"");
    DecodeRecordFormat (pcFileHeader->RECATTR ()[0], pcFileHeader->RECSIZE ()[0]);
    fprintf (stdout, "\"");
    fprintf (stdout, ",");

    // Record Attributes
    fprintf (stdout, "\"");
    DecodeRecordAttributes (pcFileHeader->RECATTR ()[1], psParameters);
    fprintf (stdout, "\"");

    // Seperate the File Entries
    fprintf (stdout, "\n");
}

bool FindTargetFile (char* szFileName, SFileLinkedList* psFileList, SFileLinkedList** psFoundFileList)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Entry Found
    bool                bEntryFound         = false;
    bool                bNoEntry            = false;

    // Check if this is the first entry
    if (psFileList->pNext == NULL)
    {
        // Create the first entry
        *psFoundFileList                = psFileList;
    }
    else
    {
        *psFoundFileList                = psFileList->pNext;

        while (!bNoEntry && !bEntryFound)
        {
            if (strcmp ((*psFoundFileList)->szFileNameNoVer, szFileName) == 0)
            {
                bEntryFound = true;
            }
            else
            {
                if ((*psFoundFileList)->pNext != NULL)
                {
                    *psFoundFileList                = (*psFoundFileList)->pNext;
                }
                else
                {
                    bNoEntry                        = true;
                }
            }
        }
    }

    return bEntryFound;
}

// It is assumed by the time this routine is called, that SetNewerFile will
// have parsed the entire file list to construct a list of latest versions
bool IsTargetFile (char* szFileName, char* szTargetExtractMaskVersion, int iTargetExtractVersion, SFileLinkedList* psFileList)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Extracted Data
    char*               szExtractedName     = NULL;
    int                 iExtractedVersion   = 0;

    bool                bTarget             = false;

    // Dynamic Entry into the Linked List
    SFileLinkedList*    pBuffer;

    // Extract the File Name and Version Number
    szExtractedName = new char[strlen (szFileName) + 1];
    strcpy (szExtractedName, szFileName);

    if (strstr (szExtractedName, ";") != NULL)
    {
        *strstr (szExtractedName, ";")  = '\0';

        iExtractedVersion   = strtol (strstr (szFileName, ";")+1, NULL, 10);
    }
    else
    {
        // Impossible to find the version number
        iExtractedVersion   = 0;
    }

    if (FindTargetFile (szExtractedName, psFileList, &pBuffer))
    {
        if (strcmp (szTargetExtractMaskVersion, "*") == 0)
        {
            // An asterisk indicates all versions
            bTarget = true;
        }
        else
        {
            // Determine if this version is correct
            if (iTargetExtractVersion > 0)
            {
                // Version must be exact
                bTarget = iTargetExtractVersion == iExtractedVersion;
            }
            else
            {
                // This is a relative version
                // So ;0 means it must be the latest version
                //    ;-1 means it must be the version prior to the latest
                // etc.
                bTarget = iExtractedVersion == (pBuffer->iVersion + iTargetExtractVersion);
            }
        }
    }

    return bTarget;
}

void SetNewerFile (char* szFileName, SFileLinkedList* psFileList)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Dynamic Entry into the Linked List
    SFileLinkedList*    pBuffer;

    // Extracted Data
    char*               szExtractedName     = NULL;
    int                 iExtractedVersion   = 0;

    // Later Version
    bool                bLaterVersion       = false;

    // Extract the File Name and Version Number
    szExtractedName = new char[strlen (szFileName) + 1];
    strcpy (szExtractedName, szFileName);

    if (strstr (szExtractedName, ";") != NULL)
    {
        *strstr (szExtractedName, ";")  = '\0';

        iExtractedVersion   = strtol (strstr (szFileName, ";")+1, NULL, 10);
    }
    else
    {
        // Impossible to find the version number
        iExtractedVersion   = 0;
    }

    if (!FindTargetFile (szExtractedName, psFileList, &pBuffer))
    {
        // By Default this must be the latest version
        bLaterVersion                   = true;

        pBuffer->pNext                  = new SFileLinkedList;

        pBuffer                         = pBuffer->pNext;
        pBuffer->szFileNameNoVer        = new char[strlen (szExtractedName) + 1];
        strcpy (pBuffer->szFileNameNoVer, szExtractedName);
        pBuffer->iVersion               = iExtractedVersion;

        pBuffer->pNext                  = NULL;
    }
    else
    {
        // Determine if this is a later version
        bLaterVersion                   = iExtractedVersion >= pBuffer->iVersion;

        if (bLaterVersion)
        {
            pBuffer->iVersion               = iExtractedVersion;
        }
    }

    delete (szExtractedName);
}

void SetFileMode (char* szFileName, STypeLinkedList* psTypeList, bool bASCII)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Dynamic Entry into the Linked List
    STypeLinkedList*    pBuffer;

    // Entry Found
    bool                bEntryFound         = false;
    bool                bNoEntry            = false;

    // Check if this is the first entry
    if (psTypeList->pNext == NULL)
    {
        // Create the first entry
        pBuffer                         = psTypeList;
    }
    else
    {
        pBuffer                         = psTypeList->pNext;

        while (!bNoEntry && !bEntryFound)
        {
            if (strcmp (pBuffer->szFileName, szFileName) == 0)
            {
                bEntryFound = true;
            }
            else
            {
                if (pBuffer->pNext != NULL)
                {
                    pBuffer                         = pBuffer->pNext;
                }
                else
                {
                    bNoEntry                        = true;
                }
            }
        }
    }

    if (bEntryFound)
    {
        pBuffer->bASCII         = bASCII;
    }
    else
    {
        pBuffer->pNext                  = new STypeLinkedList;

        pBuffer                         = pBuffer->pNext;
        pBuffer->szFileName             = new char[strlen (szFileName) + 1];
        strcpy (pBuffer->szFileName, szFileName);
        pBuffer->bASCII                 = bASCII;

        pBuffer->pNext                  = NULL;
    }
}

bool GetFileMode (char* szFileName, STypeLinkedList* psTypeList)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Dynamic Entry into the Linked List
    STypeLinkedList*    pBuffer;

    // Entry Found
    bool                bEntryFound         = false;
    bool                bNoEntry            = false;

    // Check if this is the first entry
    if (psTypeList->pNext == NULL)
    {
        // Create the first entry
        return false;
    }
    else
    {
        pBuffer                         = psTypeList->pNext;

        while (!bNoEntry && !bEntryFound)
        {
            if (strcmp (pBuffer->szFileName, szFileName) == 0)
            {
                bEntryFound = true;
            }
            else
            {
                if (pBuffer->pNext != NULL)
                {
                    pBuffer                         = pBuffer->pNext;
                }
                else
                {
                    bNoEntry                        = true;
                }
            }
        }
    }

    if (bEntryFound)
    {
        return pBuffer->bASCII;
    }
    else
    {
        return false;
    }
}

char* GetLatestFile (STypeLinkedList* psTypeList)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Dynamic Entry into the Linked List
    STypeLinkedList*    pBuffer;

    // Check if this is the first entry
    if (psTypeList->pNext == NULL)
    {
        // No entries yet (shouldn't happen)
        return (char*) "";
    }
    else
    {
        pBuffer                         = psTypeList->pNext;

        while (pBuffer->pNext != NULL)
        {
            pBuffer                         = pBuffer->pNext;
        }
    }

    return pBuffer->szFileName;
}

void ProcessFile (uint8_t* cBuffer, BRHeader* pcHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, SFileLinkedList* psFileList, STypeLinkedList* psTypeList)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // File Header
    BSFileHeader    cFileHeader;

    // Modified File Name
    char*           szModifiedFileName;

    // Target File
    bool            bTargetFile             = false;

    // Wild Card Match
    bool            bWildCardMatch          = true;

    // File Name no mask
    char*           szFileNameNoMask;

    //////////////////////////////////////////////////////////////////////////
    // Convert the File Record into a series of streams

    cFileHeader.LoadBSFileHeader    (cBuffer, pcHeader->W_RSIZE ());

    // Allocate and copy the File Name
    szFileNameNoMask    = new char[strlen((char*) cFileHeader.FILENAME ())+1];
    strcpy (szFileNameNoMask, (char*) cFileHeader.FILENAME ());

    // Strip the Version Delimiter if required
    if (strstr (szFileNameNoMask, ";"))
    {
        *strstr (szFileNameNoMask, ";") = '\0';
    }

    // See if this is a file that needs processing
    bWildCardMatch                          = wildcardfit ( psParameters->szExtractMask,
                                                            szFileNameNoMask ) == 1;

    // Delete the Non-Masked Filename
    delete (szFileNameNoMask);

    //////////////////////////////////////////////////////////////////////////
    // Handle Older Versions

    if (bWildCardMatch)
    {
        // Add the file to the list, in order to generate a linked list of latest file versions only
        SetNewerFile ((char*) cFileHeader.FILENAME (), psFileList);
    }

    //////////////////////////////////////////////////////////////////////////
    // Handle Data Extraction if required

    if (psParameters->bExtract)
    {
        // Determine if this is a file we need to extract
        if (bWildCardMatch)
        {
            // Determine if this is the Target File
            bTargetFile                         = IsTargetFile (    (char*) cFileHeader.FILENAME (),
                                                                    psParameters->szExtractMaskVersion,
                                                                    psParameters->iExtractVersion,
                                                                    psFileList  );

            // Inform the Parser that it is to ignore the VBN if this is not
            // the target revision
            psParameters->bIgnoreVBN            = !bTargetFile;

            // If this is a first pass and Smart Scan is enabled
            if (psParameters->bExtractModeSmart && psParameters->bExtractFirstPass)
            {
                if (bTargetFile)
                {
                    // Default the File Mode to ASCII
                    psParameters->bExtractModeASCII     = true;
                    psParameters->bExtractModeBinary    = false;

                    SetFileMode ((char*) cFileHeader.FILENAME (), psTypeList, true);
                }

                //////////////////////////////////////////////////////////////////////
                // DEBUG (ENHANCED)
                if (psParameters->bExtractDebugEnhanced)
                {
                    // Output the Chosen Parameters
                    fprintf (stdout, "*** DEBUG *** ");

                    fprintf (stdout, "Beginning smart parse for %s\n", (char*) cFileHeader.FILENAME ());
                }
                //////////////////////////////////////////////////////////////////////
                // END DEBUG (ENHANCED)
            }
            else
            {
                //////////////////////////////////////////////////////////////////////
                // DEBUG (ENHANCED)
                if (psParameters->bExtractDebugEnhanced)
                {
                    // Output the Chosen Parameters
                    fprintf (stdout, "*** DEBUG *** ");

                    fprintf (stdout, "Beginning parse/extract for %s\n", (char*) cFileHeader.FILENAME ());
                }
                //////////////////////////////////////////////////////////////////////
                // END DEBUG (ENHANCED)
            }

            // Store the File Size Parameters
            psParameters->bLFDetected               = false;
            psParameters->uiFilePointer             = 0;
            psParameters->uiFileSize                = (SWAPSHORT (((uint16_t*) cFileHeader.RECATTR ())[5]) * 512) - 512 + SWAPSHORT (((uint16_t*) cFileHeader.RECATTR ())[6]);
            psParameters->cFormat                   = cFileHeader.RECATTR ()[0];
            psParameters->uiRemainingRecordLength   = 0;
            psParameters->uiRemainingStartPos       = 0;

            // If this is not the first pass (or we're in single pass mode)
            if (!psParameters->bExtractFirstPass && bTargetFile)
            {
                if (psParameters->bExtractModeSmart)
                {
                    psParameters->bExtractModeASCII     = GetFileMode ((char*) cFileHeader.FILENAME (), psTypeList);
                    psParameters->bExtractModeBinary    = !psParameters->bExtractModeASCII;
                }

                //////////////////////////////////////////////////////////////////////
                // DEBUG
                if (psParameters->bExtractDebug)
                {
                    // Output the Chosen Parameters
                    fprintf (stdout, "*** DEBUG *** ");

                    fprintf (stdout, "Using ");
                    if (psParameters->bExtractModeASCII)
                    {
                        fprintf (stdout, "ASCII");
                    }
                    else if (psParameters->bExtractModeBinary)
                    {
                        fprintf (stdout, "BINARY");
                    }
                    else if (psParameters->bExtractModeRaw)
                    {
                        fprintf (stdout, "RAW");
                    }

                    fprintf (stdout, " for %s\n", (char*) cFileHeader.FILENAME ());
                }
                //////////////////////////////////////////////////////////////////////
                // END DEBUG


                // Determine what the file name is going to be
                szModifiedFileName  = new char[strlen ((char*) cFileHeader.FILENAME ()) + 1];

                if (psParameters->bExtractFolder)
                {
                    // No processing needed (yet)
                    strcpy (szModifiedFileName, (char*) cFileHeader.FILENAME ());
                }
                else
                {
                    if (strstr ((char*) cFileHeader.FILENAME (), "]") != NULL)
                    {
                        strcpy (szModifiedFileName, strstr ((char*) cFileHeader.FILENAME (), "]") + 1);
                    }
                    else
                    {
                        strcpy (szModifiedFileName, (char*) cFileHeader.FILENAME ());
                    }
                }

                if (!psParameters->bExtractWithVersion)
                {
                    if (strstr (szModifiedFileName, ";") != NULL)
                    {
                        // Remove the Semi-Colon
                        *strstr (szModifiedFileName, ";")    = '\0';
                    }
                }

                // Replace any unusable characters with _'s
                for (unsigned i = 0; i < strlen (szModifiedFileName); i++)
                {
                    switch (szModifiedFileName[i])
                    {
                        case '\\':
                        case '/':
                        case ':':
                        case '*':
                        case '?':
                        case '\"':
                        case '<':
                        case '>':
                        case '|':
                        {
                            szModifiedFileName[i]   = '_';
                            break;
                        }

                        default:
                        {
                            break;
                        }
                    }
                }

                // Open the file for writing
                *ppCurrentOutputFile    = fopen (szModifiedFileName, "wb");

                // Remove the modified file name
                delete (szModifiedFileName);
            }
        }
    }
    else
    {
        // Determine if this is the Target File
        bTargetFile                         = IsTargetFile (    (char*) cFileHeader.FILENAME (),
                                                                psParameters->szExtractMaskVersion,
                                                                psParameters->iExtractVersion,
                                                                psFileList  );
    }

    //////////////////////////////////////////////////////////////////////////
    // Dump the contents of the File Record

    if (!psParameters->bExtractFirstPass && bWildCardMatch && bTargetFile)
    {
        if (psParameters->bOutputBrief)
        {
            DumpBriefFileHeader (&cFileHeader, 2);
        }
        else if (psParameters->bOutputFull)
        {
            DumpFullFileHeader (&cFileHeader, pcHeader, psParameters);
        }
        else if (psParameters->bOutputCSV)
        {
            DumpCSVFileHeader (&cFileHeader, pcHeader, psParameters);
        }
    }

    // Clean the File Record
    cFileHeader.Clean ();
}

void ProcessSummary (uint8_t* pcRecord)
{
}

void ProcessVBNRaw (uint8_t* cBuffer, BRHeader* pcHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, bool bIsRecord)
{
    if ((psParameters->uiFilePointer + pcHeader->W_RSIZE ()) < psParameters->uiFileSize)
    {
        VMSWriteFile (psParameters, pcHeader->W_RSIZE (), cBuffer, ppCurrentOutputFile, NULL, NULL);
        psParameters->uiFilePointer   = psParameters->uiFilePointer + pcHeader->W_RSIZE ();
    }
    else
    {
        // Curiously, a record seems to be off by 1 byte?
        if (bIsRecord)
        {
            VMSWriteFile (psParameters, psParameters->uiFileSize - psParameters->uiFilePointer - 1, cBuffer, ppCurrentOutputFile, NULL, NULL);
            psParameters->uiFilePointer   = psParameters->uiFilePointer + psParameters->uiFileSize - psParameters->uiFilePointer - 1;
        }
        else
        {
            VMSWriteFile (psParameters, psParameters->uiFileSize - psParameters->uiFilePointer, cBuffer, ppCurrentOutputFile, NULL, NULL);
            psParameters->uiFilePointer   = psParameters->uiFilePointer + psParameters->uiFileSize - psParameters->uiFilePointer;
        }
    }
}

void ProcessVBNNonVar (uint8_t* cBuffer, BRHeader* pcHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, STypeLinkedList* psTypeList)
{
    const char      szEndOfLine[2]      = {'\r','\n'};

    // Variables
    uint16_t        uiRecordPointer     = 0;

    if ((psParameters->uiFilePointer + pcHeader->W_RSIZE ()) < psParameters->uiFileSize)
    {
        if (psParameters->bExtractFirstPass)
        {
            // First Pass attempts to prove whether the file is an ASCII file or not
            for (uiRecordPointer = 0; uiRecordPointer < pcHeader->W_RSIZE (); uiRecordPointer++)
            {
                if (cBuffer[uiRecordPointer] > 0x7F)
                {
                    psParameters->bExtractModeASCII = false;
                    psParameters->bIgnoreVBN        = true;
                }
            }
        }
        else
        {
            VMSWriteFile (psParameters, pcHeader->W_RSIZE (), cBuffer, ppCurrentOutputFile, NULL, NULL);
        }

        psParameters->uiFilePointer = psParameters->uiFilePointer + pcHeader->W_RSIZE ();
    }
    else
    {
        if (psParameters->bExtractFirstPass)
        {
            // First Pass attempts to prove whether the file is an ASCII file or not
            for (uiRecordPointer = 0; uiRecordPointer < (uint16_t) (psParameters->uiFileSize - psParameters->uiFilePointer); uiRecordPointer++)
            {
                if (cBuffer[uiRecordPointer] > 0x7F)
                {
                    psParameters->bExtractModeASCII = false;
                    psParameters->bIgnoreVBN        = true;
                }
            }
        }
        else
        {
            VMSWriteFile (psParameters, psParameters->uiFileSize - psParameters->uiFilePointer, cBuffer, ppCurrentOutputFile, NULL, NULL);
        }

        psParameters->uiFilePointer = psParameters->uiFilePointer + (psParameters->uiFileSize - psParameters->uiFilePointer);

        VMSWriteEOL (psParameters, ppCurrentOutputFile, false);
    }

    if (!psParameters->bExtractModeASCII && psParameters->bExtractFirstPass &&psParameters->bExtractModeSmart)
    {
        psParameters->bExtractModeBinary    = true;
        SetFileMode (GetLatestFile (psTypeList), psTypeList, false);
    }
}

void ProcessVBNVar (uint8_t* cBuffer, BRHeader* pcHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, STypeLinkedList* psTypeList)
{
    // Variables
    uint16_t        uiRecordPointer     = 0;
    uint16_t        uiRecordPointerBuff = 0;
    uint16_t        uiRecordLength;
    uint16_t        uiRecordLengthModifier;
    bool            bSkipHeader;
    bool            bLastElementWasLFCR = false;
    bool            bContainsLFCR       = false;

    bSkipHeader = psParameters->cFormat == RECORD_FORMAT_VFC;

    if (bSkipHeader)
    {
        uiRecordLengthModifier  = 2;
    }
    else
    {
        uiRecordLengthModifier  = 0;
    }

    // Set the Starting Position of the Record and shift the file pointer
    // (this handles the scenario whereby a header might span a record
    //  but not actually contain any length data)
    uiRecordPointer                     = psParameters->uiRemainingStartPos;
    psParameters->uiFilePointer         = psParameters->uiFilePointer +
                                          psParameters->uiRemainingStartPos;

    // Handle a record spanning a block
    if (psParameters->uiRemainingRecordLength > 0)
    {
        if (psParameters->bExtractFirstPass)
        {
            // First Pass requires the file to be scanned as long as it's deemed an ASCII file
            for (uiRecordPointer; (uiRecordPointer-psParameters->uiRemainingStartPos) < psParameters->uiRemainingRecordLength; uiRecordPointer++)
            {
                if (psParameters->bExtractModeASCII)
                {
                    if (cBuffer[uiRecordPointer] > 0x7F)
                    {
                        psParameters->bExtractModeASCII = false;
                    }
                }
            }
        }
        else
        {
            VMSWriteFile (psParameters, psParameters->uiRemainingRecordLength, &cBuffer[uiRecordPointer], ppCurrentOutputFile, &bLastElementWasLFCR, &bContainsLFCR);

            uiRecordPointer = uiRecordPointer + psParameters->uiRemainingRecordLength;
        }

        psParameters->uiFilePointer             =   psParameters->uiFilePointer +
                                                    psParameters->uiRemainingRecordLength;
        psParameters->uiRemainingStartPos       = 0;
        psParameters->uiRemainingRecordLength   = 0;

        // Record Pointers aren't allowed to finish on an odd byte
        if ((uiRecordPointer % 2) != 0)
        {
            uiRecordPointer++;
            psParameters->uiFilePointer++;
        }

        if (!psParameters->bExtractFirstPass)
        {
            VMSWriteEOL (psParameters, ppCurrentOutputFile, !bLastElementWasLFCR);
        }
    }

    // Reset the Reamining Start Position
    psParameters->uiRemainingStartPos   = 0;

    while ( (uiRecordPointer < pcHeader->W_RSIZE ()) && (psParameters->uiFilePointer < psParameters->uiFileSize) )
    {
        uiRecordLength  = SWAPSHORT (*((uint16_t*) &cBuffer[uiRecordPointer])) - uiRecordLengthModifier;
        uiRecordPointer = uiRecordPointer + 2 + uiRecordLengthModifier;

        if (uiRecordPointer <= pcHeader->W_RSIZE ())
        {
            if ((uiRecordPointer + uiRecordLength) >= pcHeader->W_RSIZE ())
            {
                psParameters->uiRemainingStartPos       =   0;
                psParameters->uiRemainingRecordLength   =   uiRecordLength;
                uiRecordLength                          =   pcHeader->W_RSIZE () -
                                                            uiRecordPointer;
                psParameters->uiRemainingRecordLength   =   psParameters->uiRemainingRecordLength -
                                                            uiRecordLength;
            }

            if (psParameters->bExtractFirstPass)
            {
                // First Pass requires the file to be scanned as long as it's deemed an ASCII file
                uiRecordPointerBuff = uiRecordPointer;

                for (uiRecordPointer; uiRecordPointer < (uiRecordPointerBuff + uiRecordLength); uiRecordPointer++)
                {
                    if (psParameters->bExtractModeASCII)
                    {
                        if (cBuffer[uiRecordPointer] > 0x7F)
                        {
                            psParameters->bExtractModeASCII = false;
                        }
                    }
                }
            }
            else
            {
                VMSWriteFile (psParameters, uiRecordLength, &cBuffer[uiRecordPointer], ppCurrentOutputFile, &bLastElementWasLFCR, &bContainsLFCR);

                uiRecordPointer = uiRecordPointer + uiRecordLength;
            }

            psParameters->uiFilePointer     = psParameters->uiFilePointer + 2 + uiRecordLength + uiRecordLengthModifier;

            if ((uiRecordPointer % 2) != 0)
            {
                uiRecordPointer++;
                psParameters->uiFilePointer++;
            }

            if (!psParameters->bExtractFirstPass)
            {
                VMSWriteEOL (psParameters, ppCurrentOutputFile, (psParameters->uiRemainingRecordLength == 0) && !bLastElementWasLFCR);
            }
        }
        else
        {
            // The calculated file pointer is outside the RWIN size, which means
            // only the record header exists in this buffer, therefore transfer
            // the record data to be handled in the next buffer
            psParameters->uiFilePointer             = psParameters->uiFilePointer +
                                                      ( uiRecordPointer -
                                                        pcHeader->W_RSIZE ()    );

            psParameters->uiRemainingStartPos       = uiRecordPointer -
                                                      pcHeader->W_RSIZE ();
            psParameters->uiRemainingRecordLength   = uiRecordLength;

            // A Zero length record is basically a new line
            if (psParameters->uiRemainingRecordLength == 0)
            {
                if (!psParameters->bExtractFirstPass)
                {
                    VMSWriteEOL (psParameters, ppCurrentOutputFile, (psParameters->uiRemainingRecordLength == 0) && !bLastElementWasLFCR);
                }
            }
        }
    }

    if (!psParameters->bExtractModeASCII && !psParameters->bExtractModeBinary)
    {
        psParameters->bExtractModeBinary    = true;
        SetFileMode (GetLatestFile (psTypeList), psTypeList, false);
    }
}

void ProcessVBN (uint8_t* cBuffer, BRHeader* pcHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, STypeLinkedList* psTypeList)
{
    //////////////////////////////////////////////////////////////////////
    // DEBUG (ENHANCED)
    if (psParameters->bExtractDebugEnhanced)
    {
        // Output the Chosen Parameters
        fprintf (stdout, "*** DEBUG *** ");

        fprintf (stdout, "File Pointer = 0x%08X, File Size = 0x%08X\n", psParameters->uiFilePointer, psParameters->uiFileSize);
    }
    //////////////////////////////////////////////////////////////////////
    // END DEBUG (ENHANCED)

    switch (psParameters->cFormat)
    {
        case RECORD_FORMAT_VAR:
        case RECORD_FORMAT_VFC:
        {
            if (psParameters->bExtractModeASCII || psParameters->bExtractModeBinary)
            {
                ProcessVBNVar (cBuffer, pcHeader, psParameters, ppCurrentOutputFile, psTypeList);
            }
            else if (!psParameters->bExtractFirstPass)
            {
                // Process the record in Raw Mode
                ProcessVBNRaw (cBuffer, pcHeader, psParameters, ppCurrentOutputFile, true);
            }

            break;
        }

        case RECORD_FORMAT_FIX:
        case RECORD_FORMAT_STM:
        case RECORD_FORMAT_STMLF:
        case RECORD_FORMAT_STMCR:
        {
            if (psParameters->bExtractModeASCII)
            {
                ProcessVBNNonVar (cBuffer, pcHeader, psParameters, ppCurrentOutputFile, psTypeList);
            }
            else
            {
                // Process the record in Raw Mode
                ProcessVBNRaw (cBuffer, pcHeader, psParameters, ppCurrentOutputFile, false);
            }

            break;
        }

        case RECORD_FORMAT_UDF:
        {
            if (!psParameters->bExtractFirstPass)
            {
                // Process the record in Raw Mode
                ProcessVBNRaw (cBuffer, pcHeader, psParameters, ppCurrentOutputFile, false);
            }

            break;
        }

        default:
        {
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////
    // DEBUG (ENHANCED)
    if (psParameters->bExtractDebugEnhanced)
    {
        // Output the Chosen Parameters
        fprintf (stdout, "*** DEBUG *** ");

        fprintf (stdout, "File Pointer = 0x%08X, File Size = 0x%08X\n", psParameters->uiFilePointer, psParameters->uiFileSize);
    }
    //////////////////////////////////////////////////////////////////////
    // END DEBUG (ENHANCED)
}

bool ProcessBackupSaveSetRecord (uint8_t* pcBlock, uint32_t *uiAddress, BBHeader* pcHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, SFileLinkedList* psFileList, STypeLinkedList* psTypeList, bool bLastBlock)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Read the Common Header Component
    BRHeader    cRecordHeader;

    // Read the Header
    cRecordHeader.LoadBRHeader (pcBlock);

    // Skip past the Record Header
    *uiAddress   = *uiAddress + cRecordHeader.GetLength ();

    if (cRecordHeader.W_RSIZE () == 0)
    {
        *uiAddress  = pcHeader->L_BLOCKSIZE ();
        return true;
    }

    if (    (cRecordHeader.W_RTYPE () != RECORD_VBN) &&
            (cRecordHeader.W_RTYPE () != RECORD_VOLUME) &&
            (cRecordHeader.W_RTYPE () != RECORD_NULL)
       )
    {
        // Close any open files
        if (*ppCurrentOutputFile != NULL)
        {
            fclose (*ppCurrentOutputFile);
            *ppCurrentOutputFile    = NULL;
        }
    }

    switch (cRecordHeader.W_RTYPE ())
    {
        case RECORD_NULL:
        {
            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "NULL RECORD\n");
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            break;
        }

        case RECORD_SUMMARY:
        {
            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "SUMMARY RECORD\n");
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            if (psParameters->bOutputFull)
            {
                ProcessSummary (&pcBlock[cRecordHeader.GetLength ()]);
            }
            break;
        }

        case RECORD_VOLUME:
        {
            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "VOLUME RECORD\n");
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            *uiAddress  = pcHeader->L_BLOCKSIZE ();
            return true;
            break;
        }

        case RECORD_FILE:
        {
            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "FILE RECORD\n");
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            ProcessFile (   &pcBlock[cRecordHeader.GetLength ()],
                            &cRecordHeader,
                            psParameters,
                            ppCurrentOutputFile,
                            psFileList,
                            psTypeList  );
            break;
        }

        case RECORD_VBN:
        {
            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "VBN RECORD\n");
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            if (!psParameters->bIgnoreVBN && ((*ppCurrentOutputFile != NULL) || (psParameters->bExtractFirstPass && psParameters->bExtractModeSmart)))
            {
                ProcessVBN  (   &pcBlock[cRecordHeader.GetLength ()],
                                &cRecordHeader,
                                psParameters,
                                ppCurrentOutputFile,
                                psTypeList  );
            }

            break;
        }

        default:
        {
            if (!psParameters->bExtractFirstPass && (!bLastBlock || psParameters->bExtractDebug))
            {
                fprintf (stderr, "WARNING : Invalid Record (%i).\n", cRecordHeader.W_RTYPE ());
                fprintf (stderr, "WARNING : Invalid Record (%i).\n", cRecordHeader.W_RSIZE ());
            }

            return false;
            break;
        }
    }

    *uiAddress  = *uiAddress + cRecordHeader.W_RSIZE ();

    cRecordHeader.Clean ();

    return true;
}

bool ProcessBlock (uint8_t* cBlock, BBHeader* pcBlockHeader, SBackupParameters* psParameters, FILE** ppCurrentOutputFile, SFileLinkedList* psFileList, STypeLinkedList* psTypeList, uint32_t uiBaseAddress, uint32_t uiMaxAddress)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Block Address
    uint32_t            uiBlockAddress          = 0;

    // Last Block Address
    uint32_t            uiLastBlockAddress      = 0;

    // Block Address prior to the Last Block Address
    uint32_t            uiLastLastBlockAddress  = 0;

    // Cumulative validity of this block
    bool                bValid                  = true;

    // Last Block?
    bool                bLastBlock;

    // Current Block Header
    BBHeader            cCurrentHeader;

    cCurrentHeader.LoadBBHeader (cBlock);

    // Validate the Size
    if (cCurrentHeader.Validate (pcBlockHeader))
    {
        // Skip the Block Header
        uiBlockAddress  = uiBlockAddress + pcBlockHeader->GetLength ();

        // Determine if this is the last block
        // This forms a little bit of a kludge, but it seems that
        // occasionally backup files basically have garbage at the tail end
        // of the last block, so errors on the last block will be handled,
        // but suppressed from all but debug.
        bLastBlock = (uiMaxAddress - uiBaseAddress) <= pcBlockHeader->L_BLOCKSIZE ();

        // Iterate through the current Block
        while ((uiBlockAddress < cCurrentHeader.L_BLOCKSIZE ()) && bValid)
        {
            // Preserve the Prior to Last Block Address for debugging
            uiLastLastBlockAddress  = uiLastBlockAddress;

            // Preserve the Last Block Address for debugging
            uiLastBlockAddress      = uiBlockAddress;

            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "Processing Record Address : 0x%08X\n", uiBaseAddress + uiBlockAddress);
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            // Process the Record
            bValid                  = ProcessBackupSaveSetRecord (  &cBlock[uiBlockAddress],
                                                                    &uiBlockAddress,
                                                                    pcBlockHeader,
                                                                    psParameters,
                                                                    ppCurrentOutputFile,
                                                                    psFileList,
                                                                    psTypeList,
                                                                    bLastBlock  );

            // Output Errors if the Record is invalid
            if (!bValid)
            {
                // Close any open files
                if (*ppCurrentOutputFile != NULL)
                {
                    fclose (*ppCurrentOutputFile);
                    *ppCurrentOutputFile    = NULL;
                }

                if (!psParameters->bExtractFirstPass)
                {
                    if (psParameters->bExtractDebug)
                    {
                        fprintf (stdout, "*** START DEBUG ***\n");

                        fprintf (stdout, "Last Valid Record Address : 0x%08X\n", uiLastLastBlockAddress);
                        fprintf (stdout, "Invalid Record Address    : 0x%08X\n", uiLastBlockAddress);

                        fprintf (stdout, "*** END DEBUG ***\n");
                    }
                }
            }
        }
    }
    else
    {
        // Close any open files
        if (*ppCurrentOutputFile != NULL)
        {
            fclose (*ppCurrentOutputFile);
            *ppCurrentOutputFile    = NULL;
        }

        if (!psParameters->bExtractFirstPass)
        {
            fprintf (stderr, "WARNING : Invalid block size %i Read, %i Expected.  Skipping block\n", cCurrentHeader.L_BLOCKSIZE (), pcBlockHeader->L_BLOCKSIZE ());
        }
    }

    cCurrentHeader.Clean ();

    return bValid || (bLastBlock && !psParameters->bExtractDebug);
}

// Brief explanation of main loop.
//
// The basic format of the OpenVMS Backup format is as follows :
//
// Block            Record 1
//
//                  End Record 1
//                  ...
//                  Record N
//
//                  End Record N
// End Block
// Block            Record 1
//
//                  End Record 1
//                  ...
//                  Record N
//
//                  End Record N
// End Block
//
// In that a block should be parsed sequentially, but in isolation to other
// blocks.  A block may contain 0 to many records, and in addition, each block
// should be validated based on its header.  With the first blocks header
// setting the size of the rest.  A block size can be 0 (in which case the
// entire block is disregarded) or the pre-set blocksize.  Any other size
// denotes a corrupted / invalid entry.

bool ProcessBackup (FILE* pFile, SBackupParameters* psParameters)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Current Output File
    FILE*               pCurrentOutputFile      = NULL;

    // Pointer to the Start of the File
    int32_t             pAbsStart;

    // Pointer to the End of the Fiel
    int32_t             pAbsEnd;

    // First Block Header
    BBHeader            cBlockHeader;

    // Block Data
    uint8_t*            cBlock;

    // Current File Position
    int32_t             pBlock;

    // Flag indicating the cumulative validity of the Backup Set
    bool                bValid                  = true;

    // Flag indicating a 2nd pass is needed
    bool                bSecondPass             = false;

    // File Linked List
    SFileLinkedList     sFileList;

    // Type Linked List
    STypeLinkedList     sTypeList;

    //////////////////////////////////////////////////////////////////////////
    // Default the File Linked List

    sFileList.szFileNameNoVer               = NULL;
    sFileList.iVersion                      = 0;
    sFileList.pNext                         = NULL;

    //////////////////////////////////////////////////////////////////////////
    // Default the Type Linked List

    sTypeList.szFileName                    = NULL;
    sTypeList.bASCII                        = false;
    sTypeList.pNext                         = NULL;

    //////////////////////////////////////////////////////////////////
    // Process the Backup Set

    // Buffer the File Start
    pAbsStart = ftell (pFile);

    // Load the Block Header
    cBlockHeader.LoadBBHeader (pFile);

    if (!psParameters->bOutputSuppress)
    {
        DumpHeader (&cBlockHeader, psParameters);
    }

    // Allocate the Block Buffer based upon the block size defined in
    // the first header
    cBlock  = NULL;
    cBlock  = new uint8_t[cBlockHeader.L_BLOCKSIZE ()];

    if (cBlock == NULL)
    {
        fprintf (stderr, "CRITICAL ERROR\n");
        return 0;
    }

    // Point the file buffer to the end
    fseek (pFile, 0, SEEK_END);
    pAbsEnd = ftell (pFile);

    // Point the file buffer back to the start
    fseek (pFile, pAbsStart, SEEK_SET);

    //////////////////////////////////////////////////////////
    // DEBUG
    if (psParameters->bExtractDebug)
    {
        fprintf (stdout, "*** DEBUG *** ");

        // Output the Block Address for debugging purposes
        fprintf (stdout, "STARTING FIRST FILE PARSE\n");
    }
    //////////////////////////////////////////////////////////
    // /DEBUG

    do
    {
        // Force the Algorithm to valid (needed if this is a 2nd pass)
        bValid  = true;

        // Process the entire file
        while (!feof (pFile) && bValid)
        {
            // Read the Current Block
            pBlock  = ftell (pFile);
            fread (cBlock, cBlockHeader.L_BLOCKSIZE (), sizeof(uint8_t), pFile);

            //////////////////////////////////////////////////////////////////////
            // DEBUG (ENHANCED)
            if (psParameters->bExtractDebugEnhanced)
            {
                // Output the Chosen Parameters
                fprintf (stdout, "*** DEBUG *** ");

                fprintf (stdout, "Processing Block Address  : 0x%08X\n", pBlock);
            }
            //////////////////////////////////////////////////////////////////////
            // END DEBUG (ENHANCED)

            // Process the current block
            if (!feof (pFile))
            {
                bValid  = ProcessBlock (cBlock, &cBlockHeader, psParameters, &pCurrentOutputFile, &sFileList, &sTypeList, pBlock, pAbsEnd);
            }

            if (!bValid && !psParameters->bExtractFirstPass)
            {
                //////////////////////////////////////////////////////////
                // DEBUG
                if (psParameters->bExtractDebug)
                {
                    fprintf (stdout, "*** DEBUG *** ");

                    // Output the Block Address for debugging purposes
                    fprintf (stdout, "Block Address             : 0x%08X\n", pBlock - pAbsStart);
                }
                //////////////////////////////////////////////////////////
                // /DEBUG
            }
        }

        if (psParameters->bExtractFirstPass)
        {
            // Indicate this is no longer the first pass
            psParameters->bExtractFirstPass   = false;

            // Point the file buffer back to the start
            fseek (pFile, pAbsStart, SEEK_SET);

            // Indicate a 2nd Pass is needed
            bSecondPass = true;

            //////////////////////////////////////////////////////////
            // DEBUG
            if (psParameters->bExtractDebug)
            {
                fprintf (stdout, "*** DEBUG *** ");

                // Output the Block Address for debugging purposes
                fprintf (stdout, "STARTING SECOND FILE PARSE\n");
            }
            //////////////////////////////////////////////////////////
            // /DEBUG
        }
        else
        {
            bSecondPass = false;
        }
    } while (bSecondPass);

    // Debug : If the block was invalid, dump the block
    if (!bValid)
    {
        //////////////////////////////////////////////////////////
        // DEBUG
        if (psParameters->bExtractDebug)
        {
            fprintf (stdout, "*** START DEBUG ***\n");

            fprintf (stdout, "Invalid Data Dump\n");
            for (uint32_t i = 0; i < cBlockHeader.L_BLOCKSIZE (); i++)
            {
                fprintf (stdout, "0x%04X :: 0x%02X\n", i, (uint8_t) cBlock[i]);
            }

            fprintf (stdout, "*** END DEBUG ***\n");
        }
    }

    // Close any remaining files
    if (pCurrentOutputFile != NULL)
    {
        fclose (pCurrentOutputFile);
        pCurrentOutputFile    = NULL;
    }

    // Clean Up any remaining allocated data
    cBlockHeader.Clean ();
    delete (cBlock);

    return bValid;
}
