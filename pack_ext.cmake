# Copyright (c) 2018, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/bin)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/bin DESTINATION ${CMAKE_INSTALL_PREFIX}
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT media)
endif()
if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/include)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/include DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT media)
endif()
if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/lib)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/lib DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT media)
elseif (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/lib64)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libva-install/usr/lib64 DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT media)
endif()

if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libdrm-install/usr/lib)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libdrm-install/usr/lib DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT media)
elseif (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libdrm-install/usr/lib64)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../libdrm-install/usr/lib64 DESTINATION ${CMAKE_INSTALL_PREFIX} COMPONENT media)
endif()

if (IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../msdk-install/mediasdk)
    install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../msdk-install/mediasdk DESTINATION /opt/intel USE_SOURCE_PERMISSIONS COMPONENT media)
endif()