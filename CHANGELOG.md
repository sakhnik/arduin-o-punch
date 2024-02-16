# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Try previous keys in Android to facilitate card preparation and resetting
- Lock upload URL to prevent accidental modification
- Successful startup confirmation (OK = --- -.-)

### Changed

- Fix finish control uploading: shouldn't be uploaded, the number should be 900 in QuickEvent
- Fix start time by the *last* start control punch
- Prototype casing with DC input, switch and programmator connector
- Increase sleeping time to 8 seconds

### Removed

- Batteries from the BOM (can use recycled ones from wapes)

## [1.0] - 2024-01-27

### Added

- First 10 prototypes with open casing

[unreleased]: https://github.com/sakhnik/arduin-o-punch/compare/v1.0...HEAD
[1.0]: https://github.com/sakhnik/arduin-o-punch/releases/tag/v1.0
