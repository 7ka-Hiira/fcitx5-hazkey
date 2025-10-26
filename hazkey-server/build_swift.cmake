set(SWIFT_BUILD_TYPE "${SWIFT_BUILD_TYPE}")
set(SWIFT_EXECUTABLE "${SWIFT_EXECUTABLE}")
set(SWIFT_DETECTED_LIB_PATH "${SWIFT_DETECTED_LIB_PATH}")
set(SWIFT_LINK_PATH "${SWIFT_LINK_PATH}")
set(SWIFT_WORK_DIR "${SWIFT_WORK_DIR}")
set(LLAMA_STUB_DIR "${LLAMA_STUB_DIR}")


set(SWIFT_COMMAND
    ${SWIFT_EXECUTABLE} build -c ${SWIFT_BUILD_TYPE}
    --scratch-path=${CMAKE_CURRENT_BINARY_DIR}/swift-build
    -Xswiftc -static-stdlib
    -Xlinker -L${LLAMA_STUB_DIR}
)

if(SWIFT_DETECTED_LIB_PATH)
    list(APPEND SWIFT_COMMAND -Xlinker -L${SWIFT_DETECTED_LIB_PATH})
endif()

if(SWIFT_LINK_PATH)
    list(APPEND SWIFT_COMMAND -Xlinker -L${SWIFT_LINK_PATH})
endif()

execute_process(
    COMMAND ${SWIFT_COMMAND}
    WORKING_DIRECTORY ${SWIFT_WORK_DIR}
    RESULT_VARIABLE result
)

# The first build fails for an unknown reason.
if(NOT result EQUAL 0)
    execute_process(
        COMMAND ${SWIFT_COMMAND}
        WORKING_DIRECTORY ${SWIFT_WORK_DIR}
        RESULT_VARIABLE result2
    )
    if(NOT result2 EQUAL 0)
        message(FATAL_ERROR "Swift build failed after two attempts.")
    endif()
endif()
