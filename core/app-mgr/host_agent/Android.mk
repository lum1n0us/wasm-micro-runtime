LOCAL_PATH := $(call my-dir)
#################host_agent#################
include $(CLEAR_VARS)
LOCAL_MODULE := host_agent
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ../aee/Beihai/runtime/utils/coap/er-coap/er-coap.c                                              \
../aee/Beihai/runtime/utils/coap/extension/coap_over_tcp.c                                      \
lib/attr-container.c                                                \
lib/transaction.c                                                   \
lib/linux/sync_bsp.c                                                \
src/host-agent-aee.c                                                \
src/host-agent.c                                                    \
src/host-agent-callback.c                                           \
src/host-agent-client.c                                             \
src/host-agent-imrt-link.c                                          \
src/host-agent-log.c                                                \
src/host-agent-utils.c                                              \
offline_verifier_standalone/mock_external.c                         \
offline_verifier_standalone/offbv_utiles.c                          \
offline_verifier_standalone/runtime/platform/android/bh_assert.c              \
offline_verifier_standalone/runtime/platform/android/bh_definition.c          \
offline_verifier_standalone/runtime/platform/android/bh_memory.c              \
offline_verifier_standalone/runtime/platform/android/bh_time.c                \
offline_verifier_standalone/runtime/platform/android/bh_platform_log.c        \
offline_verifier_standalone/runtime/platform/android/bh_thread.c              \
offline_verifier_standalone/runtime/vmcore_jeff/jeff-loader.c               \
offline_verifier_standalone/runtime/vmcore_jeff/jeff-runtime.c              \
offline_verifier_standalone/runtime/vmcore_jeff/jeff.c                      \
offline_verifier_standalone/runtime/vmcore_jeff/jeff-utils.c                \
offline_verifier_standalone/runtime/vmcore_jeff/jeff-print.c                \
offline_verifier_standalone/runtime/vmcore_jeff/jeff-thread.c               \
offline_verifier_standalone/runtime/vmcore_jeff/jeff-import.c               \
offline_verifier_standalone/runtime/utils/bh_log.c                          \
offline_verifier_standalone/runtime/utils/bh_list.c                         \
offline_verifier_standalone/runtime/verifier_jeff/instr_props.c             \
offline_verifier_standalone/runtime/verifier_jeff/workmap.c                 \
offline_verifier_standalone/runtime/verifier_jeff/tpool.c                   \
offline_verifier_standalone/runtime/verifier_jeff/verify_jeff.c             \
offline_verifier_standalone/runtime/verifier_jeff/constraint.c              \
offline_verifier_standalone/runtime/verifier_jeff/stackmap.c                \
offline_verifier_standalone/runtime/verifier_jeff/pre_ver_utils.c           \
offline_verifier_standalone/runtime/verifier_jeff/class_support_verifier.c  \
offline_verifier_standalone/runtime/verifier_jeff/ver_utils.c               \
offline_verifier_standalone/runtime/verifier_jeff/jeff_loader_preverify.c   \
offline_verifier_standalone/runtime/verifier_jeff/verify_method.c           \
offline_verifier_standalone/runtime/verifier_jeff/instr_typeflow.c          \
offline_verifier_standalone/runtime/verifier_jeff/bytecode.c                \
offline_verifier_standalone/runtime/verifier_jeff/pre_verify.c              \
offline_verifier_standalone/runtime/verifier_jeff/type_repr.c



LOCAL_C_INCLUDES := $(LOCAL_PATH)/../aon/include           \
$(LOCAL_PATH)/../../../external/libxml2/include     \
$(LOCAL_PATH)/../aee/Beihai/runtime/utils/coap/extension             \
$(LOCAL_PATH)/../aee/Beihai/runtime/utils/coap/er-coap               \
${LOCAL_PATH}/src                                   \
${LOCAL_PATH}/lib                        \
${LOCAL_PATH}/../aee/Beihai/products/iMRT                            \
${LOCAL_PATH}/offline_verifier_standalone          \
${LOCAL_PATH}/offline_verifier_standalone/runtime/include            \
${LOCAL_PATH}/offline_verifier_standalone/runtime/platform/include   \
${LOCAL_PATH}/offline_verifier_standalone/runtime/platform/android     \
${LOCAL_PATH}/offline_verifier_standalone/runtime/verifier_jeff      \
${LOCAL_PATH}/offline_verifier_standalone/runtime/vmcore_jeff

LOCAL_CFLAGS += -DDEBUG=\"1\" -DANDROID -Wall -Wno-int-conversion -Wno-error -Wno-unused-parameter -Wno-int-to-pointer-cast -Wno-int-to-pointer-cast -Wno-implicit-function-declaration -DRUN_ON_LINUX=1 -DAEE_AON 

LOCAL_SHARED_LIBRARIES :=  libaon
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)

