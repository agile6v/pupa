package pupa


// #cgo CFLAGS: -I/usr/local/include/pupa
// #cgo LDFLAGS: -lpupa
// #include "pupa.h"
// #include "pupa_config.h"
import "C"

import (
    "errors"
)


func PUPAInit(filename string, opType int) error {
    ret := C.pupa_init(C.CString(filename), 2, C.int(opType))
    if ret != 0 {
        return errors.New("Failed to initialize pupa.")
    }

    return nil
}


func PUPADel(key string) error {
    key_str := &C.struct_pupa_str_s{}
    key_str.len = C.int(len(key))
    key_str.data = C.CString(key)

    ret := C.pupa_del(key_str)
    if ret != 0 {
        return errors.New("Failed to delete " + key)
    }

    return nil
}


func PUPAGet(key string) (string, error) {
    key_str := &C.struct_pupa_str_s{}
    key_str.len = C.int(len(key))
    key_str.data = C.CString(key)

    value_str := &C.struct_pupa_str_s{}

    ret := C.pupa_get(key_str, value_str)
    if ret != 0 {
        return "", errors.New("Failed to get " + key)
    }

    return C.GoString(value_str.data), nil
}


func PUPASet(key string, value string) error {
    key_str := &C.struct_pupa_str_s{}
    key_str.len = C.int(len(key))
    key_str.data = C.CString(key)

    value_str := &C.struct_pupa_str_s{}
    value_str.len = C.int(len(value))
    value_str.data = C.CString(value)

    ret := C.pupa_set(key_str, value_str)
    if ret != 0 {
        return errors.New("Failed to set " + key + ":" + value)
    }

    return nil
}


func PUPAStats() (string, error) {
    stat_str := &C.struct_pupa_str_s{}

    ret := C.pupa_stats(stat_str)
    if ret != 0 {
        return "", errors.New("Failed to execute pupa stat.")
    }

    return C.GoString(stat_str.data), nil
}


func PUPAFini() error {
    ret := C.pupa_fini()
    if ret != 0 {
        return errors.New("Failed to fini ")
    }

    return nil
}
