# pupa key-value store

### About


### Memory Layout
![mem_layout][mem_layout]

The memory layout of key-value store consists of 4 parts: Header、Item、Key and Value.

`Header` is the management information area of key-value.

`Item` is the index area of key-value and contains a fixed length array of N item structure.

`Key` is

`Value`

### SDK

* [Golang](https://github.com/agile6v/pupa/tree/master/sdk/go)

* [C/C++]()

* [Lua]()

* [PHP]()

* [Python]()

* [Java]()

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
```


[mem_layout]: https://github.com/agile6v/pupa/blob/master/src/mem_layout.png
