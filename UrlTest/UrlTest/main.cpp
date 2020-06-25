//
//  main.cpp
//  UrlTest
//
//  Created by Afroz Muzammil on 1/7/16.
//  Copyright © 2016 Afroz Muzammil. All rights reserved.
//

#include <iostream>
#include <string>

std::string GetURLData(const std::string &inRequestURL);


int mac_main(int argc, const char * argv[]) {
    // insert code here...
    
    if (argc < 2) {
        printf("Usage:\nUrlTest <url>\ne.g.\nUrlTest http://google.com\n");
        return 1;
    }
    printf("Requesting URL...\n%s\n", argv[1]);
    printf("%s\n", GetURLData(argv[1]).c_str());
    return 0;
}
