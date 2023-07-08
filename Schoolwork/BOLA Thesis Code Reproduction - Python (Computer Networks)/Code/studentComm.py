import socket
import json
import studentcode_119010265 as studentcode

# Quick-and-dirty TCP Server:

# socket.AF_UNIX: Singal Unix Inter-Process Communication vs. socket.AF_INET: Inter-Net Communication IPV4
# socket.SOCK_STREAM: TCP & socket.SOCK_CLOEXEC: Close (Parent Process) on (Child Process) Excution
ss = socket.socket(socket.AF_INET, socket.SOCK_STREAM | socket.SOCK_CLOEXEC)
# Allow reuse of local address and ports on level Basic Socket
ss.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
# Bind (host=127.0.0.1, port=6000) to socket
ss.bind(('localhost', 6000))
# TCP starts listening.
# The maximum number of connections that the operating system can suspend is 10.
ss.listen(10)
print('Waiting for simulator')

# Passively accept TCP client connections, (blocking) waiting for the arrival of the connection
(clientsocket, address) = ss.accept()


def recv_commands():
    message = ""

    while (1):
        messagepart = clientsocket.recv(2048).decode()

        message += messagepart
        if message[-1] == '\n':

            jsonargs = json.loads(messagepart)
            message = ""

            if (jsonargs["exit"] != 0):
                return

            # todo: json data sanitization
            bitrate = studentcode.student_entrypoint(jsonargs["Measured Bandwidth"], jsonargs["Previous Throughput"], jsonargs["Buffer Occupancy"],
                                                     jsonargs["Available Bitrates"], jsonargs["Video Time"], jsonargs["Chunk"], jsonargs["Rebuffering Time"], jsonargs["Preferred Bitrate"])

            payload = json.dumps({"bitrate": bitrate}) + '\n'
            # Send all TCP data
            clientsocket.sendall(payload.encode())


if __name__ == "__main__":

    recv_commands()
    ss.close()
