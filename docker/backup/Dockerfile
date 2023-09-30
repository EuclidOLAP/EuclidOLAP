# docker build -t euclidolap:<version> .
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap:<version>
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap/euclidolap:v0.1.4-beta

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

COPY jdk-17_linux-x64_bin.tar.gz /usr/local/
COPY olapweb.jar /usr/local/

WORKDIR /usr/local/olap/bin/

CMD rm -rf /usr/local/jdk-17.0.3.1 \
&& tar zxf /usr/local/jdk-17_linux-x64_bin.tar.gz -C /usr/local/ \
&& export JAVA_HOME=/usr/local/jdk-17.0.3.1 \
&& export JRE_HOME=${JAVA_HOME}/jre \
&& export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib \
&& export PATH=${JAVA_HOME}/bin:$PATH:/usr/local/olap/bin/ \
&& chmod +x /usr/local/olap/bin/* \
&& echo '#!/bin/bash' > /usr/local/euclidolap-server-startd.sh \
&& echo '/usr/local/olap/bin/start.sh > /usr/local/olap/euclid-svr-stdout.log 2> /usr/local/olap/euclid-svr-stderr.log &' >> /usr/local/euclidolap-server-startd.sh \
&& chmod +x /usr/local/euclidolap-server-startd.sh \
&& /usr/local/euclidolap-server-startd.sh \
# && sleep 3 \
&& java -jar /usr/local/olapweb.jar --server.port=8761 --euclidolap.predefinedServices=127.0.0.1:8760 > /usr/local/olapweb.log 2> /usr/local/olapweb-err.log