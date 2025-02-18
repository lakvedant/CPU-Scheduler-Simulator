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

# Build the application
RUN mkdir -p build && cd build && cmake .. && make 

# Expose the port your application uses
EXPOSE 8080

# Command to run the executable
CMD ["./build/process_scheduler"]