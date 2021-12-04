cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME
        xmr2csv)

project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=c++14")

if (NOT MONERO_DIR)
    set(MONERO_DIR ~/monero)
endif()

message(STATUS MONERO_DIR ": ${MONERO_DIR}")

if (NOT MONERO_SOURCE_DIR)
    set(MONERO_SOURCE_DIR ${MONERO_DIR}
          CACHE PATH "Path to the root directory for Monero")
endif()

if (NOT MONERO_BUILD_DIR)
    # set location of monero build tree
    set(MONERO_BUILD_DIR ${MONERO_SOURCE_DIR}/build/release/
          CACHE PATH "Path to the build directory for Monero")

   if (NOT EXISTS ${MONERO_BUILD_DIR})
        # try different location
        message(STATUS "Trying different folder for monero libraries")
        set(MONERO_BUILD_DIR ${MONERO_SOURCE_DIR}/build/Linux/master/release/
            CACHE PATH "Path to the build directory for Monero" FORCE)
    endif()

endif()

if (NOT EXISTS ${MONERO_BUILD_DIR})
  message(FATAL_ERROR "Monero libraries not found in: ${MONERO_BUILD_DIR}")
endif()

message(STATUS MONERO_DIR ": ${MONERO_DIR}")

set(MONERO_SOURCE_DIR ${MONERO_DIR}
        CACHE PATH "Path to the root directory for Monero")

# set location of monero build tree
set(MONERO_BUILD_DIR ${MONERO_SOURCE_DIR}/build/release/
        CACHE PATH "Path to the build directory for Monero")

set(MY_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake"
        CACHE PATH "The path to the cmake directory of the current project")

list(APPEND CMAKE_MODULE_PATH "${MY_CMAKE_DIR}")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${MONERO_BUILD_DIR}"
        CACHE PATH "Add Monero directory for library searching")

include(MyUtils)

find_package(Monero)


# find boost
find_package(Boost COMPONENTS
        system
        filesystem
        thread
        date_time
        chrono
        regex
        serialization
        program_options
        date_time
        REQUIRED)

# include boost headers
include_directories(
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/ext/fmt)

message("Xmr_WALLET-CRYPTO_LIBRARIES ${Xmr_WALLET-CRYPTO_LIBRARIES}")

if("${Xmr_WALLET-CRYPTO_LIBRARIES}" STREQUAL "Xmr_WALLET-CRYPTO_LIBRARY-NOTFOUND")
  set(WALLET_CRYPTO "")
else()
  set(WALLET_CRYPTO ${Xmr_WALLET-CRYPTO_LIBRARIES})
endif()

add_subdirectory(ext/fmt)

# add src/ subfolder
add_subdirectory(ext/)

# add src/ subfolder
add_subdirectory(src/)

# speficie source files
set(SOURCE_FILES
        main.cpp)

# make executable called xmreg01
add_executable(${PROJECT_NAME}
        ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME}
        PROPERTIES LINKER_LANGUAGE CXX)

set(LIBRARIES
        myxrm
        myext
        wallet
        device
         ${WALLET_CRYPTO}
        blockchain_db
        cryptonote_core
        cryptonote_format_utils_basic
        cryptonote_protocol
        cryptonote_basic
        daemonizer
        cncrypto
        blocks
        lmdb
        ringct
        ringct_basic
        common
        mnemonics
        epee
        easylogging
        randomx
        hardforks
        device
        version
        sodium
        fmt::fmt-header-only
	    pcsclite
        ${Boost_LIBRARIES}
        pthread
        unbound
        curl
        crypto
        ssl)

if (Xmr_CHECKPOINTS_LIBRARIES)
    set(LIBRARIES ${LIBRARIES} checkpoints)
endif()


if(APPLE)
    set(LIBRARIES ${LIBRARIES} "-framework IOKit")
else()
    set(LIBRARIES ${LIBRARIES} atomic)
endif()


if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT WIN32)
    set(LIBRARIES ${LIBRARIES} unwind)
endif()

if (WIN32)
    set(LIBRARIES ${LIBRARIES}
            wsock32
            ntdll
            ws2_32
            Iphlpapi
            )
else()
    set(LIBRARIES ${LIBRARIES} dl)
endif()

find_package(HIDAPI)
set(LIBRARIES ${LIBRARIES} ${HIDAPI_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${LIBRARIES})
