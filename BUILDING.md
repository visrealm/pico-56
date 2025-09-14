# Building PICO-56

This document provides detailed build instructions for the PICO-56 project on different platforms.

## Prerequisites

All platforms require:
- CMake 3.12 or later
- Python 3.x with Pillow library
- ARM cross-compilation toolchain
- Git (with submodules support)

## Windows

### Using the Raspberry Pi Pico VS Code Extension

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the [Raspberry Pi Pico extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico) from the VS Code marketplace
3. The extension will automatically guide you through installing:
   - Pico SDK
   - ARM GCC toolchain
   - CMake and build tools
   - Python dependencies (including Pillow)

### Building on Windows

#### With VS Code Extension (Recommended)
1. Clone the repository:
   ```cmd
   git clone --recursive https://github.com/visrealm/pico-56.git
   ```
2. Open the `pico-56` folder in VS Code
3. The Pico extension will automatically configure the project
4. Use the extension's build commands or press `Ctrl+Shift+P` and run "Pico: Build"

#### Command Line Building
If you need to build from the command line:
1. Clone the repository:
   ```cmd
   git clone --recursive https://github.com/visrealm/pico-56.git
   cd pico-56
   ```

2. Create build directory and configure:
   ```cmd
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DPICO_SDK_FETCH_FROM_GIT=ON
   ```

3. Build:
   ```cmd
   cmake --build . --config Release
   ```

## Linux

### Install Dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential git python3 python3-pip
pip3 install Pillow
```

#### Fedora/RHEL:
```bash
sudo dnf install cmake arm-none-eabi-gcc-cs arm-none-eabi-newlib git python3 python3-pip
pip3 install Pillow
```

#### Arch Linux:
```bash
sudo pacman -S cmake arm-none-eabi-gcc arm-none-eabi-newlib git python python-pip
pip install Pillow
```

### Building on Linux

1. Clone the repository:
   ```bash
   git clone --recursive https://github.com/visrealm/pico-56.git
   cd pico-56
   ```

2. Create build directory and configure:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DPICO_SDK_FETCH_FROM_GIT=ON
   ```

3. Build:
   ```bash
   make -j$(nproc)
   ```

## macOS

### Install Dependencies

1. Install Homebrew if not already installed:
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. Install required packages:
   ```bash
   brew install cmake python3
   brew install --cask gcc-arm-embedded
   pip3 install Pillow
   ```

### Building on macOS

1. Clone the repository:
   ```bash
   git clone --recursive https://github.com/visrealm/pico-56.git
   cd pico-56
   ```

2. Create build directory and configure:
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DPICO_SDK_FETCH_FROM_GIT=ON
   ```

3. Build:
   ```bash
   make -j$(sysctl -n hw.ncpu)
   ```

## Board Variants

The project supports both Raspberry Pi Pico and Pico2. To build for a specific board:

### For Raspberry Pi Pico (default):
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico -DPICO_SDK_FETCH_FROM_GIT=ON
```

### For Raspberry Pi Pico2:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DPICO_BOARD=pico2 -DPICO_SDK_FETCH_FROM_GIT=ON
```

## Build Outputs

After a successful build, you'll find the following files in the build directory:

- `**/*.uf2` - UF2 files for flashing to the Pico
- `**/*.bin` - Binary files
- `**/*.elf` - ELF executable files

## Flashing to Hardware

1. Hold the BOOTSEL button on your Raspberry Pi Pico while connecting it to your computer via USB
2. The Pico will appear as a USB mass storage device
3. Copy the desired `.uf2` file to the Pico
4. The Pico will automatically reboot and run your program

## Troubleshooting

### Common Issues

#### Missing Python Dependencies
If you get errors about missing PIL/Pillow:
```bash
pip install Pillow
# or on some systems:
pip3 install Pillow
```

#### ARM Toolchain Issues on macOS
If you encounter "cannot read spec file 'nosys.specs'" errors:
```bash
# Uninstall problematic installations
brew uninstall --force arm-none-eabi-gcc arm-none-eabi-binutils
# Install the complete toolchain
brew install --cask gcc-arm-embedded
```

#### CMake Cannot Find SDK
If CMake complains about not finding the Pico SDK, ensure you're using:
```bash
-DPICO_SDK_FETCH_FROM_GIT=ON
```

This will automatically download and configure the correct SDK version.

#### Submodule Issues
If you get errors about missing submodules:
```bash
git submodule update --init --recursive
```

## Building Individual Episodes

The project includes various episode examples in the `episodes/` directory. These are built automatically with the main project. To build only specific episodes, you can use:

```bash
# From the build directory
cmake --build . --target ep01-vga-01-test-pattern
cmake --build . --target ep01-vga-04-boing
# etc.
```

## Development Tips

- Use `CMAKE_BUILD_TYPE=Debug` for debugging builds
- Use `make -j$(nproc)` on Linux or `make -j$(sysctl -n hw.ncpu)` on macOS for parallel builds
- The build system will automatically handle PIO file generation and image conversion
- All dependencies are automatically downloaded when using `PICO_SDK_FETCH_FROM_GIT=ON`