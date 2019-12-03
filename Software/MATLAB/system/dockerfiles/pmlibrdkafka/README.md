## PhysMod libraries built in docker
<!-- Copyright 2019, The MathWorks, Inc. -->

This is a current workaround, to deal with other build problems.

### Not to share with customers, until clarified with physmod team
Had a discussion with Andrew Bennett, who thinks it would be ok, but we
need a formal acceptance.

### What is this

A utility that will create docker files with pre-compiled libraries for one of the platforms
we support, currently debian, ubuntu, raspbian.

All files are copied to local folder using `copyPMFiles.m`

Currently, only one file has to be altered.

```
/src # gcc -c -std=c99 -fPIC -O3 -fno-loop-optimize -fno-aggressive-loop-optimizations -D_P
OSIX_C_SOURCE=199309L -DSTACK_SIZE=64 -DMODEL=HouseThermalWithKafka -DNUMST=2 -DNCSTATES=0 
-DHAVESTDIO -DMODEL_HAS_DYNAMICALLY_LOADED_SFCNS=0 -DUNIX -DCLASSIC_INTERFACE=0 -DALLOCATIO
NFCN=0 -DTID01EQ=0 -DTERMFCN=1 -DONESTEPFCN=1 -DMAT_FILE=0 -DMULTI_INSTANCE_CODE=0 -DINTEGE
R_CODE=0 -DMT=0 -DCLASSIC_INTERFACE=0 -DALLOCATIONFCN=0 -DTERMFCN=1 -DONESTEPFCN=1 -DMAT_FI
LE=0 -DMULTI_INSTANCE_CODE=0 -DINTEGER_CODE=0 -DMT=0  -DTID01EQ=0 -DMODEL=HouseThermalWithK
afka -DNUMST=2 -DNCSTATES=0 -DHAVESTDIO -DMODEL_HAS_DYNAMICALLY_LOADED_SFCNS=0 -DUNIX -I/ex
tern/include -I/simulink/include -I/rtw/c/src -I/rtw/c/src/ext_mode/common -I/rtw/c/ert  -I
. -I./ex -I./lang -I./mc -I./ne -I./pm -I./ssc_comp -I./ssc_core -I./ssc_sli -o "ssc_core_c
8d83e88.c.o" "ssc_core/ssc_core_c8d83e88.c"
ssc_core/ssc_core_c8d83e88.c:2045:33: error: unknown type name 'bool'; did you mean '_Bool'?
 ssc_core_9ALKYaJ8yERfAH4baXaMC_,bool ssc_core_IJdXUIbGjNEYaGD9ZC_YO0){
                                 ^~~~
                                 _Bool
```

Changing this instance of `bool` to `boolean_T` lets it compile, and hopefully work.

A makefile is created from a template, containing all the physmod files, and what libraries they belong to,
