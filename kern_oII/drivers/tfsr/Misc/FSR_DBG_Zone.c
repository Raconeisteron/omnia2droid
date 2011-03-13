/**
 *   @mainpage   Flex Sector Remapper : LinuStoreIII_1.2.0_b032-FSR_1.2.1p1_b129_RTM
 *
 *   @section Intro
 *       Flash Translation Layer for Flex-OneNAND and OneNAND
 *    
 *    @section  Copyright
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Copyright (C) 2003-2010 Samsung Electronics                               *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License version 2 as         *
 * published by the Free Software Foundation.                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *
 *     @section Description
 *
 */

/**
 * @file      FSR_DBG_Zone.c
 * @brief     This file contains FSR debug zone APIs
 * @author    SongHo Yoon
 * @date      11-JAN-2007
 * @remark
 * REVISION HISTORY
 * @n  11-JAN-2007 [SongHo Yoon] : modified from code of XSR
 *
 */

/*****************************************************************************/
/* header file inclusion                                                     */
/*****************************************************************************/
#define     FSR_NO_INCLUDE_BML_HEADER
#define     FSR_NO_INCLUDE_STL_HEADER

#include    "FSR.h"

/*****************************************************************************/
/* the local constant definitions                                            */
/*****************************************************************************/

/*****************************************************************************/
/* the local variable definitions                                            */
/*****************************************************************************/
/** default value is error */
PUBLIC volatile UINT32 gnFSRDbgZoneMask = FSR_DBZ_DEFAULT;

/*****************************************************************************/
/* the static function prototypes                                            */
/*****************************************************************************/

/*****************************************************************************/
/* the code implementation                                                   */
/*****************************************************************************/

/**
 * @fn              PUBLIC UINT32 FSR_DBG_GetDbgZoneMask(VOID)
 *
 * @return          current debug zone mask value
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 * @brief           This function gets the current debug zone mask
 *  
 */
PUBLIC UINT32
FSR_DBG_GetDbgZoneMask(VOID)
{
    return gnFSRDbgZoneMask;
}

/**
 * @brief           This function sets debug zone mask
 *
 * @param[in]      nMask : debug zone mask value
 *
 * @return         none
 *
 * @author          SongHo Yoon
 * @version         1.0.0
 *
 */
PUBLIC VOID
FSR_DBG_SetDbgZoneMask(UINT32  nMask)
{
    gnFSRDbgZoneMask |= nMask;
}

/**
 * @brief          This function unsets debug zone mask
 *
 * @param[in]      nMask : debug zone mask value
 *
 * @return         none
 *
 * @author         SongHo Yoon
 * @version        1.0.0
 *
 */
PUBLIC VOID
FSR_DBG_UnsetDbgZoneMask(UINT32  nMask)
{
    gnFSRDbgZoneMask ^= nMask;
}

/**
 * @brief          This function resets debug zone mask
 *
 * @return         none
 *
 * @author         SongHo Yoon
 * @version        1.0.0
 *
 */
PUBLIC VOID
FSR_DBG_ResetDbgZoneMask(VOID)
{
    gnFSRDbgZoneMask = FSR_DBZ_DEFAULT;
}

/**
 * @brief          This function sets all debug zone mask
 *
 * @return         none
 *
 * @author         SongHo Yoon
 * @version        1.0.0
 *
 */
PUBLIC VOID
FSR_DBG_SetAllDbgZoneMask(VOID)
{
    gnFSRDbgZoneMask = FSR_DBZ_ALL_ENABLE;
}

/**
 * @brief          This function unsets all debug zone mask
 *
 * @return         none
 *
 * @author         SongHo Yoon
 * @version        1.0.0
 *
 */
PUBLIC VOID
FSR_DBG_UnsetAllDbgZoneMask(VOID)
{
    gnFSRDbgZoneMask = FSR_DBZ_ALL_DISABLE;
}

