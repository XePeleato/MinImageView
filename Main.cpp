//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Main.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Open1Click(TObject *Sender)
{
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
			cargar_FD() ;
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

__fastcall int TForm1::cargarSuper_FD ( void )
{
	FileSeek(df, SUPER_BLOCK_BYTES, SEEK_SET) ;
	FileRead(df, &s_b, sizeof(s_b)) ;
	switch (s_b.s_magic)
	{
	case SUPER_MAGIC  : break ;
	case SUPER_REV    : break ;
	case SUPER_V2     : break ;
	case SUPER_V2_REV : break ;
	case SUPER_V3     : break ;
	default : ;
		Application->MessageBox (
			L"Número mágico incorrecto",
			L"Error en el superbloque"
		) ;
		FileClose(df) ;
		FileName = L"" ;
		df = -1 ;
		return 1 ;
	}
	return 0 ;
}
//---------------------------------------------------------------------------


/*  El inodo 0 no se almacena en el disco */

d2_inode inodo [ 1 + 192 ] ;

__fastcall void TForm1::cargarInodos_FD ( void )
{
	int i ;
	int ninodes = s_b.s_ninodes ;
	d2_inode * bufer = (d2_inode *)&inodo[1] ;

/*  hay 16 inodos en cada bloque de 1 KB */

	FileSeek(df, 0x4000, SEEK_SET) ;
	for (i = 0 ; i < 192/16 ; i++ )
	{
		FileRead(df, bufer, 1024) ;
		bufer = (d2_inode *)((int)bufer+1024) ;
	}
}
//---------------------------------------------------------------------------

__fastcall void TForm1::dirRaiz_FD ( void )
{
	int i, j ;
	char bufer [ 1024 ] ;
	int cont ;
	int zone;
	_v7_direct * e = (_v7_direct *)&bufer ; /* entrada */

	TTreeNode * Node1 ;
	TreeView1->Items->Clear() ;
	TreeView1->Items->Add(NULL, "/") ;
	Node1 = TreeView1->Items->Item[0] ;
	Node1->ImageIndex = 2 ;
	Node1->StateIndex = 2 ;

	dirResto_FD(1, Node1); // Cargar directorio raíz recursivamente

	TreeView1->Select(Node1) ;
	StatusBar1->Panels->Items[3]->Text = TreeView1->Selected->Text ;
}
//---------------------------------------------------------------------------

__fastcall void TForm1::dirResto_FD ( int idx, TTreeNode * nodo )   {
	char bufer [ 1024 ] ;
	int i, j;
	int seek;
	int cont = 0;
	int zone;
	_v7_direct * e = (_v7_direct *)&bufer ; /* entrada */

	FileSeek(df, 0x1000*(int)(inodo[idx].d2_zone[0]), SEEK_SET);

	for ( i = 0, zone = 0, seek = 0 ; (i + zone) < (inodo[idx].d2_size+1023)/1024 && zone < V2_NR_DZONES; i++, seek++ ) {
		// Comprobar si hay más bloques directos
		if (i % 4 == 0 && inodo[idx].d2_zone[zone + 1] != 0) {
			FileSeek(df, 0x1000*(int)(inodo[idx].d2_zone[zone + 1]), SEEK_SET);
			zone = zone + 1;
            seek = 0;
		}
		FileRead(df, &bufer, 1024) ;
		for ( j = 0 ; j < 16 ; j++) {
			if (e[j].d_name[0] == '\0') continue ;
			TTreeNode * added = TreeView1->Items->AddChild(nodo, e[j].d_name) ;
			InoTreeNode * addedInoTN = dynamic_cast<InoTreeNode *>(added);
			addedInoTN->inode = e[j].d_ino;
			strcpy(addedInoTN->nombre, e[j].d_name);
			addedInoTN->tam = inodo[e[j].d_ino].d2_size;
			addedInoTN->tipo = inodo[e[j].d_ino].d2_mode;
            addedInoTN->mod = inodo[e[j].d_ino].d2_mtime;

			//nodo->Item[cont]->Data = IntToStr(e[j].d_ino);

			if ((inodo[e[j].d_ino].d2_mode & I_TYPE) == I_REGULAR)
			{
				nodo->Item[cont]->ImageIndex = 1 ;
				nodo->Item[cont]->StateIndex = 1 ;
			}

			else if ((inodo[e[j].d_ino].d2_mode & I_TYPE) == I_DIRECTORY)
			{
				nodo->Item[cont]->ImageIndex = 2 ;
				nodo->Item[cont]->StateIndex = 2 ;
                char ref[DIRSIZ] = {0};
				char ref1[DIRSIZ] = {0};
				ref[0] = '.';
				ref1[0] = '.'; ref1[1] = '.';

				if (strcmp(e[j].d_name, ".") && strcmp(e[j].d_name, "..")) {
					printf("\nHERE WE GO %s\n", e[j].d_name);
					dirResto_FD(e[j].d_ino, nodo->Item[cont]);
					// Volver a la posición donde estábamos
					//FileSeek(df, 0x1000*(int)(inodo[idx].d2_zone[zone] + (seek + 37) * 1024),SEEK_SET);
				}
			}
			cont = cont + 1 ;
		}
	}
}
//---------------------------------------------------------------------------

__fastcall void TForm1::cargar_FD ( void )
{
	if (cargarSuper_FD() != 0) return ;
	cargarInodos_FD() ;
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

 __fastcall void TForm1::extraerFichero (int ino) 
 {
	char * bufer = (char *) malloc(sizeof(char) * inodo[ino].d2_size);
	int i;
	int zona = 0;

	// Leer todos los bloques directos
	while (inodo[ino].d2_zone[zona] != 0 && zona < V2_NR_DZONES) {
		char * ptr = &bufer[zona * 1024];
		leerBloque(inodo[ino].d2_zone[zona], ptr);
		zona++;
	}

	// Probamos si está el indirecto simple
	if (inodo[ino].d2_zone[zona] != 0) {
		int bloques[1024]; // Caben 1024 enteros en un bloque de 4096 bytes
		for (i = 0; i < 1024; i++) {
		   char * ptr = &bufer[(zona + i) * 1024];
			leerBloque(bloques[i], ptr);	
		}
	}

	// TODO: Probar si está el indirecto doble

	if (SaveDialog1->Execute()) {
		int fichero = FileCreate(SaveDialog1->FileName);
		FileWrite(fichero, bufer, sizeof(char) * inodo[ino].d2_size);
		FileClose(fichero);
	}
 }
//---------------------------------------------------------------------------

void __fastcall TForm1::Extraer1Click(TObject *Sender)
{
	InoTreeNode * addedInoTN = dynamic_cast<InoTreeNode *>(clickedNode);
	if ((inodo[addedInoTN->inode].d2_mode & I_TYPE) == I_REGULAR) {
		extraerFichero(addedInoTN->inode);
	} else if ((inodo[addedInoTN->inode].d2_mode & I_TYPE) == I_DIRECTORY) {
        // TODO: Implementar carpetas
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
        InoTreeNode * itNode = dynamic_cast<InoTreeNode *>(node);
		if ((itNode->tipo & I_TYPE) == I_DIRECTORY) {
			for (int i = 0; i < node->Count; i++) {
                itNode = dynamic_cast<InoTreeNode *>(node->Item[i]);
				TListItem *items = ListView1->Items->Add();
				items->Caption = itNode->nombre;
				items->SubItems->Add(IntToStr(itNode->tam));
				items->SubItems->Add(IntToStr(itNode->tipo));
				items->SubItems->Add(DateTimeToStr(UnixToDateTime(itNode->mod)));

				if ((itNode->tipo & I_TYPE) == I_DIRECTORY) {
					items->StateIndex = 2;
				} else if ((itNode->tipo & I_TYPE) == I_REGULAR) {
				   items->StateIndex = 1;
                }

			}
		}


	}
}
//---------------------------------------------------------------------------

