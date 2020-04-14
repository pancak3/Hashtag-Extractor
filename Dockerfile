FROM alpine:latest
RUN apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing openmpi openmpi-dev
RUN apk add --no-cache libgomp make g++
RUN apk add --no-cache openssh valgrind
COPY . /build
RUN cd /build && make clean && make
RUN cd /build && mpirun --allow-run-as-root -np 4 ./tp smallTwitter.json lang.csv
