from setuptools import setup, find_packages

setup(
    name='pyolap',
    version='0.1.6',
    packages=find_packages(),

    install_requires=['wcwidth'],

    author="czg",
    author_email="euclidolap@outlook.com",

    description="Python Olap on EuclidOLAP.",

    url="https://github.com/EuclidOLAP/EuclidOLAP",

    keywords=["olap", "analysis", "BI", "statistics", "dimensional", "cube"]
)
