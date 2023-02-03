build_lib_example(
  NAME thz-macro-central
  SOURCE_FILES thz-macro-central.cc
  LIBRARIES_TO_LINK
    ${libthz}
    ${libcore}
    ${libantenna}
    ${libconfig-store}
    ${libnetwork}
    ${libmobility}
    ${libinternet}
    ${libapplications}
)

build_lib_example(
  NAME thz-nano-adhoc
  SOURCE_FILES thz-nano-adhoc.cc
  LIBRARIES_TO_LINK
    ${libthz}
    ${libcore}
    ${libnetwork}
    ${libmobility}
    ${libinternet}
    ${libapplications}
)
