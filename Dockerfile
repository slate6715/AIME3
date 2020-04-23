FROM centos

LABEL description="Environment to build the AIME3 MUD source code"

RUN yum update -y

RUN yum install -y epel-release && yum install -y --enablerepo=PowerTools binutils \
    make sed gawk gcc-c++ libconfig libconfig-devel autoconf automake vim \
    nano python3 python3-devel boost boost-devel boost-python3 boost-python3-devel \
    libargon2 libargon2-devel

COPY . /opt/aime3

WORKDIR /opt/aime3

RUN autoreconf -i && PYTHON_VERSION=3 ./configure && make

EXPOSE 6715

VOLUME ["/opt/aime3/data", "/opt/aime3/src"]

CMD ["./src/aime3"]