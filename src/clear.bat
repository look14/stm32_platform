@ECHO OFF

DEL .\bootloader\project\lists\*.lst
DEL .\bootloader\project\lists\*.map
DEL .\bootloader\project\output\*__i
DEL .\bootloader\project\output\*.crf
DEL .\bootloader\project\output\*.d
DEL .\bootloader\project\output\*.o
DEL .\bootloader\project\output\*.iex
DEL .\bootloader\project\output\*.axf
DEL .\bootloader\project\output\*.plg
DEL .\bootloader\project\output\*.htm
DEL .\bootloader\project\output\*.lnp
DEL .\bootloader\project\output\*.sct
DEL .\bootloader\project\output\*.tra
DEL .\bootloader\project\output\*.bak
DEL .\bootloader\project\output\*._ia

DEL .\application\project\lists\*.lst
DEL .\application\project\lists\*.map
DEL .\application\project\output\*__i
DEL .\application\project\output\*.crf
DEL .\application\project\output\*.d
DEL .\application\project\output\*.o
DEL .\application\project\output\*.iex
DEL .\application\project\output\*.axf
DEL .\application\project\output\*.plg
DEL .\application\project\output\*.htm
DEL .\application\project\output\*.lnp
DEL .\application\project\output\*.sct
DEL .\application\project\output\*.tra
DEL .\application\project\output\*.bak
DEL .\application\project\output\*._ia

DEL .\application\project\sram_lists\*.lst
DEL .\application\project\sram_lists\*.map
DEL .\application\project\sram_output\*__i
DEL .\application\project\sram_output\*.crf
DEL .\application\project\sram_output\*.d
DEL .\application\project\sram_output\*.o
DEL .\application\project\sram_output\*.iex
DEL .\application\project\sram_output\*.axf
DEL .\application\project\sram_output\*.plg
DEL .\application\project\sram_output\*.htm
DEL .\application\project\sram_output\*.lnp
DEL .\application\project\sram_output\*.sct
DEL .\application\project\sram_output\*.tra
DEL .\application\project\sram_output\*.bak
DEL .\application\project\sram_output\*._ia

DEL .\pc\tcl_project\*.sdf
RMDIR /s/q .\pc\tcl_project\ipch

DEL .\pc\tcl_project\Debug\*.obj
DEL .\pc\tcl_project\Debug\*.sbr
DEL .\pc\tcl_project\Debug\*.bsc
DEL .\pc\tcl_project\Debug\*.pch
DEL .\pc\tcl_project\Debug\*.pdb
DEL .\pc\tcl_project\Debug\*.ilk
DEL .\pc\tcl_project\Debug\*.idb
DEL .\pc\tcl_project\Debug\*.log
DEL .\pc\tcl_project\Debug\*.tlog
DEL .\pc\tcl_project\Debug\*.res
DEL .\pc\tcl_project\Debug\*.rc
DEL .\pc\tcl_project\Debug\*.manifest
DEL .\pc\tcl_project\Debug\*.lastbuildstate
DEL .\pc\tcl_project\Debug\*.exp
DEL .\pc\tcl_project\Debug\*.lib
DEL .\pc\tcl_project\Release\*.obj
DEL .\pc\tcl_project\Release\*.sbr
DEL .\pc\tcl_project\Release\*.bsc
DEL .\pc\tcl_project\Release\*.pch
DEL .\pc\tcl_project\Release\*.pdb
DEL .\pc\tcl_project\Release\*.ilk
DEL .\pc\tcl_project\Release\*.idb
DEL .\pc\tcl_project\Release\*.log
DEL .\pc\tcl_project\Release\*.tlog
DEL .\pc\tcl_project\Release\*.res
DEL .\pc\tcl_project\Release\*.rc
DEL .\pc\tcl_project\Release\*.manifest
DEL .\pc\tcl_project\Release\*.lastbuildstate
DEL .\pc\tcl_project\Release\*.exp
DEL .\pc\tcl_project\Release\*.lib

DEL .\pc\test_project\*.sdf
RMDIR /s/q .\pc\test_project\ipch

DEL .\pc\test_project\Debug\*.obj
DEL .\pc\test_project\Debug\*.sbr
DEL .\pc\test_project\Debug\*.bsc
DEL .\pc\test_project\Debug\*.pch
DEL .\pc\test_project\Debug\*.pdb
DEL .\pc\test_project\Debug\*.ilk
DEL .\pc\test_project\Debug\*.idb
DEL .\pc\test_project\Debug\*.log
DEL .\pc\test_project\Debug\*.tlog
DEL .\pc\test_project\Debug\*.res
DEL .\pc\test_project\Debug\*.rc
DEL .\pc\test_project\Debug\*.manifest
DEL .\pc\test_project\Debug\*.lastbuildstate
DEL .\pc\test_project\Debug\*.exp
DEL .\pc\test_project\Debug\*.lib
DEL .\pc\test_project\Debug\*.cache
DEL .\pc\test_project\Release\*.obj
DEL .\pc\test_project\Release\*.sbr
DEL .\pc\test_project\Release\*.bsc
DEL .\pc\test_project\Release\*.pch
DEL .\pc\test_project\Release\*.pdb
DEL .\pc\test_project\Release\*.ilk
DEL .\pc\test_project\Release\*.idb
DEL .\pc\test_project\Release\*.log
DEL .\pc\test_project\Release\*.tlog
DEL .\pc\test_project\Release\*.res
DEL .\pc\test_project\Release\*.rc
DEL .\pc\test_project\Release\*.manifest
DEL .\pc\test_project\Release\*.lastbuildstate
DEL .\pc\test_project\Release\*.exp
DEL .\pc\test_project\Release\*.lib
DEL .\pc\test_project\Release\*.cache

RMDIR /s/q .\mac\test_project\Build
RMDIR /s/q .\mac\test_project\DerivedData

::PAUSE