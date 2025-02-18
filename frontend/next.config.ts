/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  async rewrites() {
    return [
      {
        source: "/api/processes/:count",
        destination: "http://localhost:8080/api/processes/:count", // Fixed URL
      },
      {
        source: "/api/schedule",
        destination: "http://localhost:8080/api/schedule", // Added missing route
      },
    ];
  },
};

module.exports = nextConfig;
