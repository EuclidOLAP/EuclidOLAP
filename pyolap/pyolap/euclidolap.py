import socket

from pyolap import communication
from pyolap.mdmodels import Dimension
from pyolap.mdmodels import Cube

comm = communication


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
        protocol_payload = self.cliSocket.recv(int.from_bytes(protocol_head[0:4], "little") - 6)

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
        statement = f"build cube [{cube_name}]"\
                    + "\ndimensions " + " ".join([f"[{dim.name}]" for dim in dimensions])\
                    + "\nmeasures " + " ".join([f"[{mea}]" for mea in measures])\
                    + ";"
        # print(statement)
        self.execute(statement)
        return Cube(cube_name, self)

    def get_cube_by_name(self, cube_name: str):
        return Cube(cube_name, self)
