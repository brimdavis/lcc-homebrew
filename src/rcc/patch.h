//
// enable|disable various debug patches with #define|#undef
//


////////////////////////////////////////////////
//
// PATCH_2OP : two operand overwrite
//
////////////////////////////////////////////////

//
// work around two-operand overwrite bug with new addload() routine called by rtarget
// using two-operand patch from JW's comp.compilers.lcc post of April 4 2013
//
#define PATCH_2OP_RTARGET_ADDLOAD

//
// original sledgehammer workaround for two operand overwrite bug, force rtarget to always reload
//
#undef PATCH_2OP_RTARGET_SLEDGEHAMMER


//
// work around two operand overwrite by swapping registers in emit()
//
#undef PATCH_2OP_EMIT_SWAPREG


//
// die with assert on two-operand overwrite
// emit() will still produce an assembly time error even if this is turned off
//
#undef PATCH_2OP_EMIT_ASSERT
