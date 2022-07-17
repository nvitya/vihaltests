# Test / Example projects for the [VIHAL](https://github.com/nvitya/vihal) Library

In order to compile and run the examples here the following SW and HW required:
 - VIHAL sources, clone it from the GitHub: [github.com/nvitya/vihal](https://github.com/nvitya/vihal)
 - [Eclipse CDT](https://www.eclipse.org/cdt/downloads.php), the latest should be fine
 - [Eclipse Embedded CDT Plugin](https://projects.eclipse.org/projects/iot.embed-cdt), you can install it from the Eclipse Marketplace (available from the Help menu) 
 - Working GCC Cross-Compiler for your target:
   - GCC Compilers for ARM Cortex-M: [ARM Official](https://developer.arm.com/downloads/-/gnu-rm), [xpack](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/)
   - GCC Compilers for RISC-V: [xpack](https://github.com/xpack-dev-tools/riscv-none-elf-gcc-xpack/releases/)
 - Appropriate HW with connections (example dependent)
 - A debug probe (some boards have integrated), its drivers and a working GDBServer (e.g. J-Link or OpenOCD)

## Getting / Symlinking the VIHAL

The examples here require that the VIHAL sources ([github.com/nvitya/vihal](https://github.com/nvitya/vihal)) are accessible in the "vihal" subdirectory within the root directory of this "vihaltests".

The easiest way is to download the VIHAL library into the "vihal" subdirectory.

Or, if you have more projects with the VIHAL, you can create a link (symlink) to an existing one. On Windows you can achieve this using the mklink command (must be executed with Administrator privileges):

  ```c:\work\vihaltests-master>mklink /d vihal c:\work\vihal-master```

on Linux:

  ```/work/vihaltests $ ln -s /work/vihal vihal```

## Importing Projects into Eclipse Workspace

Every example has an associated Eclipse project. Importing the project into the Eclipse workspace require the following steps:
 * From File menu select "Import..."
 * From the "General" cathegory select "Existing Projects into Workspace", and click "Next"
 * Select "Select root directory" and browse for the project root directory (e.g. C:\work\vihaltests-master\blinkled)
 * The project should appear in the list
 * Click "Finish" and the project shoul be appear in the workspace

## Compiling Project

From the Build Configurations select your Board (e.g. BOARD_MIN_F103). Then Select "Build Project" and the project should compile.

If the GCC cross-compiler was not found then you need to specify it (once) in the preferences:
  - For ARM: "Window" / "Preferences" / "MCU" / "Global ARM Toolchains Path"
  - For RISC-V: "Window" / "Preferences" / "MCU" / "Global RISC-V Toolchains Path"
  - For Xtensa (ESP32) there are some special instructions here: [https://github.com/nvitya/vihal/tree/main/xtensa/ESP](https://github.com/nvitya/vihal/tree/main/xtensa/ESP)

## Running / Debugging the Project

After a successful compilation you need to do the following to download and start the application:
 * From "Run" menu select "Debug Configurations..."
 * Double click on your favorite debug probe type (e.g.: GDB OpenOCD Debugging), this creates a new entry   
 * At the "Main" page select the application. When the C/C++ Application is empty just click on the "Search project..." button
   * If your executable is not listed at the "Search Project..." button, then close the window, issue a "Refresh" in the whole project tree and Open the "Debug configurations..." again. This is a known bug in the Eclipse when using external builder
 * At the "Debugger" set up the GDBServer parameters. 
   * "Start the xxx GDB server locally": You can specify, that the GDBServer started automatically or not. For the beginning it may be simpler to use this possibility. I prefer external start because I can see if the debug probe works propery or not and the download starts slightly faster.
   * "Start Debugger": this option must be on, the default should be fine, or you can browse the gdb in the compiler directory. On Linux you can specify "gdb-multiarch"
 * At the "Startup" page you can setup some extra options. I usually turn off "Set breakpoint at: main" so that the applications starts immediately after clicking the debug buffon
 * clicking on the "Debug" should start the application on the HW

This debug configuration must be made only once. The repeated debugging can be achieved by clicking on the green bug speed button in the eclipse.

## Eclipse CDT Tuning

I strongly recommend to set the following options in the "Window" / "Preferences":
 * "Run/Debug" / "Launching": At "Launch Operation" select "Always Launch the previously launched application"
 * "C/C++" / "Indexer": At "Build configuration for the indexer" select "Use active build configuration"
 * In the newer Eclipse versions are some autocomplete options deactivated. You can activate them in the "Preferences" here: "C/C++" / "Editor" / "Content Assist" / "Advanced"
