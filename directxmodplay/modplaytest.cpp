// modplaytest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "DirectXMODPlay.h"
#include "moduleplayer.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.

	HWND hWnd = GetForegroundWindow();
	if (hWnd == NULL)
	    hWnd = GetDesktopWindow();

	using namespace std;
	ifstream module;
	try 
    {
		module.open("E:\\MUSIC\\MODS\\IMAGINE.XM", ios_base::in | ios_base::binary);
    }
    catch( ios_base::failure f ) 
    {
      cout << "Caught an exception." << endl;
	  return -1;
    }
	using namespace DXModPlay;

	DirectXMODPlayer *Player = new DirectXMODPlayer();
	Player->Init(hWnd);
	if (!Player->Load(module, MODULE_XM))
	{
		std::cout << "Could not load module" << endl;
		return -2;
	}
	Player->SetVolume(255);
	Player->Play();
	
	Sleep(1000000);
	return 0;
}



