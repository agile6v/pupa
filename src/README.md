## PUPA C/C++ SDK

>PUPA provides the following APIs, you can use them in your program. Since all operations read local shared memory and read action is lock-free operation, so don't worry about performance.

>NOTICE: All write actions need to be locked in your program, which means you need to call them serially. Unless you can guarantee that only one write action is called at the same time. (write actions: Delete & Set)

* Initialize the pupa store.

    * path - A normal file which stores memory data.
    * key_count - The maximum number of keys that can be stored. It will be ignored if `op_type` is specified as `PUPA_OP_TYPE_RO`.
    Note: key_count will be ignored if the filename already exists.
    * op_type - Operation type. One of `PUPA_OP_TYPE_RO` and `PUPA_OP_TYPE_RW`. `PUPA_OP_TYPE_RO` for read only, `PUPA_OP_TYPE_RW` for read write.
    * return - PUPA_OK if this function

    ```c
    int pupa_init(char *path, int key_count, int op_type);
    ```

* Delete key-value data according to the specified key.
    ```c
    int pupa_del(pupa_str_t *key);
    ```


* Get the value of the key. If the key does not exist then error is returned.
    ```c
    int pupa_get(pupa_str_t *key, pupa_str_t *value);
    ```

* Set key with the value. If the key already exists, the value will be overwritten.
    ```c
    int pupa_set(pupa_str_t *key, pupa_str_t *value);
    ```

* Get the information & statistics about pupa store. It will return data in json format.
    ```c
    int pupa_stats(pupa_str_t *stat_json);
    ```

* End to use
    ```c
    int pupa_fini();
    ```

#### Example

```c
#include <iostream>
#include <stdio.h>
#include "pupa.h"

const std::string cache_file = "./pupa.store";

int main()
{
    int         ret;
    pupa_str_t  key, value;

    ret = pupa_init((char *) cache_file.c_str(), 10, PUPA_OP_TYPE_RW);
    if (ret != PUPA_OK) {
        printf("Failed to initialize pupa.\n");
        return ret;
    }

    pupa_str_set(&key, "PUPA");
    pupa_str_set(&value, "Hello");

    ret = pupa_set(&key, &value);
    if (ret != PUPA_OK) {
        printf("Failed to set %.*s.\n", key.len, key.data);
        return ret;
    }

    ret = pupa_get(&key, &value);
    if (ret != PUPA_OK) {
        printf("Failed to get %.*s.\n", key.len, key.data);
        return ret;
    }

    printf("\nGot %.*s : %.*s\n", key.len, key.data, value.len, value.data);

    pupa_fini();

    return 0;
}
```
