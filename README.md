# EuclidOLAP

EuclidOLAP is an in-memory multi-dimensional database that presents data in a logical multi-dimensional data model and provides insight into the intelligence information within the data.

- EuclidOLAP has the ability to perform real-time aggregation operations without pre-aggregation of data in advance.
- One cube has the ability to repeatedly associate the same dimension, the dimension plays different dimensional roles.
- A query can be associated with multiple cubes, and queries associated with multiple cubes will not cause a significant drop in query performance.
- Support for MDX(Mutil Dimensional Expressions), which is more suitable for querying multi-dimensional data models than SQL.

## Basic Concept

Dimensions and cubes are core concepts in multidimensional domain models.

A dimension is similar to a coordinate axis, and any two different dimensions are perpendicular to each other, a cube is similar to a multidimensional space.

One cube is associated with at least one dimension, the dimensions associated with the cube indicate the business modeling of the cube, and the cube itself stores quantifiable data, such as sales amount, cost, profit, etc.

![dim_dimrole_cube](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/images/dim_dimrole_cube.png)

There are two cubes **`Online Store`** and **`Logistics.test`** in the image above.

**`Online Store`** associates four dimensions **`Store Type`**, **`Payment Method`**, **`Goods`** and **`Calendar`**.

The cube **`Logistics.test`** is somewhat special. The two dimensions **`Region`** and **`Calendar`** are each associated with **`Logistics.test`** twice. In EuclidOLAP, the same dimension can be associated with the same cube multiple times, which means that this dimension play different roles in the business modeling of the cube.



A dimension includes all members of the business direction it describes. On a cube, select a specific member in each dimension, and the combination of these members will point to a specific measure value.

![modeling](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/images/modeling.png)

All the measure values of a cube can be imagined as being stored in a huge multi-dimensional array. This huge multi-dimensional array will be compressed first and then stored in memory. EuclidOLAP uses this data structure to provide external query capabilities for logical multidimensional data.

In EuclidOLAP, 1G memory can store 60 millions to 100 millions leaf measure values.



## Hello World

### Requirements for environment

1. 64-bit Linux operating system is recommended, preferably **CentOS-7** or Redhat.
2. gcc compilation tool, preferably **gcc11**, some syntaxes may not be supported by lower versions of gcc compiler.
3. **flex**(lex in Linux), Lexical analysis tool.
4. **bison**(yacc in Linux), Syntax analysis tool.



If you are using CentOS-7, you can follow the steps below to prepare your program's compilation environment.

```shell
$ yum -y install centos-release-scl

$ yum -y install devtoolset-11-gcc

$ scl enable devtoolset-11 bash
```

Set the path of the gcc command into an environment variable.

```shell
$ which gcc
/opt/rh/devtoolset-11/root/usr/bin/gcc

$ cat >> /etc/profile << EOF
export PATH=/opt/rh/devtoolset-11/root/usr/bin:\$PATH
EOF

$ source /etc/profile
```

```shell
$ yum -y install flex

$ yum -y install bison
```



### Compile and run

Get the source code, compile the source code and give the binary execute permission.

```shell
$ git clone https://github.com/EuclidOLAP/EuclidOLAP.git

$ cd EuclidOLAP/src

$ make

$ chmod +x server

$ chmod +x euclid
```



Start up.

```shell
$ ./server &

$ cat log/euclid.log 
info - node mode [ m ]
EuclidCommand processor thread [140300922853120] <0>.
Net service startup on port 8760
```

After starting the service, check the log file. If there is **`Net service startup on port 8760`**, the startup is successful.



Create some metadata.

```shell
$ ./euclid --file=demo-meta.txt
$ ./euclid --file=demo-create-members.txt
```



Import measure data of cubes.

```shell
$ ./euclid --file=demo-data.txt
```



Execute MDX query.

```shell
$ ./euclid
```

```sh
olapcli > with member measure.SSSSSS as lookUpCube("logistics.test", "(measure.cost)") select {(measure.[sales amount]), (measure.SSSSSS)} on 0, children(Calendar.[ALL].[2021]) on 1 from [Online Store] ;
```



If the query executes successfully, the following information will be displayed on the console.

```
	 -        sales amount              SSSSSS
	Q1             4320.00        123718276.77
	Q2             4320.00        123718276.77
	Q3            22088.00        123718276.77
	Q4            21231.00        123718276.77
```

<video id="video" controls="" preload="none" poster="">
      <source id="mp4" src="https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/videos/olapweb.mp4" type="video/mp4">
</videos>