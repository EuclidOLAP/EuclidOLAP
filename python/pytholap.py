import socket
from comm import *


class OlapClient:
    serverHost = "127.0.0.1"
    serverPort = 8760
    cliSocket = None

    def __init__(self, server_host, server_port):
        self.serverHost = server_host
        self.serverPort = server_port

    def connect(self):
        self.cliSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.cliSocket.connect((self.serverHost, self.serverPort))
        protocol_size = 6
        self.cliSocket.send(protocol_size.to_bytes(4, 'little') + INTENT__TERMINAL_CONTROL__BYTES)
        resp = self.cliSocket.recv(6)
        print(f"response from server: INTENT {int.from_bytes(resp[4:], 'little')}")

    def close(self):
        self.cliSocket.close()

    def execute(self, ex_mdx: str):
        ex_mdx += '\0'
        mdx_bytes = ex_mdx.encode("UTF8", "little")
        mdx_bytes_len = len(mdx_bytes)
        protocol_size = 4 + 2 + 4 + mdx_bytes_len
        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        # print(protocol_size.to_bytes(4, 'little') + INTENT__MDX__BYTES + \
        #       mdx_bytes_len.to_bytes(4, 'little') + mdx_bytes)
        # print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        self.cliSocket.send(protocol_size.to_bytes(4, 'little') + INTENT__MDX__BYTES + \
                            mdx_bytes_len.to_bytes(4, 'little') + mdx_bytes)
        protocol_head = self.cliSocket.recv(6)
        protocol_payload = self.cliSocket.recv(int.from_bytes(protocol_head[0:4], "little") - 6)
        print(f"in execute() - response from server: INTENT {int.from_bytes(protocol_head[4:], 'little')}")
