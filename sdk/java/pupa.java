/*
 * Copyright (C) agile6v
 */

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

  private static final String path = "pupa_java.store";
  private static final int KEY_COUNT = 10;
  public  static final int PUPA_RO = 1;
  public  static final int PUPA_RW = 2;

  public static void main(String[] args) {
    PUPA pupa = new PUPA();
    boolean ret = pupa.init(path, KEY_COUNT, PUPA_RW);
    if (ret != true) {
      System.out.println("Failed to init.");
    }

    String key = "Hello";
    String value = "pupa";

    try {
      ret = pupa.set(key, value);
      if (ret != true) {
        System.out.println("Failed to set.");
      }
    } catch (Exception e) {

    }

    String stats = pupa.stats();
    System.out.println("Stats: ");
    System.out.println(stats);
  }
}