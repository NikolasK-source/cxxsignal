#
# Copyright (C) 2022 Nikolas Koesling <nikolas@koesling.info>.
# This program is free software. You can redistribute it and/or modify it under the terms of the MIT License.
#

target_sources(${Target} PRIVATE cxxsignal/SignalHandler.hpp)
target_sources(${Target} PRIVATE cxxsignal/Ignore.hpp)
target_sources(${Target} PRIVATE cxxsignal/Default.hpp)
