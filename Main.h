//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Dialogs.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
//---------------------------------------------------------------------------

#define EXTERN extern

#include "InoTreeNode.h"
#include "..\fda\usr\src\include\ansi.h"
#include "..\fda\usr\src\include\dirent.h"
#include "..\fda\usr\src\include\sys\stat.h"
#include "..\fda\usr\src\include\sys\types.h"
#include "..\fda\usr\src\include\minix\const.h"
#include "..\fda\usr\src\servers\fs\const.h"
#include "..\fda\usr\src\servers\fs\super.h"
#include "..\fda\usr\src\servers\fs\type.h"


class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TTreeView *TreeView1;
	TOpenDialog *OpenDialog1;
	TImageList *ImageList1;
	TSplitter *Splitter1;
	TListView *ListView1;
	TStatusBar *StatusBar1;
	TPopupMenu *PopupMenu1;
	TMenuItem *Extraer1;
	TSaveDialog *SaveDialog1;
	TMainMenu *MainMenu1;
	TMenuItem *Open1;
	void __fastcall Open1Click(TObject *Sender);
	void __fastcall Extraer1Click(TObject *Sender);
	void __fastcall TreeView1CreateNodeClass(TCustomTreeView *Sender, TTreeNodeClass &NodeClass);
	void __fastcall TreeView1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);

private:	// User declarations
public:		// User declarations
	AnsiString FileName ;
	ino_t *inos ;
	int df ;
	int tam ;
	struct super_block s_b ;
    TTreeNode * clickedNode;
	__fastcall TForm1(TComponent* Owner);
	__fastcall int cargarSuper_FD ( void ) ;
	__fastcall void cargarInodos_FD ( void ) ;
	__fastcall void dirRaiz_FD ( void ) ;
	__fastcall void dirResto_FD ( int idx, TTreeNode * nodo ) ;
	__fastcall void cargar_FD ( void ) ;
	__fastcall void cargar_HD ( void ) ;
    __fastcall void leerBloque(zone_t bloque, void *destino);
	__fastcall void extraerFichero ( int ino ) ;
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
