#pragma once

#include "PiSubmarine/Telemetry/IDeserializer.h"

namespace PiSubmarine::Telemetry::Protobuf
{
    class Deserializer final : public IDeserializer
    {
    public:
        [[nodiscard]] Error::Api::Result<Api::Snapshot> Deserialize(
            std::span<const std::byte> bytes) const override;
    };
}
