FROM debian:bullseye

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    [any other dependencies]

WORKDIR /app

# Copy your source code
COPY . .

# Build your application
RUN cmake . && make

# Expose the port your app runs on
EXPOSE 8080

# Command to run your executable
CMD ["./process_schdeduler"]