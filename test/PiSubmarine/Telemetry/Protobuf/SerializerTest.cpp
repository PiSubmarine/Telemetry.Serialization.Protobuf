#include <gtest/gtest.h>

#include "PiSubmarine/Telemetry/Protobuf/Deserializer.h"
#include "PiSubmarine/Telemetry/Protobuf/Serializer.h"

namespace PiSubmarine::Telemetry::Protobuf
{
    TEST(SerializerTest, RoundTripsCompleteSnapshot)
    {
        const Api::Snapshot snapshot{
            .Depth = 12.5_m,
            .DistanceToSeaFloor = 0.7_m,
            .BatteryState = Battery::Telemetry::Api::State{
                .PackVoltage = Volts{15.2},
                .ChargerVoltage = Volts{16.4},
                .PackCurrent = Amperes{-3.1},
                .ChargerCurrent = Amperes{0.4},
                .ChargerTemperature = Celsius{31.0},
                .PackTemperature = Celsius{28.0},
                .MonitorTemperature = Celsius{29.5},
                .RemainingCapacity = AmpereHours{4.8},
                .StateOfCharge = NormalizedFraction{0.65},
                .TimeToFull = std::chrono::milliseconds{420000},
                .TimeToEmpty = std::chrono::milliseconds{1337000}},
            .Thrusters = std::array<Motor::Telemetry::Api::State, 4>{
                Motor::Telemetry::Api::State{
                    .Operational = Motor::Telemetry::Api::OperationalState::Operational,
                    .ActiveFaults = static_cast<Motor::Telemetry::Api::Faults>(0),
                    .ActiveWarnings = static_cast<Motor::Telemetry::Api::Warnings>(0)},
                Motor::Telemetry::Api::State{
                    .Operational = Motor::Telemetry::Api::OperationalState::Degraded,
                    .ActiveFaults = Motor::Telemetry::Api::Faults::Overcurrent,
                    .ActiveWarnings = static_cast<Motor::Telemetry::Api::Warnings>(0)},
                Motor::Telemetry::Api::State{
                    .Operational = Motor::Telemetry::Api::OperationalState::Faulted,
                    .ActiveFaults = Motor::Telemetry::Api::Faults::Overtemperature,
                    .ActiveWarnings = Motor::Telemetry::Api::Warnings::Temperature},
                Motor::Telemetry::Api::State{
                    .Operational = Motor::Telemetry::Api::OperationalState::Operational,
                    .ActiveFaults = static_cast<Motor::Telemetry::Api::Faults>(0),
                    .ActiveWarnings = static_cast<Motor::Telemetry::Api::Warnings>(0)}},
            .BallastPosition = NormalizedFraction{0.25}};

        Serializer serializer;
        Deserializer deserializer;

        const auto serializeResult = serializer.Serialize(snapshot);
        ASSERT_TRUE(serializeResult.has_value());
        EXPECT_FALSE(serializeResult->empty());

        const auto deserializeResult = deserializer.Deserialize(*serializeResult);
        ASSERT_TRUE(deserializeResult.has_value());
        EXPECT_EQ(*deserializeResult, snapshot);
    }
}
