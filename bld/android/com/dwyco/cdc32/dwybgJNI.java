/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.dwyco.cdc32;

public class dwybgJNI {
  public final static native int dwyco_test_funny_mutex(int jarg1);
  public final static native int dwyco_background_processing(int jarg1, int jarg2, String jarg3, String jarg4, String jarg5, String jarg6);
  public final static native int dwyco_background_sync(int jarg1, String jarg2, String jarg3, String jarg4, String jarg5, String jarg6, String jarg7);
  public final static native void dwyco_set_aux_string(String jarg1);
  public final static native void dwyco_clear_contact_list();
  public final static native int dwyco_add_contact(String jarg1, String jarg2, String jarg3);
  public final static native void dwyco_signal_msg_cond();
  public final static native void dwyco_wait_msg_cond(int jarg1);
  public final static native void dwyco_write_token(String jarg1);
}
