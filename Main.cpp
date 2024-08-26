//---------------------------------------------------------------------------

#include <vcl.h>
#include <filesystem>

#pragma hdrstop

#include "Main.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
using namespace minixfs;

TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

namespace std {
    typedef long clock_t;
}
void __fastcall TForm1::Open1Click(TObject *Sender)
{
	fs = new MinixFS();

	if (OpenDialog1->Execute())
	{
		FileName = OpenDialog1->FileName ;
		__try {
        if ((df = FileOpen(FileName, fmOpenRead | fmShareDenyNone)) < 0)
		{
			Application->MessageBox (
				L"El fichero no puede abrirse para lectura",
				L"Error de apertura"
			) ;
			FileName = L"" ;
			df = -1 ;
			return ;
		}
		} __except(GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
   ? EXCEPTION_EXECUTE_HANDLER
   : EXCEPTION_CONTINUE_SEARCH) {
            Application->MessageBox (
				L"El fichero no puede abrirse para lectura",
				L"Error de apertura"
			) ;
		}

		tam = FileSeek(df, 0, SEEK_END) ;
		if ((tam % 512) != 0) {
			Application->MessageBox (
				L"El tamaño del fichero debe ser multiplo de 512",
				L"Error en el tamaño de la imagen"
			) ;
			FileName = L"" ;
			df = -1 ;
			return ;
		}
		if (tam == 80*2*18*512) { /* deberia ser un disquete doble cara HD */
		//	cargar_FD() ;


		std::filesystem::path mstr = AnsiString(FileName).c_str();
		fs->setup(mstr.u8string());
		fs->loadEntries();
        cargar_FD();
		}
		else /* suponemos que se trata de un disco duro */
		{
			cargar_HD() ;
        }
	}
}
//---------------------------------------------------------------------------

//         Bloques de 1 KB
// 0x00400 superbloque:
//         00 03 00 00 00 00 01 00 01 00 10 00 00 00 00 00
//         FF FF FF FF 68 01 00 00 5A 4D 00 00 00 10 00 00
// 0x01400 mapa de bits de inodos:
//         F0 FF FF FF 0F 00 00 00 00 00 00 00 00 00 00 00
// 0x02000 mapa de bits de bloques
//         FF FF FF FF 01 00 00 00 00 00 00 00 00 00 00 00
// 0x03000 mapa de bits de ???????
//         FF FF FF FF FF FF FF FF 03 00 00 00 00 00 00 00
// 0x04000 tabla de inodos 12 K --> 12K/64 = 12*16 = 3*64 = 192 inodos
// 0x10000 directorio raíz

//---------------------------------------------------------------------------


// ---------------------------------------------------------------------------

__fastcall void TForm1::dirRaiz_FD ( void )
{
	/*
	int i, j ;
	char bufer [ 1024 ] ;
	int cont ;
	int zone;
	_v7_direct * e = (_v7_direct *)&bufer ; */ /* entrada */
							/*
	TTreeNode * Node1 ;
	TreeView1->Items->Clear() ;
	TreeView1->Items->Add(NULL, "/") ;
	Node1 = TreeView1->Items->Item[0] ;
	Node1->ImageIndex = 2 ;
	Node1->StateIndex = 2 ;

	dirResto_FD(1, Node1); // Cargar directorio raíz recursivamente

	TreeView1->Select(Node1) ;
	StatusBar1->Panels->Items[3]->Text = TreeView1->Selected->Text ;
	*/
    TreeView1->Items->Clear() ;
	std::filesystem::path root = "/";
	auto dirsRaiz = fs->listFolder(root);
	TTreeNode * Node1 ;
	TreeView1->Items->Add(NULL, "/") ;
	Node1 = TreeView1->Items->Item[0] ;
	Node1->ImageIndex = 2 ;
	Node1->StateIndex = 2 ;
	InoTreeNode * nuevoInoTN = dynamic_cast<InoTreeNode *>(Node1);
	nuevoInoTN->inode = 1;
    nuevoInoTN->entry = dirsRaiz[0];
	nuevoInoTN->path = root;
    dirResto_FD(nuevoInoTN, dirsRaiz);



	TreeView1->Select(Node1) ;
	StatusBar1->Panels->Items[3]->Text = TreeView1->Selected->Text ;
}
//---------------------------------------------------------------------------

__fastcall void TForm1::dirResto_FD (InoTreeNode * parent, std::vector<DirEntry *> dirs )   {
	for (const auto& f : dirs) {
		TTreeNode * nuevo = TreeView1->Items->AddChild(dynamic_cast<TTreeNode *>(parent), f->mEntry.d_name);
		InoTreeNode * nuevoInoTN = dynamic_cast<InoTreeNode *>(nuevo);
		nuevoInoTN->inode = f->mEntry.d_ino;
		nuevoInoTN->entry = f;
		nuevoInoTN->path = parent->path / f->mEntry.d_name;

		if ((f->mIno.d2_mode & I_TYPE) == I_REGULAR) {
			nuevoInoTN->ImageIndex = 1;
			nuevoInoTN->StateIndex = 1;
		} else if ((f->mIno.d2_mode & I_TYPE) == I_DIRECTORY) {
			nuevoInoTN->ImageIndex = 2;
			nuevoInoTN->StateIndex = 2;

			if (strcmp(f->mEntry.d_name, "..") && strcmp(f->mEntry.d_name, ".")) {
				std::filesystem::path p = parent->path / f->mEntry.d_name;
				dirResto_FD(nuevoInoTN, fs->listFolder(parent->path / f->mEntry.d_name));
			}
		}
	}

}
//---------------------------------------------------------------------------

