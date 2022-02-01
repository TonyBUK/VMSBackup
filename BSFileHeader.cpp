#include <memory.h>
#include "BSFileHeader.h"

//////////////////////////////////////////////////////////////////////////////
// Block Header

// Header Iteration is interesting...
//
// The premise is pretty simple :
// The File Header is really a collection of file alias'
// Size / Type / Data
// Whilst this is typically ordered, this order cannot be relied upon, instead, a simple
// iteration

#define K_FILENAME 42               /* file name                        */
#define K_STRUCLEV 43               /* file structure level             */
#define K_FID 44                    /* file ID                          */
#define K_BACKLINK 45               /* directory ID back link           */
#define K_FILESIZE 46               /* file size in blocks              */
#define K_UIC 47                    /* file owner UIC                   */
#define K_FPRO 48                   /* file protection mask             */
#define K_RPRO 49                   /* record protection mask           */
#define K_ACLEVEL 50                /* access level                     */
#define K_UCHAR 51                  /* file characteristics             */
#define K_RECATTR 52                /* record attributes area           */
#define K_REVISION 53               /* revision number                  */
#define K_CREDATE 54                /* creation date                    */
#define K_REVDATE 55                /* revision date                    */
#define K_EXPDATE 56                /* expiration date                  */
#define K_BAKDATE 57                /* backup date                      */

#define K_VERLIMIT 75               /* (FA) File version limit          */
#define K_HIGHWATER 79              /* (FA) Highwater mark              */

BSFileHeader::BSFileHeader ()
{
    m_pcFILENAME        = NULL;
    m_pcSTRUCLEV        = NULL;
    m_pcFID             = NULL;
    m_pcBACKLINK        = NULL;
    m_pcFILESIZE        = NULL;
    m_pcUIC             = NULL;
    m_pcFPRO            = NULL;
    m_pcRPRO            = NULL;
    m_pcACLEVEL         = NULL;
    m_pcUCHAR           = NULL;
    m_pcRECATTR         = NULL;
    m_pcREVISION        = NULL;
    m_pcCREDATE         = NULL;
    m_pcREVDATE         = NULL;
    m_pcEXPDATE         = NULL;
    m_pcBAKDATE         = NULL;

    m_pcVERLIMIT        = NULL;
    m_pcHIGHWATER       = NULL;
}

void BSFileHeader::Clean ()
{
    if (m_pcFILENAME)
    {
        m_pcFILENAME->Clean ();
        delete (m_pcFILENAME);
        m_pcFILENAME        = NULL;
    }

    if (m_pcSTRUCLEV)
    {
        m_pcSTRUCLEV->Clean ();
        delete (m_pcSTRUCLEV);
        m_pcSTRUCLEV        = NULL;
    }

    if (m_pcFID)
    {
        m_pcFID->Clean ();
        delete (m_pcFID);
        m_pcFID        = NULL;
    }

    if (m_pcBACKLINK)
    {
        m_pcBACKLINK->Clean ();
        delete (m_pcBACKLINK);
        m_pcBACKLINK        = NULL;
    }

    if (m_pcFILESIZE)
    {
        m_pcFILESIZE->Clean ();
        delete (m_pcFILESIZE);
        m_pcFILESIZE        = NULL;
    }

    if (m_pcUIC)
    {
        m_pcUIC->Clean ();
        delete (m_pcUIC);
        m_pcUIC        = NULL;
    }

    if (m_pcFPRO)
    {
        m_pcFPRO->Clean ();
        delete (m_pcFPRO);
        m_pcFPRO        = NULL;
    }

    if (m_pcRPRO)
    {
        m_pcRPRO->Clean ();
        delete (m_pcRPRO);
        m_pcRPRO        = NULL;
    }

    if (m_pcACLEVEL)
    {
        m_pcACLEVEL->Clean ();
        delete (m_pcACLEVEL);
        m_pcACLEVEL        = NULL;
    }

    if (m_pcUCHAR)
    {
        m_pcUCHAR->Clean ();
        delete (m_pcUCHAR);
        m_pcUCHAR        = NULL;
    }

    if (m_pcRECATTR)
    {
        m_pcRECATTR->Clean ();
        delete (m_pcRECATTR);
        m_pcRECATTR        = NULL;
    }

    if (m_pcREVISION)
    {
        m_pcREVISION->Clean ();
        delete (m_pcREVISION);
        m_pcREVISION        = NULL;
    }

    if (m_pcCREDATE)
    {
        m_pcCREDATE->Clean ();
        delete (m_pcCREDATE);
        m_pcCREDATE        = NULL;
    }

    if (m_pcREVDATE)
    {
        m_pcREVDATE->Clean ();
        delete (m_pcREVDATE);
        m_pcREVDATE        = NULL;
    }

    if (m_pcEXPDATE)
    {
        m_pcEXPDATE->Clean ();
        delete (m_pcEXPDATE);
        m_pcEXPDATE        = NULL;
    }

    if (m_pcBAKDATE)
    {
        m_pcBAKDATE->Clean ();
        delete (m_pcBAKDATE);
        m_pcBAKDATE        = NULL;
    }

    if (m_pcVERLIMIT)
    {
        m_pcVERLIMIT->Clean ();
        delete (m_pcVERLIMIT);
        m_pcVERLIMIT       = NULL;
    }

    if (m_pcHIGHWATER)
    {
        m_pcHIGHWATER->Clean ();
        delete (m_pcHIGHWATER);
        m_pcHIGHWATER      = NULL;
    }
}

