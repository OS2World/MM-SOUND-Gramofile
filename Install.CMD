/* Install skript for gramofile */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs

SAY ''
SAY 'Gramofile 1.6'
SAY ''
SAY 'This install skript will create a gramofile folder on your'
SAY 'desktop.'
SAY ''
'@PAUSE'
SAY ''

thisDir=DIRECTORY()
gramofile=thisDir||'\bin\gramofile.exe'

rc=SysCreateObject('WPFolder','Gramofile 1.6','<WP_DESKTOP>','OBJECTID=<FLD_GRAMOFILE>;','update')
IF rc=1 then DO
	rc=SysCreateObject('WPProgram','Gramofile 1.6','<FLD_GRAMOFILE>','EXENAME='||gramofile||';OBJECTID=<PGM_GRAMOFILE>;','update')
	rc=SysCreateObject('WPShadow','Docs','<FLD_GRAMOFILE>','SHADOWID='||thisDir||'\Docs;OBJECTID=<SHW_GRAMOFILEDOCS>;','update')
	rc=SysCreateObject('WPShadow','COPYING','<FLD_GRAMOFILE>','SHADOWID='||thisDir||'\Docs\Copying;OBJECTID=<SHW_GRAMOFILELICENSE>;','update')
	rc=SysCreateObject('WPShadow','README.OS2','<FLD_GRAMOFILE>','SHADOWID='||thisDir||'\README.OS2;OBJECTID=<SHW_GRAMOFILEREADME>;','update')
	SAY 'Done'
	SAY ''
	'@PAUSE'
	EXIT	
END

Say 'Cannot create folder. Ending...'
SAY ''
'@PAUSE'	