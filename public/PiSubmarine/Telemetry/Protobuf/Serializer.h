#pragma once

#include "PiSubmarine/Telemetry/ISerializer.h"

namespace PiSubmarine::Telemetry::Protobuf
{
    class Serializer final : public ISerializer
    {
    public:
        [[nodiscard]] Error::Api::Result<std::vector<std::byte>> Serialize(
            const Api::Snapshot& snapshot) const override;
    };
}
