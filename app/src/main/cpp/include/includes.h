#pragma once

// C++ headers
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <limits>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <unordered_set>
#include <utility>

// C headers
#include <unistd.h>
#include <fcntl.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <jni.h>
#include <android/log.h>

// Custom external headers

#include <dobby.h>

// Internal headers
#include "input.h"
#include "log.h"
#include "Misc.h"
#include "JNIStuff.h"
#include "ptr.h"
#include "GameValues.h"
#include "Pointers.h"
#include "Patches.h"
#include "Hooks.h"
#include "../Menu.h"