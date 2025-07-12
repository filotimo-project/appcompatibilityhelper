image_name := "localhost/windowsapphelper:dev"
default: build

build-container:
    #!/usr/bin/env bash
    echo "Building development container image..."
    podman build -t {{image_name}} -f Containerfile .

build: build-container
    #!/usr/bin/env bash
    set -euo pipefail        
    mkdir -p build
    
    podman run --rm \
        --userns=keep-id \
        --volume "$(pwd):/workspace:Z" \
        --workdir /workspace \
        {{image_name}} \
        bash -c '
            set -euo pipefail
            # Configure and build the project
            cd build
            cmake .. \
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE:-Release} \
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX:-/usr}
            
            make -j$(nproc)
        '

install: build
    #!/usr/bin/env bash
    set -euo pipefail

    podman run --rm \
        --userns=keep-id \
        --volume "$(pwd):/workspace:Z" \
        --workdir /workspace/build \
        {{image_name}} \
        bash -c '
            # Install to prefix directory
            echo "Installing to prefix directory..."
            mkdir -p /workspace/build/prefix
            make install DESTDIR=/workspace/build/prefix
        '

    echo "Copying files to system directories..."
    sudo ostree admin unlock || true
    sudo cp -r build/prefix/usr/* /usr/
