/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef __DISP_ASSERT_LAYER_H__
#define __DISP_ASSERT_LAYER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DAL_STATUS_OK                = 0,

	DAL_STATUS_NOT_READY         = -1,
	DAL_STATUS_INVALID_ARGUMENT  = -2,
	DAL_STATUS_LOCK_FAIL         = -3,
	DAL_STATUS_LCD_IN_SUSPEND    = -4,
	DAL_STATUS_FATAL_ERROR       = -10,
} DAL_STATUS;


typedef enum {
	DAL_COLOR_BLACK     = 0x000000,
	DAL_COLOR_WHITE     = 0xFFFFFF,
	DAL_COLOR_RED       = 0xFF0000,
	DAL_COLOR_GREEN     = 0x00FF00,
	DAL_COLOR_BLUE      = 0x0000FF,
	DAL_COLOR_TURQUOISE = (DAL_COLOR_GREEN | DAL_COLOR_BLUE),
	DAL_COLOR_YELLOW    = (DAL_COLOR_RED | DAL_COLOR_GREEN),
	DAL_COLOR_PINK      = (DAL_COLOR_RED | DAL_COLOR_BLUE),
} DAL_COLOR;


/* Display Assertion Layer API */

unsigned int DAL_GetLayerSize(void);

DAL_STATUS DAL_Init(unsigned int layerVA, unsigned int layerPA);
DAL_STATUS DAL_SetColor(unsigned int fgColor, unsigned int bgColor);
DAL_STATUS DAL_Clean(void);
DAL_STATUS DAL_Printf(const char *fmt, ...);
DAL_STATUS DAL_OnDispPowerOn(void);
DAL_STATUS DAL_LowMemoryOn(void);
DAL_STATUS DAL_LowMemoryOff(void);
#ifdef __cplusplus
}
#endif

#endif // __DISP_ASSERT_LAYER_H__