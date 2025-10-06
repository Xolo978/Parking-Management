#!/bin/bash
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if [ -f /etc/os-release ]; then
            . /etc/os-release
            OS=$ID
        else
            OS="unknown"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    else
        OS="unknown"
    fi
    echo $OS
}

install_linux() {
    echo "Detected Linux distribution: $1"
    echo ""
    
    case $1 in
        ubuntu|debian|pop|mint)
            echo "Installing dependencies for Debian/Ubuntu..."
            sudo apt update
            sudo apt install -y build-essential git cmake
            sudo apt install -y libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev
            sudo apt install -y libgl1-mesa-dev libglu1-mesa-dev
            sudo apt install -y libasound2-dev
            
            echo "Installing raylib..."
            if ! dpkg -l | grep -q libraylib-dev; then
                cd /tmp
                git clone --depth 1 https://github.com/raysan5/raylib.git
                cd raylib
                mkdir -p build && cd build
                cmake -DBUILD_SHARED_LIBS=ON ..
                make -j$(nproc)
                sudo make install
                sudo ldconfig
                cd /tmp && rm -rf raylib
            else
                echo "raylib already installed"
            fi
            ;;
            
        fedora|rhel|centos)
            echo "Installing dependencies for Fedora/RHEL..."
            sudo dnf groupinstall -y "Development Tools"
            sudo dnf install -y git cmake
            sudo dnf install -y libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel
            sudo dnf install -y mesa-libGL-devel mesa-libGLU-devel
            sudo dnf install -y alsa-lib-devel
            
            echo "Installing raylib..."
            cd /tmp
            git clone --depth 1 https://github.com/raysan5/raylib.git
            cd raylib
            mkdir -p build && cd build
            cmake -DBUILD_SHARED_LIBS=ON ..
            make -j$(nproc)
            sudo make install
            sudo ldconfig
            cd /tmp && rm -rf raylib
            ;;
            
        arch|manjaro)
            echo "Installing dependencies for Arch Linux..."
            sudo pacman -Syu --noconfirm
            sudo pacman -S --noconfirm base-devel git cmake
            sudo pacman -S --noconfirm libx11 libxrandr libxi libxcursor libxinerama
            sudo pacman -S --noconfirm mesa
            sudo pacman -S --noconfirm alsa-lib
            sudo pacman -S --noconfirm raylib
            ;;
            
        *)
            echo "Unknown Linux distribution. Attempting generic installation..."
            echo "Please install manually: build-essential, cmake, X11 dev libs, OpenGL, raylib"
            exit 1
            ;;
    esac
}

install_macos() {
    echo "Installing dependencies for macOS..."
    
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found. Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    
    echo "Installing dependencies via Homebrew..."
    brew install cmake
    brew install raylib
}

OS=$(detect_os)

case $OS in
    ubuntu|debian|pop|mint|fedora|rhel|centos|arch|manjaro)
        install_linux $OS
        ;;
    macos)
        install_macos
        ;;
    *)
        echo "Unsupported operating system: $OS"
        echo "Please install dependencies manually."
        exit 1
        ;;
esac

echo ""
echo "========================================="
echo "  Building Parking Management System"
echo "========================================="
echo ""

cd "$(dirname "$0")"

if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo "Compiling..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "  Installation Complete!"
    echo "========================================="
    echo ""
    echo "To run the application:"
    echo "  cd build"
    echo "  ./parking_lot"
    echo ""
else
    echo ""
    echo "Build failed. Please check the error messages above."
    exit 1
fi
