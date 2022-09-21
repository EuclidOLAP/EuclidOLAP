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

There are two cubes `Online Store` and `Logistics.test` in the image above.

`Online Store` associates four dimensions `Store Type`, `Payment Method`, `Goods` and `Calendar`.

The cube `Logistics.test` is somewhat special. The two dimensions `Region` and `Calendar` are each associated with `Logistics.test` twice. In EuclidOLAP, the same dimension can be associated with the same cube multiple times, which means that this dimension play different roles in the business modeling of the cube.



A dimension includes all members of the business direction it describes. On a cube, select a specific member in each dimension, and the combination of these members will point to a specific measure value.

![modeling](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/images/modeling.png)

All the measure values of a cube can be imagined as being stored in a huge multi-dimensional array. This huge multi-dimensional array will be compressed first and then stored in memory. EuclidOLAP uses this data structure to provide external query capabilities for logical multidimensional data.

In EuclidOLAP, 1G memory can store 60 millions to 100 millions leaf measure values.