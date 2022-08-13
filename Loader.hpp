#include "LazyImporter.h"

#include <cstdint>
#include <memory>
#include <Windows.h>

#include "CRT.hpp"
#include "xor.hpp"

#include <iostream>
#include <string>

#define Exit LI_FN(ExitProcess)(EXIT_SUCCESS)

using namespace std;