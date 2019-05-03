# pupa key-value library

[![Build Status](https://travis-ci.org/agile6v/pupa.svg?branch=master)](https://travis-ci.org/agile6v/pupa)

`pupa` is a key-value library that implements MVCC. It has the following features:

* Easy to use: API is simple and easy to understand, please refer to SDK documentation.
* Lightweight: Less than 1000 lines code.
* Persistence: Using mmap to persist the memory data to disk.
* Lock-Free: Read & Write without lock.
* Multi-Language API: You can choose the appropriate SDK to bring this library into your project.
* Data-shared: Data can be shared among processes.


### Memory Layout
![mem_layout][mem_layout]

The memory layout of key-value store consists of 4 parts: Header、Item、Key and Value.

`Header` is area which manages the information of key-value.

`Item` is the index area of key-value and contains a fixed length array of N `pupa_store_item_t` structure. Each item stores the offset and length of Key & Value.

`Key` is area which stores the data of Key ends with '\0'.

`Value` is area which stores the data of Value ends with '\0'.

#### Memory size

Total Memory Size = Header + Item + Key + Value

Header: 120byte (fixed)

Item: key_count * 20byte * 2

Key: key_count * 64byte (average_key_len) * 2

Value: key_count * 256byte (average_value_len) * 2

### SDK

* [Golang](https://github.com/agile6v/pupa/tree/master/sdk/go)

* [C/C++](https://github.com/agile6v/pupa/tree/master/src)

* [Lua](https://github.com/agile6v/pupa/tree/master/sdk/lua)

* [PHP](https://github.com/agile6v/pupa/tree/master/sdk/php)

* [JAVA](https://github.com/agile6v/pupa/tree/master/sdk/java)


### Tool

```shell
About: PUPA Debug Tool

Usage: pupa_tool [option] command

Commands:
    set     Set key to the string value.
            For example: set key value
    get     Get the value of the key.
            For example: get key
    del     Delete the specified key.
            For example: del key
    stat    Statistics and Information about the pupa store.

Options:
    -f      Specify the store file of the PUPA. If not specified,
            pupa.store file will be used in the current directory.
    -n      Specify the number of the key. If not specified,
            default is 1000.
```
Note: Since the pupa.so is installed by default in the /usr/local/lib/, so set LD_LIBRARY_PATH=/usr/local/lib/ to make sure pupa_tool can be found it.


[mem_layout]: https://github.com/agile6v/pupa/blob/master/src/mem_layout.png

### Limitation
All write actions need to be locked in your program, which means you need to call them serially. Unless you can guarantee that only one write action is called at the same time. write actions: Delete & Set.
