#include "library.h"

#include <iostream>
#include "MinixFS.h"
#include "minixfs_operations.h"

#include <chrono>

void runDokan(minixfs::MinixFS& fs) {
    DOKAN_OPTIONS dokanOptions;
    ZeroMemory(&dokanOptions, sizeof(dokanOptions));

    dokanOptions.Version = DOKAN_VERSION;
    dokanOptions.SingleThread = true;
    dokanOptions.Timeout = 0;
    dokanOptions.GlobalContext = reinterpret_cast<ULONG64>(&fs);
    dokanOptions.MountPoint = L"M:\\";
    dokanOptions.Options |= DOKAN_OPTION_ALT_STREAM;
    //dokanOptions.Options |= DOKAN_OPTION_WRITE_PROTECT;
    dokanOptions.Options |= DOKAN_OPTION_CURRENT_SESSION;
    dokanOptions.Options |= DOKAN_OPTION_STDERR;// | DOKAN_OPTION_DEBUG;

    DOKAN_OPERATIONS dokanOperations;
    ZeroMemory(&dokanOperations, sizeof(DOKAN_OPERATIONS));
    minixfs::setup(dokanOperations);

    DokanInit();
    auto status = DokanMain(&dokanOptions, &dokanOperations);

    DokanShutdown();
}


int hello(const char *path) {
    auto *fs = new minixfs::MinixFS();
    //auto status = fs->setup(L"C:\\Users\\Eduardo\\fda.img");
    auto status = fs->setup(path);
    std::string message;
    switch (status) {
        case minixfs::SetupState::ErrorFile:
            message = "Error al abrir el fichero.";
            break;
        case minixfs::SetupState::ErrorFormat:
            message = "Formato incorrecto. Es un disquete de Minix?";
            break;
        case minixfs::SetupState::Success:
            message = "Disquete cargado con éxito.";
            break;
    }
    std::cout << message << std::endl;
    runDokan(*fs);
    return 0;
}

int noInput() {
    std::cout << "Uso: MinImage.exe <disquete>" << std::endl;
    return -1;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        return hello("Y:\\fda.img");
        //return noInput();
    }
    return hello(argv[1]);
}


