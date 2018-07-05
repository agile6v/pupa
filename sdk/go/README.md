## PUPA Golang SDK

>PUPA provides the following APIs, you can use them in your program. Since all operations read local shared memory, so don't worry about performance issues.


* PUPAInit(filename string, keyCount int, opType int) error

* PUPADel(key string) error

* PUPAGet(key string) (string, error)

* PUPASet(key string, value string) error

* PUPAStats() (string, error)

* PUPAFini() error