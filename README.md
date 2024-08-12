# EEG Project

## How to use Project

require [esp-idf](https://docs.espressif.com/projects/esp-idf/en/v5.0.2/esp32/get-started/index.html) v5.0.2

```bash
idf.py build
```

## Example folder contents

The project **EEG Project** contains one source file [main.cpp](main/main.cpp). The file is located in folder [main](main).

this projects are built using ESP-IDF(CMake & python). The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
.
├── components
│   ├── fft_test
│   │   ├── CMakeLists.txt
│   │   ├── fft_test.cpp
│   │   └── fft_test.h
│   └── kissfft                             # fft library
│       ├── CMakeLists.txt
│       ├── _kiss_fft_guts.h
│       ├── kiss_fft.c
│       ├── kiss_fft.h
│       ├── kiss_fft_log.h
│       ├── kiss_fftr.c
│       └── kiss_fftr.h
├── main                                    # EEG Project Sources
│   ├── Bpmpulse.cpp
│   ├── CMakeLists.txt
│   ├── EyeBlinkingChecker.cpp
│   ├── Fulldowndetect.cpp
│   ├── GetRawData.cpp
│   ├── LeadOff.cpp
│   ├── Uart.cpp
│   ├── includes                            # EEG includes header
│   │   ├── Bpmpulse.h
│   │   ├── EyeBlinkingChecker.h
│   │   ├── Fulldowndetect.h
│   │   ├── GetRawData.h
│   │   ├── LeadOff.h
│   │   └── Uart.h
│   └── main.cpp
├── run_clang_format.sh
└── sdkconfig
```
