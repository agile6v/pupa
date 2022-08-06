# pupa key-value library

[![Build Status](https://travis-ci.org/agile6v/pupa.svg?branch=master)](https://travis-ci.org/agile6v/pupa)

`pupa` is a small, efficient key-value library. It has the following features:

* Easy to use: API is simple and easy to understand, please refer to SDK documentation.
* Lightweight: Less than 2000 lines code and no external dependencies.
* Persistence: Using mmap to persist the memory data to disk.
* Efficient: Multi-Reader & Single-Writer without lock and zero-copies for read and write.
* Multi-Language API: You can choose the appropriate SDK to bring this library into your project.
* Data-shared: Data can be shared among processes.


### PUPA Structure
![mem_layout][mem_layout]

The memory layout of key-value store consists of 4 parts: Header、Item、Key and Value.

`Header`: an area that manages the meta information of the area Item、Key and Value. Meta information likes which area is being used (There are two equal-sized area for Item、Key and Value areas.) or how much size is currently used, etc.

`Item(index)` : index area, which uses a dense index to manage data, uses two equal-sized areas (one for reading, another for writing) to avoid locks for writing and reading operations. Whenever a writing operation (set, delete) is performed, copy the area for reading to snapshot area. After the writing operation is successful, the snapshot area becomes the reading area. Each item contains the offset based on Key and Value area. The key and value data are written in the Key and Value areas respectively. After each writing operation, all items  are sorted in alphabetical order based on the key. (Note: only the Item area is sorted and the item does not store key or value data) The storage space occupied is also very efficient, the size of each item is equal to sizeof(pupa_store_item_t) + max_ver_num * sizeof(pupa_store_item_val_t)).

`key`: an area where key is stored. Each writing operation is appended to this area. When the current (for reading & writing) area is full, the compaction operation will be performed. The compaction will copy the data of the current area to the snapshot area. After the compaction is completed, the snapshot area will become the current area.  Although there are two areas to be used, but the capacity has only one. The data of key ends with '\0'.

`Value`: an area where data is stored. Its processing logic is the same as the Key area. A key may contain multiple versions of the value, the version number of the last value written is 1. The data of Value ends with '\0'.


#### Memory size

`Total Memory Size` = Header + Item + Key + Value

`Header`: 128byte (fixed)

`Item`: key_count * (16byte + 16byte * max_ver_num) * 2

`Key`: key_count * 64byte (average_key_len) * 2

`Value`: key_count * 256byte (average_value_len) * 2

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
                         get key -v 0
                         get key -v 3
    del     Delete the specified key.
            For example: del key
    stat    Statistics and Information about the pupa store.

Options:
    -f      Specify the store file of the PUPA. If not specified,
            pupa.store file will be used in the current directory.
    -v      Specify the key-value version. If not specified,
            the latest one returned. 0 for all versions.
    -n      Specify the number of the key. If not specified,
            default is 1000.

```
Note: Since the pupa.so is installed by default in the /usr/local/lib/, so set LD_LIBRARY_PATH=/usr/local/lib/ to make sure pupa_tool can find it.


[mem_layout]: https://github.com/agile6v/pupa/blob/master/src/mem_layout.png

### Limitation
All writing operations need to be locked in your program, which means you need to call them serially. Unless you can guarantee that only one write action is called at the same time. writing operations: Delete & Set.
