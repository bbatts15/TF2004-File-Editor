#include "mainwindow.h"
#include "ui_mainwindow.h"

//https://ps2linux.no-ip.info/playstation2-linux.com/docs/howto/display_docef7c.html?docid=75

void ITF::readData(){

    swizzled = true; //if loading an ITF, the data will be swizzled

    QByteArray txtrString = "TXTR";
    QByteArrayMatcher matcher(txtrString);
    QTableWidgetItem currentItem;
    long location = 0;
    long startLocation = 0;
    long currentPos = 0;
    long contentLength = 0;
    int colorCount = 0;
    std::tuple <int8_t, int8_t> nibTup;
    fileLength = parent->fileData.readInt(4, 4);

    /*Load header data*/
    parent->fileData.currentPosition = 15;
    versionNum = parent->fileData.readInt(1);
    headerLength = parent->fileData.readInt();
    parent->fileData.currentPosition += 3; //skip the "PS2" label
    propertyByte = parent->fileData.readInt(1);
    unknown4Byte1 = parent->fileData.readHex(4).toInt(nullptr, 16);
    width = parent->fileData.readInt();
    height = parent->fileData.readInt();
    unknown4Byte2 = parent->fileData.readHex(4).toInt(nullptr, 16);
    paletteCount = parent->fileData.readInt();
    //qDebug() << Q_FUNC_INFO << "palette count:" << paletteCount << " found at " << parent->fileData.currentPosition;
    unknown4Byte3 = parent->fileData.readHex(4).toInt(nullptr, 16);
    unknown4Byte4 = parent->fileData.readHex(4).toInt(nullptr, 16);
    /*End header data. Now we can remake the file with any edits.*/

    if(paletteCount == 0){
        paletteCount = 1;
    }

    paletteList.resize(paletteCount);
    if (propertyByte & 1){
        colorCount = 256;
    } else {
        colorCount = 16;
    }
    //qDebug() << Q_FUNC_INFO << "Color count: " << colorCount;
    parent->fileData.currentPosition = matcher.indexIn(parent->fileData.dataBytes, 0)+4;
    startLocation = parent->fileData.currentPosition; //this will be used later to remove the palette from the content
    dataLength = parent->fileData.readInt();
    contentLength = dataLength;
    //qDebug() << Q_FUNC_INFO << "content length: " << contentLength;
    for (int i = 0; i<paletteCount;i++){
        paletteList[i].size = colorCount;
        paletteList[i].paletteColors.resize(colorCount);
        for (int j = 0; j<colorCount; j++){
            paletteList[i].paletteColors[j].R = parent->fileData.readUInt(1);
            paletteList[i].paletteColors[j].G = parent->fileData.readUInt(1);
            paletteList[i].paletteColors[j].B = parent->fileData.readUInt(1);
            paletteList[i].paletteColors[j].A = parent->fileData.readUInt(1);
        }
    }


    int pixelIndex = 0;
    if (propertyByte & 1){
        //256 palette case. nice and easy since each pixel uses 1 byte to refer to the palette
        contentLength -= (paletteCount*1024); //remove the length of the palette section before getting to the pixels
        pixelList.resize(contentLength);
        for (int i = parent->fileData.currentPosition; i < startLocation + dataLength; i++){
            pixelList[pixelIndex] = parent->fileData.readInt(1);
            pixelIndex += 1;
        }
    } else {
        //16 palette case. this is tougher since each pixel is only half a byte (nibble?) and we can only refer to whole bytes.
        //however every image should be an even number of pixels so we can just grab them in pairs.
        //byte_to_nib here to get a tuple of both nibbles
        contentLength -= (paletteCount*64); //remove the length of the palette section before getting to the pixels
        pixelList.resize(contentLength*2);
        for(int i = parent->fileData.currentPosition; i < startLocation + dataLength; i++){
            nibTup = parent->binChanger.byte_to_nib(parent->fileData.mid(location+i, 1));
            pixelList[pixelIndex] = std::get<0>(nibTup);
            pixelIndex += 1;
            pixelList[pixelIndex] = std::get<1>(nibTup);
            pixelIndex += 1;
        }
    }

    //qDebug() << Q_FUNC_INFO << "Pixel list length: " << pixelList.size() << "vs content length" << contentLength;


    //that should be it for loading data.
    //after this, have a function catching the table cell changed slot
    //needs to check if value is between 0 and 255 to be a valid color
    //then only change the associated value if valid

    //the only issue I see with this is turning the data BACK into an ITF file.
    //We don't currently know what all of the header data stands for, which could be an issue

}

