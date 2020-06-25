//
//  main.cpp
//  TestDylib
//
//  Created by Afroz Muzammil on 8/4/16.
//  Copyright (c) 2016 Afroz Muzammil. All rights reserved.
//

#include <iostream>
#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, const char * argv[]) {
    if (argc < 2 || !strcasecmp(argv[1], "-h") || !strcasecmp(argv[1], "--help")) {
        printf("Usage:\nTestDylib <dylib path> [[function name] [dylib path] ...]\n");
        return EPERM;
    }
    void *pDylib = NULL;
    int retVal(0);
    for (int i=1; i<argc; ++i) {
        if (NULL != pDylib && access(argv[i], R_OK) == 0) {
            dlclose(pDylib);
            pDylib = NULL;
        }
        if (pDylib == NULL) {
            pDylib = dlopen(argv[i], RTLD_LAZY);
            if (NULL == pDylib) {
                printf("cannot open '%s'\nError: %s", argv[i], dlerror());
                retVal = ENOENT;
            }
            else
                printf("%s loaded\n", argv[i]);
            continue;
        }
        void *pSym = dlsym(pDylib, argv[i]);
        if (NULL == pSym) {
            printf("\t'%s' not found\nError: %s\n", argv[i], dlerror());
            retVal = ESRCH;
        }
        else
            printf("\t%s found\n", argv[i]);
    }
    if (NULL != pDylib)
        dlclose(pDylib);
    return retVal;
}
