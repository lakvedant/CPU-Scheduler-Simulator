# Use Debian Bullseye as the base image
FROM debian:bullseye

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    openssl \
    libssl-dev \
    zlib1g-dev \
    curl \
    libasio-dev  # Correct package name for Debian

# Install Crow (C++ Web Framework)
RUN git clone https://github.com/CrowCpp/Crow.git /tmp/crow \
    && cd /tmp/crow \
    && mkdir build && cd build \
    && cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF \
    && make install \
    && cd / && rm -rf /tmp/crow

# Create application directory
WORKDIR /app

# Copy CMake configuration and source files
COPY CMakeLists.txt .
COPY main.cpp .

# Build the application
RUN mkdir -p build && cd build && cmake .. && make 

# Expose the port your application uses
EXPOSE 8080

# Command to run the executable
CMD ["./build/process_scheduler"]
