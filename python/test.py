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


create_standard_demo_metadata()


def test_01():
    a_str = 'python;'
    print(len(a_str))
    a_str += "\0"
    print(len(a_str))


# test_01()
