# Test / Example projects
Tests / Examples for the VIHAL (https://github.com/nvitya/vihal) Library

In order to compile and run the examples here the following SW and HW required:
 - The VIHAL Core, clone from the GitHub: [VIHAL](https://github.com/nvitya/vihal)
 - [Eclipse CDT](https://www.eclipse.org/cdt/downloads.php), the latest should be fine
 - [Eclipse Embedded CDT Plugin](https://projects.eclipse.org/projects/iot.embed-cdt), you can install it from the Eclipse Marketplace (available from the Help menu) 
 - Working GCC Cross-Compiler for your target:
   - GCC Compilers for ARM Cortex-M: [ARM Official](https://developer.arm.com/downloads/-/gnu-rm), [xpack](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/)
   - GCC Compilers for RISC-V: [xpack](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/)
 - Appropriate HW with connections (example dependent)
 - A debug probe (some boards have integrated), its drivers and a working GDBServer (e.g. J-Link or OpenOCD)

## Symlinking NVCM Core

The examples here require the NVCM core ([https://github.com/nvitya/nvcm]) sources in the "nvcm" subdirectory within the root directory of the "nvcmtests".
The easiest way to achieve this is to create a symlink.

Before importing these examples into Eclipse CDT make a symlink in the exampes root with the name "vihal" that points to the "vihal" library!


Assuming you have downloaded the example sources into c:\work\nvcmtests-master and you have downloaded the NVCM core into c:\work\nvcm-master the following command is required on Windows:

    c:\work\nvcmtests-master>mklink /d nvcm c:\work\nvcm-master

## Importing Projects into Eclipse Workspace

Every example has an associated Eclipse project. Importing the project into the Eclipse workspace require the following steps:
 * From File menu select "Import..."
 * From the "General" cathegory select "Existing Projects into Workspace", and click "Next"
 * Select "Select root directory" and browse for the project root directory (e.g. C:\work\nvcmtests-master\blinkled)
 * The project should appear in the list
 * Click "Finish" and the project shoul be appear in the workspace

## Compiling Project

From the Build Configurations select your Board (e.g. BOARD_MIN_F103). Then Select "Build Project" and the project should compile.
You might need set the GNU ARM Toolchain Path in the "Window" / "Preferences" / "MCU" / "Global ARM Toolchains Path".

## Running / Debugging the Project

After a successful compilation you need to do the following to download and start the application:
 * From "Run" menu select "Debug Configurations..."
 * Double click on your favorite debug probe type (e.g.: GDB OpenOCD Debugging)
 * This creates a new entry
 * At the "Main" page select the application. When the C/C++ Application is empty just click on search project
 * At the "Debugger" set up the GDBServer parameters. (This requires some knowledge, I might be give some hints later)
 * At the "Startup" page you can setup some extra options. I suggest to turn off "Set breakpoint at: main"
 * clicking on the "Debug" should start the application on the HW

## Eclipse CDT Tuning

I strongly recommend to set the following options in the "Window" / "Preferences":
 * "Run/Debug" / "Launching": At "Launch Operation" select "Always Launch the previously launched application"
 * "C/C++" / "Indexer": At "Build configuration for the indexer" select "Use active build configuration"
