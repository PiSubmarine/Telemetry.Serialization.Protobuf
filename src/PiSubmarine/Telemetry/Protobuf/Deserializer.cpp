#include "PiSubmarine/Telemetry/Protobuf/Deserializer.h"

#include <array>
#include <cstdint>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Telemetry/ErrorCode.h"
#include "TelemetrySnapshot.pb.h"

namespace PiSubmarine::Telemetry::Protobuf
{
    namespace
    {
        [[nodiscard]] Error::Api::Error MakeTelemetryError(const ErrorCode code)
        {
            return Error::Api::MakeError(Error::Api::ErrorCondition::ContractError, make_error_code(code));
        }

        [[nodiscard]] Motor::Telemetry::Api::OperationalState FromProto(
            const ::pisubmarine::telemetry::protobuf::MotorState_OperationalState state)
        {
            switch (state)
            {
            case ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_OPERATIONAL:
                return Motor::Telemetry::Api::OperationalState::Operational;
            case ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_DEGRADED:
                return Motor::Telemetry::Api::OperationalState::Degraded;
            case ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_FAULTED:
                return Motor::Telemetry::Api::OperationalState::Faulted;
            }

            return Motor::Telemetry::Api::OperationalState::Operational;
        }

        [[nodiscard]] Error::Api::Result<Battery::Telemetry::Api::State> DeserializeBatteryState(
            const ::pisubmarine::telemetry::protobuf::BatteryState& protoState)
        {
            try
            {
                Battery::Telemetry::Api::State state{
                    .PackVoltage = Volts{protoState.pack_voltage()},
                    .ChargerVoltage = Volts{protoState.charger_voltage()},
                    .PackCurrent = Amperes{protoState.pack_current()},
                    .ChargerCurrent = Amperes{protoState.charger_current()},
                    .ChargerTemperature = Celsius{protoState.charger_temperature()},
                    .PackTemperature = Celsius{protoState.pack_temperature()},
                    .MonitorTemperature = Celsius{protoState.monitor_temperature()},
                    .RemainingCapacity = AmpereHours{protoState.remaining_capacity()},
                    .StateOfCharge = NormalizedFraction{protoState.state_of_charge()}};

                if (protoState.has_time_to_full_ms())
                {
                    state.TimeToFull = std::chrono::milliseconds(protoState.time_to_full_ms());
                }

                if (protoState.has_time_to_empty_ms())
                {
                    state.TimeToEmpty = std::chrono::milliseconds(protoState.time_to_empty_ms());
                }

                return state;
            }
            catch (...)
            {
                return std::unexpected(MakeTelemetryError(ErrorCode::InvalidPayload));
            }
        }
    }

    Error::Api::Result<Api::Snapshot> Deserializer::Deserialize(std::span<const std::byte> bytes) const
    {
        ::pisubmarine::telemetry::protobuf::TelemetrySnapshot protoSnapshot;
        if (!protoSnapshot.ParseFromArray(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<int>(bytes.size())))
        {
            return std::unexpected(MakeTelemetryError(ErrorCode::DeserializationFailed));
        }

        if (protoSnapshot.thrusters_size() != 4)
        {
            return std::unexpected(MakeTelemetryError(ErrorCode::InvalidPayload));
        }

        Api::Snapshot snapshot{};

        if (protoSnapshot.has_depth_meters())
        {
            snapshot.Depth = Meters{protoSnapshot.depth_meters()};
        }

        if (protoSnapshot.has_distance_to_seafloor_meters())
        {
            snapshot.DistanceToSeaFloor = Meters{protoSnapshot.distance_to_seafloor_meters()};
        }

        if (protoSnapshot.has_battery_state())
        {
            const auto batteryResult = DeserializeBatteryState(protoSnapshot.battery_state());
            if (!batteryResult.has_value())
            {
                return std::unexpected(batteryResult.error());
            }

            snapshot.BatteryState = *batteryResult;
        }

        for (int index = 0; index < protoSnapshot.thrusters_size(); ++index)
        {
            const auto& protoThruster = protoSnapshot.thrusters(index);
            snapshot.Thrusters[static_cast<std::size_t>(index)] = Motor::Telemetry::Api::State{
                .Operational = FromProto(protoThruster.operational()),
                .ActiveFaults = static_cast<Motor::Telemetry::Api::Faults>(protoThruster.active_faults()),
                .ActiveWarnings = static_cast<Motor::Telemetry::Api::Warnings>(protoThruster.active_warnings())};
        }

        if (protoSnapshot.has_ballast_position())
        {
            try
            {
                snapshot.BallastPosition = NormalizedFraction{protoSnapshot.ballast_position()};
            }
            catch (...)
            {
                return std::unexpected(MakeTelemetryError(ErrorCode::InvalidPayload));
            }
        }

        return snapshot;
    }
}
