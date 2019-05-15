#User Guide

##Introduction

This project is aim to demonstrate wasm app management and programming model of WAMR.

##Build all binaries
Execute the script build.sh then alll binaries including wasm application files are generated in 'out' directory.
>./build.sh

###Out directory structure
out/
├── host_tool
├── simple
└── wasm-apps
    ├── event_publisher.wasm
    ├── event_subscriber.wasm
    ├── request_handler.wasm
    ├── request_sender.wasm
    ├── sensor.wasm
    └── timer.wasm
 
host_tool:
  A samll testing tool to send request to WAMR. See usage of this tool by executing "./host_tool -h".
  >./host_tool -h

simple:
  The simple application with WAMR runtime built in. See usage of this application by executing "./simple -h".
  >./simple -h

wasm-apps:
  5 sample wasm applications which demonstrate all APIs of WAMR programming model.

###Note:
You can uncomment below line in CMakeLists.txt to enalbe UART mode. In this guide we use TCP mode instead.
>\#add_definitions (-DCONNECTION_UART)

##Run
- Enter the out directory
  >cd ./out/

- Startup the 'simple' process works in TCP server mode
  >./simple -s

  You would see "App Manager started." is printed.
  >App Manager started.

- Query all installed applications
  >./host_tool -q

- Install a wasm application
  >./host_tool -i TestApp -f ./wasm-apps/request_handler.wasm

- Send request to the installed wasm app
  >./host_tool -r /url1 -A GET

- Uninstall the wasm app
  >./host_tool -u TestApp

###Output example

####Output of simple

App Manager started.
connection established!
sent 137 bytes to host
Query Applets success!
Attribute container dump:
Tag: Applets Info
Attribute list:
  key: num, type: int, value: 0x0

connection lost, and waiting for client to reconnect...
connection established!
Install WASM app success!
sent 16 bytes to host
WASM app 'TestApp' started
connection lost, and waiting for client to reconnect...
connection established!
Send request to app TestApp success.
App TestApp got request, url /url1, action 1
[resp] ### user resource 1 handler called
[resp] ###### dump request ######
[resp] sender: -3
[resp] url: /url1
[resp] action: 1
[resp] payload:
Attribute container dump:
Tag:  �
Attribute list:

[resp] #### dump request end ###
[resp] response payload len 134
[resp] reciver: -3, mid:1527931117
sent 150 bytes to host
Wasm app process request success.
connection lost, and waiting for client to reconnect...
connection established!
sent 137 bytes to host
Query Applets success!
Attribute container dump:
Tag: Applets Info
Attribute list:
  key: num, type: int, value: 0x1
  key: applet1, type: string, value: TestApp
  key: heap1, type: int, value: 0xc000

connection lost, and waiting for client to reconnect...
connection established!
App instance main thread exit.
Uninstall WASM app successful!
sent 16 bytes to host
connection lost, and waiting for client to reconnect...


####Output of host_tool

$ ./host_tool -q

response status 69
{
        "num":  0
}

$ ./host_tool -i TestApp -f ./wasm-apps/request_handler.wasm

response status 65

$ ./host_tool -r /url1 -A GET

response status 69
{
        "key1": "value1",
        "key2": "value2"
}

$ ./host_tool -q

response status 69
{
        "num":  1,
        "applet1":      "TestApp",
        "heap1":        49152
}

$ ./host_tool -u TestApp

response status 66