void BSFileHeader::LoadBSFileHeader (uint8_t* cBuffer, uint32_t uiRSize)
{
    uint32_t        uiAddress   = 2;

    while (uiAddress < uiRSize)
    {
        // Initially the Block is unidentified
        m_pUnidentified = new BSAHeader;

        m_pUnidentified->LoadBSAHeader (&cBuffer[uiAddress]);

        // Increment the Address
        uiAddress = uiAddress + m_pUnidentified->GetLength ();

        switch (m_pUnidentified->W_TYPE ())
        {
            case K_FILENAME:
            {
                if (m_pcFILENAME == NULL)
                {
                    m_pcFILENAME    = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_STRUCLEV:
            {
                if (m_pcSTRUCLEV == NULL)
                {
                    m_pcSTRUCLEV    = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_FID:
            {
                if (m_pcFID == NULL)
                {
                    m_pcFID         = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_BACKLINK:
            {
                if (m_pcBACKLINK == NULL)
                {
                    m_pcBACKLINK    = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_FILESIZE:
            {
                if (m_pcFILESIZE == NULL)
                {
                    m_pcFILESIZE    = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_UIC:
            {
                if (m_pcUIC == NULL)
                {
                    m_pcUIC         = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_FPRO:
            {
                if (m_pcFPRO == NULL)
                {
                    m_pcFPRO        = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_RPRO:
            {
                if (m_pcRPRO == NULL)
                {
                    m_pcRPRO        = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_ACLEVEL:
            {
                if (m_pcACLEVEL == NULL)
                {
                    m_pcACLEVEL     = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_UCHAR:
            {
                if (m_pcUCHAR == NULL)
                {
                    m_pcUCHAR       = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_RECATTR:
            {
                if (m_pcRECATTR == NULL)
                {
                    m_pcRECATTR     = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_REVISION:
            {
                if (m_pcREVISION == NULL)
                {
                    m_pcREVISION    = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_CREDATE:
            {
                if (m_pcCREDATE == NULL)
                {
                    m_pcCREDATE     = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_REVDATE:
            {
                if (m_pcREVDATE == NULL)
                {
                    m_pcREVDATE     = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_EXPDATE:
            {
                if (m_pcEXPDATE == NULL)
                {
                    m_pcEXPDATE     = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_BAKDATE:
            {
                if (m_pcBAKDATE == NULL)
                {
                    m_pcBAKDATE     = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_VERLIMIT:
            {
                if (m_pcVERLIMIT == NULL)
                {
                    m_pcVERLIMIT   = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            case K_HIGHWATER:
            {
                if (m_pcHIGHWATER == NULL)
                {
                    m_pcHIGHWATER   = m_pUnidentified;
                }
                else
                {
                    delete (m_pUnidentified);
                }
                break;
            }

            default:
            {
                // Block can't be identified, so de-allocate
                m_pUnidentified->Clean ();
                delete (m_pUnidentified);

                break;
            }
        }
    }
}

uint8_t* BSFileHeader::FILENAME ()
{
    if (m_pcFILENAME)
    {
        return m_pcFILENAME->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::STRUCLEV ()
{
    if (m_pcSTRUCLEV)
    {
        return m_pcSTRUCLEV->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::FID ()
{
    if (m_pcFID)
    {
        return m_pcFID->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::BACKLINK ()
{
    if (m_pcBACKLINK)
    {
        return m_pcBACKLINK->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::FILESIZE ()
{
    if (m_pcFILESIZE)
    {
        return m_pcFILESIZE->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::UIC ()
{
    if (m_pcUIC)
    {
        return m_pcUIC->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::FPRO ()
{
    if (m_pcFPRO)
    {
        return m_pcFPRO->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::RPRO ()
{
    if (m_pcRPRO)
    {
        return m_pcRPRO->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::ACLEVEL ()
{
    if (m_pcACLEVEL)
    {
        return m_pcACLEVEL->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::UCHAR ()
{
    if (m_pcUCHAR)
    {
        return m_pcUCHAR->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::RECATTR ()
{
    if (m_pcRECATTR)
    {
        return m_pcRECATTR->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint16_t* BSFileHeader::RECSIZE ()
{
    if (m_pcRECATTR)
    {
        return (uint16_t*) &m_pcRECATTR->T_TEXT ()[2];
    }
    else
    {
        return (uint16_t*) 0;
    }
}

uint8_t* BSFileHeader::REVISION ()
{
    if (m_pcREVISION)
    {
        return m_pcREVISION->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::CREDATE ()
{
    if (m_pcCREDATE)
    {
        return m_pcCREDATE->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::REVDATE ()
{
    if (m_pcREVDATE)
    {
        return m_pcREVDATE->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::EXPDATE ()
{
    if (m_pcEXPDATE)
    {
        return m_pcEXPDATE->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::BAKDATE ()
{
    if (m_pcBAKDATE)
    {
        return m_pcBAKDATE->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::VERLIMIT ()
{
    if (m_pcVERLIMIT)
    {
        return m_pcVERLIMIT->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}

uint8_t* BSFileHeader::HIGHWATER ()
{
    if (m_pcHIGHWATER)
    {
        return m_pcHIGHWATER->T_TEXT ();
    }
    else
    {
        return (uint8_t*) "";
    }
}
