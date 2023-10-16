# Use a base Linux distribution image
FROM ubuntu:23.10

# Set environment variables if needed
#ENV MY_VARIABLE=value

# Set the shell to /bin/bash
SHELL ["/bin/bash", "-c"]

# Install necessary tools and dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    python3 \
    python3-pip \
    python3-venv \
    libssl-dev \
    openssl \
    omniorb \
    libboost-graph-dev

# Virtual environment
# RUN mkdir -p /env
# WORKDIR /env
# RUN python3 -m venv .
# RUN source bin/activate

# RUN pip3 install --upgrade pip
RUN pip3 install --upgrade pip --break-system-packages

# Install pybind11
RUN pip3 install pybind11 --break-system-packages

# Download omniORB
RUN mkdir -p /src
WORKDIR /src
RUN wget https://sourceforge.net/projects/omniorb/files/omniORB/omniORB-4.3.1/omniORB-4.3.1.tar.bz2
RUN tar -xvf omniORB-4.3.1.tar.bz2

# Build omniORB
WORKDIR /src/omniORB-4.3.1
RUN mkdir build
WORKDIR /src/omniORB-4.3.1/build
RUN ../configure --with-openssl
RUN make
RUN make install

# Optionally, you can set the working directory
WORKDIR /sti3

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libcurl4-openssl-dev

# Optionally, copy your source code or files into the container
COPY . /sti3
WORKDIR /sti3
RUN mkdir -p /sti3/build
WORKDIR /sti3/build
RUN rm -rf *
RUN cmake ..
RUN cmake --build . --parallel 4
RUN make DESTDIR=/sti3 install

# Define the default command to run when the container starts
CMD ["/bin/bash"]
