cmake_minimum_required(VERSION 3.10)

# set the project name
project(VMSBackup)

# add the executable
add_executable(VMSBackup VMSBackup.cpp BBHeader.cpp BRHeader.cpp BSAHeader.cpp BSFileHeader.cpp Process.cpp timevmstounix.c wildcards.c)
