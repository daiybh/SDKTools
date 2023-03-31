import socket
import json
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 1983  # Port to listen on (non-privileged ports are > 1023)

msg = {
	"name": "connect",
	"input": {
	"park_id": "P0003",
    "channel_id": "2",
	"timestamp": 1677893097
	}
}
def sendClient(ip):
    data="%CLOD0041RISEPOLE00\r\nBS20220001,192.168.0.243,20220407163254\r\n25@"
    data=f"%CLOD0038RISEPOLE00\r\nBS20220001,{ip},20220407163254\r\n25@"
    v=   "%CLOD0047CARDNUM 00\r\nBS20220001,platenum,192.0.0.51,20230330233903\r\n5D@"
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, 2345))
        print(len(data))
        bb = data.encode()
        s.sendall(bb)
        print("wait.....")
        data = s.recv(1024)
        print("recv done")

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    while True:
        try:
            conn, addr = s.accept()
            with conn:
                print(f"Connected by {addr}")
                
                while True:
                    data = conn.recv(1024)
                    if not data:
                        print("no data faile.d");
                        break
                    print(f"recv\r\n  {data}")
                    s5 = data.decode("utf-8")
                    #print(s)
                    ip = s5.split("\r\n")[1].split(',')[2]
                    print("ip",ip)
                    conn.sendall(data)
                    sendClient(ip)
                    print("send over")
        except Exception as ex:
            print(ex)
        print("connect over")

print("oooooooooooooooooooooooo")