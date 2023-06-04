![](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/github-readme/project.png)

[![Build](https://img.shields.io/badge/Build-passing-green)]()
[![Version](https://img.shields.io/badge/Version-v0.1.3--beta-yellow)](https://github.com/EuclidOLAP/EuclidOLAP/releases)
[![Docker](https://img.shields.io/badge/Docker-available-blue)]()
[![Website](https://img.shields.io/badge/Website-up-842fd3)](http://www.euclidolap.com/)
[![License](https://img.shields.io/badge/License-Apache--2.0-orange)]()

An in-memory multidimensional database.

# Table of contents
- [Overview](#overview)
- [Features](#features)
- [Quick start](#quick_start)
    - [Installation](#installation)
    - [Demo model](#demo_model)
    - [Query examples](#query_examples)
- [Why use EuclidOLAP?](#why_use_euclidolap)
- [Future planning](#future_planning)
- [License](#license)

# 1. Overview <a name="overview"></a>

<a href="http://www.euclidolap.com/" target="_blank">EuclidOLAP</a> organizes data through dimensions and cubes and provides the ability to perform in-depth analysis of data. EuclidOLAP can help you perform faster and more flexible data queries, data analysis, and data mining, whether for detailed or aggregated data, or cross-model correlation data.

![](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/img/olap-mdb-md.webp)

# 2. Features <a name="features"></a>
- Authentic multidimensional data model.
- Provide real-time aggregate computing capabilities for data of any granularity.
- Support for MDX(Multi-Dimensional Expressions).
- An EuclidOLAP service instance can be deployed in as little as one minute.
- Stand-alone mode or distributed architecture.

# 3. Quick start <a name="quick_start"></a>
Quickly deploy an instance of the EuclidOLAP service, and then perform multidimensional queries based on a demo model.

## 3.1. Installation <a name="installation"></a>
Run an EuclidOLAP service process on Linux by starting a Docker container or directly executing binary.

### 3.1.1. Docker
Run the following command, quickly launch a Docker container of EuclidOLAP service.
```bash
$ docker run -d -p 8760:8760 -p 8761:8761 --name olap euclidolap/euclidolap:v0.1.3-beta
```

### 3.1.2. Executing binary
Launch an EuclidOLAP service by executing binary on Linux.

On **Centos** or **RedHat** platform, execute the following code:
```bash
$ wget https://github.com/EuclidOLAP/EuclidOLAP/releases/download/v0.1.2-beta/EuclidOLAP_v0.1.2-beta_centos.tar.gz
$ tar zxf EuclidOLAP_v0.1.2-beta_centos.tar.gz
$ cd EuclidOLAP_v0.1.2-beta_centos
$ ./euclid-svr
```

On **Ubuntu** or **Debian** platform, execute the following code:
```bash
$ wget https://github.com/EuclidOLAP/EuclidOLAP/releases/download/v0.1.2-beta/EuclidOLAP_v0.1.2-beta_ubuntu.tar.gz
$ tar zxf EuclidOLAP_v0.1.2-beta_ubuntu.tar.gz
$ cd EuclidOLAP_v0.1.2-beta_ubuntu/
$ ./euclid-svr
```

## 3.2. Demo model <a name="demo_model"></a>
The EuclidOLAP service is preset with a sample data model, which is a cube of airline turnover data that correlates three dimensions: Aircraft Type Dimension, Service Type Dimension, and Date Dimension.
![](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/github-readme/demo-model-cube.webp)
For more details on this demo, refer to <a href="http://www.euclidolap.com/doc/concepts/md-model" target="_blank">Multidimensional Model</a> and <a href="http://www.euclidolap.com/doc/concepts/demo-example" target="_blank">Demo Example</a>.

## 3.3. Query examples <a name="query_examples"></a>
You can now execute multidimensional queries based on this sample model.

EuclidOLAP uses MDX(Multi-Dimensional Expressions) as query language, for more details on MDX please refer to <a href="http://www.euclidolap.com/doc/mdx-manual" target="_blank">MDX Manual</a>.

If you are running the EuclidOLAP service by starting a Docker container, execute the `docker exec -it olap /bin/bash` command to enter the Docker container first.

Execute the EuclidOLAP client tool.

```bash
$ ./euclid-cli
```

### 3.3.1. Example 1

The following MDX statement will query the turnover for the three years 2020 ~ 2022, run it in the EuclidOLAP client tool, and you will see the results in the table below.

```
select
{
  Date.[ALL].[2020],
  Date.[ALL].[2021],
  Date.[ALL].[2022]
} on columns,
{
  [measure].Turnover
} on rows
from [Airline Turnover];
```
<table>
    <tr>
        <td></td>
        <td>2020</td>
        <td>2021</td>
        <td>2022</td>
    </tr>
    <tr>
        <td>Turnover</td>
        <td>5475632678</td>
        <td>6559193037</td>
        <td>7444450321</td>
    </tr>
</table>

### 3.3.2. Example 2
The following MDX statement will query the turnover corresponding to each service type in 2022.

```
select
{
  [measure].Turnover
} on columns,
{
  [Service Type].[ALL].[Premium Economy Class],
  [Service Type].[ALL].[Basic Economy Class],
  [Service Type].[ALL].[Economy Plus],
  [Service Type].[ALL].[Business Suite],
  [Service Type].[ALL].[Premium Business],
  [Service Type].[ALL].[First Class Suite],
  [Service Type].[ALL].[Private Jet Charter]
} on rows
from [Airline Turnover];
```
<table>
    <tr><td></td><td>Turnover</td></tr>
    <tr><td>Premium Economy Class</td><td>3873331625</td></tr>
    <tr><td>Basic Economy Class</td><td>5495863584</td></tr>
    <tr><td>Economy Plus</td><td>4941650730</td></tr>
    <tr><td>Business Suite</td><td>2189850692</td></tr>
    <tr><td>Premium Business</td><td>1376448022</td></tr>
    <tr><td>First Class Suite</td><td>1053424586</td></tr>
    <tr><td>Private Jet Charter</td><td>548706797</td></tr>
</table>

### 3.3.3. Example 3
The following MDX statement queries the annual turnover of each Boeing aircraft type, aircraft types was displayed on rows, and years was displayed on columns, since the cube has only one measure that is Turnover, it is displayed by default.
```
select
{
  [Date].[ALL].[2020],
  [Date].[ALL].[2021],
  [Date].[ALL].[2022]
} on rows,
{
  [Aircraft Type].[ALL].Boeing.[Boeing 747], 
  [Aircraft Type].[ALL].Boeing.[Boeing 777], 
  [Aircraft Type].[ALL].Boeing.[Boeing 787 Dreamliner]
} on columns
from [Airline Turnover];
```
<table>
    <tr>
        <td></td>
        <td>Boeing 747</td>
        <td>Boeing 777</td>
        <td>Boeing 787 Dreamliner</td>
    </tr>
    <tr>
        <td>2020</td>
        <td>514564410</td>
        <td>709722281</td>
        <td>886223969</td>
    </tr>
    <tr>
        <td>2021</td>
        <td>604367094</td>
        <td>871148561</td>
        <td>1070311455</td>
    </tr>
    <tr>
        <td>2022</td>
        <td>705093902</td>
        <td>985844701</td>
        <td>1197908493</td>
    </tr>
</table>

### 3.3.4. Example 4
The following MDX statement will calculate the percentage of total turnover that comes from each service type, broken down by year. This will be done using a custom formula, the formula will be defined as a member in the query language.
```
with
member [measure].proportion as ([measure].Turnover) / ([Service Type].[ALL], [measure].Turnover)
select
{
  Date.[ALL].[2020],
  Date.[ALL].[2021],
  Date.[ALL].[2022]
} on columns,
members([Service Type], LEAFS) on rows
from [Airline Turnover]
where ([measure].proportion, [Aircraft Type].[ALL].[Airbus].[Airbus A380]);
```

<table>
    <tr>
        <td></td>
        <td>2020</td>
        <td>2021</td>
        <td>2022</td>
    </tr>
    <tr>
        <td>Premium Economy Class</td>
        <td>0.20</td>
        <td>0.20</td>
        <td>0.20</td>
    </tr>
    <tr>
        <td>Basic Economy Class</td>
        <td>0.29</td>
        <td>0.28</td>
        <td>0.27</td>
    </tr>
    <tr>
        <td>Economy Plus</td>
        <td>0.25</td>
        <td>0.25</td>
        <td>0.26</td>
    </tr>
    <tr>
        <td>Business Suite</td>
        <td>0.11</td>
        <td>0.12</td>
        <td>0.12</td>
    </tr>
    <tr>
        <td>Premium Business</td>
        <td>0.07</td>
        <td>0.07</td>
        <td>0.07</td>
    </tr>
    <tr>
        <td>First Class Suite</td>
        <td>0.05</td>
        <td>0.05</td>
        <td>0.06</td>
    </tr>
    <tr>
        <td>Private Jet Charter</td>
        <td>0.03</td>
        <td>0.03</td>
        <td>0.03</td>
    </tr>
</table>

# 4. Why use EuclidOLAP? <a name="why_use_euclidolap"></a>
Deploying the EuclidOLAP service is very simple, there is no need to rely on a running environment other than the Linux operating system, by starting a Docker container you can immediately run an EuclidOLAP stand-alone service, and it only takes a few minutes to build an EuclidOLAP distributed cluster service.

When you use EuclidOLAP for data analysis, you only need to care about the semantic layer close to the real business model, and do not need to care about problems such as data aggregation, sparse dimension and dense dimension that are not directly related to the business, so EuclidOLAP's Ad-Hoc query capability is very powerful, which can better support you to conduct exploratory and random data analysis behavior.

EuclidOLAP also extends the multidimensional data model, for example, adding the concept of dimensional role, which means that a cube can be associated with the same dimension multiple times, and each association indicates that this dimension represents a different role, which can better support certain data analysis scenarios, for example, statistical analysis of international trade ocean transport ship logistics data, which contains the ship's departure date and port call date.

# 5. Future planning <a name="future_planning"></a>
- Use GPU to improve the operation efficiency of real-time aggregate computing.
- Support real-time aggregation calculation and pre-aggregation calculation two modes.
- Use cloud-native elastic computing capabilities to achieve rapid real-time analysis of ultra-large-scale datasets (greater than 1,000 billion detailed data).
- Human-machine dialogue and machine assistant data analysis mode based on AI.

# 6. License <a name="license"></a>
EuclidOLAP is under <a href="http://www.apache.org/licenses/LICENSE-2.0" target="_blank">Apache License Version 2.0</a>.