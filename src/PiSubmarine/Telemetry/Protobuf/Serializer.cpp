#include "PiSubmarine/Telemetry/Protobuf/Serializer.h"

#include <cstdint>
#include <string>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "PiSubmarine/Telemetry/ErrorCode.h"
#include "TelemetrySnapshot.pb.h"

namespace PiSubmarine::Telemetry::Protobuf
{
    namespace
    {
        [[nodiscard]] ::pisubmarine::telemetry::protobuf::MotorState_OperationalState ToProto(
            const Motor::Telemetry::Api::OperationalState state)
        {
            switch (state)
            {
            case Motor::Telemetry::Api::OperationalState::Operational:
                return ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_OPERATIONAL;
            case Motor::Telemetry::Api::OperationalState::Degraded:
                return ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_DEGRADED;
            case Motor::Telemetry::Api::OperationalState::Faulted:
                return ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_FAULTED;
            }

            return ::pisubmarine::telemetry::protobuf::MotorState_OperationalState_OPERATIONAL;
        }

        void FillBatteryState(
            const Battery::Telemetry::Api::State& state,
            ::pisubmarine::telemetry::protobuf::BatteryState* protoState)
        {
            protoState->set_pack_voltage(state.PackVoltage.Value);
            protoState->set_charger_voltage(state.ChargerVoltage.Value);
            protoState->set_pack_current(state.PackCurrent.Value);
            protoState->set_charger_current(state.ChargerCurrent.Value);
            protoState->set_charger_temperature(state.ChargerTemperature.Value);
            protoState->set_pack_temperature(state.PackTemperature.Value);
            protoState->set_monitor_temperature(state.MonitorTemperature.Value);
            protoState->set_remaining_capacity(state.RemainingCapacity.Value);
            protoState->set_state_of_charge(static_cast<double>(state.StateOfCharge));

            if (state.TimeToFull.has_value())
            {
                protoState->set_time_to_full_ms(state.TimeToFull->count());
            }

            if (state.TimeToEmpty.has_value())
            {
                protoState->set_time_to_empty_ms(state.TimeToEmpty->count());
            }
        }
    }

    Error::Api::Result<std::vector<std::byte>> Serializer::Serialize(const Api::Snapshot& snapshot) const
    {
        ::pisubmarine::telemetry::protobuf::TelemetrySnapshot protoSnapshot;

        if (snapshot.Depth.has_value())
        {
            protoSnapshot.set_depth_meters(snapshot.Depth->Value);
        }

        if (snapshot.DistanceToSeaFloor.has_value())
        {
            protoSnapshot.set_distance_to_seafloor_meters(snapshot.DistanceToSeaFloor->Value);
        }

        if (snapshot.BatteryState.has_value())
        {
            FillBatteryState(*snapshot.BatteryState, protoSnapshot.mutable_battery_state());
        }

        for (const auto& thruster : snapshot.Thrusters)
        {
            auto* protoThruster = protoSnapshot.add_thrusters();
            protoThruster->set_operational(ToProto(thruster.Operational));
            protoThruster->set_active_faults(static_cast<std::uint32_t>(thruster.ActiveFaults));
            protoThruster->set_active_warnings(static_cast<std::uint32_t>(thruster.ActiveWarnings));
        }

        if (snapshot.BallastPosition.has_value())
        {
            protoSnapshot.set_ballast_position(static_cast<double>(*snapshot.BallastPosition));
        }

        std::string serialized;
        if (!protoSnapshot.SerializeToString(&serialized))
        {
            return std::unexpected(Error::Api::MakeError(
                Error::Api::ErrorCondition::DeviceError,
                make_error_code(ErrorCode::SerializationFailed)));
        }

        std::vector<std::byte> bytes;
        bytes.reserve(serialized.size());
        for (const char character : serialized)
        {
            bytes.push_back(static_cast<std::byte>(character));
        }

        return bytes;
    }
}
