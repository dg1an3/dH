
FieldCOMps.dll: dlldata.obj FieldCOM_p.obj FieldCOM_i.obj
	link /dll /out:FieldCOMps.dll /def:FieldCOMps.def /entry:DllMain dlldata.obj FieldCOM_p.obj FieldCOM_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del FieldCOMps.dll
	@del FieldCOMps.lib
	@del FieldCOMps.exp
	@del dlldata.obj
	@del FieldCOM_p.obj
	@del FieldCOM_i.obj
