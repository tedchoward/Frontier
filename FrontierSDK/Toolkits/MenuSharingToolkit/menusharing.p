
{ © copyright 1991-93 UserLand Software, Inc. All RIghts Reserved.}
{ Pascal Conversion by Peter N Lewis <peter@stairways.com.au> }

unit MenuSharing;

interface

	uses
		AppleEvents,Components,Menus, MyCallProc;
	
{$PUSH}
{$ALIGN MAC68K}

	type
		tysharedmenurecord = record	{ must match scripting system record structure }
			idmenu:SInt16;
			flags:SInt16;
			hmenu:MenuHandle;
		end;

	const
		flhierarchic_mask = $0001;
		flhierarchic_bit = 0;
		flinserted_mask = $0002;
		flinserted_bit = 1;

	type
		tymenuarray = array[0..0] of tysharedmenurecord;
		ptrmenuarray = ^tymenuarray;
		hdlmenuarray = ^ptrmenuarray;

	type
		tyMSerrordialog = ProcPtr; { procedure tyMSerrordialog(var theString:Str255); }
		tyMSeventfilter = ProcPtr; { procedure tyMSeventfilter(var theEvent:EventRecord); }
		tyMSmenusinstaller = ProcPtr; { procedure tyMSmenusinstaller(hndl:hdlmenuarray); }
		
	type
		tyMSglobals = record {Menu Sharing globals, all in one struct}
			serverid:OSType;{identifier for shared menu server}
			clientid:OSType; {id of this application}
			hsharedmenus:hdlmenuarray; {data structure that holds shared menus}
			fldirtysharedmenus:Boolean; {if true, menus are reloaded next time app comes to front}
			flscriptcancelled:Boolean; {set true by calling CancelSharedScript}
			flscriptrunning:Boolean; {true if a script is currently running}
			flinitialized:Boolean; {true if InitSharedMenus was successful}
			idscript:longint; {the server's id for the currently running script, makes it easy to kill it}
			menuserver:ComponentInstance; {3.0} 
			serverversion: longint; { 4.1 }
			scripterrorcallback:tyMSerrordialog; {3.0}
			eventfiltercallback:tyMSeventfilter; {3.0}
			menusinsertercallback: tyMSmenusinstaller; { 4.1 }
			menusremovercallback: tyMSmenusinstaller; { 4.1 }
		end;

	var
	{$J+}
		MSglobals:tyMSglobals; {menu sharing globals}

{basic Menu Sharing routines}

	function InitSharedMenus (msed:tyMSerrordialog; msef:tyMSeventfilter):Boolean;

	function SharedMenuHit (menu,item:integer):Boolean;
	
	function SharedScriptRunning:Boolean;
	
	function CancelSharedScript:Boolean;
	
	function CheckSharedMenus (menu:integer):Boolean;
	
	function SharedScriptCancelled (var event,reply:AppleEvent):Boolean;
	

{special-purpose routines}

	function DisposeSharedMenus:Boolean;

	function IsSharedMenu (menu:integer):Boolean;
	
	function EnableSharedMenus (enable:Boolean):Boolean;
	
	function RunSharedMenuItem (menu,item:integer):Boolean;
	
	function SetMenusInserterCallback( installer: tyMSmenusinstaller): Boolean;
	
	function SetMenusRemoverCallback( remover: tyMSmenusinstaller): Boolean;
	
{$ALIGN RESET}
{$POP}

implementation

end.	