void ITF::populatePalette(){
    int paletteIndex = parent->ListLevels->currentIndex();
    if(paletteIndex == -1){
        paletteIndex = 0;
    }
    qDebug() << Q_FUNC_INFO << "Function called. Palette index: " << paletteIndex;
    qDebug() << Q_FUNC_INFO << "Palette colors: " << paletteList[paletteIndex].paletteColors.size();
    parent->createTable(paletteList[paletteIndex].paletteColors.size(), 7);
    QStringList columnNames = {"Palette Index", "Red", "Green", "Blue", "Alpha", "Original", "Current"};
    parent->PaletteTable->setHorizontalHeaderLabels(columnNames);
    parent->PaletteTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    for(int i = 0; i < paletteList[paletteIndex].paletteColors.size(); i++){
        parent->PaletteTable->blockSignals(1);
        //I actually hate this part. Needing to make an item for every single cell feels so overcomplicated but that's how tables work, I guess.
        QTableWidgetItem *cellText0 = parent->PaletteTable->item(i,0);
        if (!cellText0){
            cellText0 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,0,cellText0);
        }
        cellText0->setText(QString::number(i));
        QTableWidgetItem *cellText = parent->PaletteTable->item(i,1);
        if (!cellText){
            cellText = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,1,cellText);
        }
        cellText->setText(QString::number(paletteList[paletteIndex].paletteColors[i].R));
        QTableWidgetItem *cellText2 = parent->PaletteTable->item(i,2);
        if (!cellText2){
            cellText2 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,2,cellText2);
        }
        cellText2->setText(QString::number(paletteList[paletteIndex].paletteColors[i].G));
        QTableWidgetItem *cellText3 = parent->PaletteTable->item(i,3);
        if (!cellText3){
            cellText3 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,3,cellText3);
        }
        cellText3->setText(QString::number(paletteList[paletteIndex].paletteColors[i].B));
        QTableWidgetItem *cellText4 = parent->PaletteTable->item(i,4);
        if (!cellText4){
            cellText4 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,4,cellText4);
        }
        cellText4->setText(QString::number(paletteList[paletteIndex].paletteColors[i].A));
        QTableWidgetItem *cellText5 = parent->PaletteTable->item(i,5);
        if (!cellText5){
            cellText5 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,5,cellText5);
        }
        cellText5->setBackground(QColor::fromRgb(paletteList[paletteIndex].paletteColors[i].R,paletteList[paletteIndex].paletteColors[i].G,paletteList[paletteIndex].paletteColors[i].B));
        QTableWidgetItem *cellText6 = parent->PaletteTable->item(i,6);
        if (!cellText6){
            cellText6 = new QTableWidgetItem;
            parent->PaletteTable->setItem(i,6,cellText6);
        }
        cellText6->setBackground(QColor::fromRgb(paletteList[paletteIndex].paletteColors[i].R,paletteList[paletteIndex].paletteColors[i].G,paletteList[paletteIndex].paletteColors[i].B));
        parent->PaletteTable->blockSignals(0);
    }
}

void ITF::editPalette(int row, int column){
    int changedValue = parent->PaletteTable->item(row, column)->text().toInt(nullptr, 10);
    int paletteIndex = parent->ListLevels->currentIndex();
    qDebug() << Q_FUNC_INFO << "Changed value: " << parent->PaletteTable->item(row, column)->text();
    qDebug() << Q_FUNC_INFO << "Row: " << row << " Column " << column;
    if (changedValue < 256 and changedValue >= 0 ){
        qDebug() << Q_FUNC_INFO << "Valid color value";
        switch (column){
        case 1: paletteList[paletteIndex].paletteColors[row].R = changedValue; break;
        case 2: paletteList[paletteIndex].paletteColors[row].G = changedValue; break;
        case 3: paletteList[paletteIndex].paletteColors[row].B = changedValue; break;
        case 4: paletteList[paletteIndex].paletteColors[row].A = changedValue; break;
        }
        QTableWidgetItem *cellText5 = parent->PaletteTable->item(row, 6);
        cellText5->setBackground(QColor::fromRgb(paletteList[paletteIndex].paletteColors[row].R,paletteList[paletteIndex].paletteColors[row].G,paletteList[paletteIndex].paletteColors[row].B));
        qDebug() << Q_FUNC_INFO << "cell text" << cellText5->text();
    } else {
        qDebug() << Q_FUNC_INFO << "Not a valid color value.";
        switch (column){
        case 1: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].R); break;
        case 2: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].G); break;
        case 3: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].B); break;
        case 4: parent->PaletteTable->item(row,column)->text() = QString::number(paletteList[paletteIndex].paletteColors[row].A); break;
        }
    }
}

