#ifndef __ESF_SYS_DAEMON_H__
#define __ESF_SYS_DAEMON_H__

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

void InitDaemon() 
{
    pid_t pid;

    if ((pid = fork() ) != 0 )
    {
        exit(0);
    }

    setsid();

    signal( SIGINT,  SIG_IGN);
    signal( SIGHUP,  SIG_IGN);
    signal( SIGPIPE, SIG_IGN);
    signal( SIGTTOU, SIG_IGN);
    signal( SIGTTIN, SIG_IGN);
    signal( SIGCHLD, SIG_IGN);
    signal( SIGTERM, SIG_IGN);
	
    struct sigaction sig;

    sig.sa_handler = SIG_IGN;
    sig.sa_flags = 0;
    sigemptyset( &sig.sa_mask);
    sigaction( SIGHUP,&sig,NULL);

    if ((pid = fork() ) != 0 )
    {
        exit(0);
    }

    umask(0);
    setpgrp();
}

#endif
