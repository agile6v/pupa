# pupa key-value library [![Build Status](https://travis-ci.org/agile6v/pupa.svg?branch=master)](https://travis-ci.org/agile6v/pupa)

### About

`pupa` is lightweight、efficient、persistence and lock-free key-value library.

### Memory Layout
![mem_layout][mem_layout]

The memory layout of key-value store consists of 4 parts: Header、Item、Key and Value.

`Header` is the management information area of key-value.

`Item` is the index area of key-value and contains a fixed length array of N `pupa_store_item_t` structure. Each item stores the offset and length of Key & Value.

`Key` is area which stores the data of Key ends with '\0'.

`Value` is area which stores the data of Value ends with '\0'.

##### Memory size

Header: 120byte (fixed)

Item: key_count * 20byte * 2

Key: key_count * 64byte (average_key_len) * 2

Value: key_count * 256byte (average_value_len) * 2

### SDK

* [Golang](https://github.com/agile6v/pupa/tree/master/sdk/go)

* [C/C++]()

* [Lua](https://github.com/agile6v/pupa/tree/master/sdk/lua)


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
    stat    Statistics and Information about the pupa cache.

Options:
    -f      Specify the cache file of the PUPA. If not specified,
            pupa.store file will be used in the current directory.
    -n      Specify the number of the key. If not specified,
            default is 1000.
```


[mem_layout]: https://github.com/agile6v/pupa/blob/master/src/mem_layout.png
