/*
    Copyright � 1995-2005, The AROS Development Team. All rights reserved.
    $Id$
    
    Main for genlibdefs. A tool to generate files for building modules.
*/
#include "genmodule.h"

int main(int argc, char **argv)
{
    char *s;
    struct functions *functions = functionsinit();
    struct config *cfg = initconfig(argc, argv, LIBDEFS);
    
    writeinclibdefs(cfg);
    
    return 0;
}
