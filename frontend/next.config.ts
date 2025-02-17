/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  async rewrites() {
    return [
      {
        source: "/api/processes",
        destination: "http://localhost:8080/processes",
      },
    ];
  },
};

module.exports = nextConfig;
