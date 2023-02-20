# docker build -t euclidolap:<version> .
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap:<version>
# docker run -d -p 8760:8760 -p 8761:8761 --name euclidolap euclidolap/euclidolap:v0.1.2-beta

FROM centos:7.9.2009

RUN mkdir -p /usr/local/olap/data/
RUN mkdir -p /usr/local/olap/meta/

COPY demo-*.txt /usr/local/olap/
COPY euclid-svr /usr/local/olap/
COPY euclid-svr.conf /usr/local/olap/
COPY euclid-cli /usr/local/olap/

COPY data/* /usr/local/olap/data/
COPY meta/* /usr/local/olap/meta/

COPY docker-depending/jdk-17_linux-x64_bin.tar.gz /usr/local/
COPY docker-depending/olapweb.jar /usr/local/

WORKDIR /usr/local/olap/

CMD rm -rf /usr/local/jdk-17.0.3.1 \
&& tar zxf /usr/local/jdk-17_linux-x64_bin.tar.gz -C /usr/local/ \
# && rm -f /usr/local/jdk-17_linux-x64_bin.tar.gz \
# && echo 'export JAVA_HOME=/usr/local/jdk-17.0.3.1' >> /root/.bashrc \
# && echo 'export JRE_HOME=${JAVA_HOME}/jre' >> /root/.bashrc \
# && echo 'export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib' >> /root/.bashrc \
# && echo 'export PATH=${JAVA_HOME}/bin:$PATH' >> /root/.bashrc \
&& export JAVA_HOME=/usr/local/jdk-17.0.3.1 \
&& export JRE_HOME=${JAVA_HOME}/jre \
&& export CLASSPATH=.:${JAVA_HOME}/lib:${JRE_HOME}/lib \
&& export PATH=${JAVA_HOME}/bin:$PATH \
&& echo '#!/bin/bash' > /usr/local/euclidolap-server-startd.sh \
&& echo '/usr/local/olap/euclid-svr > /usr/local/olap/euclid-svr-stdout.log 2> /usr/local/olap/euclid-svr-stderr.log &' >> /usr/local/euclidolap-server-startd.sh \
&& chmod +x /usr/local/euclidolap-server-startd.sh \
&& /usr/local/euclidolap-server-startd.sh \
# && sleep 3 \
&& java -jar /usr/local/olapweb.jar --server.port=8761 --euclidolap.predefinedServices=127.0.0.1:8760 > /usr/local/olapweb.log 2> /usr/local/olapweb-err.log