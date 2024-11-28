FROM ubuntu:latest

# Install dependencies gcc
RUN apt-get update && apt-get install -y \
    gcc g++\
    && rm -rf /var/lib/apt/lists/*

# add in the files
COPY ./calculate_pi.cpp /build/calculate_pi.cpp

# compile the code
RUN g++ /build/calculate_pi.cpp -o /build/calculate_pi -fopenmp

CMD ["/build/calculate_pi 1000000"]
# ENTRYPOINT [ "/build/calculate_pi" ]