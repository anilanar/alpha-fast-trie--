//
//  main.cpp
//  str_manip_test
//
//  Created by Anil Anar on 15.06.2014.
//  Copyright (c) 2014 Urnware. All rights reserved.
//

#include <stdio.h>
#include <gtest/gtest.h>

GTEST_API_ int main(int argc, char ** argv)
{
    printf("Running main() from gtest_main.cc\n");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

