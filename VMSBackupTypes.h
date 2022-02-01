#ifndef __VMSBackupTypes_H__
#define __VMSBackupTypes_H__

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////
// Global Type Definitions

// Primarily command line parameters with some dynamic data logged during the
// file processing phases.
struct SBackupParameters
{
    // Verbose Options
    bool            bOutputSuppress;
    bool            bOutputBrief;
    bool            bOutputFull;
    bool            bOutputCSV;

    // Extraction Options

    // Extract
    bool            bExtract;

    // Manual selection of Extract Mode

    // Smart Mode auto selects ASCII/Binary per file based on the contents.

    // ASCII Mode will perform file interpretations related to end of line
    // protocols.

    // Binary Mode will extract the data without interpretation with the
    // exception of records, which shall strip the length fields.

    // Raw Mode will extract the data with NO interpreation, leaving records
    // with their length fields intact.
    bool            bExtractModeSmart;
    bool            bExtractModeASCII;
    bool            bExtractModeBinary;
    bool            bExtractModeRaw;

    // A 2 pass mechanism is required if :
    // 1/ Smart Parsing is in effect
    // 2/ Latest Revision Extacting is suppressed
    bool            bExtractFirstPass;
    bool            bIgnoreVBN;

    // Current File Data
    bool            bLFDetected;
    uint32_t        uiFilePointer;
    uint32_t        uiFileSize;
    int8_t          cFormat;
    uint16_t        uiRemainingStartPos;
    uint16_t        uiRemainingRecordLength;

    // Extract Mask
    char*           szExtractMask;
    char*           szExtractMaskVersion;
    int             iExtractVersion;

    // Folder Extract
    bool            bExtractFolder;

    // Extract Older Versions
    bool            bExtractWithVersion;

    // Debug Mode
    bool            bExtractDebug;
    bool            bExtractDebugEnhanced;
};

// Linked List used by the algorithm determining whether to extract a file
// revision
struct SFileLinkedList
{
    char*               szFileNameNoVer;
    int                 iVersion;

    SFileLinkedList*    pNext;
};

// Linked List used by the Smart Parser algorithm to determine whether a file
// is ASCII or Binary
struct STypeLinkedList
{
    char*               szFileName;
    bool                bASCII;

    STypeLinkedList*    pNext;
};

#endif // __VMSBackupTypes_H__
