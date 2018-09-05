/*
 * Copyright (C) agile6v
 */

package com.agile6v.pupa;

import java.io.IOException;

class PUPA {
    static {
        System.loadLibrary("pupa_java");
    }

    public native boolean init(String path, int keyCount, int OpType);
    public native boolean fini();
    public native boolean set(String key, String value) throws IOException;
    public native boolean delete(String key);
    public native String get(String key) throws IOException;
    public native String stats();

    private static final String DEFAULT_PATH = "pupa_java.store";
    private static final int KEY_COUNT = 10;
    public  static final int PUPA_RO = 1;
    public  static final int PUPA_RW = 2;

    public PUPA() throws Exception {
        boolean ret = this.init(DEFAULT_PATH, 100, PUPA_RW);
        if (ret != true) {
            throw new Exception("Failed to initialize the pupa.");
        }
    }

    public PUPA(String path, int keyCount, int OpType) throws Exception {
        boolean ret = this.init(path, keyCount, OpType);
        if (ret != true) {
            throw new Exception("Failed to initialize the pupa.");
        }
    }

    public boolean destroy() {
        return fini();
    }
}