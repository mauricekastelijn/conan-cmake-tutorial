
# **Managing Dependencies in a Multi-Library C++ Project with Conan 2.0 and CMake**

This hands-on tutorial walks seasoned C++ developers through using **Conan 2.0**
with **CMake** to manage dependencies and build a real-world multi-library
project. We’ll focus on a Windows development environment (Visual Studio
toolchain) using CMake’s **Ninja generator**, while highlighting how to stay
cross-platform. Along the way, we’ll compare Conan to NuGet, implement a
step-by-step project with increasing complexity, and delve into advanced
debugging, dependency graph analysis, performance optimizations, and setting up
**GitHub Actions** for CI/CD (including caching Conan packages).

## **1. Introduction**

C++ projects often grow into complex systems with multiple libraries and
external dependencies. Manually managing library builds and ensuring the correct
versions across platforms can be error-prone. Modern tools like Conan (a C/C++
package manager) and CMake can significantly streamline this process. In this
tutorial, you will:

- Understand **why Conan is advantageous** for C++ dependency management (versus
  NuGet and others).
- Set up a cross-platform C++ development environment on Windows using Visual
  Studio’s compiler with CMake’s Ninja build system.
- **Build a multi-library project step-by-step**: starting from a simple library
  and scaling up to multiple libraries and an executable, all managed with Conan
  and built with CMake.
- Learn to use Conan’s tools for **dependency graph analysis and debugging** to
  resolve conflicts or issues.
- Apply **performance optimizations** in builds (like faster build tools and
  binary caching).
- Configure **GitHub Actions CI/CD** to automate building and dependency
  management, including caching of Conan packages for faster builds.

**How to Use This Tutorial:** Each section contains explanations of core
concepts with examples, followed by hands-on steps you can follow. Try to work
through the “Follow Along” and “Exercise” subsections actively – these will
reinforce your understanding by doing. All code snippets are provided for
reference. Let’s begin with a quick overview of Conan and how it compares to
NuGet.

## **2. Conan vs NuGet: High-Level Comparison**

Before diving in, let’s compare **Conan** with **NuGet** in the context of C++
development. Both are package managers, but they differ in design and use cases:

- **Cross-Platform vs. Platform-Specific:** Conan was designed for
  cross-platform C/C++ projects. It runs on Windows, Linux, macOS, and others,
  and can handle platform-specific binaries and cross-compilation seamlessly
  ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=With%20its%20unique%20novel%20architecture,from%20weeks%20to%20hours))
  ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=Truly%20universal%2C%20any%20platform%2C%20any,build%20system%2C%20any%20compiler)).
  NuGet is primarily used in the .NET ecosystem and Visual C++ projects; it’s
  tightly integrated with Visual Studio on Windows. NuGet **can** manage C++
  libraries (as .nupkg packages), but it’s largely Windows/MSBuild-centric and
  not as natural on Linux or macOS. In practice, Conan’s multi-platform support
  and consistent workflow across OSes is a major advantage ([Maintaining
  multiple projects which consume conan packages - Stack
  Overflow](https://stackoverflow.com/questions/75309049/maintaining-multiple-projects-which-consume-conan-packages#:~:text=I%27m%20about%20to%20migrate%20from,by%20project%2C%20dependency%20by%20dependency)).

- **Ecosystem and Packages:** Conan provides access to **ConanCenter**, a
  central repository with thousands of popular C and C++ libraries pre-packaged
  ([How to Create a Modern C Project with CMake and Conan – Abstract
  Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=With%20C%20and%20C%2B%2B%20building,popular%20C%20and%20C%2B%2B%20libraries)).
  ConanCenter’s packages often include precompiled binaries for multiple OSes
  and compiler versions, or recipes to build from source if needed ([How to
  Create a Modern C Project with CMake and Conan – Abstract
  Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=Installing%20packages%20is%20nearly%20as,a%20library%20statically%20or%20dynamically)).
  NuGet’s C++ package ecosystem is much smaller; as one developer notes, “NuGet
  C++ doesn't have a lot of packages” compared to alternatives ([The state of
  C++ package management: The big three :
  r/cpp](https://www.reddit.com/r/cpp/comments/1ek7fxo/the_state_of_c_package_management_the_big_three/#:~:text=From%20my%20experience%20nuget%20c%2B%2B,vcpkg%20has%20a%20lot%20more)).
  Conan’s broad library availability and support for building from source means
  you’re more likely to find the C++ library you need and have it work
  out-of-the-box on your platform ([Maintaining multiple projects which consume
  conan packages - Stack
  Overflow](https://stackoverflow.com/questions/75309049/maintaining-multiple-projects-which-consume-conan-packages#:~:text=I%27m%20about%20to%20migrate%20from,by%20project%2C%20dependency%20by%20dependency)).

- **Binary Compatibility and Build Configuration:** C++ binaries are sensitive
  to compiler version, standard library, architecture, build type
  (Debug/Release), etc. Conan was built to handle this complexity: it tracks
  detailed settings (OS, compiler, runtime, architecture, etc.) for each package
  and ensures you get binaries compatible with your project’s configuration
  ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=Conan%20can%20create%2C%20upload%20to,over%20the%20ABI%20binary%20compatibility))
  ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=Storing%20your%20pre,the%20compliance%20required%20by%20policies)).
  If a precompiled binary isn’t available for your exact setup, Conan can fall
  back to building from source. NuGet packages, on the other hand, often include
  prebuilt libs targeting a limited set of configurations (e.g. maybe only one
  CRT or architecture), making it easy to encounter compatibility issues if your
  setup differs. Conan’s model of packaging per configuration (and tools like
  **profiles** to manage those settings) gives it an edge in C++ binary
  compatibility management.

- **Decentralization and Repositories:** NuGet by default uses the centralized
  nuget.org repository for packages. Conan is decentralized – while ConanCenter
  is popular for open-source libs, you can easily host your own Conan server or
  Artifactory repository for internal binaries. This offers flexibility for
  enterprise use and offline scenarios. Teams can store and retrieve proprietary
  packages in their own servers with Conan, supporting a fully **offline or
  private package ecosystem**. NuGet can work with private feeds too, but
  Conan’s model is built around the idea of multiple remotes and promoting
  packages through development/staging/production repositories.

