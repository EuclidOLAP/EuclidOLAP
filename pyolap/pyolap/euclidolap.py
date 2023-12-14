import socket

from pyolap import communication
from pyolap.mdmodels import Dimension

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
            dim_list.append(Dimension(dim_name))

        return dim_list