void ITF::writeITF(){
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output ITF"), QDir::currentPath() + "/ITF/", parent->tr("Texture Files (*.itf)"));
    QFile itfOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    std::tuple<int8_t, int8_t> nibtup;

    if(!swizzled){
        //if the user exported to BMP, we'll need to re-swizzle the texture
        //unfortunately we can't yet :)
        //swizzle();
    }

    if (itfOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&itfOut);

        qDebug() << Q_FUNC_INFO << "Writing ITF header info";
        itfOut.write("FORM");
        parent->binChanger.intWrite(itfOut, fileLength);
        itfOut.write("ITF0HDR");
        parent->binChanger.byteWrite(itfOut, versionNum);
        parent->binChanger.intWrite(itfOut, headerLength);
        itfOut.write("PS2");
        parent->binChanger.byteWrite(itfOut, propertyByte);
        parent->binChanger.intWrite(itfOut, unknown4Byte1);
        parent->binChanger.intWrite(itfOut, height);
        parent->binChanger.intWrite(itfOut, width);
        parent->binChanger.intWrite(itfOut, unknown4Byte2);
        parent->binChanger.intWrite(itfOut, paletteCount);
        parent->binChanger.intWrite(itfOut, unknown4Byte3);
        parent->binChanger.intWrite(itfOut, unknown4Byte4);
        itfOut.write("TXTR");
        parent->binChanger.intWrite(itfOut, dataLength);
        for(int i = 0; i<paletteCount;i++){
            for(int j = 0; j < paletteList[i].paletteColors.size(); j++){
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].R);
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].G);
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].B);
                parent->binChanger.byteWrite(itfOut, paletteList[i].paletteColors[j].A);
            }
        }
        if(propertyByte&1){
            //256 color
            for (int i = 0; i<pixelList.size();i++){
                parent->binChanger.byteWrite(itfOut, pixelList[i]);
            }
        } else {
            //16 color
            //combine both nibbles into a byte, then write that byte
            for (int i=0; i<pixelList.size();i+=2){
                std::get<0>(nibtup) = pixelList[i];
                std::get<1>(nibtup) = pixelList[i+1];
                //qDebug() << Q_FUNC_INFO << parent->binChanger.nib_to_byte(nibtup);
                parent->binChanger.byteWrite(itfOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }

        //and that should be it

    }

}

