
BASE_MODULE:=  testlib exe1 testproject lib

include $(foreach m,$(BASE_MODULE),$(m)/currdir.mk)



