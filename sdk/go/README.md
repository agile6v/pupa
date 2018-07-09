## PUPA Golang SDK

>PUPA provides the following APIs, you can use them in your program. Since all operations read local shared memory and read action is lock-free operation, so don't worry about performance.

NOTICE: All write actions need to be locked in your program, which means you need to call them serially.



* Initialize the pupa cache

    * filename - A normal file which stores memory data.
    * keyCount - The maximum number of keys that can be stored. It will be ignored if opType is specified as `PUPAOpTypeR`.
    * opType - Operation Type. One of `PUPAOpTypeR` and `PUPAOpTypeRW`.


    ```golang
    func PUPAInit(filename string, keyCount int, opType int) error
    ```

* Delete key-value data according to the specified key.
    ```golang
    func PUPADel(key string) error
    ```


* Get the value of the key. If the key does not exist then error is returned.
    ```golang
    func PUPAGet(key string) (string, error)
    ```

* Set key with the value. If the key already exists, the value will be overwritten.
    ```golang
    func PUPASet(key string, value string) error
    ```

* Get the information & statistics about pupa cache. It will return data in json format.
    ```golang
    func PUPAStats() (string, error)
    ```

* End to use
    ```golang
    func PUPAFini() error
    ```

**Note:** Please install the library of pupa before using this APIs.


#### Example

```golang
package main

import (
    "fmt"
    "github.com/agile6v/pupa"
)


func main() {
    var err error
    var key, value, stats string

    err = pupa.PUPAInit("./pupa.cache", 2, pupa.PUPAOpTypeRW)
    if err != nil {
        fmt.Println("error: ", err)
        return

    }

    key = "Hello"
    value = "pupa.PUPA"

    err = pupa.PUPASet(key, value)
    if err != nil {
        fmt.Println("error: ", err)
        return

    }

    stats, err = pupa.PUPAStats()
    if err != nil {
        fmt.Println("error: ", err)
        return

    }

    fmt.Println("** stat: ", stats)

    value, err = pupa.PUPAGet(key)
    if err != nil {
        fmt.Println("error: ", err)
        return

    }

    fmt.Println("** Got " + key + " : " + value)

    err = pupa.PUPADel(key)
    if err != nil {
        fmt.Println("error: ", err)
        return

    }

    stats, err = pupa.PUPAStats()
    if err != nil {
        fmt.Println("error: ", err)
        return

    }

    fmt.Println("** stat: ", stats)

    err = pupa.PUPAFini()
    if err != nil {
        fmt.Println("error: ", err)
        return

    }
}

```