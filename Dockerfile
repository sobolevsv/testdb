FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y \
      build-essential \
      libboost-dev libboost-program-options-dev \
      cmake

WORKDIR /build

COPY . .

RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN make

ENTRYPOINT ["./columndb"]
