#include "mainwindow.h"
#include "ui_mainwindow.h"

void ProgWindow::openVBIN(){
    fileMode = "VBIN";
    VBIN vbinFile;
    //clearWindow();
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VBIN"), QDir::currentPath() + "/VBIN/", tr("Model Files (*.vbin)"));
    if (!fileIn.isNull()){
        changeMode(2);
        vbinFile.filePath = fileIn;
        vbinFile.highestLOD = 0;
        vbinFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        vbinFile.fileName = fileInfo.fileName();
//        fileData.dataBytes.clear();

//        QFile inputFile(fileIn);
//        inputFile.open(QIODevice::ReadOnly);
//        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        if(vbinFile.readData()){
            messageError("There was an error reading " + vbinFile.fileName);
            return;
        }
        qDebug() << Q_FUNC_INFO << "File data read.";
        vbinFiles.push_back(vbinFile);
        createLevelList(vbinFile.highestLOD);
        createFileList();
        createMultiRadios();
    } else {
        messageError("There was an error opening the file.");
    }

}

void ProgWindow::openITF(){
    fileMode = "ITF";
    ITF itfFile;
    deleteMultiRadios();
    clearWindow();
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select ITF"), QDir::currentPath() + "/ITF/", tr("Texture Files (*.itf)"));
    if (!fileIn.isNull()){
        changeMode(1);
        itfFile.filePath = fileIn;
        itfFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        itfFile.fileName = fileInfo.fileName();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        itfFile.readData();
        itfFiles.push_back(itfFile);
        createLevelList(itfFile.paletteCount);
        createFileList();
        itfFile.populatePalette();
        qDebug() << Q_FUNC_INFO << "File data read.";
    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openVAC(){
    fileMode = "Tone";
    clearWindow();
    QString fileIn = QFileDialog::getOpenFileName(this, tr("Select VAC"), QDir::currentPath() + "/VAC/", tr("Audio Files (*.vac)"));
    if (!fileIn.isNull()){
        changeMode(5);
        vacFile->filePath = fileIn;
        vacFile->parent = this;
        fileData.readFile(fileIn);

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        vacFile->tempRead();
        qDebug() << Q_FUNC_INFO << "File data read.";
    } else {
        messageError("There was an error opening the file.");
    }

}

void ProgWindow::openDefinition(bool binary){
    int passed=0;
    DefinitionFile openedFile;
    QString fileIn;
    clearWindow();
    if(binary){
        fileIn = QFileDialog::getOpenFileName(this, tr("Select BMD"), QDir::currentPath(), tr("Database Definition Files (*.BMD)"));
    } else {
        fileIn = QFileDialog::getOpenFileName(this, tr("Select TMD"), QDir::currentPath(), tr("Database Definition Files (*.TMD)"));
    }
    if (!fileIn.isNull()){
        changeMode(4);
        openedFile.filePath = fileIn;
        openedFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.binary = binary;
        openedFile.database = false;
        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        if (passed != -1){
            createDBButtons();
            openedFile.createDBTree();
            definitions.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

void ProgWindow::openDatabase(bool binary){
    int passed=0;
    DatabaseFile openedFile;
    QString fileIn;
    clearWindow();
    if(binary){
        fileIn = QFileDialog::getOpenFileName(this, tr("Select BDB"), QDir::currentPath(), tr("Database Definition Files (*.BDB)"));
    } else {
        fileIn = QFileDialog::getOpenFileName(this, tr("Select TDB"), QDir::currentPath(), tr("Database Definition Files (*.TDB)"));
    }
    if (!fileIn.isNull()){
        changeMode(4);
        openedFile.filePath = fileIn;
        openedFile.parent = this;
        fileData.readFile(fileIn);

        QFile inputFile(fileIn);
        inputFile.open(QIODevice::ReadOnly);
        QFileInfo fileInfo(inputFile);
        openedFile.fileName = fileInfo.fileName();
        openedFile.binary = binary;
        openedFile.database = true;
        fileData.dataBytes = inputFile.readAll();

        qDebug() << Q_FUNC_INFO << "File data loaded.";
        passed = openedFile.readData();
        qDebug() << Q_FUNC_INFO << "File data read.";
        if (passed != -1){
            createDBButtons();
            openedFile.createDBTree();
            databases.push_back(openedFile);
        } else {
            messageError("There was an error reading the file.");
        }

    } else {
        messageError("There was an error opening the file.");
    }
}

