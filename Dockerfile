FROM debian

LABEL description="Environment to build the AIME3 MUD source code"

RUN apt-get -y update && apt-get install -y && \
    apt-get install -y binutils make csh sed gawk \
    g++ libconfig++ libconfig++-dev automake

COPY . /opt/aime3

WORKDIR /opt/aime3

RUN mkdir -p /opt/aime3/m4 && autoreconf -i && ./configure && make

EXPOSE 6715

VOLUME ["/opt/aime3/data", "/opt/aime3/src"]

CMD ["./src/aime3"]