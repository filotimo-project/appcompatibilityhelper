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
        --volume "$(pwd):/workspace" \
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
        --volume "$(pwd):/workspace" \
        --workdir /workspace/build \
        {{image_name}} \
        bash -c '
            # Install to host system
            echo "Installing to host system..."
            mkdir -p /workspace/build/prefix
            make install DESTDIR=/workspace/build/prefix
        '
    
    #sudo cp build/prefix/usr/lib64/qt6/plugins/kf6/krunner/bazaarrunner.so /usr/lib64/qt6/plugins/kf6/krunner/bazaarrunner.so
