# $Id$
# 
# GMAT: General Mission Analysis Tool.
# 
# CMAKE script file for GMAT GUI
# This file must be installed in the src/console directory
#
# Author: Joris T. Olympio
# Modified For Production By: Josh Fisher
#  
# DO NOT MODIFY THIS FILE UNLESS YOU KNOW WHAT YOU ARE DOING!
#

MESSAGE("==============================")
MESSAGE("GMAT Console setup " ${VERSION})

SET(TargetName GmatConsole)

# ====================================================================
# source files
SET(CONSOLE_SRCS 
    driver.cpp 
    ConsoleAppException.cpp
    PrintUtility.cpp
    ConsoleMessageReceiver.cpp
    ConfigurationManager.cpp
    json11.cpp
    LogManager.cpp
    NngServer.cpp
    Utils.cpp
    orb_propagation/OrbitPropagation.cpp
    orb_propagation/OrbitPropagationTle.cpp
    sgp4/SGP4.cpp    
    sgp4/ast2Body.cpp
    sgp4/astIOD.cpp  
    sgp4/astMath.cpp  
    sgp4/astTime.cpp  
    sgp4/coordFK5.cpp  
    sgp4/DTM_12.cpp  
    sgp4/EopSpw.cpp  
    sgp4/MSIS_Vers.cpp  
)

INCLUDE_DIRECTORIES(AFTER 
                    .  
                    orb_propagation
                    sgp4)

# ====================================================================
# Recursively find all include files, which will be added to IDE-based
# projects (VS, XCode, etc.)
FILE(GLOB_RECURSE CONSOLE_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h)

# ====================================================================
# compilation

# add the install targets
ADD_EXECUTABLE(${TargetName} ${CONSOLE_SRCS} ${CONSOLE_HEADERS})

# The debug Console should have the same postfix as top-levl CMakeLists.txt
# This is required because CMAKE_DEBUG_POSTFIX only applies to libraries
# and not executables as of CMake 3.0.2
SET_TARGET_PROPERTIES(${TargetName} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

# ====================================================================
# Link libraries
TARGET_LINK_LIBRARIES(${TargetName} PRIVATE GmatUtil)
TARGET_LINK_LIBRARIES(${TargetName} PRIVATE GmatBase)

TARGET_LINK_LIBRARIES(${TargetName} PRIVATE -pthread)
TARGET_LINK_LIBRARIES(${TargetName} PRIVATE log4cpp)
TARGET_LINK_LIBRARIES(${TargetName} PRIVATE nng)
TARGET_LINK_LIBRARIES(${TargetName} PRIVATE mstch)

# ====================================================================
# Add source/header files to IDE-based project source groups
# Macro defined in top-level CMakeLists.txt
_ADDSOURCEGROUPS("")

# Create build outputs in bin directory
_SETOUTPUTDIRECTORY(${TargetName} bin)

# Override debug output directory
SET_TARGET_PROPERTIES(${TargetName} PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GMAT_BUILDOUTPUT_DEBUGDIR}
  )

# Specify where to install (make install or VS "INSTALL" project)
INSTALL( TARGETS ${TargetName}
  DESTINATION bin
  )

# Set RPATH to find shared libraries in default locations on Mac/Linux
if(UNIX)
  if(APPLE)
    SET(MAC_BASEPATH "../${GMAT_MAC_APPBUNDLE_PATH}/Frameworks/")
    SET_TARGET_PROPERTIES(${TargetName} PROPERTIES INSTALL_RPATH
      "@loader_path/${MAC_BASEPATH}"
      )
  else()
    SET_TARGET_PROPERTIES(${TargetName} PROPERTIES INSTALL_RPATH
      "\$ORIGIN/"
      )
  endif()
endif()