- **Build System Integration:** NuGet is tied to MSBuild/.vcxproj for C++
  consumption (Visual Studio automatically adds include/lib paths for installed
  NuGet packages). Conan, in contrast, is build-system agnostic. It integrates
  well with CMake (our focus here) – e.g., generating CMake find scripts – but
  it can also work with other systems like Meson, Makefiles, or even Visual
  Studio solutions via custom generators ([Conan 2.0: C and C++ Open Source
  Package
  Manager](https://conan.io/why-conan#:~:text=While%20providing%20completely%20transparent%20integration,etc%2C%20and%20it%20can%20be)).
  Conan operates outside the build system: you run Conan to install deps and
  generate files, then run your build. This decoupling means you can use Conan
  with any C++ project, regardless of how it’s built.

- **Dependency Resolution and Versioning:** Both tools handle transitive
  dependencies and version resolution, but Conan offers more C++-specific
  features. Conan supports version ranges and lockfiles to manage updates and
  ensure reproducible builds ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=Standard%20versioning%20mechanisms%20like%20version,same%20dependencies%20for%20reproducible%20builds))
  ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=header,builds%20are%20performed)).
  Conan can also produce a **dependency graph** in JSON or other formats for
  analysis ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=custom%20binary%20compatibility%2C%20command%20wrappers,or%20dynamic%20configuration)),
  helping in debugging version conflicts or diagnosing dependency trees. NuGet’s
  version resolution is more limited (primarily for .NET assemblies), and it
  lacks Conan’s detailed graph and build info outputs that are tailored for C++
  needs.

