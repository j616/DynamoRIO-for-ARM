# **********************************************************
# Copyright (c) 2009 VMware, Inc.    All rights reserved.
# **********************************************************

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# 
# * Neither the name of VMware, Inc. nor the names of its contributors may be
#   used to endorse or promote products derived from this software without
#   specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.

# Support clients specifying a version number to the find_package() command:

set(PACKAGE_VERSION 2.0.0)

if("${PACKAGE_FIND_VERSION_MAJOR}" EQUAL 0)
  # No version specified: assume compatible
  set(PACKAGE_VERSION_COMPATIBLE 1)
elseif("${PACKAGE_FIND_VERSION_MAJOR}" LESS 2)
  # Asking for lesser major version == backward compatible
  set(PACKAGE_VERSION_COMPATIBLE 1)
elseif("${PACKAGE_FIND_VERSION_MAJOR}" EQUAL 2)
  # Asking for lesser version == backward compatible
  if("${PACKAGE_FIND_VERSION_MINOR}" EQUAL 0)
    set(PACKAGE_VERSION_EXACT 1)
  elseif("${PACKAGE_FIND_VERSION_MINOR}" LESS 0)
    set(PACKAGE_VERSION_COMPATIBLE 1)
  else()
    # We are relatively forward compatible except features we added
    # but better err on the side of caution
    set(PACKAGE_VERSION_UNSUITABLE 1)
  endif()
else()
  set(PACKAGE_VERSION_UNSUITABLE 1)
endif()
