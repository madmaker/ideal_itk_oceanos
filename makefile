all: pre x64
cppcheck: cppcheck_

!if ( $(TEAMCENTER_MAJOR_VERSION) >= 9 )
x64: pre x64_ clean_x64_
!else
x64: pre
!endif

target_x64 = oceanos_custom_idealplm
target_x64_ext = dll

!if "$(target_x64_ext)" == "exe"
mt_target_x64_ext = 1
target_x64_is_dll =
target_x64_is_exe = itk_main.obj
!else
mt_target_x64_ext = 2
target_x64_is_dll = /DLL
target_x64_is_exe =
!endif

clean_x64_:
	@del /Q /S /F .\bin\x64\*.lib
	@del /Q /S /F .\bin\x64\*.exp
	@del /Q /S /F .\bin\x64\*.ilk

pre:
	@if '$(TEAMCENTER_MAJOR_VERSION)' EQU '' ( @echo ***TEAMCENTER_MAJOR_VERSION must be defined*** & exit 1 )
	@if '$(TEAMCENTER_MINOR_VERSION)' EQU '' ( @echo ***TEAMCENTER_MINOR_VERSION must be defined*** & exit 1 )
	@if '$(TEAMCENTER_SUBMINOR_VERSION)' EQU '' ( @echo ***TEAMCENTER_SUBMINOR_VERSION must be defined*** & exit 1 )
	@if '$(TEAMCENTER_SUBSUBMINOR_VERSION)' EQU '' ( @echo ***TEAMCENTER_SUBSUBMINOR_VERSION must be defined*** & exit 1 )

x64_:
	@if not exist .\obj\x64 mkdir .\obj\x64
	@if not exist .\obj\x64\lib mkdir .\obj\x64\lib
	@if not exist .\bin\x64 mkdir .\bin\x64

	cl\
		./src/*.c*\
		./src/action_handlers/*.c*\
		./src/actions/*.c*\
		./src/properties/*.c*\
		/c\
		/nologo\
		/EHsc\
		/W4\
		/Ox\
		/Oy-\
		/MD\
		/D "TEAMCENTER_MAJOR_VERSION=$(TEAMCENTER_MAJOR_VERSION)"\
		/D "TEAMCENTER_MINOR_VERSION=$(TEAMCENTER_MINOR_VERSION)"\
		/D "TEAMCENTER_SUBMINOR_VERSION=$(TEAMCENTER_SUBMINOR_VERSION)"\
		/D "TEAMCENTER_SUBSUBMINOR_VERSION=$(TEAMCENTER_SUBSUBMINOR_VERSION)"\
		/D "STEP" /D "AP203" /D "AP214" /D "WIN32_LEAN_AND_MEAN" /D "WNT" /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE"\
		/D "_SECURE_SCL=0" /D "_HAS_TR1=0" /D "WNT" /D "_WIN32" /D "WIN32" /D "_WIN64" /D "_INTEL=1" /D "CRTAPI1=_cdecl"\
		/D "CRTAPI2=cdecl" /D "IMAN_TAO" /D "BYPASS_FLEX" /D "POMDLL"\
		/D "IPLIB=none"\
		/Fo"./obj/x64/"

	link\
		"./obj/x64/*.obj"\
		/OUT:"./bin/x64/$(target_x64).$(target_x64_ext)"\
		$(target_x86_is_exe)\
		$(target_x64_is_dll)\
		/DEBUG\
		/NOLOGO\
		/SUBSYSTEM:CONSOLE\
		/NODEFAULTLIB:libcmt\
		"wsock32.lib" "advapi32.lib" "msvcrt.lib" "oldnames.lib" "kernel32.lib" "winmm.lib"\
		$(LIBS_X64)\
		/MANIFEST\
		/ManifestFile:"./obj/x64/$(target_x64).manifest"

	mt.exe -manifest "./obj/x64/$(target_x64).manifest" -outputresource:"./bin/x64/$(target_x64).$(target_x64_ext)";$(mt_target_x64_ext)
	
	
cppcheck_:
	cppcheck.exe\
		./src\
		$(CPPCHECK_I_VALUE)\
		--report-progress\
		--force\
		--inline-suppr\
		--enable=all\
		--verbose\
		--xml-version=2