In summary, **Conan is often the better choice for C++ dependency management**
due to its cross-platform support, rich C++ package ecosystem, and integration
with C++ build workflows ([Benefits of using Conan in software development —
Engineering Business Outcomes |
Infogain](https://www.infogain.com/blog/benefits-of-using-conan-in-software-development/#:~:text=Conan%20has%20gained%20recognition%20as,io)).
NuGet remains useful in Windows-only or mixed C#/.NET projects, but Conan’s
flexibility and C++ focus make it a go-to for modern C++ projects. Now that we
know *why* we’re using Conan, let’s set up our environment and tools.

## **3. Setting Up the Development Environment (Windows-focused)**

In this section, we’ll configure the tools needed for our tutorial. We target
**Windows** as the primary platform (using Visual Studio’s C++ compiler), but
will keep an eye on multi-platform support. The major tools required are Conan,
CMake, a C++ compiler, and Ninja.

### **3.1 Installing Conan 2.0**

Conan is a Python-based application. The easiest way to install Conan 2.x is via
pip:

- First, ensure you have Python 3 installed. (Visual Studio 2022 comes with an
  optional Python, or download from Python.org.)
- Install Conan globally by running: 

```bash
$ pip install conan==2.*
```

This will install the latest Conan 2.x release. After installation, verify by
running `conan --version` in a terminal, which should display a 2.x version.

**Multi-platform note:** On Linux or macOS, Conan can be installed similarly via
pip. Conan is the same across platforms; differences will come from the compiler
and profile settings, not Conan itself.

### **3.2 C++ Compiler and CMake on Windows**

We’ll use **Microsoft Visual C++ (MSVC)** as our compiler, which comes with
Visual Studio. If you haven’t already:

- Install **Visual Studio 2022** (Community Edition is fine) and include the
  “Desktop development with C++” workload. This provides the MSVC compiler,
  headers, libraries, and tools like the Visual Studio Developer Command Prompt.
- During installation, also select the CMake and Ninja components (Visual
  Studio’s installer has options for CMake tools and the Ninja build system). If
  you forget, you can separately install [CMake](https://cmake.org) and
  [Ninja](https://ninja-build.org) manually. Visual Studio 2022 bundles CMake
  3.x, so you may not need a separate install ([How to Create a Modern C Project
  with CMake and Conan – Abstract
  Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=Windows)).

**Ninja** is a fast build tool that will run our compile/link commands. We’ll
use Ninja via CMake’s generation step. Having Ninja.exe on your PATH is needed
for CMake to use it.

### **3.3 Environment Setup – Using the Developer Command Prompt**

On Windows, to use MSVC from the command line (especially with Ninja), you must
load the environment variables that set up the compiler paths. The recommended
way is to use the **“x64 Native Tools Command Prompt”** that comes with Visual
Studio ([How to Create a Modern C Project with CMake and Conan – Abstract
Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=Windows)).
This is a special console that has all needed environment variables (PATH, LIB,
INCLUDE) configured for the 64-bit MSVC toolchain.

- **Open the Developer Command Prompt:** Find **“x64 Native Tools Command Prompt
  for VS 2022”** from your Start menu (you can search for "x64 Native Tools" in
  the Windows search) ([How to Create a Modern C Project with CMake and Conan –
  Abstract
  Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=match%20at%20L1061%20To%20find,you%20have%20Visual%20Studio%20installed)).
  Launch it, and you’ll get a terminal ready for MSVC. All commands in this
  tutorial assume you’re in this environment on Windows. (On Linux/macOS, you
  can use a normal terminal as the compiler is usually in PATH by default.)

### **3.4 Conan Configuration and Profiles**

When you installed Conan, it likely created a default profile for you (which
captures default settings like your OS and default compiler). Let’s ensure Conan
is configured for MSVC:

- Run `conan profile list` to see available profiles. By default you should see
  a `default` profile.
- Run `conan profile show` to inspect it. On a Windows machine with
  Visual Studio, it should show settings like `os=Windows`, `compiler=msvc` or
  `compiler=Visual Studio` (depending on Conan version), the version (e.g. 17 or
  16), `build_type`, and `arch=x86_64`. If it shows GCC or something else, you
  might need to generate a new profile using `conan profile detect --force`
  while in the Developer Command Prompt. This will auto-detect Visual Studio as
  the compiler.

In case you have non-default profiles set up, you will see them listed here, e.g.:
```bash
$ conan profile list
Profiles found in the cache:
vs2022_x64_profile
vs2022_x64_profile_base
vs2022_x64_profile_debug
vs2022_x64_profile_debug_asan
vs2022_x64_profile_relwithdebinfo
```

You can inspect the non-default profile `vs2022_x64_profile` using:
```bash
$ conan profile show -pr:b vs2022_x64_profile
```

For multi-platform builds, you can create separate profiles, e.g., a `linux-gcc`
profile with GCC settings, a `windows-clang` profile, etc. Profiles allow you to
switch compiler/OS settings easily. In this tutorial, we’ll mostly use the
default (Windows + MSVC) profile, but we will mention how to adapt commands for
other platforms as we go.

### **3.5 Understanding Conan 'Host' and 'Build' Profiles**

Conan uses profiles to define the settings, options, and environment for building and consuming packages. In cross-compilation scenarios, Conan distinguishes between 'host' and 'build' profiles to manage the differences between the machine where the package is built and the machine where the package will run.

This allows you to manage cross-compilation scenarios effectively. By specifying the appropriate profiles, you ensure that Conan correctly handles the differences between the build and target environments, leading to successful builds and deployments.

#### **Host Profile**

The 'host' profile defines the environment where the package will be used or run. This includes settings like the target operating system, architecture, and compiler. For example, if you are building a package that will run on an ARM-based embedded device, the host profile would specify the ARM architecture and the appropriate compiler for that target.

#### **Build Profile**

The 'build' profile defines the environment where the package is being built. This includes settings for the build machine's operating system, architecture, and compiler. For example, if you are building the package on a Windows machine with an x86_64 architecture, the build profile would specify these settings.

#### **Example Profiles**

Here are example profiles for a cross-compilation scenario where you are building on a Windows machine for an ARM-based Linux target.

**Host Profile (`host_profile`)**:
```ini
[settings]
os=Linux
arch=armv8
compiler=gcc
compiler.version=10
compiler.libcxx=libstdc++11
build_type=Release
```

**Build Profile (`build_profile`)**:
```ini
[settings]
os=Windows
arch=x86_64
compiler=Visual Studio
compiler.version=16
compiler.runtime=MD
build_type=Release
```

### **3.5 Project Structure Overview**

Our goal project is a small multi-library application. For illustration, imagine
we are building a **Vector Math** toolkit consisting of:
- A library `vectormath2d` for 2D vector operations.
- A library `vectormath3d` for 3D vector operations, which depends on
  `vectormath2d` (perhaps to reuse some common functionality).
- A main application `dot_product_app` that uses both libraries to compute dot
  products and print results.
- Additionally, we’ll bring in an **external** library via Conan (for example, a
  logging library like `spdlog` or formatting library `fmt` for output).

This setup gives us multiple internal libraries and at least one external
dependency managed by Conan. We will build it incrementally:
1. Create the first library with Conan and CMake.
2. Add an external dependency to that library (using Conan to fetch it).
3. Create a second library depending on the first.
4. Create the final executable linking everything.
5. Iterate on this with debugging and performance considerations.

The directory structure will evolve, but by the end it might look like:
```
/MyProject
├── conanfile.txt   (or conanfile.py describing deps)
├── CMakeLists.txt  (top-level CMake)
├── vectormath2d/
│   ├── CMakeLists.txt
│   └── include/ and src/ files for 2D library
├── vectormath3d/
│   ├── CMakeLists.txt
│   └── include/ and src/ for 3D library
└── app/
    ├── CMakeLists.txt
    └── src/ for main application
```

*(If you prefer, you can start with a single CMakeLists.txt and gradually split
it out as we add libraries.)*

Let’s start implementing this step by step.

## **4. Step-by-Step Project Implementation**

In this section we go from nothing to a complete multi-library project using
Conan and CMake. Follow along by typing the code and commands yourself to get
hands-on experience.

### **4.1 Creating the First Library with CMake and Conan**

**Goal:** Set up a simple C++ library project managed by CMake, and use Conan to
add an external dependency to it.

Let’s begin with `vectormath2d` – a library for 2D vector math. It will have a
function to compute dot product of two 2D vectors, and we’ll use an external
library for logging the result (to demonstrate Conan). We’ll use **spdlog** (a
header-only logging library) as our Conan dependency in this library.

**Project Initialization:**
1. Create a new directory for your project and navigate into it:
   ```bash
   $ mkdir MyProject && cd MyProject
   ```
2. Create a `vectormath2d` subdirectory:
   ```bash
   $ mkdir vectormath2d
   ```
3. Inside `vectormath2d`, create a simple header and source:
   - `vectormath2d/include/vectormath2d.hpp`:
     ```cpp
     #pragma once
     #include <string>
     // Function to compute 2D dot product and return a formatted string
     std::string dot2d(int x1, int y1, int x2, int y2);
     ```
   - `vectormath2d/src/vectormath2d.cpp`:
     ```cpp
     #include "vectormath2d.hpp"
     #include <spdlog/spdlog.h>    // using spdlog from Conan
     #include <spdlog/fmt/fmt.h>  // for formatting (if needed)
     
     std::string dot2d(int x1, int y1, int x2, int y2) {
         int result = x1 * x2 + y1 * y2;
         spdlog::info("Computed 2D dot product: {}", result);
         return fmt::format("({},{})·({},{}) = {}", x1, y1, x2, y2, result);
     }
     ```
   Here, our function calculates the dot product and uses **spdlog** to log an
   info message. Spdlog also brings in the {fmt} library for formatting (we use
   `fmt::format` to return a formatted string).

**CMake Configuration for Library:**

4. Create `vectormath2d/CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(vectormath2d LANGUAGES CXX)
   add_library(vectormath2d STATIC src/vectormath2d.cpp)
   target_include_directories(vectormath2d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
   # Conan will provide spdlog package, we’ll link it later after Conan integration.
   ```
   This defines a static library and its include path. Note: We haven’t yet
   linked spdlog; that will come from Conan’s package configuration.

5. Create a top-level `CMakeLists.txt` in the project root:
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(MyProject LANGUAGES CXX)
   
   # Add subdirectory for vectormath2d library
   add_subdirectory(vectormath2d)
   ```
   For now, it just includes our library subfolder. We’ll expand this as we add
   more components.

6. **Conan Configuration for Dependencies:** In the project root, create a Conan
   recipe. The simplest way is using a `conanfile.txt` for consumption:
   ```ini
   [requires]
   spdlog/1.11.0
   
   [generators]
   CMakeDeps
   CMakeToolchain
   ```
   This declares that we require spdlog (a specific version from ConanCenter)
   and we want Conan to generate CMake files to help find the package. The
   `CMakeDeps` generator will create `spdlog-config.cmake` files for CMake, and
   `CMakeToolchain` will generate a `conan_toolchain.cmake` file that sets up
   CMake with Conan’s settings. Conan 2.0 recommends using these generators for
   CMake integration.

7. **Install the dependency with Conan:** While in the project root, run Conan
   install to resolve and download spdlog:
   ```bash
   $ conan install . --output-folder build --build=missing
   ```
   Here we tell Conan to install requirements for the current directory (which
   has `conanfile.txt`), put generated files into a `build` folder, and
   `--build=missing` allows Conan to build from source if a precompiled package
   is not found (for spdlog, it’s header-only so no build needed, but this is a
   good habit). Conan will:
   - Download spdlog/1.11.0 (and its dependency fmt/x.y.z) from ConanCenter.
   - Create `build/conan_toolchain.cmake` and
     `build/spdlogTarget-release.cmake`, etc., files because of the CMakeDeps
     generator.

   **Note:** This will use the default profile for build and host, since none is
   specified with `-pr`. When cross-compiling, you need to specify both the host
   and build profiles to ensure that Conan correctly handles the differences
   between the build and target environments. You can specify these profiles
   using the `-pr:h` and `-pr:b` flags in the `conan install` command.

   For example, if you have a host profile named `vs2022_x64_profile_debug` and
   a build profile named `vs2022_x64_profile`, you can use the following command
   to install dependencies:

   ```bash
   $ conan install . --output-folder build --build=missing -pr:h vs2022_x64_profile_debug -pr:b vs2022_x64_profile
   ```

   **Tip:** If you see an error or it’s taking too long, ensure you have
   internet access to ConanCenter, and that your profile is set correctly. On
   success, you should have `spdlog` in your local cache and generated config
   files. Conan’s output will list the dependency graph and which packages were
   installed.

8. **Integrate Conan with CMake:** Now that Conan prepared files, we use them in
   our CMake build. We’ll link `spdlog` to our target:
   - Modify the top-level `CMakeLists.txt` to ensure our CMakeLists knows to find spdlog. Add near the top of `CMakeLists.txt`:
     ```cmake
     find_package(spdlog REQUIRED CONFIG)  # Find spdlog via config file
     ```
     This will use the files Conan generated to locate spdlog. Now update the vectormath2d CMakeLists to link spdlog:
     ```cmake
     target_link_libraries(vectormath2d PUBLIC spdlog::spdlog)
     ```
     The Conan CMakeDeps generator defines an imported target `spdlog::spdlog`
     that we can link against. This ensures our library knows about `spdlog`’s
     include directories (even though spdlog is header-only, linking it helps
     transitively if other components use spdlog too).
     

9. **Build the Library:** We’re ready to build vectormath2d. From the project root (and ensure you’re in the Visual Studio dev prompt on Windows):
   - Call CMake to configure the project. We'll use the generated toolchain file for CMake. You can pass it via command line:
     ```bash
     $ cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake  -DCMAKE_BUILD_TYPE=Release .
     $ cmake --build build
     ```
     Let’s unpack this:
     - The Conan toolchain file sets up compiler flags and defines (e.g., for
       any special runtime settings Conan uses).
     - We call CMake to configure the project. `-B build` specifies the build
       directory (where CMakeCache.txt will go). `-G Ninja` chooses the Ninja
       generator (for multi-core build). And we pass the Conan toolchain file
       that was generated earlier so that CMake uses the settings from Conan (this
       file is in the `build/` directory because we used `--output-folder build`
       with conan). `-DCMAKE_BUILD_TYPE=Release` sets a Release build since Ninja
       is a single-config generator (with Visual Studio’s multi-config generator
       you’d specify config during build instead).
     - Then `cmake --build build` invokes Ninja to compile. If all is set, this
       should compile `vectormath2d.cpp`. Conan’s toolchain set up include paths
       so the `#include <spdlog/spdlog.h>` is found (from Conan cache), and link
       settings (though header-only spdlog doesn’t produce a .lib, the fmt lib
       might, but spdlog vendored fmt in newer versions).

   - The modern way is cleaner: including the toolchain using CMake presets:
     ```bash
     $ cmake --list-presets=all
     $ cmake --preset conan-release
     $ cmake --build build
     ```
     Here, `cmake --list-presets=all` lists the available CMake presets, which
     were generated based on the Conan profile selected during conan install.
     This step ensures you select a valid preset for the build. Then, `cmake
     --preset conan-release` configures the project using the appropriate
     Conan-generated preset. The preset handles defining the toolchain and build
     type.
   
   If the build succeeds, we have our first library built and it’s using an
   external dependency via Conan. Congrats!

**Follow Along – Test the Library (Optional):** To verify `vectormath2d` works,
you could create a small temporary C++ file in `build/` that calls `dot2d` and
run it. But we’ll soon build an actual application in Section 4.3. For now,
ensure there were no build errors. If there are errors finding spdlog,
double-check you included the Conan generated files properly in CMake and that
the Conan install step was done *before* CMake configure.

### **4.2 Adding Another Library and Internal Dependency**

Now that one library is set up, let’s add the `vectormath3d` library, which will
depend on `vectormath2d`. This demonstrates an internal dependency (one library
using another within the same project).

**Implement vectormath3d:**
1. Create `vectormath3d/` directory with include and src:
   - `vectormath3d/include/vectormath3d.hpp`:
     ```cpp
     #pragma once
     #include <string>
     // Function to compute 3D dot product
     std::string dot3d(int x1,int y1,int z1, int x2,int y2,int z2);
     ```
   - `vectormath3d/src/vectormath3d.cpp`:
     ```cpp
     #include "vectormath3d.hpp"
     #include "vectormath2d.hpp"  // use 2D for partial calculation
     #include <spdlog/spdlog.h>
     #include <spdlog/fmt/fmt.h>
     
     std::string dot3d(int x1,int y1,int z1, int x2,int y2,int z2) {
         int result = x1*x2 + y1*y2 + z1*z2;
         spdlog::info("Computed 3D dot product: {}", result);
         // maybe reuse 2D dot to format part of the message (demonstration)
         std::string xyPart = dot2d(x1, y1, x2, y2);
         return fmt::format("{} + ({}*{} = {})", 
                             xyPart, z1, z2, z1*z2, result);
     }
     ```
   Here `dot3d` uses `dot2d` from the first library to format part of the dot
   product (just for demonstration of using the other library’s function). It
   also logs via spdlog.

2. Create `vectormath3d/CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(vectormath3d LANGUAGES CXX)
   add_library(vectormath3d STATIC src/vectormath3d.cpp)
   target_include_directories(vectormath3d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
   # Link to vectormath2d and spdlog
   target_link_libraries(vectormath3d PUBLIC vectormath2d spdlog::spdlog)
   ```
   We link `vectormath2d` (so vectormath3d can use dot2d) and also link spdlog
   since this source uses it too. Linking spdlog here ensures if an app links
   only vectormath3d, it still pulls in spdlog dependency transitively.

3. Add the new library to the top-level `CMakeLists.txt`:
   ```cmake
   add_subdirectory(vectormath3d)
   ```
   Place this after adding vectormath2d to ensure the 2D library is built first
   (CMake will figure out dependency order from link libraries anyway).

4. No changes needed in Conan config; we’re still using spdlog. Our conanfile
   already has it, and vectormath3d reuses it. (If vectormath3d needed a
   different external package, we’d add it to conanfile requires.)

**Build updated project:**

5. Run Conan install again, since we added a new
   consumer of spdlog (the dependencies didn’t change, but generally after
   modifying conanfile or profiles, re-run it):
   ```bash
   conan install . --output-folder build --build=missing
   ```
   It should be quick and say everything is up to date (spdlog already in
   cache).

6. Build with CMake:
   ```bash
   cmake --build build
   ```
   (No need to re-run cmake configure if CMakeLists were added via
   add_subdirectory and you run build in the same folder; CMake will detect
   changes. If in doubt, run the full `cmake -B build ...` or `cmake --preset
   conan-release` command again, then build.)

   Now both libraries should compile. You have two static libs:
   `vectormath2d.lib` and `vectormath3d.lib` (on Windows) or `.a` files on other
   platforms.

**Follow Along – Verify internal linking:** To ensure internal dependency is
resolved, you might deliberately introduce an error (e.g., call a function from
vectormath2d that doesn’t exist) to see if the build fails, then fix it.
However, if `dot3d` linked and compiled fine, it means CMake and Conan are
correctly linking the libraries. We’ll test everything in the next step by
building an executable.

### **4.3 Building the Executable and Final Integration**

Now we add the `dot_product_app` application that uses both libraries to produce
an output. This will also test that all dependencies (spdlog, fmt) are properly
linked through.

**Create the application:**
1. Make an `app/` directory.
2. Create `app/main.cpp`:
   ```cpp
   #include <iostream>
   #include "vectormath2d.hpp"
   #include "vectormath3d.hpp"
   
   int main() {
       std::string result2d = dot2d(1,2, 3,4);
       std::string result3d = dot3d(1,2,3, 4,5,6);
       std::cout << "Result from 2D library: " << result2d << "\\n";
       std::cout << "Result from 3D library: " << result3d << "\\n";
       return 0;
   }
   ```
   This simply calls our two library functions and prints the results.

3. Create `app/CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(dot_product_app LANGUAGES CXX)
   add_executable(dot_product_app main.cpp)
   # Link the executable with both libraries
   target_link_libraries(dot_product_app PRIVATE vectormath2d vectormath3d)
   ```
   We link against both libraries. Because those libraries already link spdlog,
   the executable will indirectly get spdlog as well (transitively, since
   vectormath2d and 3d link it PUBLICly in our config).

4. Add the app to top-level CMakeLists:
   ```cmake
   add_subdirectory(app)
   ```
   (After the library subdirectories.)

**Build and Run:**

5. Run Conan install if needed (no new dependencies added, still spdlog only).
6. Build the project:
   ```bash
   cmake --build build
   ```
   This will compile main.cpp and link the executable. If everything is set, the
   link should succeed and produce `dot_product_app.exe` (on Windows) in the
   `build/app/` folder (Ninja’s default is to put binaries in subfolder of build
   corresponding to target folder structure).
   
7. Run the application to test:
   ```bash
   ./build/app/dot_product_app.exe
   ```
   Expected output:
   ```
   Result from 2D library: (1,2)·(3,4) = 11
   Result from 3D library: (1,2)·(4,5) = 14 + (3*6 = 18) = 32
   ```
   Also, since we used spdlog, you’ll see log messages printed to console
   (spdlog defaults to stdout):
   ```
   [info] Computed 2D dot product: 11
   [info] Computed 3D dot product: 32
   ```
   This confirms our multi-library project is working and the dependency
   (spdlog) is correctly integrated by Conan.

**Exercise:** Try switching the build to Debug mode. Clean the `build/`
directory (or create a new one), run Conan install with `-s build_type=Debug` or
use a profile for Debug, configure CMake with `-DCMAKE_BUILD_TYPE=Debug`, and
rebuild. Ensure that the program still runs (you should see “[debug]” log
prefixes if spdlog is configured to change log level, but by default it might
still show info). This tests Conan providing Debug-compatible libraries. On
Windows, Conan will ensure the correct MSVC runtime (MD vs MDd) is used for
spdlog’s fmt dependency if it were a binary. If anything fails, inspect the
error and ensure configurations match (Conan’s default profile might use MD/MDd
runtime for Release/Debug which matches Visual Studio default).

Now we have a fully functioning project with two libraries and an executable,
using Conan to manage *spdlog* (and its dependency *fmt*). Next, we’ll discuss
some advanced aspects: how to debug issues in dependency resolution, analyze the
dependency graph, and optimize our build and binaries.

## **5. Advanced Debugging and Dependency Graph Analysis**

As projects grow, you might encounter issues such as missing dependencies,
version conflicts, or unexpected behaviors (e.g., ODR violations, ABI
mismatches). Conan and CMake provide tools to **analyze the dependency graph**
and help debug such problems.

### **5.1 Inspecting the Dependency Graph**

Conan 2.0 offers commands to inspect the dependency graph without actually
building everything. One useful command is `conan graph info`. From our project
directory, try:

```bash
conan graph info .
```

This will output a breakdown of the dependency graph for the current project’s
conanfile. It lists each node (our `conanfile` as the root, then spdlog/1.11.0,
and fmt (as a dependency of spdlog)). For each node, you see settings like OS,
compiler, etc., options, and whether a binary is found. The output is detailed;
for example, you might see something like:

```
conanfile:
  name: conanfile
  ID: 0
  requirements:
    spdlog/1.11.0
spdlog/1.11.0:
  ID: 1
  recipe: Cached
  binary: Cached
  requires:
    fmt/8.1.1
fmt/8.1.1:
  ID: 2
  recipe: Cached
  binary: Cached
```

(This is a simplified representation.) The key point is you can quickly verify
which versions of packages are in use and whether they came from cache or need
building. Conan can also produce this in JSON for programmatic analysis or an
HTML graph. For instance, `conan graph info . --graph file.html` will generate
an interactive HTML dependency graph ([Graph output for conan info command —
conan 1.66.0
documentation](https://docs.conan.io/1/extending/template_system/info_graph.html#:~:text=documentation%20docs,take%20the%20same%20template))
that you can open in a browser to visualize nodes and edges (helpful in complex
projects).

If you suspect a dependency conflict (e.g., two libs require different versions
of the same package), the graph will show both and Conan will typically warn or
error out during install. Resolving it might involve specifying a consistent
version or using Conan’s **version ranges** or **overrides**. As an advanced
scenario, you could add something like this to your `conanfile.txt` to force a
specific version:
```ini
[tool_requires]
fmt/8.1.1
```
(This is illustrative; Conan’s model for overrides in 2.0 might differ.) The
idea is to ensure only one version of fmt is used if, say, another package
brought a conflicting version.

### **5.2 CMake Debugging Tips**

When things go wrong on the CMake side (e.g., a library not found or linking
fails):
- **Enable verbose makefile output:** Run `cmake --build build --verbose` (for
  Ninja) to see the actual compile/link commands. This can show if include paths
  or library files from Conan are missing.
- **Check `conan_toolchain.cmake`:** Open this file to see what Conan injected.
  It sets variables like `CMAKE_CXX_FLAGS`, defines, etc., and loads Conan’s
  profile settings. If something like the runtime library or architecture seems
  off, you might need to adjust the profile or your CMake call.
- **Use CMake’s find_package logs:** Since we used `find_package(spdlog REQUIRED
  CONFIG)`, if that fails CMake will say it couldn’t find spdlog. Conan’s
  CMakeDeps generator outputs files into the build directory (e.g.,
  `build/spdlogTargets.cmake`). Ensure CMAKE_PREFIX_PATH is pointing to the
  build folder (Conan’s toolchain should handle that). If not, you may manually
  add `-DCMAKE_PREFIX_PATH=build` to your CMake configure.

### **5.3 Common Issues and Conan Solutions**

**Issue: Missing Prebuilt Binary / Long Build Times.** If Conan outputs that
it’s building a dependency from source (especially large ones like Boost), it
can slow down your build. You have a few options:
- Build that dependency once and upload it to a server (Artifactory or a local
  cache) so others don’t need to rebuild it. Conan excels at storing prebuilt
  binaries for reuse ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=Conan%20can%20create%2C%20upload%20to,over%20the%20ABI%20binary%20compatibility))
  ([Conan 2.0: C and C++ Open Source Package
  Manager](https://conan.io/why-conan#:~:text=Storing%20your%20pre,the%20compliance%20required%20by%20policies)).
  If you’re continuously integrating, consider setting up a Conan cache on a
  network drive or using an artifact repository.
- Use package options to reduce bloat. For example, if using Boost, you can
  optionally disable parts you don’t need via Conan options, resulting in less
  to build.
- Ensure you’re not mixing Debug and Release inadvertently – if you built
  everything in Release but accidentally try to link a Debug app, Conan might
  rebuild dependencies in Debug. Keep your profiles consistent.

**Issue: ODR Violations or Multiple Runtime Libraries.** On Windows, mixing
runtime (MT vs MD or debug vs release) incorrectly can cause issues. Conan’s
default profiles typically use MD for Release, MDd for Debug (for MSVC) which
matches Visual Studio’s defaults. If you override these in CMake, be careful to
also adjust Conan settings. The Conan profile setting `compiler.runtime` governs
this. If you need static runtime (MT), you’d create a profile that sets
`compiler.runtime=MT` and Conan will prefer packages built with MT. Using
inconsistent runtime between libraries will lead to linker warnings or crashes
at runtime – Conan helps by tagging package IDs with the runtime, avoiding
accidental mix, but only if you use it correctly.

**Issue: Diamond Dependency Conflict.** Suppose in the future `vectormath2d` and
`vectormath3d` become independent packages, and both depend on a common third
library but at different versions – this is a diamond scenario. Conan will
detect the conflict on `conan install` and typically raise an error. To fix it,
align the versions or use Conan’s `requires` with version ranges so that a
single version can satisfy both. For example, using a range `[>=1.0 <2.0]` for a
dependency in both might let Conan pick a common version.

**Conan Tools for Debugging:** Conan has a `--verbose` and `--debug` flag you
can use on any command to see more internal logs. If something weird happens,
`conan install . -pr default -o "*:some_option=value" --debug` could shed light.
Also, you can always run `conan search package/name` to see what’s in your local
cache or what’s available remotely, in case you suspect a wrong version is being
pulled.

### **5.4 Analyzing and Improving Build Performance**

**Use Ninja or Parallel Builds:** We already chose Ninja which is typically
faster than MSBuild for incremental builds and uses all cores effectively. This
improves compile times.

**Ccache for C++ (Linux/macOS):** On Linux, you might integrate ccache to avoid
recompiling unchanged code. On Windows, you can use clcache for a similar
effect. These can be integrated via Conan as build requirements (ConanCenter has
a `ccache` package that you can add under `[tool_requires]` so that Conan sets
it up in your environment). This is advanced, but worth knowing if build times
become an issue.

**Profile Guided Optimization (PGO) or Link Time Optimization (LTO):** These are
outside Conan’s direct scope but can be enabled via CMake toolchain flags. If
performance of the final binary is crucial, enabling LTO (e.g., `/GL` and
`/LTCG` in MSVC or `-flto` in GCC/Clang) can yield faster executables. You can
add such flags via CMake’s `target_compile_options` or via a toolchain file.
Just ensure all dependencies are built with compatible flags. Header-only libs
like spdlog are fine; compiled libs like fmt might need to be compiled with LTO
as well. In Conan, if you needed to, you could build fmt from source with LTO by
propagating compiler flags – but this goes deep into package recipes. For our
use case, default builds are fine.

**Dependency Graph Optimization:** Sometimes a large graph can be optimized by
making some dependencies **private** or **optionally disabled** if not used. For
example, if a library has an optional dependency that our project doesn’t need,
turning it off in Conan can save build time. Keep an eye on the graph (`conan
graph info`) to spot if you’re pulling in unexpected packages.

**System vs Package Manager Trade-offs:** While Conan can manage almost
everything, you might choose to use system-provided libraries for certain things
in development (to skip building them) and Conan for others. This is doable by
marking some requirements as `external` or using `SystemPackageTool` in Conan
recipes, but it complicates reproducibility. Generally, stick to Conan for
consistency, and rely on CI caching (next section) to mitigate performance
issues.

With these debugging and optimization tips, you can tackle most issues that
arise in a Conan+CMake based build. Now, let’s automate our setup with
Continuous Integration, using GitHub Actions as an example.

## **6. CI/CD with GitHub Actions: Building and Caching Dependencies**

Automating builds and tests via CI/CD ensures your project stays buildable in
clean environments and catches issues early. We will set up a basic GitHub
Actions workflow for our project, focusing on two key aspects:
- Running Conan and CMake to build on each push.
- Caching Conan packages between runs to avoid re-downloading and rebuilding
  dependencies each time (a huge time saver, especially on Windows where compile
  times are high).

### **6.1 Setting up a GitHub Actions Workflow**

Create a file in your repository: **`.github/workflows/ci.yml`** (for example).
We’ll write a workflow that runs on push and pull request:

```yaml
name: C++ CI

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build-and-test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3

      - name: Set up Python
        uses: actions/setup-python@v4
        with:
          python-version: "3.11"

      - name: Install Conan
        run: pip install conan==2.*

      - name: Cache Conan dependencies
        id: cache-conan
        uses: actions/cache@v2
        with:
          path: C:\Users\runneradmin\.conan2
          key: conan-${{ runner.os }}-${{ hashFiles('**/conanfile.txt') }}
          
      - name: Create default profile (if cache miss)
        if: steps.cache-conan.outputs.cache-hit != 'true'
        run: conan profile detect --force

      - name: Install deps (Conan)
        run: conan install . --output-folder build --build=missing

      - name: Configure CMake
        run: cmake --preset conan-release

      - name: Build
        run: cmake --build build --config Release

      - name: Run unit tests
        run: ctest --output-on-failure -C Release
        working-directory: build
```

Let’s break down what’s happening here:

- **Checkout and Setup Python:** We use the official actions to get our source
  code and a Python environment on the runner. Conan requires Python, so we
  ensure Python 3.11 is installed.

- **Install Conan:** Simply pip install Conan 2.x on the runner. (Alternatively,
  there is a community action `actions/setup-conan@v1` that can install and
  cache Conan, but here we do it manually for clarity.)

- **Cache Conan dependencies:** This uses the `actions/cache` to preserve the
  Conan home directory between workflow runs ([cmake - Installing dependencies
  with conan on github actions takes too long - Stack
  Overflow](https://stackoverflow.com/questions/72565963/installing-dependencies-with-conan-on-github-actions-takes-too-long#:~:text=,hit%20%21%3D%20%27true)).
  Conan’s default home on Windows runner is `C:\Users\runneradmin\.conan2` for
  Conan v2 (for Conan1 it was `.conan`). We cache the whole directory. The cache
  `key` is composed of the OS and a hash of `conanfile.txt`. This means if the
  conanfile doesn’t change, the cache is reused. If we update dependencies in
  conanfile, the hash changes and a new cache will be saved (to avoid stale
  data). Caching the Conan folder speeds up builds significantly by reusing
  downloaded packages ([cmake - Installing dependencies with conan on github
  actions takes too long - Stack
  Overflow](https://stackoverflow.com/questions/72565963/installing-dependencies-with-conan-on-github-actions-takes-too-long#:~:text=You%20can%20use%20the%20GitHub,to%20speed%20up%20your%20workflow)).

- **Create default profile if cache missed:** When the cache is restored, it
  includes the default profile. But on a cache miss (first run), we need to
  setup a default profile. We run `conan profile detect --force` ([cmake -
  Installing dependencies with conan on github actions takes too long - Stack
  Overflow](https://stackoverflow.com/questions/72565963/installing-dependencies-with-conan-on-github-actions-takes-too-long#:~:text=,detect))
  to auto-generate a default matching the runner’s compiler. On GitHub’s
  windows-latest image, that will detect MSVC (Visual Studio Build Tools are
  preinstalled on runners). We could also explicitly provide a profile file in
  the repo if needed.

- **Install deps (Conan):** This is the same `conan install` command we used
  locally. It will find packages either in the cache (populated from the cache
  step) or download as needed. If the cache was hit, most likely spdlog and fmt
  are already there, so this step will be quick. We still allow
  `--build=missing` in case a package for the specific MSVC version isn’t in the
  cache (Conan might then build it, though spdlog is header-only so it won’t).

- **Configure CMake and Build:** We run the same CMake configuration and build
  as we did locally.

- **Run tests:** If we had any (say, a CTest unit test), this is where we’d
  execute them. In our example, we might not have tests, so this step could be
  omitted. But demonstrating `ctest` is useful if you add tests to the project.
  We use the `-C Release` to specify configuration because on Windows, CTest
  might need to know which config (since VS can have multiple, but with Ninja
  it’s single-config so this is just to be explicit).

This workflow essentially automates what we did manually: install Conan, install
deps, build with CMake, and optionally run tests. The caching part ensures that
subsequent runs (e.g., for multiple commits or PRs) don’t re-download all
dependencies each time, greatly reducing CI time ([cmake - Installing
dependencies with conan on github actions takes too long - Stack
Overflow](https://stackoverflow.com/questions/72565963/installing-dependencies-with-conan-on-github-actions-takes-too-long#:~:text=You%20can%20use%20the%20GitHub,to%20speed%20up%20your%20workflow)).

**Multi-platform CI:** To make this truly cross-platform, you could extend the
job to a matrix, for example:

```yaml
jobs:
  build-and-test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [windows-latest, ubuntu-latest, macos-latest]
```

And adjust steps for each OS (mostly paths and possibly installing a compiler on
Linux). Conan will handle each platform’s dependencies as long as profiles match
(Conan auto-detect on each will pick gcc/clang on Linux/macOS). You’d also want
to adjust the cache path for Linux (`~/.conan2`). The rest of the steps remain
largely the same. This way, your CI can verify the project builds on all major
OSes, using Conan to provide the right libraries on each.

**Dependency Updates in CI:** By running `conan install` on each build, your CI
will always pull the latest patch releases that satisfy your requirements (if
you use version ranges or update the conanfile). If you want to lock
dependencies to exact versions for reproducibility, Conan’s lockfile feature can
freeze them. You could check in a `conan.lock` file and use `conan install .
--lockfile conan.lock ...` in CI to ensure the exact same dependency versions
are used everywhere. Generating and updating that lockfile would be another step
(perhaps run manually or via a scheduled job when you want to update
dependencies).

**Cache Limitations:** Note that GitHub Actions cache has a size limit
(currently ~5GB per cache). Conan packages, especially big ones (think Qt, Boost
compiled for multiple archs), can fill this quickly. In our simple case, spdlog
and fmt are tiny, so no issue. For large deps, consider only caching your
project’s specific dependencies, or using Conan’s own `conan download`/`conan
upload` to a remote cache. Our key includes the OS and conanfile hash, which
helps prevent the cache from growing endlessly across different configurations.

### **6.2 Automating Dependency Management**

One benefit of using Conan in CI is that adding or updating a dependency becomes
as simple as editing the `conanfile` and pushing the code. The CI will fail if
the new dependency can’t be resolved or built, alerting you early. Some tips for
automation:
- **Pinning versions vs Ranges:** During development you might allow version
  ranges (e.g., `spdlog/[>=1.10 <2.0]`) so you automatically get minor updates.
  But this can lead to non-reproducible builds if a new version is released.
  It’s often safer to pin to a specific version in your conanfile for CI
  reliability, and periodically update it manually (or via a Dependabot-like
  service for Conan, which some exist).
- **Conan lockfiles for releases:** When you’re preparing a release, you can
  generate a lockfile to freeze all dependency versions. Store that with the
  release or in your repo, so that you (and others) can always rebuild that
  exact configuration later. Conan 2’s lockfiles are simpler than Conan 1’s and
  are meant to be part of reproducible workflows ([What's New in Conan 2.0 C/C++
  Package Manager -
  YouTube](https://www.youtube.com/watch?v=NM-xp3tob2Q#:~:text=What%27s%20New%20in%20Conan%202,user%20defined%20global%20binary)).
- **Promoting packages:** In a corporate setup with multiple teams, you might
  want CI to pull from an internal Artifactory rather than ConanCenter, perhaps
  after your QA team approves a library version. Conan’s remotes and Artifactory
  support allow staging of packages. While our example uses the public
  ConanCenter, switching to an internal remote is just a matter of `conan remote
  add` and adjusting recipes (or using `conan config install` in CI to set up
  your remotes).

Our CI/CD setup for this tutorial is simple but covers the essentials: automated
builds on multiple platforms with dependency management handled by Conan and
speed boosted by caching.

## **7. Conclusion and Next Steps**

In this comprehensive tutorial, we covered how to leverage Conan 2.0 and CMake
to manage dependencies in a multi-library C++ project on Windows (and beyond).
Let’s recap the journey and key takeaways:

- **Conan vs NuGet:** You learned the differences and why Conan shines for C++:
  multi-platform support, a rich package ecosystem (ConanCenter) with thousands
  of libraries, and granular control over binary compatibility ([How to Create a
  Modern C Project with CMake and Conan – Abstract
  Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=With%20C%20and%20C%2B%2B%20building,popular%20C%20and%20C%2B%2B%20libraries))
  ([Maintaining multiple projects which consume conan packages - Stack
  Overflow](https://stackoverflow.com/questions/75309049/maintaining-multiple-projects-which-consume-conan-packages#:~:text=I%27m%20about%20to%20migrate%20from,by%20project%2C%20dependency%20by%20dependency)).
  Unlike NuGet, Conan is built for C++ from the ground up, ensuring your Windows
  projects can easily extend to Linux/Mac with the same dependency files.

- **Environment and Tools:** We set up a Windows environment using Visual
  Studio’s compiler with the Ninja generator in CMake. Using Ninja with MSVC
  required using the Developer Command Prompt so that `cl.exe` was on PATH ([How
  to Create a Modern C Project with CMake and Conan – Abstract
  Expression](https://abstractexpr.com/2023/04/19/how-to-create-a-modern-c-project-with-cmake-and-conan/#:~:text=Windows)).
  We configured Conan profiles to match our compiler settings, a crucial step
  for getting the correct binaries.

- **Hands-On Project Build:** Step by step, we created a real multi-library
  project:
  - Started with one library and integrated an external Conan package (spdlog),
    seeing how Conan installs packages and CMake find_package integrates them.
  - Expanded to multiple libraries with internal dependencies (linking one
    library to another via CMake’s target link, which Conan handles
    transparently for includes/links).
  - Built an executable using those libraries and verified everything links and
    runs, showing that Conan-managed dependencies (spdlog/fmt) were correctly
    linked in the final app.
  - Each step was accompanied by actual code and build commands that you could
    follow along and reproduce.

- **Advanced Debugging & Optimization:** We explored tools for diagnosing
  issues:
  - Conan’s `graph info` to inspect the dependency graph and detect conflicts or
    mistakes in versioning.
  - CMake and Conan logging to troubleshoot missing includes or libraries.
  - Common pitfalls like mismatched build types or runtimes and how to address
    them with Conan profiles and settings.
  - Build performance improvements through using Ninja, caching, and potentially
    tools like ccache or LTO for further optimizations. Conan’s ability to reuse
    prebuilt binaries dramatically improves build times and is central to
    performance ([Conan 2.0: C and C++ Open Source Package
    Manager](https://conan.io/why-conan#:~:text=Conan%20can%20create%2C%20upload%20to,over%20the%20ABI%20binary%20compatibility)).

- **CI/CD Integration:** We wrote a GitHub Actions workflow that automates the
  build. Key points included setting up Conan on the runner, using
  `actions/cache` to cache Conan’s package cache between runs ([cmake -
  Installing dependencies with conan on github actions takes too long - Stack
  Overflow](https://stackoverflow.com/questions/72565963/installing-dependencies-with-conan-on-github-actions-takes-too-long#:~:text=,hit%20%21%3D%20%27true)),
  and running the same CMake build. This ensures continuous integration builds
  are fast and consistent with local development. We also discussed scaling that
  to multiple platforms and how Conan facilitates that by handling different
  binaries for each platform in the same project.

**Interactive Exercise Ideas (Post-Tutorial):**
- Try adding a new dependency to the project. For example, add **Eigen** (a C++
  math library) via Conan and use it in one of the vector functions. Update
  `conanfile.txt` with `eigen/3.4.0`, run Conan install, adjust CMake to find
  Eigen (Conan CMakeDeps will generate `Eigen3::Eigen` target), and update code.
  This will reinforce adding and integrating new Conan packages.
- Experiment with Conan options. For instance, spdlog can be used as a
  header-only or as a compiled library. By default it’s header-only. Try setting
  an option in conanfile like:
  ```ini
  [requires]
  spdlog/1.11.0
  [options]
  spdlog:header_only=False
  ```
  This will force Conan to build spdlog as a library. See how the build process
  changes (Conan will compile spdlog, and your link step will link an actual
  spdlog.lib). This helps understand how Conan can manage not just *which*
  dependencies, but *how* they are built.
- Create a deliberate version conflict: modify the project to use two different
  versions of a library (perhaps add another dummy library that requires
  `fmt/9.x` while spdlog wants fmt/8.x). Observe how Conan handles it (it should
  flag a conflict). Then resolve it by aligning versions. This exercise teaches
  how to manage version compatibility.
- Try the project on Linux or macOS. Use a g++ or clang environment, ensure
  Conan profile is set (Conan’s `default` on Linux likely already picks gcc).
  You might need to adjust the logging format (spdlog default on *nix might
  prefix with date/time). The CMake and Conan parts remain the same. This will
  show the portability of the setup.

By mastering Conan with CMake, you’ve gained a powerful skill to simplify C++
project development. No more manually wrangling .libs and include paths for each
dependency or worrying if everyone on the team has the right version of a
library. Conan ensures a consistent, reproducible dependency setup across all
developers and CI systems, while CMake handles the build in a platform-agnostic
way.

**Next Steps:** In real-world projects, you can extend these concepts further.
Explore Conan’s ability to create your **own packages** (e.g., package
`vectormath2d` and `vectormath3d` as reusable components if you plan to reuse
them in other projects). Look into Conan’s **workspaces or multi-repo flows** if
you maintain libraries as separate repositories – Conan can help orchestrate
dependencies across repos as well. Additionally, keep an eye on Conan’s evolving
ecosystem: new tools (like the `conan lock` commands or recipe repositories) can
further improve C++ dependency management.

Happy coding with Conan and CMake – may your builds be fast and your dependency
graphs clean and green! ([Benefits of using Conan in software development —
Engineering Business Outcomes |
Infogain](https://www.infogain.com/blog/benefits-of-using-conan-in-software-development/#:~:text=Conan%20has%20gained%20recognition%20as,io))
([Conan 2.0: C and C++ Open Source Package
Manager](https://conan.io/why-conan#:~:text=Conan%20can%20create%2C%20upload%20to,over%20the%20ABI%20binary%20compatibility))

