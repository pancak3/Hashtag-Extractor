FROM alpine:latest
RUN apk add --no-cache -X http://dl-cdn.alpinelinux.org/alpine/edge/testing openmpi openmpi-dev
RUN apk add --no-cache libgomp make g++
COPY . /build
RUN cd /build && make
