## PUPA PHP SDK

>PUPA provides the following APIs, you can use them in your program. Since all operations read local shared memory and read action is lock-free operation, so don't worry about performance.

>NOTICE: All write actions need to be locked in your program, which means you need to call them serially. Unless you can guarantee that only one write action is called at the same time. (write actions: Delete & Set)


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
