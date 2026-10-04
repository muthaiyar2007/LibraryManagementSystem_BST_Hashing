FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends g++ && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN g++ -std=c++17 server.cpp -o library_server

EXPOSE 10000

CMD ["./library_server"]