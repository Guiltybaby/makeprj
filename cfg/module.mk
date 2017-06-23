
BASE_MODULE:= testlib exe1 testproject

include $(foreach m,$(BASE_MODULE),$(m)/currdir.mk)



