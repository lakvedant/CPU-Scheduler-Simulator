FROM debian:bullseye

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    curl

# Create app directory
WORKDIR /app

# Copy CMake configuration and source files
COPY CMakeLists.txt .
COPY main.cpp .
COPY ./vcpkg ./vcpkg

# Install Crow using vcpkg
RUN ./vcpkg/bootstrap-vcpkg.sh && \
    ./vcpkg/vcpkg install crow

# Build the application
RUN mkdir -p build && cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=/app/vcpkg/scripts/buildsystems/vcpkg.cmake && make 

# Expose the port your application uses
EXPOSE 8080

# Command to run the executable
CMD ["./build/process_scheduler"]