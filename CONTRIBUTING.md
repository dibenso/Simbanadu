# Contributing to Simbanadu

Thanks for your interest in contributing to Simbanadu!

Simbanadu is a personal arcade puzzle game built with C and [raylib](https://www.raylib.com/). Contributions, suggestions, bug reports, and ideas are welcome.

## How to Contribute

### Reporting Bugs

If you find a bug, please [open an issue](https://github.com/dibenso/Simbanadu/issues) and include:

- A clear description of the problem.
- Steps to reproduce the issue.
- What you expected to happen and what actually happened.
- Any relevant error messages or screenshots.

### Suggesting Features

Have an idea for a new feature, gameplay improvement, or quality-of-life enhancement? Open an issue to describe your idea and why it would improve the game.

### Submitting Changes

1. Fork the repository.
2. Clone your fork and create a branch for your changes.
3. Make your changes, keeping them focused and easy to review.
4. Build and run the game to verify your changes.
5. Commit your changes with a clear, descriptive commit message.
6. Push your branch and open a pull request against the main repository.

## Development Setup

Simbanadu currently targets macOS and requires CMake, a C99-compatible compiler, and raylib installed through Homebrew.

Install the dependencies with:

```bash
brew install cmake raylib
```

Build and run the game using:

```bash
./build_and_run.sh
```

This builds the project with debug mode enabled and launches the game. Check the [README](README.md) for additional setup and build instructions.

## Guidelines

When contributing, please keep the following in mind:

- **Code style:** Follow the existing C coding style and project structure.
- **Scope:** Keep changes focused on a specific bug fix, feature, or improvement.
- **Testing:** Build and run the game after making changes to catch regressions.
- **Documentation:** Update the README or other documentation when your changes affect setup, build instructions, or gameplay.
- **Communication:** Be respectful, constructive, and open to feedback during discussions and reviews.

## Questions?

If you're unsure about an idea or how to get started, feel free to [open an issue](https://github.com/dibenso/Simbanadu/issues) to discuss it.

Thanks for helping make Simbanadu better! ❤️
