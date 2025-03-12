# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Display the total byte count in the status line.
- Display line number.
- Use alternate screen buffer rather than regular screen.
- Display status bar in white background.
- Enhance the visual experience of command palette.
- Support line wrapping to avoid exceeding screen width.
- Use printf() instead of write() in Editor::writeStr().

### Fixed

- Fixed editor not warning when trying to quit('q') with unsaved changes.

## [0.0.1] - 2025-3-10

### Added

- Initial release of the MINIE Editor.
