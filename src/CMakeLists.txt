include("${_build_cmake_local}")

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)

set(TELEMETRY_PROTO "${CMAKE_CURRENT_SOURCE_DIR}/PiSubmarine/Telemetry/Protobuf/TelemetrySnapshot.proto")
protobuf_generate_cpp(TELEMETRY_PROTO_SRCS TELEMETRY_PROTO_HDRS "${TELEMETRY_PROTO}")

message("Adding library: ${PISUBMARINE_MODULE_NAME}")
add_library(${PISUBMARINE_MODULE_NAME}
        PiSubmarine/Telemetry/Protobuf/Serializer.cpp
        PiSubmarine/Telemetry/Protobuf/Deserializer.cpp
        "../public/PiSubmarine/Telemetry/Protobuf/Serializer.h"
        "../public/PiSubmarine/Telemetry/Protobuf/Deserializer.h"
        ${TELEMETRY_PROTO_SRCS}
        ${TELEMETRY_PROTO_HDRS})
target_include_directories(${PISUBMARINE_MODULE_NAME} PUBLIC "../public" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
PiSubmarineInitTarget(${PISUBMARINE_MODULE_NAME})

PiSubmarineAddDependency("https://github.com/PiSubmarine/Telemetry.Serialization.Api.git" "")
target_link_libraries(${PISUBMARINE_MODULE_NAME} PUBLIC PiSubmarine.Telemetry.Serialization.Api)
target_link_libraries(${PISUBMARINE_MODULE_NAME} PRIVATE protobuf::libprotobuf)