void ITF::writeBMP(){
    QString fileOut = QFileDialog::getSaveFileName(parent, parent->tr("Select Output BMP"), QDir::currentPath() + "/BMP/", parent->tr("Texture Files (*.bmp)"));
    QFile bmpOut(fileOut);
    QFile file(fileOut);
    file.open(QFile::WriteOnly|QFile::Truncate);
    file.close();

    std::tuple<int8_t, int8_t> nibtup;

    /*Swizzling currently commented out to make sure the rest works right in the first place.*/
    if(swizzled){
        unswizzle_4bit();
    }

    int dataOffset = 0; //this will be where the pixel data starts in the BMP
    //calculate by adding header length to palette length

    int numColors = 0;

    if (propertyByte&1){
        numColors = 256;
        dataOffset = 54 + (paletteCount*1024);
    } else {
        numColors = 16;
        dataOffset = 54 + (paletteCount*64);
    }

    std::vector<int> reversePixels = pixelList;
    //::reverse(reversePixels.begin(), reversePixels.end());

    if (bmpOut.open(QIODevice::ReadWrite)){
        QDataStream fileStream(&bmpOut);

        bmpOut.write("BM");
        parent->binChanger.intWrite(bmpOut, fileLength);
        parent->binChanger.intWrite(bmpOut, 0); //reserved
        parent->binChanger.intWrite(bmpOut, dataOffset);
        parent->binChanger.intWrite(bmpOut, 40);    //size of info header
        parent->binChanger.intWrite(bmpOut, width);
        parent->binChanger.intWrite(bmpOut, height);
        parent->binChanger.shortWrite(bmpOut, 1);   //number of planes

        if (propertyByte&1){
            parent->binChanger.shortWrite(bmpOut, 8);
        } else {
            parent->binChanger.shortWrite(bmpOut, 4);
        }

        parent->binChanger.intWrite(bmpOut, 0); //compression type
        parent->binChanger.intWrite(bmpOut, height*width/2);
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, x
        parent->binChanger.intWrite(bmpOut, 0); //pixels per meter, y
        parent->binChanger.intWrite(bmpOut, numColors);
        parent->binChanger.intWrite(bmpOut, 0); //important colors. 0 for all.

        for (int i = 0; i<numColors; i++){
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].B);
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].G);
            parent->binChanger.byteWrite(bmpOut, paletteList[parent->ListLevels->currentIndex()].paletteColors[i].R);
            parent->binChanger.byteWrite(bmpOut, 0); // no alpha in bmp.
        }

        if (propertyByte&1){
            for (int i = 0; i<reversePixels.size(); i++) {
                parent->binChanger.byteWrite(bmpOut, reversePixels[i]);
            }
        } else {
            for (int i = 0; i<reversePixels.size(); i+=2) {
                std::get<1>(nibtup) = reversePixels[i];
                std::get<0>(nibtup) = reversePixels[i+1];
                parent->binChanger.byteWrite(bmpOut, parent->binChanger.nib_to_byte(nibtup));
            }
        }
    }
}

void ITF::swizzle(){
    std::vector<int> swizzledImage;
    swizzledImage.resize(pixelList.size());

    //block height and width must be powers of 2
    int blockwidth = 64;
    int blockheight = 32;
    int startBlockPos = width*blockwidth;

    int rowblocks = width/blockwidth;
    int pagex = 0;
    int pagey = 0;
    int px = 0;
    int py = 0;
    int blockx = 0;
    int blocky = 0;
    int block = 0;
    int bx = 0;
    int by = 0;
    int column = 0;
    int cx = 0;
    int cy = 0;
    int cw = 0;
    int page = 0;
    int block_address = 0;
    int pixelIndex = 0;
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            pagex = i/blockwidth;
            pagey = j/blockheight;
            page = pagex+(pagey*rowblocks);

            px = i-(blockx*blockwidth);
            py = j-(blocky*blockheight);

            blockx = px/8;
            blocky = py/8;
            block = blockx + (blocky*8);

            bx = px - (blockx*8);
            by = py - (blocky*8);

            column = by/2;

            cx= bx;
            cy = by-column*2;
            cw = cx+(cy*8);

            swizzledImage[pixelIndex] = pixelList[startBlockPos+(page*blockwidth*blockheight)+(block*blockwidth)+(column*blockheight)+cw];

        }
    }
    pixelList = swizzledImage;
}

