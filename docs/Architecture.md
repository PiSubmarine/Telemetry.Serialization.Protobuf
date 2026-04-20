# Telemetry.Serialization.Protobuf

`PiSubmarine.Telemetry.Serialization.Protobuf` implements
`Telemetry.Serialization.Api` using protobuf as the on-wire format.

## Responsibility

This module owns:

- protobuf schema for `Telemetry.Api::Snapshot`
- serializer implementation
- deserializer implementation

It does not own:

- UDP transport
- telemetry production
- lease handling
