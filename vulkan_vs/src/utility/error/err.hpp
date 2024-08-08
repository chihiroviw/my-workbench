#pragma once
#include <stdio.h>
#include <iostream>

void exitStr(const char* err){
    printf("ERROR: %s\n",err);
    exit(1);
}