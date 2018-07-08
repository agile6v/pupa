# pupa key-value store

### About


### Memory Layout
![mem_layout][mem_layout]

### SDK


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
            pupa.cache file will be used in the current directory.
```


[mem_layout]: https://github.com/agile6v/pupa/blob/master/src/mem_layout.png
