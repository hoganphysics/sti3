## manylinux

# Use a base Linux distribution image
# FROM ubuntu:23.10
FROM quay.io/pypa/manylinux_2_28_x86_64

# Set environment variables if needed
#ENV MY_VARIABLE=value
ENV PATH="/opt/python/cp310-cp310/bin:${PATH}"

# Set the shell to /bin/bash
SHELL ["/bin/bash", "-c"]

# Install necessary tools and dependencies
RUN dnf -y update && \
    dnf -y install \
    gcc-c++ \
    make \
    cmake \
    git \
    wget \
    python3 \
    python3-pip \
    python3-devel \
    openssl-devel \
    openssl \
    libcurl-devel \
    boost-devel



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

# RUN dnf -y install \
#     libcurl4-openssl-dev
# RUN dnf install -y libcurl-devel

# Copy source code into the container
# COPY ../../../sti3 /sti3
COPY . /sti3

# Generate IDL stubs
WORKDIR /sti3/src/network
RUN ./compileIDL.sh


# Create a symbolic link to the Python interpreter
RUN ln -s /opt/python/cp310-cp310/bin/python3 /usr/local/bin/python

# Set the Python executable for CMake
ENV Python_EXECUTABLE=/usr/local/bin/python

RUN dnf install -y python3-devel

# *** Need to modify sti3/CMakeList.txt FindPython to take Development.Module

# Build STI
WORKDIR /sti3
RUN mkdir -p /sti3/build
WORKDIR /sti3/build
RUN rm -rf *
RUN cmake -DOPENSSL_ROOT_DIR=/usr/lib64/ -DOPENSSL_CRYPTO_LIBRARY=/usr/lib64/libcrypt.so -DOPENSSL_SSL_LIBRARY=/usr/lib64/libssl.so ..

# RUN cmake ..
# RUN cmake --build . --parallel 20
# RUN make DESTDIR=/sti3 install

# Define the default command to run when the container starts
CMD ["/bin/bash"]
