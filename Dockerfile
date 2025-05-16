# Set the base image
FROM ubuntu:22.04                

# Set a working directory inside the container
WORKDIR /app

# Copy project into working directory
COPY ./src /app/src
COPY ./test /app/test

# Install dependencies from working directory
COPY install/lib/ /usr/local/lib/
COPY install/include/ /usr/local/include/

# Install dependencies from apt-get 
RUN apt-get update \
 && apt-get install -y build-essential \
 && apt-get install -y libusb-1.0.0-dev \
 && apt-get install -y libssl-dev \
 && apt-get install -y cmake \
 && apt-get install -y libboost-all-dev \
 && apt-get install -y pkg-config 

# Compile the code
RUN mkdir /app/build \
  && cd /app/build \
  && cmake ../test/proxy \
  && make

# Set the entry point to run your application
ENTRYPOINT ["/app/build/proxy"]




