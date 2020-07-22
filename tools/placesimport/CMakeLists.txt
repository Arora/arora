project(arora-placesimport)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5WebKit REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/history ${CMAKE_CURRENT_SOURCE_DIR}/../../src/utils)
add_definitions(-DNO_BROWSERAPPLICATION)
set(Arora_placesimport_SRCS
	main_placesimport.cpp
	../../src/autosaver.cpp
	../../src/modelmenu.cpp
	../../src/history/history.cpp
	../../src/history/historymanager.cpp
	../../src/utils/singleapplication.cpp
)

add_executable(arora-placesimport ${Arora_placesimport_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
qt5_use_modules(arora-placesimport Core)
qt5_use_modules(arora-placesimport Widgets)
qt5_use_modules(arora-placesimport Sql)
qt5_use_modules(arora-placesimport WebKit)
qt5_use_modules(arora-placesimport WebKitWidgets)
