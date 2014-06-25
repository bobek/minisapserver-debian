/*****************************************************************************
 * sapserver.cpp : SAP discovery service mini-server
 ****************************************************************************
 * Copyright (C) 1998-2004 VideoLAN
 * $Id: sapserver.cpp 342 2008-06-30 17:05:19Z courmisch $ 
 *
 * Authors: Arnaud Schauly <gitan@via.ecp.fr>
 *          Clément Stenac <zorglub@via.ecp.fr>
 *          Damien Lucas <nitrox@videolan.org>
 *          Rémi Denis-Courmont <rem # videolan.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <string>
#include <vector>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

using namespace std;

#include "sapserver.h"
#include "program.h"
#include "message.h"
#include "broadcast.h"
#include "parser.h"

#ifdef CONFIG_SLP
    #include <slp.h>
    #include "lslp.h"
#endif

/*************************************************
 * Signal handler
 *************************************************/
static int should_exit = 0;

static void
exit_handler (int signum)
{
    signal (signum, exit_handler);
    should_exit = 1;
}


/*************************************************
 * Display the help
 *************************************************/
static void
help(void)
{
  printf(PACKAGE_NAME"\n"
  "Options:\n"
  "  -d      Use this to daemonize the process (run in the background)\n"
  "  -f      Use this to give a configuration file "
                "(default is %s)\n"
  "  -c      Same as -f\n"
  "  -h      Display this help\n"
  "  -t      \"slp\" or \"sap\". sap is default\n"
  "  -s      Display a dot for each packet sent\n"
  "  -u      Unregister services (SLP only)\n"
  "  -V      Display version information\n", DEFAULT_CONF);
}

static void
version(void)
{
    puts(PACKAGE_NAME" version "PACKAGE_VERSION"\n"
    "Copyright (C) 1999-2005 VideoLAN project\n"
    "This is free software; see the source for copying conditions.\n"
    "There is NO warranty; not even for MERCHANTABILITY or\n"
    "FITNESS FOR A PARTICULAR PURPOSE.");
}

typedef struct
{
    struct sockaddr_storage addr;
    socklen_t addrlen;
    Message *message;
} Announce;

#ifndef HAVE_CLEARENV
extern char **environ;

static int
clearenv (void)
{
	environ = NULL;
	return 0;
}
#endif

/*************************************************
 * The main function
 *************************************************/
int main(int argc, char *argv[])
{
    int result;
    const struct option opts[] =
        {
                { "conf",       required_argument, NULL, 'c' },
                { "config",     required_argument, NULL, 'c' },
                { "daemon",     no_argument,       NULL, 'd' },
                { "daemonize",  no_argument,       NULL, 'd' },
                { "file",       required_argument, NULL, 'f' },
                { "help",       no_argument,       NULL, 'h' },
                { "dotmode",    no_argument,       NULL, 's' },
                { "type",       required_argument, NULL, 't' },
                { "unregister", no_argument,       NULL, 'u' },
                { "version",    no_argument,       NULL, 'V' },
                { NULL,         no_argument,       NULL, '\0'}
        };


    setvbuf (stdout, NULL, _IOLBF, 0);
    setvbuf (stderr, NULL, _IOLBF, 0);
    setlocale(LC_ALL, "");
    Config config(DEFAULT_CONF);

    /* Parse the command line */
    while((result = getopt_long(argc,argv,"c:df:hst:uV", opts, NULL))>0)
    {
        switch(result)
        {
            case 'c':
            case 'f':
                config.SetFile(optarg);
                break;
            case 's':
                config.SetDotMode(true);
                break;
            case 'd':
                config.SetDaemonMode(true);
                break;
            case 't':
                config.SetType(optarg);
                break;
            case 'u':
                config.SetReverse(true);
                break;
            case 'h':
                help();
                return 0;
            case 'V':
                version();
                return 0;
            default:
                return -1;
        }
    }

    /* clean up environment */
    clearenv();
    /* make sure stdin, stdout and stderr exists */
    if (dup (2) < 3)
        return 1;

    /* Get the programs */
    puts("+ Parsing configuration file\n");
    if(config.Parse())
    {
        fputs("+ Parsing failed\n", stderr);
        return 1;
    }

    /* TODO check configuration */
    printf("+ %u programs loaded\n",config.Programs.size());
    if (config.Programs.size() == 0)
    {
        puts ("+ Nothing to do. Exiting.");
        return 0;
    }

    if(config.GetType() == TYPE_SAP)
    {
        printf("+ Packet TTL set to %u\n",config.GetTTL());
        printf("+ Running as %s.\n",
               config.GetDaemonMode() ? "daemon" : "program");

        /* Create the broadcast object */
        Broadcast broadcast (config.GetTTL (), config.GetInterface ());

        vector<Announce *> announces;

        for (unsigned int i = 0; i < config.Programs.size(); i++)
        {
            Program *program = config.Programs[i];

            Announce *announce = new Announce;
            if (broadcast.GuessDestination (program->GetAddress ().c_str (),
                                            &announce->addr,
                                            &announce->addrlen))
            {
                fputs("- Network initialization failed. Aborting\n", stderr);
                delete announce;
                /* FIXME memory leak in announces */
                return 1;
            }

            /* Create a new message */
            Message *message = new Message(0x4212+i, "1.2.3.4");
            /* Add the program */
            message->AddProgram(config.Programs[i]);

            announce->message = message;
            announces.push_back( announce );
        }

        /* Forking if necessary */
        if(config.GetDaemonMode())
        {
            puts("+ Forking ... \n");
            daemon(0,0);
        }

        unsigned n = config.Programs.size();
        lldiv_t d = lldiv (1000000000LL * config.GetDelay() / n, 1000000000);
        struct timespec delay;
        delay.tv_sec = d.quot;
        delay.tv_nsec = d.rem;

        signal (SIGINT, exit_handler);
        signal (SIGTERM, exit_handler);
        signal (SIGHUP, exit_handler);
        signal (SIGQUIT, exit_handler);
        setvbuf (stdout, NULL, _IONBF, 0);

        while(!should_exit)
        {
            for( unsigned int i = 0; i< announces.size() ; i ++ )
            {
                Announce *announce = announces[i];
                /* Send the message */
                if (broadcast.Send (announce->message ,
                                    (struct sockaddr *)&announce->addr,
                                    announce->addrlen))
                {
                    fputs ("- Message send failed\n", stderr);
                }
                else
                if(config.GetDotMode())
                    fputc('.', stdout);

                /* Wait for next sending */
                if(should_exit)
                    break;
                nanosleep( &delay, NULL );
            }
        }

        for( unsigned int i = 0; i < announces.size() ; i ++ )
        {
            Announce *announce = announces[i];
            delete announce->message;
            delete announce;
        }
    }
    else
    {
#ifndef CONFIG_SLP
        fputs("- SLP not compiled in...Aborting\n", stderr);
        return 1;
#endif
#ifdef CONFIG_SLP
        SLP slp;

        for(unsigned int i=0 ; i<config.Programs.size(); i++)
        {
            if(!config.GetReverse())
            {
                slp.Register(config.Programs[i]);
            }
            else
            {
                slp.UnRegister(config.Programs[i]);
            }
        }
        if(!config.GetReverse())
        {
            puts("+ Programs registered. To unregister them, run me with -u");
        }
        else
        {
            puts("+ Programs unregistered");
        }
#endif
    }

    puts("Done.");

    return 0;
}
