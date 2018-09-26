FROM ubuntu:18.04

ENV DEBIAN_FRONTEND noninteractive
ENV LIBVA_VERSION 2.2.1.pre1-20180921
ENV LIBVAUTILS_VERSION 2.2.1.pre1-20180921
ENV MEDIADRIVER_VERSION intel-media-18.3.pre3
ENV GMMLIB_VERSION intel-gmmlib-18.3.pre1
ENV MSDK_VERSION intel-mediasdk-18.2.pre1


RUN apt-get update && \
    apt-get install -y \
    autoconf \
    git \
    curl \
    libdrm-dev \
    libgl1-mesa-glx \
    libgl1-mesa-dev \
    libtool \
    libx11-dev \
    openbox \
    unzip \
    xorg \
    xorg-dev \
    && apt-get clean all

RUN mkdir /opt/src && \
    cd /opt/src && \
    curl -sSLO https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.sh && \
    sh cmake-3.8.2-Linux-x86_64.sh --prefix=/usr/local --exclude-subdir && \
    rm cmake-3.8.2-Linux-x86_64.sh

RUN cd /opt/src && \
    curl -sSLO https://www.samba.org/ftp/ccache/ccache-3.2.8.tar.bz2 && \
    tar xf ccache-3.2.8.tar.bz2 && \
    cd ccache-3.2.8 && \
    ./configure --prefix=/usr && \
    make && \
    make install

RUN mkdir -p /usr/lib/ccache && \
    cd /usr/lib/ccache && \
    ln -sf /usr/bin/ccache gcc && \
    ln -sf /usr/bin/ccache g++ && \
    ln -sf /usr/bin/ccache cc && \
    ln -sf /usr/bin/ccache c++ && \
    ln -sf /usr/bin/ccache clang && \
    ln -sf /usr/bin/ccache clang++ && \
    ln -sf /usr/bin/ccache clang-4.0 && \
    ln -sf /usr/bin/ccache clang++-4.0

RUN cd /opt/src && \
    curl -o libva.zip -sSL https://github.com/intel/libva/archive/${LIBVA_VERSION}.zip && \
    unzip libva.zip && \
    cd libva-${LIBVA_VERSION} && \
    ./autogen.sh --prefix=/usr --libdir=/usr/lib/x86_64-linux-gnu && \
    make -j8 && \
    make install

RUN cd /opt/src && \
    curl -o gmmlib.zip -sSL https://github.com/intel/gmmlib/archive/${GMMLIB_VERSION}.zip && \
    unzip gmmlib.zip && \
    mkdir build_gmmlib && \
    cd build_gmmlib && \
    cmake ../gmmlib-${GMMLIB_VERSION} && \
    make -j8 && \
    make install

RUN cd /opt/src && \
    curl -o media-driver.zip -sSL https://github.com/intel/media-driver/archive/${MEDIADRIVER_VERSION}.zip && \
    unzip media-driver.zip && \
    mkdir build_media && \
    cd build_media && \
    cmake ../media-driver-${MEDIADRIVER_VERSION} && \
    make -j8 && \
    make install

RUN cd /opt/src && \
    curl -o libva-utils.zip -sSL https://github.com/intel/libva-utils/archive/${LIBVAUTILS_VERSION}.zip && \
    unzip libva-utils.zip && \
    cd libva-utils-${LIBVAUTILS_VERSION} && \
    ./autogen.sh && \
    ./configure --prefix=/usr && \
    make -j8 && \
    make install && \
    make check

RUN cd /opt/src && \
    curl -o msdk.zip -sSL https://github.com/Intel-Media-SDK/MediaSDK/archive/${MSDK_VERSION}.zip && \
    unzip msdk.zip && \
    mkdir build_msdk && \
    cd build_msdk && \
    cmake ../MediaSDK-${MSDK_VERSION} && \
    make -j8 && \
    make install

ADD ubuntu_export.sh /opt/src/ubuntu_export.sh
