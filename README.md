<!-- https://www.cnblogs.com/Tifahfyf/p/18853500 -->

# EuclidOLAP

# What is a Multidimensional Database?

From the perspective of how data is organized, databases can generally be classified into the following categories:

## Relational Database
Data is stored in two-dimensional tables (rows and columns), much like spreadsheets. Best suited for recording structured information such as customer lists, order records, and inventory details. Commonly used in daily transaction processing systems (OLTP).

## Graph Database
Data is organized as "nodes" and "edges," focusing on the relationships between entities — such as friendship networks, supply chains, or transaction flows. Ideal for managing complex networks like social media, financial risk control, and fraud detection.

## Time-Series Database
Designed specifically to record data over time, like a timeline where new data points are continuously added. Suitable for monitoring systems, logging, and IoT sensor data.

## Multidimensional Database
Data is organized as a "hypercube" in multidimensional space. Each axis represents a business dimension — for example, Region, Date, Organization, Task, Equipment, Event, or Status. Each data point corresponds to a specific position in this multidimensional space.

In a multidimensional database, we typically distinguish:
- Dimension: The perspectives for analysis, such as Time, Region, Department.
- Measure: The numerical values being calculated or aggregated, such as Sales Amount, Inventory Quantity, Completion Rate.

Compared to traditional relational databases, a multidimensional database offers significant advantages:
- Multi-perspective analysis: Freely combine different dimensions for cross-analysis — e.g., view sales data by Region, Month, and Product Category.
- Fast aggregation and drill-down: Easily summarize or zoom into details — from national sales to city-level, even down to a specific product on a specific day.
- Better decision support: Enables faster discovery of trends, issues, and opportunities, especially when handling large, complex datasets.

Typical application scenarios include:
- Military battlefield situation analysis
- Government resource planning and logistics
- Business sales, finance, and supply chain performance analysis

A multidimensional database forms the core foundation of OLAP (Online Analytical Processing) systems and is an essential technology for complex data analysis.