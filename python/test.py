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


def query_mdx():
    olap_cli = OlapClient("192.168.66.66", 8760)
    olap_cli.connect()
    olap_cli.execute("""with
  member [Measures].SSSSSS as
    sum({ ([Measures].XXX), ([Measures].YYY), ([Measures].ZZZ) })
  member [Measures].XXX
    as ([Measures].[sales count]) * 10
  member [Measures].YYY
    as (([Measures].XXX) + 111)
  member [Measures].ZZZ
    as ([Measures].YYY) / 8 - 222.22
  member [Measures].QQQ
    as sum({([Measures].[sales count]), ([Measures].[sales])})
  member Dates.VVV
    as sum({ ([Dates].[ALL].[2022]), ([Dates].[ALL].[2023]) })
  member [Measures].IIIIII
    as IIF( ([Measures].SSSSSS) > 2000000, 200, 404 )
  member [Measures].FFFFFF
    as IIF( ([Measures].IIIIII) = 200, 321, 900000000 )
select
  { ([Measures].SSSSSS), ([Measures].IIIIII), ([Measures].FFFFFF) } on 0,
  filter(
    members(Dates),
    ((([Measures].SSSSSS) <= 30000)or (([Measures].SSSSSS) >= 40000))
      and ((([Measures].SSSSSS) <= 360000) or (([Measures].SSSSSS) >= 370000))
  ) on 1
from [Andes Online Store];""")
    olap_cli.close()


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


def main():
    # test_01()
    # create_standard_demo_metadata()
    query_mdx()


main()
