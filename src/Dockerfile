# docker build -t euclidolap:0.1 .
FROM centos:7.9.2009

RUN mkdir -p /usr/local/olap/data/
RUN mkdir -p /usr/local/olap/meta/

COPY demo-*.txt /usr/local/olap/
COPY server /usr/local/olap/
COPY euclid /usr/local/olap/
COPY euclid-cli.conf /usr/local/olap/

COPY data/* /usr/local/olap/data/
COPY meta/* /usr/local/olap/meta/

WORKDIR /usr/local/olap/

CMD /usr/local/olap/server