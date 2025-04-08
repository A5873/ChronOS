# Contributing to ChronOS

First of all, thank you for considering contributing to ChronOS! This project serves as both an educational tool and a practical implementation of OS scheduling concepts. Your contributions help make this project better for everyone.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Development Workflow](#development-workflow)
- [Pull Request Process](#pull-request-process)
- [Testing Requirements](#testing-requirements)
- [Documentation Guidelines](#documentation-guidelines)
- [Community](#community)

## Code of Conduct

This project follows a Code of Conduct adapted from the [Contributor Covenant](https://www.contributor-covenant.org/). By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## Development Setup

### Prerequisites
- A C compiler (GCC or Clang recommended)
- Make build system
- Git
- (Optional) cppcheck for static analysis

### Getting Started

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR-USERNAME/ChronOS.git
   cd ChronOS
   ```
3. Add the original repository as an upstream remote:
   ```bash
   git remote add upstream https://github.com/A5873/ChronOS.git
   ```
4. Create a new branch for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

### Building the Project

```bash
# Regular build
make

# Debug build with debug symbols
make debug

# Release build with optimizations
make release

# Build and run tests
make test
make run_test
```

## Coding Standards

### General Guidelines
- Use 4 spaces for indentation, not tabs
- Keep lines under 80 characters when possible
- Use descriptive variable and function names
- Follow a consistent naming convention:
  * `snake_case` for variables and functions
  * `UPPER_SNAKE_CASE` for constants and macros

### C-Specific Guidelines
- Always initialize variables
- Use const whenever appropriate
- Follow ANSI C (C89/C90) standards for maximum compatibility
- Include proper header guards in all .h files
- Keep functions small and focused on a single task
- Comment your code where necessary, but prefer self-documenting code

### Code Structure
- Separate implementation (.c) from interface (.h)
- Group related functions together
- Place type definitions and function prototypes in header files
- Maintain a clean separation of concerns between components

## Development Workflow

1. Sync your fork with the upstream repository:
   ```bash
   git fetch upstream
   git merge upstream/master
   ```
2. Create a feature branch:
   ```bash
   git checkout -b feature/my-feature
   ```
3. Make your changes, adhering to the coding standards
4. Commit your changes with descriptive messages:
   ```bash
   git commit -m "Add feature: description of the feature"
   ```
5. Push your branch to your fork:
   ```bash
   git push origin feature/my-feature
   ```
6. Create a Pull Request from your fork to the main repository

## Pull Request Process

1. Update the README.md or documentation with details of changes if appropriate
2. Ensure your code passes all tests and CI checks
3. Fill in the Pull Request template with all relevant information
4. Request a review from a maintainer
5. Address any feedback provided by reviewers
6. Once approved, a maintainer will merge your PR

### Pull Request Requirements
- PRs should focus on a single feature or fix
- Include tests for new functionality
- All CI checks must pass
- Code must follow the project's coding standards
- Significant changes should include documentation updates

## Testing Requirements

All code contributions should include appropriate tests:

1. Unit tests for individual functions
2. Integration tests for interacting components
3. Performance tests for critical paths

Tests should:
- Be placed in the `tests/` directory
- Follow the same naming convention as the code they test
- Be comprehensive but concise
- Include both positive and negative test cases
- Be runnable through the existing test framework

## Documentation Guidelines

### Code Documentation
- Use comments to explain "why", not "what"
- Document all public functions in header files
- Include parameter descriptions and return values
- Explain any non-obvious implementations

### Project Documentation
- Update README.md for user-facing changes
- Add to documentation for new features
- Create examples of usage when appropriate
- Use clear, concise language

## Community

- Join discussions in GitHub Issues
- Help others who have questions
- Be respectful and considerate in all communications
- Review pull requests from other contributors
- Share your improvements and ideas

---

Thank you for your contributions! Together we can make ChronOS a valuable resource for learning about operating system concepts.

