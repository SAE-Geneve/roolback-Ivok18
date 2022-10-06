file(GLOB_RECURSE Game_SRC src/game/*.cpp include/game/*.h)
file(GLOB_RECURSE Network_SRC src/network/*.cpp include/network/*.h)

source_group("Game"				FILES ${Game_SRC})
source_group("Network"				FILES ${Network_SRC})

find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_library(GameLib STATIC ${Game_SRC} ${Network_SRC})
target_include_directories(GameLib PUBLIC include/)
target_link_libraries(GameLib PUBLIC CoreLib)
if(ENABLE_SQLITE_STORE)
	target_compile_definitions(CoreLib PUBLIC "ENABLE_SQLITE=1")
    target_link_libraries(GameLib PUBLIC unofficial::sqlite3::sqlite3)
endif(ENABLE_SQLITE_STORE)
#set_target_properties(GameLib PROPERTIES UNITY_BUILD ON)
set_target_properties (GameLib PROPERTIES FOLDER Game)

add_data_folder(GameLib)
set_target_properties (GameLib_Copy_Data PROPERTIES FOLDER Game/Main)

file(GLOB main_SRC main/*.cpp)
foreach(main_file ${main_SRC})
    get_filename_component(main_project_name ${main_file} NAME_WE )
    add_executable(${main_project_name} ${main_file})
    target_link_libraries(${main_project_name} PRIVATE GameLib)
    set_target_properties (${main_project_name} PROPERTIES FOLDER Game/Main)
endforeach()
