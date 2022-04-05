# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
FROM ubuntu:20.04

EXPOSE 80

## for apt to be noninteractive
ENV DEBIAN_FRONTEND noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN true

RUN apt-get update && apt-get install -y \
    apache2 \
    python2 \
    wget \
    gcc \
    build-essential \
    libgmp3-dev \
    python-dev \
    libxml2 \
    libxml2-dev \
    zlib1g-dev \ 
    libtool \
    bison \
    flex

# Python 2 needs special pip download
RUN alias python=python2
RUN wget https://bootstrap.pypa.io/pip/2.7/get-pip.py
RUN python2 get-pip.py
# libraries need second line for installing old python-igraph from source

# need to install old python-igraph (no 2.7 compatibility with new one)
RUN alias yacc="bison"
RUN wget https://github.com/igraph/python-igraph/releases/download/0.8.2/python-igraph-0.8.2.tar.gz
RUN pip install python-igraph-0.8.2.tar.gz

# These commands copy your files into the specified directory in the image
# and set that as the working location
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp

# This command compiles your app using GCC, adjust for your source code
RUN make install

# setup apache
RUN a2enmod cgi
COPY apache.conf /etc/apache2/sites-enabled/000-default.conf

### could only get to run with permissions 777
### instructions for change env, but couldn't figure out
# RUN chmod -R 750 install/web/
# RUN chmod g+s install/web/
# RUN chmod g+w install/web/data/

RUN chmod -R 777 /usr/src/myapp/install/web

RUN echo "ServerName localhost" >> /etc/apache2/apache2.conf
RUN service apache2 restart
CMD apachectl -D FOREGROUND
LABEL Name=occam Version=0.0.1
