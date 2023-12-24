import socket
import struct

from pyolap import communication
from pyolap.mdmodels import Dimension
from pyolap.mdmodels import Cube

import pyolap.utils

comm = communication


class OLAPQueryBuilder:
    cubeName: str = None
    rowsAxisStr: str = None
    colsAxisStr: str = None

    def from_cube(self, cube_name: str):
        self.cubeName = cube_name
        return self

    def set_rows(self, axis_str: str):
        self.rowsAxisStr = axis_str
        return self

    def set_columns(self, axis_str: str):
        self.colsAxisStr = axis_str
        return self

    def gen_mdx(self):
        mdx = f"select\n{self.rowsAxisStr} on 1,\n{self.colsAxisStr} on 0\nfrom [{self.cubeName}];"
        # print(f">>>>>>>>>>>>>>>>>>>>>>>\n{mdx}\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        return mdx


class MultidimensionalQueryingResult:
    protocol_head: bytes  # 6 bytes
    protocol_payload: bytes

    def __init__(self, protocol_head, protocol_payload):
        self.protocol_head = protocol_head
        self.protocol_payload = protocol_payload

    def __str__(self):
        if int.from_bytes(self.protocol_payload[0:4], 'little') == 2:
            self.__print_2D_table()
        else:
            self.__print_xD_table()

        return ""

    def __print_2D_table(self):
        # print(f"len(self.protocol_payload) = {len(self.protocol_payload)}")
        payload = self.protocol_payload
        cursor = 4

        row_head_width = int.from_bytes(payload[cursor: cursor + 4], 'little')
        cursor += 4
        row_head_thick = int.from_bytes(payload[cursor: cursor + 4], 'little')
        cursor += 4

        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  " + f"row_head_width = {row_head_width}")
        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  " + f"row_head_thick = {row_head_thick}")

        row_head_table = [[None for _ in range(row_head_thick)] for _ in range(row_head_width)]
        for s in range(row_head_width):
            for t in range(row_head_thick):
                member_id = int.from_bytes(payload[cursor: cursor + 8], 'little')
                cursor += 8
                if payload[cursor] == '\0':
                    cursor += 1
                    row_head_table[s][t] = ""
                    # print("<Null Member Name>")
                else:
                    tmp_sub_arr = payload[cursor:]
                    tmp_0_pos = tmp_sub_arr.find('\0'.encode("UTF8"))
                    cursor += tmp_0_pos + 1
                    row_head_table[s][t] = bytes(tmp_sub_arr[:tmp_0_pos]).decode("UTF8")
                    # print(bytes(tmp_sub_arr[:tmp_0_pos]).decode("UTF8"))

        # print(row_head_table)

        col_head_width = int.from_bytes(payload[cursor: cursor + 4], 'little')
        cursor += 4
        col_head_thick = int.from_bytes(payload[cursor: cursor + 4], 'little')
        cursor += 4

        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  " + f"col_head_width = {col_head_width}")
        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  " + f"col_head_thick = {col_head_thick}")

        col_head_table = [[None for _ in range(col_head_width)] for _ in range(col_head_thick)]
        for s in range(col_head_width):
            for t in range(col_head_thick):
                member_id = int.from_bytes(payload[cursor: cursor + 8], 'little')
                cursor += 8
                if payload[cursor] == '\0':
                    cursor += 1
                    col_head_table[t][s] = ""
                    # print("<Null Member Name>")
                else:
                    tmp_sub_arr = payload[cursor:]
                    tmp_0_pos = tmp_sub_arr.find('\0'.encode("UTF8"))
                    cursor += tmp_0_pos + 1
                    col_head_table[t][s] = bytes(tmp_sub_arr[:tmp_0_pos]).decode("UTF8")
                    # print(bytes(tmp_sub_arr[:tmp_0_pos]).decode("UTF8"))

        # print(col_head_table)

        whole_table_width = row_head_thick + col_head_width
        whole_table_height = col_head_thick + row_head_width
        whole_table = [["" for _ in range(whole_table_width)] for _ in range(whole_table_height)]

        # print(whole_table)

        for s in range(row_head_width):
            for t in range(row_head_thick):
                whole_table[s + col_head_thick][t] = row_head_table[s][t]

        for t in range(col_head_thick):
            for s in range(col_head_width):
                whole_table[t][s + row_head_thick] = col_head_table[t][s]

        # print(whole_table)

        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        rs_len = int.from_bytes(payload[cursor: cursor + 8], 'little')
        cursor += 8
        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  " + f"rs_len = {rs_len}")

        measures = []

        for i in range(rs_len):
            # print(type(struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]))
            # print(f"\t{struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]}")
            measure_val = struct.unpack('d', payload[cursor: cursor + 8])[0]
            cursor += 8
            # print(f"measure_val[{i}] = {measure_val}")
            measures.append(measure_val)

        measure_index = 0

        for r in range(row_head_width):
            for c in range(col_head_width):
                whole_table[r + col_head_thick][c + row_head_thick] = measures[measure_index]
                measure_index += 1

        # print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
        # print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
        # print(whole_table)
        # print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
        # print("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")

        col_console_width_ls = [0] * whole_table_width

        for c in range(whole_table_width):
            for r in range(whole_table_height):
                td_content = whole_table[r][c]
                if isinstance(td_content, float):
                    td_content = str(td_content)
                console_str_width = pyolap.utils.get_console_str_width(td_content)
                if console_str_width > col_console_width_ls[c]:
                    col_console_width_ls[c] = console_str_width

        # print(col_console_width_ls)
        # col_console_width_ls = [ele + 2 for ele in col_console_width_ls]
        # print(col_console_width_ls)

        row_split_line = '+' + '+'.join(['-' * (ele + 2) for ele in col_console_width_ls]) + '+'
        for r in range(whole_table_height):
            print(row_split_line)
            print("|", end="")
            for c in range(whole_table_width):
                td_content = whole_table[r][c]
                if isinstance(td_content, float):
                    td_content = str(td_content)
                console_str_width = pyolap.utils.get_console_str_width(td_content)
                print(' ' + ' ' * (col_console_width_ls[c] - console_str_width) + td_content, end=" |")
            print()
        print(row_split_line)

    def __print_xD_table(self):
        payload = self.protocol_payload

        cursor = 0
        ax_count = int.from_bytes(payload[cursor:4], 'little')
        cursor += 4
        print(f"@@@@@ ax_count = {ax_count}")

        for i in range(ax_count):
            print("-------------------------------------------------")
            print(f"@@@@@ i = {i}")
            s_len = int.from_bytes(payload[cursor: cursor + 4], 'little')
            cursor += 4
            t_len = int.from_bytes(payload[cursor: cursor + 4], 'little')
            cursor += 4
            print(f"@@@@@ s_len = {s_len}")
            print(f"@@@@@ t_len = {t_len}")
            for s in range(s_len):
                for t in range(t_len):
                    member_id = int.from_bytes(payload[cursor: cursor + 8], 'little')
                    cursor += 8
                    if payload[cursor] == '\0':
                        cursor += 1
                        print("<Null Member Name>")
                    else:
                        tmp_sub_arr = payload[cursor:]
                        tmp_0_pos = tmp_sub_arr.find('\0'.encode("UTF8"))
                        cursor += tmp_0_pos + 1
                        print(bytes(tmp_sub_arr[:tmp_0_pos]).decode("UTF8"))

        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        rs_len = int.from_bytes(payload[cursor: cursor + 8], 'little')
        cursor += 8
        print(f"rs_len = {rs_len}")
        for i in range(rs_len):
            # print(type(struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]))
            # print(f"\t{struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]}")
            measure_val = struct.unpack('d', payload[cursor: cursor + 8])[0]
            cursor += 8
            print(f"measure_val[{i}] = {measure_val}")


