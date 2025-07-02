# Dns-proxy
DNS proxy in C

**How to build**
```
make
```
After run program  only with ```sudo```

**How I tested**

I run the program with config file in current directory ```dns_proxy_server.conf```
```
sudo ./DNSproxy
```
and then in another terminal I run ```dig @127.0.0.1 google.com```

If config file is not found you can create it and again run the program.


**Example config file**
```
[config]
dns_server=8.8.8.8
type_responses=3
blacklist=google.com
```


Result of ```dig @127.0.0.1 google.com``` with status **NXDOMAIN**

```
; <<>> DiG 9.20.9 <<>> @127.0.0.1 google.com
; (1 server found)
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NXDOMAIN, id: 19841
;; flags: qr rd ra; QUERY: 1, ANSWER: 0, AUTHORITY: 0, ADDITIONAL: 0
;; WARNING: Message has 23 extra bytes at end

;; QUESTION SECTION:
;google.com.                    IN      A

;; Query time: 0 msec
;; SERVER: 127.0.0.1#53(127.0.0.1) (UDP)
;; WHEN: Wed Jul 02 13:13:00 EEST 2025
;; MSG SIZE  rcvd: 51
```

Program output:
```
DNS server: 8.8.8.8
Response: 3
Blacklist: google.com

[#] DNS proxy server is running

[+] Waiting for user...
Received from client: google.com
[!] Blacklisted domain reseved!
[+] Sent blocked response to user
```
