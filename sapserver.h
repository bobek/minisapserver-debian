/*****************************************************************************
 * sapserver.h : SAP discovery service classes definition
 ****************************************************************************
 * Copyright (C) 1998-2004 VideoLAN
 * $Id: sapserver.h 345 2009-01-11 19:50:41Z courmisch $ 
 *
 * Authors: Fabrice Ollivier <cif@via.ecp.fr>
 *          Arnaud Schauly <gitan@via.ecp.fr>
 *          Clément Stenac <zorglub@via.ecp.fr>
 *          Damien Lucas <nitrox@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#include "config.h"

#define MSGBUFSIZE 256

#define DEFAULT_CONF SYSCONFDIR"/sap.cfg"
#define DEFAULT_TTL 255
#define DEFAULT_SCOPE '8'
#define DEFAULT_PORT 1234
#define DEFAULT_DELAY 5 /* wait DELAY seconds before each sending */


#define TYPE_SAP 1
#define TYPE_SLP 2

#define SAP_ON 1
#define SAP_OFF 0
#define SAP_IPV4 4
#define SAP_IPV6 6
