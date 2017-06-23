STATIC_LIBRARY:="archvive"
SHARED_LIBRARY:="dynamic shared"
EXECUTABLE:="exe"
ELF_T:="elf"

define my-dir
$(strip \
  $(eval LOCAL_MODULE_MAKEFILE := $$(lastword $$(MAKEFILE_LIST))) \
  $(if $(filter $(BUILD_SYSTEM)/% $(OUT_DIR)/%,$(LOCAL_MODULE_MAKEFILE)), \
    $(error my-dir must be called before including any other makefile.) \
   , \
    $(patsubst %/,%,$(dir $(LOCAL_MODULE_MAKEFILE))) \
   ) \
 )
endef

define autoseachsubdir
$(call autoseach-subdir-makefile,$(LOCAL_DIR))
endef

define autoseach-subdir-makefile
$(wildcard $(1)/*/currdir.mk)
endef