void ITF::unswizzle_4bit(){
    //https://github.com/neko68k/rtftool/blob/master/RTFTool/rtfview/p6t_v2.cpp
    //https://forum.xentax.com/viewtopic.php?t=3516
    
    std::vector<int> swizzledImage = pixelList;
    int w = width;
    int h = height;
    int entry;

    if(!swizzled){
        return;
    }
    // Make a copy of the swizzled input and clear buffer
            byte[] Swizzled = new byte[Buf.Length - Where];
            Array.Copy(Buf, Where, Swizzled, 0, Swizzled.Length);

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            // get the pen
            int index = (y * w) + x ;
            //byte uPen = (byte)(Swizzled[index >> 1] >> ((index & 1) * 4) & 0xF);
            
            // swizzle
            int pageX = x &(~0x7f);
            int pageY = y &(~0x7f);

            int pages_horz = (w + 127) / 128;
            int pages_vert = (h + 127) / 128;

            int page_number = (pageY / 128) * pages_horz + (pageX / 128);

            int page32Y = (page_number / pages_vert) * 32;
            int page32X = (page_number % pages_vert) * 64;

            int page_location = page32Y * h * 2 + page32X * 4;

            int locX = x & 0x7f;
            int locY = y & 0x7f;

            int block_location = ((locX & (~0x1f)) >> 1) * h + (locY & (~0xf)) * 2;
            int swap_selector = (((y + 2) >> 2) & 0x1) * 4;
            int posY = (((y & (~3)) >> 1) + (y & 1)) & 0x7;

            int column_location = posY * h * 2 + ((x + swap_selector) & 0x7) * 4;

            int byte_num = (x >> 3) & 3;     // 0,1,2,3
            int bits_set = (y >> 1) & 1;     // 0,1            (lower/upper 4 bits)
            
            byte uPen = (byte)(Swizzled[page_location + block_location + column_location + byte_num] >> ((index & 1) * 4) & 0xF);
            Buf[Where + (index >> 1)] = (byte)((Buf[Where + (index >> 1)] & -bits_set) | (uPen << (bits_set * 4)));
            
            //entry = swizzledImage[page_location + block_location + column_location + byte_num]; - I think I replaced this with the line above
            //entry = (int)((entry >> ((y >> 1) & 0x01) * 4) & 0x0F);          - I think this code is wrong, I think it needs to be a byte not an int - Brad
            pixelList[index] = Buf;                                         //I think this entry is replaced with Buf now but I'm not sure
        }
    }
}

void ITF::unswizzle(){
    //https://gist.github.com/Fireboyd78/1546f5c86ebce52ce05e7837c697dc72
    std::vector<int> swizzledImage = pixelList;
    byte[] InterlaceMatrix = {
        0x00, 0x10, 0x02, 0x12,
        0x11, 0x01, 0x13, 0x03,
    };

    int[] Matrix        = { 0, 1, -1, 0 };
    int[] TileMatrix    = { 4, -4 };

    var pixels = new byte[width * height];
    var newPixels = new byte[width * height];
    
    int d = 0;
    int s = 0; 

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < (width >> 1); x++)
        {
            var p = pixelList[s++];

            swizzledImage[d++] = (byte)(p & 0xF);
            swizzledImage[d++] = (byte)(p >> 4);
        }
    }

    // not sure what this was for, but it actually causes issues
    // we can just use width directly without issues! I think that loop is for rearrancing the pixels? - But our tile matrix is 4x4 and each tile should be 2x4 pixels
    //var mw = width;

    //if ((mw % 32) > 0)
    //    mw = ((mw / 32) * 32) + 32;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            var oddRow = ((y & 1) != 0);

            var num1 = (int)((y / 4) & 1);
            var num2 = (int)((x / 4) & 1);
            var num3 = (y % 4);

            var num4 = ((x / 4) % 4);

            if (oddRow)
                num4 += 4;

            var num5 = ((x * 4) % 16);
            var num6 = ((x / 16) * 32);

            var num7 = (oddRow) ? ((y - 1) * width) : (y * width);

            var xx = x + num1 * TileMatrix[num2];
            var yy = y + Matrix[num3];

            var i = InterlaceMatrix[num4] + num5 + num6 + num7;
            var j = yy * width + xx;

            pixelList[j] = swizzledImage[i];
        }
    }

    if(paletteList.size() == 16){
        std::vector<int> result = pixelList;
        var result = new byte[width * height];
        
        s = 0;
        d = 0;

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < (width >> 1); x++)
                result[d++] = (byte)((pixelList[s++] & 0xF) | (pixelList[s++] << 4));
        }
        
        pixelList = result;
    }
}

void ITF::unswizzle_8bit(){
    //https://github.com/neko68k/rtftool/blob/master/RTFTool/rtfview/p6t_v2.cpp
    std::vector<int> swizzledImage = pixelList;

        if(!swizzled)
        {
            return;
        }

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int block_location = (y & (~0xf)) * width + (x & (~0xf)) * 2;
                int swap_selector = (((y + 2) >> 2) & 0x1) * 4;
                int posY = (((y & (~3)) >> 1) + (y & 1)) & 0x7;
                int column_location = posY * width * 2 + ((x + swap_selector) & 0x7) * 4;

                int byte_num = ((y >> 1) & 1) + ((x >> 2) & 2);     // 0,1,2,3

                pixelList[(y * width) + x] = swizzledImage[block_location + column_location + byte_num];
            }
        }
}
