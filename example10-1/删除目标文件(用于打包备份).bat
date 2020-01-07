del /Q project\RVMDK\Obj\*.*
del /Q project\RVMDK\List\*.*
del /Q project\RVMDK\JLinkLog.txt
del /Q project\RVMDK\*.bak
del /Q project\RVMDK\*.dep
rd /Q /S project\RVMDK\Obj
rd /Q /S project\RVMDK\List

del /Q project\EWARMv5\Project.dep
del /Q project\EWARMv5\Flash
del /Q project\EWARMv5\CpuRAM
del /Q project\EWARMv5\settings
rd  /Q /S project\EWARMv5\Flash
rd /Q /S project\EWARMv5\CpuRAM
rd /Q /S project\EWARMv5\ExtSRAM
rd /Q /S project\EWARMv5\settings


pause