#ifndef __MYDRV_H__
#define __MYDRV_H__

#define PRINT SIOCDEVPRIVATE
#define REPRINT (SIOCDEVPRIVATE+1)

struct my_userinfo {
    char* string;          
};

#endif