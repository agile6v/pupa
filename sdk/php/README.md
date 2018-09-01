## PUPA PHP SDK

>PUPA provides the following APIs, you can use them in your program. Since all operations read local shared memory and read action is lock-free operation, so don't worry about performance.

>NOTICE: All write actions need to be locked in your program, which means you need to call them serially. Unless you can guarantee that only one write action is called at the same time. (write actions: Delete & Set)

* Initialize the pupa store.

    * path - A normal file which stores memory data.
    * key_count - The maximum number of keys that can be stored. It will be ignored if `op_type` is specified as `PUPA_RO`.
    Note: key_count will be ignored if the filename already exists.
    * op_type - Operation type. One of `PUPA_RO` and `PUPA_RW`. `PUPA_RO` for read only, `PUPA_RW` for read write.
    * return - true if success, otherwise false

    ```php
    bool pupa_init(string $path, int key_count, int op_type);
    ```

* Delete key-value data according to the specified key.
    ```php
    bool pupa_del(string $key);
    ```

* Get the value of the key. If the key does not exist then error is returned.
    ```php
    string pupa_get(string $key);
    ```

* Set key with the value. If the key already exists, the value will be overwritten.
    ```php
    bool pupa_set(string $key, string $value);
    ```

* Get the information & statistics about pupa store. It will return data in json format.
    ```php
    string pupa_stats(void);
    ```

* End to use
    ```php
    bool pupa_fini(void);
    ```



#### Installation

```shell
phpize
./configure --with-pupa
make
make install
```

**Note:** Please install the library of pupa before installing this php module.


#### Example

```php
<?php

$ret = pupa_init("pupa.store", 100, PUPA_RW);
if ($ret != true) {
    echo "Failed to initialize pupa.\n";
    return;
}

$key = "pupa";
$value = "world";

$ret = pupa_set($key, $value);
if ($ret != true) {
    echo "Failed to set.\n";
    return;
}

$ret = pupa_get($key);

echo "** Got $key : $ret\n";

$ret = pupa_del($key);
if ($ret != true) {
    echo "Failed to delete.\n";
    return;
}

$ret = pupa_stats();

echo "** Stats:\n $ret\n";

$ret = pupa_fini();
if ($ret != true) {
    echo "Failed to fini.\n";
    return;
}
```
