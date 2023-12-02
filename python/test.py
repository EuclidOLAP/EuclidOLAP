from pytholap import *
import demo_standard_metadata


def create_standard_demo_metadata():
    olap_cli = OlapClient("192.168.66.66", 8760)
    olap_cli.connect()
    for meta_stat in demo_standard_metadata.metadata_statements:
        # print("--------------------------------------------------------------------------")
        # print(meta_stat)
        olap_cli.execute(meta_stat)
    olap_cli.close()


# create_standard_demo_metadata()


def test_01():
    a_str = 'python;'
    print(len(a_str))
    a_str += "\0"
    print(len(a_str))
    print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
    tuple_a = OlapClient("192.168.66.66", 8760)  # (1, 22, 333)
    tuple_b = OlapClient("192.168.66.66", 8760)  # (1, 22, 333)
    tuple_c = tuple_b
    print(tuple_a == tuple_b)
    print(tuple_a is tuple_b)
    print(tuple_c is tuple_b)
    print(id(tuple_a))
    print(id(tuple_b))
    print(id(tuple_c))


test_01()
