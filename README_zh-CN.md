<!-- [Read English Version](README.md) -->

# EuclidOLAP
多维数据库内核

## 什么是多维数据库？

从数据库产品所提供的数据组织方式来看，数据库可以做如下分类：

### 关系型数据库（Relational Database）
![Relational Database](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/img/relational_database.png "Relational Database")
数据以二维表格（行与列）的方式存储，就像一个又一个电子表格。适用于记录结构化信息，如客户名单、订单记录、库存清单等。典型应用场景是日常事务处理系统（OLTP）。

### 图数据库（Graph Database）
![Graph Database](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/img/graph_database.png "Graph Database")
数据以“节点（Node）+边（Edge）”的方式组织，强调实体之间的关系，比如“朋友关系”、“供应链关系”、“交易链条”。非常适合处理社交网络、金融风控、反欺诈等复杂关系网络。

### 时序数据库（Time-Series Database）
![Time-Series Database](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/img/time-series_database.png "Time-Series Database")
专门为按时间顺序记录的数据设计，可以想象成一条时间轴，数据按时间点不断增加。适合用于监控、日志管理、传感器数据采集等场景。

### 多维数据库（Multidimensional Database）
![Multidimensional Database](https://euclidolap-presentations.oss-us-west-1.aliyuncs.com/img/multidimensional_database.png "Multidimensional Database")
数据以多维空间中的“超立方体”（Hypercube）方式组织。每一个坐标轴代表一个业务维度，比如地区、日期、组织、任务、装备、事件、状态等。每条数据就像落在一个坐标点上。

在多维数据库中，通常会区分：
- 维度（Dimension）：分析的角度，比如“日期”、“组织”、“地区”。
- 度量（Measure）：需要被统计或计算的数值，比如“销售额”、“库存量”、“完成率”。

多维数据库在数据分析领域与关系型数据库相比，具有显著优势：
- 多角度分析：可以自由组合维度进行交叉分析，比如“按地区、按月份、按产品类别”查看销售数据。
- 快速汇总与下钻：支持灵活地从宏观到细节地查看数据，比如从“全国销售额”下钻到“某城市某天某产品”的销售。
- 高效支持决策：帮助决策者快速找到趋势、问题和机会，尤其在数据量大、结构复杂时优势更明显。

常见的应用场景包括：
- 企业销售分析、财务分析、供应链效率评估
- 政府资源调度与保障
- 国防领域的战场态势分析等

多维数据分析能力是OLAP系统的关键特性，是衡量其是否真正具备深度洞察力与支撑复杂业务决策、多场景分析能力的核心标准。