class OlapContext:
    serverHost: str
    serverPort: int
    cliSocket: socket

    def __init__(self, server_host="127.0.0.1", server_port=8760):
        self.serverHost = server_host
        self.serverPort = server_port

        self.cliSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.cliSocket.connect((self.serverHost, self.serverPort))

        protocol_size = 6
        self.cliSocket.send(protocol_size.to_bytes(4, 'little') + comm.INTENT__TERMINAL_CONTROL__BYTES)
        resp_package = self.cliSocket.recv(6)
        # print(f"response from server: INTENT {int.from_bytes(resp[4:], 'little')}")

    def close(self):
        self.cliSocket.close()

    def execute(self, mdx: str):
        mdx += '\0'
        mdx_bytes = mdx.encode("UTF8", "little")
        mdx_bytes_len = len(mdx_bytes)
        protocol_size = 4 + 2 + 4 + mdx_bytes_len

        self.cliSocket.send(protocol_size.to_bytes(4, 'little')
                            + comm.INTENT__MDX__BYTES
                            + mdx_bytes_len.to_bytes(4, 'little')
                            + mdx_bytes)

        protocol_head = self.cliSocket.recv(6)
        protocol_intent = int.from_bytes(protocol_head[4:6], "little")

        payload_size = int.from_bytes(protocol_head[0:4], "little") - 6
        protocol_payload = self.cliSocket.recv(payload_size)
        while (payload_size - len(protocol_payload)) > 0:
            payload_fragment = self.cliSocket.recv(payload_size - len(protocol_payload))
            if len(payload_fragment) < 1:
                raise Exception("payload_fragment size is ZERO.")
            protocol_payload += payload_fragment

        # print(f">>> protocol_intent = {protocol_intent}")
        if protocol_intent == comm.INTENT__MULTIDIM_RESULT_BIN:
            return MultidimensionalQueryingResult(protocol_head, protocol_payload)

    # @staticmethod
    def create_dimensions(self, *dims):

        creating_dimensions_statement = "create dimensions " + " ".join([f"[{d}]" for d in dims]) + ";"
        self.execute(creating_dimensions_statement)

        dim_list = []
        for dim_name in dims:
            dim_list.append(Dimension(dim_name, self))

        return dim_list

    def get_dimension_by_name(self, dim_name: str) -> Dimension:
        return Dimension(dim_name, self)

    def create_members(self, dim: Dimension, members_info: list):

        new_ms_info = []
        for m_info in members_info:
            if isinstance(m_info, str):
                new_ms_info.append(f"[{m_info}]")
            else:
                ms_info_ls: list = m_info
                new_ms_info.append(".".join([f"[{ele}]" for ele in ms_info_ls]))

        statement = "create members\n" + ",\n".join([f"[{dim.name}].{ele}" for ele in new_ms_info]) + ";"
        # print(">>>>>>>>>>>>>>>>>>>>>>>>!!!")
        # print(statement)
        # print(">>>>>>>>>>>>>>>>>>>>>>>>???")
        self.execute(statement)

    def build_cube(self, cube_name: str, dimensions: list[Dimension], measures: list):
        statement = f"build cube [{cube_name}]" \
                    + "\ndimensions " + " ".join([f"[{dim.name}]" for dim in dimensions]) \
                    + "\nmeasures " + " ".join([f"[{mea}]" for mea in measures]) \
                    + ";"
        # print(statement)
        self.execute(statement)
        return Cube(cube_name, self)

    def get_cube_by_name(self, cube_name: str):
        return Cube(cube_name, self)

    def query(self, olap_qb: OLAPQueryBuilder):
        mdx = olap_qb.gen_mdx()
        return self.execute(mdx)
