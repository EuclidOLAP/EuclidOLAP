# EuclidOLAP

www.euclidolap.com



## Introduce

EuclidOLAP is a multidimensional database product that enables users to easily analyze data in real-time. With its in-memory technology, EuclidOLAP provides fast query performance, making it an ideal solution for organizations that require quick access to their data.

![olap](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/img/olap.png){width=400px}

One of the key benefits of EuclidOLAP is its ability to present data in a true multidimensional model, this allows users to view data from multiple angles. Additionally, EuclidOLAP offers a range of analysis capacities, including roll-up, drill-down, pivot, slicing, and dicing, which make it easy for users to query and analyze data in any way they want.

EuclidOLAP also helps users dig deeper into the valuable information hidden in their data. With its advanced analytical capabilities, EuclidOLAP allows users to uncover trends, patterns, and insights that would be difficult or impossible to detect using traditional methods.



## Hello World

Run the following command, quickly launch a Docker container of EuclidOLAP server.
```
docker run -d -p 8760:8760 -p 8761:8761 --name olap euclidolap/euclidolap:v0.1.3-beta
```

The container hosting two services, the **EuclidOLAP** service and the **OLAP Web** service, is now operational.
**EuclidOLAP** occupies port 8760, is a multidimensional database server-side process.
**OLAP Web** occupies port 8761, is a web management console for the EuclidOLAP.

A demo data model of airline turnover has been built in, which correlates three dimensions: Date, Aircraft Type and Service Type, and a Turnover measure that represents detailed turnover value.

Here are two MDX (Multidimensional Expressions) query statements that are used to query the demo data model.

```
select
{[Date].[ALL].[2020], [Date].[ALL].[2021], [Date].[ALL].[2022]} on rows,
{
    [Aircraft Type].[ALL].Boeing.[Boeing 747], 
    [Aircraft Type].[ALL].Boeing.[Boeing 777], 
    [Aircraft Type].[ALL].Boeing.[Boeing 787 Dreamliner]
} on columns
from [Airline Turnover];
```
The first MDX statement queries the annual turnover of each Boeing aircraft type, aircraft types was displayed on rows, and years was displayed on columns, since the cube has only one measure that is Turnover, it is displayed by default.



```
with member [measure].proportion as ([measure].Turnover) / ([Service Type].[ALL], [measure].Turnover)
select
{Date.[ALL].[2020],Date.[ALL].[2021],Date.[ALL].[2022]} on 1,
members([Service Type], LEAFS) on 0
from [Airline Turnover]
where ([measure].proportion, [Aircraft Type].[ALL].[Airbus].[Airbus A380]);
```
The second MDX statement will calculate the percentage of total turnover that comes from each service type, broken down by year. This will be done using a custom formula, the formula will be defined as a member in the query language.


Next, you can execute the MDX statements above in two ways.

### Use a browser to access the OLAP Web.
Use a browser to access the 192.168.66.236:8761

> **note:** You need to change the IP to the address of your server that running the EuclidOLAP docker container.



Click the connect button directly.
![olapweb-idx](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/github-readme/olapweb-idx.png)



Enter the MDX statements and click the exec button to execute the query.
![olapweb-demo-q1](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/github-readme/olapweb-demo-q1.png)
![olapweb-demo-q2](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/github-readme/olapweb-demo-q2.png)



### Go inside the container and using the EuclidOLAP client tool.

Go inside the container.
```
docker exec -it olap /bin/bash
```



Execute the following command, demo MDX statements above has been written to the demo-airline.txt file.

```
./euclid-cli --file=demo-airline.txt
```

The query results are displayed as shown below.
![demo-result.png](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/github-readme/demo-result.png)
