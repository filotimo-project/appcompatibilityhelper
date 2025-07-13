FROM fedora:42
RUN dnf5 update -y && \
    dnf5 install -y \
        cmake \
        make \
        gcc \
        gcc-c++ \
        rpm-build \
        rpmdevtools \
        extra-cmake-modules \
        kf6-krunner-devel \
        kf6-ki18n-devel \
        kf6-kcoreaddons-devel \
        kf6-kdbusaddons \
        kf6-kirigami-devel \
        kf6-kio-devel \
        qt6-qtbase-devel \
        qt6-qtdeclarative-devel \
        qt6-qtsvg-devel \
        qt6-qt5compat-devel \
        && dnf5 clean all

# Set up workspace directory
RUN mkdir -p /workspace
WORKDIR /workspace

# Set environment variables for development
ENV CMAKE_BUILD_TYPE=Release
ENV CMAKE_INSTALL_PREFIX=/usr
ENV QT_QPA_PLATFORM=offscreen

# Default command
CMD ["/bin/bash"]
