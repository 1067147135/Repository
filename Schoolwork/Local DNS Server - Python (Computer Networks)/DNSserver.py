import socket
import dnslib

# Maintain a cache here.
# cache[qname] = responseMessage
cache = {}


def DNS_helper(qname):
    print("Query IP address of:", qname)
    isCNAME = False
    # List of records about the domain name and IPv4 address of Root DNS servers.
    # Data source: https://www.iana.org/domains/root/servers
    DNS_servers = [
        '198.41.0.4',
        '199.9.14.201',
        '192.33.4.12',
        '199.7.91.13',
        '192.203.230.10',
        '192.5.5.241',
        '192.112.36.4',
        '198.97.190.53',
        '192.36.148.17',
        '192.58.128.30',
        '193.0.14.129',
        '199.7.83.42',
        '202.12.27.33'
    ]
    # qnamelist = ['com', 'baidu', 'www', '']
    qnamelist = qname.split(".")
    # qnamelist = ['www', 'baidu', 'com']
    qnamelist.pop(len(qnamelist) - 1)
    # qnamelist = ['www.', 'baidu.', 'com.']
    for i in range(0, len(qnamelist)-1):
        qnamelist[i] = qnamelist[i] + "."
    # qnamelist = ['com.', 'baidu.', 'www.']
    qnamelist.reverse()
    # Select a root server from 13 root_DNS_servers to begin the iteration.
    # Do iterations, take www.baidu.com as an example:
    # 1. domain = 'com.' 2. domain = 'baidu.com.' 3.domain = 'www.baidu.com.'
    domain = ""
    for i in qnamelist:
        domain = i + domain
        r = dnslib.DNSRecord.question(domain, qtype="A")
        # Considering that some servers may be temporarily slow to respond,
        # it will query another server after this query's timeout
        for j in DNS_servers:
            # Send packet r to name server j and get response rr
            try:
                rr = r.send(j, timeout=5)
                # Parse response DNS packet and get DNSRecord instance.
                res = dnslib.DNSRecord.parse(rr)
                if (res.header.rcode == 0):
                    print("Pass by server:", j)
                    break
            except:
                continue
        # print(res)
        # Update found DNS servers in rr
        if len(res.ar) > 0:
            DNS_servers.clear()
            for k in res.ar:
                DNS_servers.append(k.rdata.__str__())
        elif len(res.auth) > 0:
            DNS_servers.clear()
            res0, _ = DNS_helper(res.auth[0].rdata.__str__())
            for l in res0.rr:
                DNS_servers.append(l.rdata.__str__())
        elif res.a.rtype == 5:
            isCNAME = True
    print("Get candidate IP address of", qname)
    return res, isCNAME


def local_DNS_Server():
    flag = input("Select mode (0 or 1): ")
    print("server ready...")

    serverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    serverSocket.bind(('127.0.0.1', 1234))

    if (flag == '0'):
        public_servers = ['77.68.88.76', '185.43.51.84', '45.225.123.24',
                          '202.164.44.246', '46.246.29.69', '223.31.121.171',
                          '103.113.200.10', '193.159.232.5', '77.68.45.252',
                          '188.165.187.102', '177.20.178.12', '46.228.199.116',
                          '217.79.177.220', '213.184.225.37', '45.225.123.88'
                          ]

        while True:
            message, clientAddress = serverSocket.recvfrom(2048)
            message_parse = dnslib.DNSRecord.parse(message)
            print("Query IP address of:", message_parse.q.qname)

            # Judge whether the query is already in the cache.
            if message_parse.q.qname in cache:
                responseMessage = cache[message_parse.q.qname]
                # Set the id of response packet same with query packet. (dig)
                responseMessage.header.id = message_parse.header.id
                serverSocket.sendto(responseMessage.pack(), clientAddress)
            else:
                r = dnslib.DNSRecord.question(message_parse.q.qname, qtype="A")
                for j in public_servers:
                    try:
                        rr = r.send(j, timeout=5)
                        res = dnslib.DNSRecord.parse(rr)
                        if (res.header.rcode == 0):
                            print("Query public server:", j)
                            break
                    except:
                        continue
                # Set the id of response packet same with query packet. (dig)
                res.header.id = message_parse.header.id
                res.header.ra = 1  # recursive available
                res.q.qtype = 1  # Type"A"
                # Clear the AUTHORITY SECTION and ADDITIONAL SECTION to make the response packet simpler.
                res.auth = []
                res.ar = []
                # Send the response packet back to the terminal.
                serverSocket.sendto(res.pack(), clientAddress)
                # Add the response packet into the cache.
                cache[message_parse.q.qname] = res

    while True:  # Keep the local dns server working.
        message, clientAddress = serverSocket.recvfrom(2048)
        # Parse DNS packet data and return DNSRecord instance.
        message_parse = dnslib.DNSRecord.parse(message)

        # Judge whether the query is already in the cache.
        if message_parse.q.qname in cache:
            responseMessage = cache[message_parse.q.qname]
            # Set the id of response packet same with query packet. (dig)
            responseMessage.header.id = message_parse.header.id
            serverSocket.sendto(responseMessage.pack(), clientAddress)
        else:
            # qname = 'www.baidu.com.'
            qname = str(message_parse.q.qname)
            res, isCNAME = DNS_helper(qname)
            while isCNAME:
                domain = str(res.rr[0].rdata)
                res1, isCNAME = DNS_helper(domain)
                # Get Answer list, for example and then add these answers to response packet:
                # ;; ANSWER SECTION:
                # www.a.shifen.com.       300     IN      A       163.177.151.109
                # www.a.shifen.com.       300     IN      A       163.177.151.110
                rrlist = res1.rr
                for i in rrlist:
                    res.rr.append(i)
            # Modify some parameters of response packets
            # Set the id of response packet same with query packet. (dig)
            res.header.id = message_parse.header.id
            res.header.ra = 1  # recursive available
            res.q.qtype = 1  # Type "A"
            # Clear the AUTHORITY SECTION and ADDITIONAL SECTION to make the response packet simpler.
            res.auth = []
            res.ar = []
            # Send the response packet back to the terminal.
            serverSocket.sendto(res.pack(), clientAddress)
            # Add the response packet into the cache.
            cache[message_parse.q.qname] = res


if __name__ == '__main__':
    try:
        local_DNS_Server()
    except KeyboardInterrupt:
        pass
