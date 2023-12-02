import socket
import struct

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
        protocol_intent = int.from_bytes(protocol_head[4:6], "little")
        protocol_payload = self.cliSocket.recv(int.from_bytes(protocol_head[0:4], "little") - 6)
        if protocol_intent == INTENT__MULTIDIM_RESULT_BIN:
            self.print_md_querying_result(protocol_payload)
        else:
            print(f"in execute() - response from server: INTENT {int.from_bytes(protocol_head[4:], 'little')}")

    @staticmethod
    def print_md_querying_result(protocol_payload: bytes):
        cursor = 0
        ax_count = int.from_bytes(protocol_payload[cursor:4], 'little')
        cursor += 4
        print(f"@@@@@ ax_count = {ax_count}")

        for i in range(ax_count):
            print("-------------------------------------------------")
            print(f"@@@@@ i = {i}")
            s_len = int.from_bytes(protocol_payload[cursor: cursor + 4], 'little')
            cursor += 4
            t_len = int.from_bytes(protocol_payload[cursor: cursor + 4], 'little')
            cursor += 4
            print(f"@@@@@ s_len = {s_len}")
            print(f"@@@@@ t_len = {t_len}")
            for s in range(s_len):
                for t in range(t_len):
                    member_id = int.from_bytes(protocol_payload[cursor: cursor + 8], 'little')
                    cursor += 8
                    if protocol_payload[cursor] == '\0':
                        cursor += 1
                        print("<Null Member Name>")
                    else:
                        tmp_sub_arr = protocol_payload[cursor:]
                        tmp_0_pos = tmp_sub_arr.find('\0'.encode("UTF8"))
                        cursor += tmp_0_pos + 1
                        print(tmp_sub_arr[:tmp_0_pos])

        print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
        rs_len = int.from_bytes(protocol_payload[cursor: cursor + 8], 'little')
        cursor += 8
        print(f"rs_len = {rs_len}")
        for i in range(rs_len):
            # print(type(struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]))
            # print(f"\t{struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]}")
            measure_val = struct.unpack('d', protocol_payload[cursor: cursor + 8])[0]
            cursor += 8
            print(f"measure_val[{i}] = {measure_val}")
            pass
