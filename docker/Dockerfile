# docker build -t euclidolap:<version> .
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap:<version>
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap/euclidolap:v0.1.4.1

FROM centos:7.9.2009

RUN mkdir -p /usr/local/olap/bin/
RUN mkdir -p /usr/local/olap/config/
RUN mkdir -p /usr/local/olap/data/
RUN mkdir -p /usr/local/olap/log/
RUN mkdir -p /usr/local/olap/profiles/

COPY bin/* /usr/local/olap/bin/
COPY config/* /usr/local/olap/config/
COPY data/* /usr/local/olap/data/
COPY log/* /usr/local/olap/log/
COPY profiles/* /usr/local/olap/profiles/

# COPY jdk-17_linux-x64_bin.tar.gz /usr/local/
# COPY olapweb.jar /usr/local/

WORKDIR /usr/local/olap/bin/

CMD /usr/local/olap/bin/start.sh > /usr/local/olap/euclid-svr-stdout.log 2> /usr/local/olap/euclid-svr-stderr.log