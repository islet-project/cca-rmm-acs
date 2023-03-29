#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited or its affiliates. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

file(GLOB VAL_SRC
    "${ROOT_DIR}/val/realm/inc/*.h"
    "${ROOT_DIR}/val/realm/src/*.c"
    "${ROOT_DIR}/val/realm/src/*.S"
    "${ROOT_DIR}/val/common/inc/*.h"
    "${ROOT_DIR}/val/common/src/*.c"
    "${ROOT_DIR}/val/common/src/*.S"
)

#Create compile list files
list(APPEND COMPILE_LIST ${VAL_SRC})
set(COMPILE_LIST ${COMPILE_LIST} PARENT_SCOPE)

# Create VAL library
add_library(${VAL_LIB} STATIC ${VAL_SRC})

target_include_directories(${VAL_LIB} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ROOT_DIR}/val/realm/inc/
    ${ROOT_DIR}/val/common/inc/
    ${ROOT_DIR}/plat/common/inc/
    ${ROOT_DIR}/plat/targets/${TARGET}/inc/
    ${ROOT_DIR}/plat/driver/inc/
    ${ROOT_DIR}/test/database/
)

unset(VAL_SRC)
