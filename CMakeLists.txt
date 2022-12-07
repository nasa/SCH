cmake_minimum_required(VERSION 2.6.4)
project(CFS_SCH C)

include_directories(fsw/inc)

aux_source_directory(fsw/src APP_SRC_FILES)

# Create the app module
add_cfe_app(sch ${APP_SRC_FILES})
