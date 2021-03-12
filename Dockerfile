FROM debian:9

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        ccache \
        cmake \
        curl \
        gawk \
        gcc \
        g++ \
        git \
        grep \
        lbzip2 \
        libncurses-dev \
        make \
        ninja-build \
        wget \
        zip \
    && apt-get autoclean -y && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -fr /var/lib/apt

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        lsb-release \
        software-properties-common \
        apt-transport-https \
        gnupg \
        ca-certificates \
    && echo "deb http://apt.llvm.org/stretch/ llvm-toolchain-stretch-11 main" >> "/etc/apt/sources.list.d/llvm-11.list" && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
        clang-format-11 \
    && apt-get autoclean -y && \
    apt-get autoremove -y && \
    apt-get clean && \
    rm -fr /var/lib/apt
