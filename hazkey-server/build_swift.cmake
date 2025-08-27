set(SWIFT_BUILD_TYPE "${SWIFT_BUILD_TYPE}")
set(SWIFT_EXECUTABLE "${SWIFT_EXECUTABLE}")
set(SWIFT_WORK_DIR "${SWIFT_WORK_DIR}")
set(LLAMA_STUB_DIR "${LLAMA_STUB_DIR}")

execute_process(
    COMMAND ${SWIFT_EXECUTABLE} build -c ${SWIFT_BUILD_TYPE}
        --scratch-path=${CMAKE_CURRENT_BINARY_DIR}/swift-build
        -Xswiftc -static-stdlib
        -Xlinker -L${LLAMA_STUB_DIR}
        -Xlinker -L/usr/lib/swift/linux
        -Xlinker -L/usr/lib/swift/lib/swift/linux
        -Xlinker -L/usr/local/lib/swift/linux
        -Xlinker -L/opt/swift/lib/swift/linux
    WORKING_DIRECTORY ${SWIFT_WORK_DIR}
    RESULT_VARIABLE result
)

# The first build fails for an unknown reason.
if(NOT result EQUAL 0)
    execute_process(
        COMMAND ${SWIFT_EXECUTABLE} build -c ${SWIFT_BUILD_TYPE}
            --scratch-path=${CMAKE_CURRENT_BINARY_DIR}/swift-build
            -Xswiftc -static-stdlib
            -Xlinker -L${LLAMA_STUB_DIR}
            -Xlinker -L/usr/lib/swift/linux
            -Xlinker -L/usr/lib/swift/lib/swift/linux
            -Xlinker -L/usr/local/lib/swift/linux
            -Xlinker -L/opt/swift/lib/swift/linux
        WORKING_DIRECTORY ${SWIFT_WORK_DIR}
        RESULT_VARIABLE result2
    )
    if(NOT result2 EQUAL 0)
        message(FATAL_ERROR "Swift build failed after two attempts.")
    endif()
endif()
