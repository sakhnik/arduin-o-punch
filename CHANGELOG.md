# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Try previous keys in Android to facilitate card preparation and resetting
- Lock upload URL to prevent accidental modification
- Successful startup confirmation (OK = --- -.-)
- Set both date and time with UNIX timestamp
- Record retaining period in days

### Changed

- Fix finish control uploading: shouldn't be uploaded, the number should be 900 in QuickEvent
- Fix start time by the *last* start control punch
- Prototype casing with DC input, switch and programmator connector
- Fix DNF when finish wasn't punched (finishTime = 0xEEEE)
- Use arduino clock for millisecond measurement of clock during synchronization

### Removed

- Batteries from the BOM (can use recycled ones from wapes)
- Time measurement from custom point, always from midnight
- Sleeping on timeout in favour of the power off button

## [1.0] - 2024-01-27

### Added

- First 10 prototypes with open casing

[unreleased]: https://github.com/sakhnik/arduin-o-punch/compare/v1.0...HEAD
[1.0]: https://github.com/sakhnik/arduin-o-punch/releases/tag/v1.0
