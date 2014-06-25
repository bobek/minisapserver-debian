/*****************************************************************************
 * slp.cpp : SLP registration class
 ****************************************************************************
 * Copyright (C) 1998-2003 VideoLAN
 * $Id: slp.cpp 298 2005-07-20 18:20:54Z courmisch $
 *
 * Authors: Clément Stenac <zorglub@via.ecp.fr>
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


#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

#include "sapserver.h"
#include "program.h"

#ifdef CONFIG_SLP
  #include <slp.h>
#endif

#include "lslp.h"

using namespace std;

SLP::SLP()
{
    if( SLPOpen( NULL, SLP_FALSE, &slp_handle ) != SLP_OK)
    {
	fprintf(stderr,"- Unable to initialize SLP\n");
	abort();
    } 
}


SLP::~SLP(){return;}


void RegReport(SLPHandle slp_handle,SLPError slp_error,void* cookie)
{
}

char * BuildService(Program *p)
{
    string address = p->GetAddress();
    if (address.find(':') != string::npos)
        address = "[" + address + "]";

    string url = "udp:@" + address + ":" + p->GetPort();

#if 0
    /* This code is for the new naming scheme after vlc 0.7.0 */
    /* service:vlc.services.videolan.org://MRL//NAME */
    url += "//";
    url += p->GetName();
#endif

    string msg = "service:vlc.services.videolan.org://" + url;

    return strdup(msg.c_str());
}

int SLP::Register(Program *p)
{
    char *psz_msg = BuildService(p);
 
    string name;
    name += p->GetName();

    fprintf(stdout,"+ Registering %s (name: %s)\n",psz_msg,name.data());

    SLPReg (    slp_handle, 
                psz_msg,
                SLP_LIFETIME_MAXIMUM,
                NULL,
                name.data(),
                SLP_TRUE,
                RegReport,
                NULL );

    if(psz_msg) free(psz_msg);

    return 0;
}

int SLP::UnRegister(Program *p)
{
    char *psz_msg = BuildService(p);
   
    fprintf(stdout,"+ Unregistering %s\n",psz_msg);

    SLPDereg (	slp_handle,
		psz_msg,
		RegReport,
		NULL );

    return 0;
}
