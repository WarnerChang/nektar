message(STATUS "verifying file...
     file='/tmp/ybao/nektar++/ThirdParty/tinyxml_2_4_3-1.tar.bz2'")

set(verified 0)

# If an expected md5 checksum exists, compare against it:
#
if(NOT "a00f5e3b547b803977c5744547a688f7" STREQUAL "")
  execute_process(COMMAND ${CMAKE_COMMAND} -E md5sum "/tmp/ybao/nektar++/ThirdParty/tinyxml_2_4_3-1.tar.bz2"
    OUTPUT_VARIABLE ov
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE rv)

  if(NOT rv EQUAL 0)
    message(FATAL_ERROR "error: computing md5sum of '/tmp/ybao/nektar++/ThirdParty/tinyxml_2_4_3-1.tar.bz2' failed")
  endif()

  string(REGEX MATCH "^([0-9A-Fa-f]+)" md5_actual "${ov}")

  string(TOLOWER "${md5_actual}" md5_actual)
  string(TOLOWER "a00f5e3b547b803977c5744547a688f7" md5)

  if(NOT "${md5}" STREQUAL "${md5_actual}")
    message(FATAL_ERROR "error: md5sum of '/tmp/ybao/nektar++/ThirdParty/tinyxml_2_4_3-1.tar.bz2' does not match expected value
  md5_expected: ${md5}
    md5_actual: ${md5_actual}
")
  endif()

  set(verified 1)
endif()

if(verified)
  message(STATUS "verifying file... done")
else()
  message(STATUS "verifying file... warning: did not verify file - no URL_MD5 checksum argument? corrupt file?")
endif()