__fastcall void TForm1::cargar_FD ( void )
{
	dirRaiz_FD() ;
	return ;
}
//---------------------------------------------------------------------------

__fastcall void TForm1::cargar_HD ( void )
{
	return ;
}
//---------------------------------------------------------------------------

__fastcall void TForm1::leerBloque(zone_t bloque, void *destino)
{
	FileSeek(df, 0x1000 * (int)bloque, SEEK_SET);
	FileRead(df, destino, 0x1000);
}
//---------------------------------------------------------------------------

 __fastcall void TForm1::extraerFichero (InoTreeNode * ino)
 {


	// TODO: Probar si está el indirecto doble

	if (SaveDialog1->Execute()) {
		int fichero = FileCreate(SaveDialog1->FileName);
		char * buffer = (char *) malloc(ino->entry->mIno.d2_size * sizeof(char));
		fs->readInode(ino->entry->mIno, buffer, ino->entry->mIno.d2_size, 0);
		FileWrite(fichero, buffer, sizeof(char) * ino->entry->mIno.d2_size);
		FileClose(fichero);
        free(buffer);
	}
 }

 __fastcall void TForm1::anadirFichero (std::filesystem::path path) {
	 OpenDialog1->Filter = "*";
	if (OpenDialog1->Execute())
	{
		FileName = OpenDialog1->FileName ;
		if ((df = FileOpen(FileName, fmOpenRead | fmShareDenyNone)) < 0)
		{
			Application->MessageBox (
				L"El fichero no puede abrirse para lectura",
				L"Error de apertura"
			) ;
			FileName = L"" ;
			df = -1 ;
			return ;
		}
		tam = FileSeek(df, 0, SEEK_END) ;
        char * buffer = (char *) malloc(tam * sizeof(char));
		FileRead(df, buffer, tam);

		//auto path = ino->path.parent_path().wstring();

		auto wndPath = std::filesystem::path(FileName.c_str());
		auto newPath = path / wndPath.filename();
		unsigned long written;
		auto wNewPath = newPath.wstring();
		fs->createFile(wNewPath, false);
		fs->writeFile(wNewPath, buffer, tam, &written, 0);

	}
 }
//---------------------------------------------------------------------------

void __fastcall TForm1::Extraer1Click(TObject *Sender)
{
	InoTreeNode * addedInoTN = dynamic_cast<InoTreeNode *>(clickedNode);
	if ((addedInoTN->entry->mIno.d2_mode & I_TYPE) == I_REGULAR) {
		extraerFichero(addedInoTN);
	} else if ((addedInoTN->entry->mIno.d2_mode & I_TYPE) == I_DIRECTORY) {
		// TODO: Implementar carpetas
   	}
}

void __fastcall TForm1::Anadir1Click(TObject *Sender)
{
	InoTreeNode * addedInoTN = dynamic_cast<InoTreeNode *>(clickedNode);
	if ((addedInoTN->entry->mIno.d2_mode & I_TYPE) == I_REGULAR) {
		anadirFichero(addedInoTN->path.parent_path());
	} else if ((addedInoTN->entry->mIno.d2_mode & I_TYPE) == I_DIRECTORY) {
		anadirFichero(addedInoTN->path);
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TreeView1CreateNodeClass(TCustomTreeView *Sender, TTreeNodeClass &NodeClass)
          
{
	NodeClass = __classid(InoTreeNode);	
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TreeView1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
	if (Button == mbRight) {
		clickedNode = TreeView1->GetNodeAt(X, Y);
	} else if (Button == mbLeft) {
        ListView1->Items->Clear();
		TTreeNode * node = TreeView1->GetNodeAt(X, Y);
		if (!node) return;

        InoTreeNode * itNode = dynamic_cast<InoTreeNode *>(node);
		if ((itNode->entry->mIno.d2_mode & I_TYPE) == I_DIRECTORY) {
			for (int i = 0; i < node->Count; i++) {
                itNode = dynamic_cast<InoTreeNode *>(node->Item[i]);
				TListItem *items = ListView1->Items->Add();
				items->Caption = itNode->entry->mEntry.d_name;
				items->SubItems->Add(IntToStr((int)itNode->entry->mIno.d2_size));
				items->SubItems->Add(IntToStr(itNode->entry->mIno.d2_mode));
				items->SubItems->Add(DateTimeToStr(UnixToDateTime(itNode->entry->mIno.d2_mtime)));

				if ((itNode->entry->mIno.d2_mode & I_TYPE) == I_DIRECTORY) {
					items->StateIndex = 2;
				} else if ((itNode->entry->mIno.d2_mode & I_TYPE) == I_REGULAR) {
				   items->StateIndex = 1;
                }

			}
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Borrar1Click(TObject *Sender)
{
	InoTreeNode * deleteInoTN = dynamic_cast<InoTreeNode *>(clickedNode);
	if ((deleteInoTN->entry->mIno.d2_mode & I_TYPE) == I_REGULAR) {
		fs->deleteEntry(deleteInoTN->path);
        TreeView1->Items->Delete(clickedNode);
	}
}
//---------------------------------------------------------------------------

