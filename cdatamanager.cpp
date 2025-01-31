#include "cdatamanager.h"
#include <QDebug>
#include <QtEndian>

CDataManager::CDataManager()
{

}

QVector<qrModule> CDataManager::generateByteArray(QString string)
{
    QByteArray inputByteString = string.toUtf8();                                                       //Входная строка
    quint16 inputByteSize = inputByteString.size();                                                     //Длина строки в байтах
    quint16 inputBitSize = inputByteString.size() * 8;                                                  //Длина строки в битах
    quint16 inputByteSizeBigEndian = qToBigEndian(inputByteSize);
                                                                                                        //размера строки в разных версиях

    quint8 encodeMethod = 0x04;                                                                         //Код способа кодирования                                                                           //Определение версии

    setVersion(inputBitSize);
    int correction = (m_version < 9) ? 0 : 8;                                                           //Для работы с различиями в размере переменной

    quint8 fillerByte1 = 236;
    quint8 fillerByte2 = 17;
    bool whichFillerByte = 1;

    qrModule outputByteString[m_outputByteStringSize];                                                  //Выходной массив


        // Внесение кода способа кодирования
        for(int i = 0; i < 4; ++i)
            setBit(&outputByteString, i, getBit(&encodeMethod, i + 4), sizeof(outputByteString));
        // Внесение количества байтов
        for(int i = 0; i < 8 + correction; ++i)
            setBit(&outputByteString, i + 4, getBit(&inputByteSizeBigEndian, i + 8 - correction), sizeof(outputByteString));
        // Внесение входной строки
        for(int i = 0; i < inputBitSize; ++i)
            setBit(&outputByteString, i + 12 + correction, getBit(inputByteString.data(), i), sizeof(outputByteString));
        // Внесение нулей
        for(int i = 0; i < 4; ++i)
            setBit(&outputByteString, i + 12 + correction + inputBitSize, 0, sizeof(outputByteString));
        // Внесение чередующихся байтов
        for(int i = inputBitSize + 16 + correction; i < m_outputByteStringSize * 8; ++i)
        {
            if(whichFillerByte)
                setBit(&outputByteString, i, getBit(&fillerByte1, i % 8), sizeof(outputByteString));
            else
                setBit(&outputByteString, i, getBit(&fillerByte2, i % 8), sizeof(outputByteString));
            if( (i + 1) % 8 == 0)
                whichFillerByte = !whichFillerByte;
        }


    qDebug() << "Строка: ";
    printBuffer(&outputByteString, sizeof(outputByteString));
    qDebug() << "Количество байтов: ";
    printBuffer(&inputByteSizeBigEndian, sizeof(inputByteSizeBigEndian));
    qDebug() << "Количество битов: " << inputBitSize;
    qDebug() << "Версия: " << m_version;
    return fillBlocks(outputByteString);
}





void CDataManager::setVersion(int bitCount)
{
    int versionsBitCountL[40] = {152, 272, 440, 640, 864, 1088, 1248, 1552, 1856, 2192,
                                   2592, 2960, 3424, 3688, 4184, 4712, 5176, 5768, 6360, 6888,
                                   7456, 8048, 8752, 9392, 10208, 10960, 11744, 12248, 13048, 13880,
                                   14744, 15640, 16568, 17528, 18448, 19472, 20528, 21616, 22496, 23648};
    int versionsBitCountM[40] = {128, 224, 352, 512, 688, 864, 992, 1232, 1456, 1728,
                                   2032, 2320, 2672, 2920, 3320, 3624, 4056, 4504, 5016, 5352,
                                   5712, 6256, 6880, 7312, 8000, 8496, 9024, 9544, 10136, 10984,
                                   11640, 12328, 13048, 13800, 14496, 15312, 15936, 16816, 17728, 18672};
    int versionsBitCountQ[40] = {104, 176, 272, 384, 496, 608, 704, 880, 1056, 1232,
                                   1440, 1648, 1952, 2088, 2360, 2600, 2936, 3176, 3560, 3880,
                                   4096, 4544, 4912, 5312, 5744, 6032, 6464, 6968, 7288, 7880,
                                   8264, 8920, 9368, 9848, 10288, 10832, 11408, 12016, 12656, 13328};
    int versionsBitCountH[40] = {72, 128, 208, 288, 368, 480, 528, 688, 800, 976,
                                   1120, 1264, 1440, 1576, 1784, 2024, 2264, 2504, 2728, 3080,
                                   3248, 3536, 3712, 4112, 4304, 4768, 5024, 5288, 5608, 5960,
                                   6344, 6760, 7208, 7688, 7888, 8432, 8768, 9136, 9776, 10208};

    int *pBitCountArray;
    switch (m_corLevel) {
        case corLevel::L:
            pBitCountArray = versionsBitCountL;
            break;
        case corLevel::M:
            pBitCountArray = versionsBitCountM;
            break;
        case corLevel::Q:
            pBitCountArray = versionsBitCountQ;
            break;
        case corLevel::H:
            pBitCountArray = versionsBitCountH;
            break;
    }
    for(int i = 0; i < 40; ++i)
    {
//        int test = *(pBitCountArray + i);
        if(*(pBitCountArray + i) > bitCount)
        {
            if((bitCount + (i < 9 ? 16 : 24)) > *(pBitCountArray + i))              //Если сумма кол-ва битов с информацией и кол-ва служебных битов больше
                                                                                    //максимального кол-ва битов выбранной версии - берется версия на 1 больше
                                                                                    //Кол-во служебных битов до 9 версии - 16, после - 24
                ++i;
            m_version = i;
            m_outputByteStringSize = (*(pBitCountArray + i)) / 8;
            return;
        }
    }
}

QVector<int> CDataManager::getBlocks()      //Возвращает массив, где кол-во элементов - кол-во блоков, значение элемента - кол-во байтов в блоке
{
    int *pBlocksCountArray; //Получаем количество блоков
    int bigBlocksCount;
    int blockByteCount;
    QVector <int> blocks;

    switch (m_corLevel) {
        case corLevel::L:
            pBlocksCountArray = m_versionsBlocksCountL;
            break;
        case corLevel::M:
            pBlocksCountArray = m_versionsBlocksCountM;
            break;
        case corLevel::Q:
            pBlocksCountArray = m_versionsBlocksCountQ;
            break;
        case corLevel::H:
            pBlocksCountArray = m_versionsBlocksCountH;
            break;
    }


    m_blocksCount = *(pBlocksCountArray + m_version);
    blockByteCount = m_outputByteStringSize / m_blocksCount; //Сколько стандартных блоков
    bigBlocksCount = m_outputByteStringSize % m_blocksCount; //Сколько больших блоков (+1 байт)

    for(int i = 0; i < m_blocksCount - bigBlocksCount; ++i)  //Формируем массив вида {[кол-во байт в блоке], [кол-во байт в блоке]...}
        blocks << blockByteCount;

    if(bigBlocksCount > 0)
        for(int i = 0; i < bigBlocksCount; ++i)
            blocks << blockByteCount + 1;

    return blocks;
}

QVector<qrModule> CDataManager::fillCorBlock(QVector<qrModule> inputBlock)
{
    QVector<qrModule> block = inputBlock;

    int dataSize = block.size(); // Кол-во байт в блоке

    if(dataSize < m_correctionBytesCount)   //Если в блоке данных меньше байтов, чем должно быть байтов коррекции - разница дополняется нулями
        for(int j = 0; j < m_correctionBytesCount - dataSize; ++j)
            block << 0;

    int *pPolynominal;                      //В зависимости от кол-ва байтов корреции выбирается многочлен
    switch (m_correctionBytesCount) {
        case 7:
            pPolynominal = m_correctionBytesCount7Polynominal;
            break;
        case 10:
            pPolynominal = m_correctionBytesCount10Polynominal;
            break;
        case 13:
            pPolynominal = m_correctionBytesCount13Polynominal;
            break;
        case 15:
            pPolynominal = m_correctionBytesCount15Polynominal;
            break;
        case 16:
            pPolynominal = m_correctionBytesCount16Polynominal;
            break;
        case 17:
            pPolynominal = m_correctionBytesCount17Polynominal;
            break;
        case 18:
            pPolynominal = m_correctionBytesCount18Polynominal;
            break;
        case 20:
            pPolynominal = m_correctionBytesCount20Polynominal;
            break;
        case 22:
            pPolynominal = m_correctionBytesCount22Polynominal;
            break;
        case 24:
            pPolynominal = m_correctionBytesCount24Polynominal;
            break;
        case 26:
            pPolynominal = m_correctionBytesCount26Polynominal;
            break;
        case 28:
            pPolynominal = m_correctionBytesCount28Polynominal;
            break;
        case 30:
            pPolynominal = m_correctionBytesCount30Polynominal;
            break;
    }

    for(int i = 0; i < dataSize; ++i)
    {
        quint8 a = block[0];
        block.pop_front();
        block.push_back(0);
        if(a == 0)
            continue;
        quint8 b = m_reverseGaloisField[a];
        for(int j = 0; j < m_correctionBytesCount; ++j)
        {
            quint8 c = (b + *(pPolynominal + j)) % 255;
            block[j] = block[j] ^ m_galoisField[c];
        }
    }
    return block;
}

QVector<qrModule> CDataManager::bitToQuint8(QVector<qrModule> bitStream)
{
    QVector <qrModule> retStream;

    for(int i = 0; i < bitStream.size() * 8; ++i)
        if(getBit(bitStream.data(), i))
            retStream << 1;
        else
            retStream << 0;

    return retStream;
}

QVector<quint8> CDataManager::fillBlocks(qrModule byteString[])
{
    int *pCorrectionBytesCountArray;
    switch (m_corLevel) {
        case corLevel::L:
            pCorrectionBytesCountArray = m_versionsCorrectionBytesCountL;
            break;
        case corLevel::M:
            pCorrectionBytesCountArray = m_versionsCorrectionBytesCountM;
            break;
        case corLevel::Q:
            pCorrectionBytesCountArray = m_versionsCorrectionBytesCountQ;
            break;
        case corLevel::H:
            pCorrectionBytesCountArray = m_versionsCorrectionBytesCountH;
            break;
    }

    m_correctionBytesCount = *(pCorrectionBytesCountArray + m_version);

    QVector<int> blocksByteCount = getBlocks();
    QVector<QVector<qrModule>> byteBlocks;
    int shift = 0;                                                                  //Сдвиг на следующий блок

    for(int i = 0; i < blocksByteCount.size(); ++i)
    {
        QVector<qrModule> byteBlock;

        for(int j = 0; j < blocksByteCount[i]; ++j)
            byteBlock << byteString[j + shift];

        byteBlocks << byteBlock;
        shift += blocksByteCount[i];
    }

    QVector<QVector<qrModule>> corBlocks;

    foreach(QVector<qrModule> block, byteBlocks)
        corBlocks << fillCorBlock(block);

    QVector<qrModule> dataStream;

    for(int i = 0; i < blocksByteCount.last(); ++i)
        for(int j = 0; j < byteBlocks.size(); ++j)
        {
            if (byteBlocks[j].size() > i)
                dataStream << byteBlocks[j][i];
        }
    for(int i = 0; i < m_correctionBytesCount; ++i)
        for(int j = 0; j < corBlocks.size(); ++j)
        {
            dataStream << corBlocks[j][i];
        }

    return bitToQuint8(dataStream);
}

bool CDataManager::getBit(void *pData, unsigned int idx)
{
    unsigned int byteIdx = idx / 8;
    unsigned int bitIdx = idx % 8;
    unsigned char *pByteBuffer = (unsigned char*)pData;
    unsigned char test = 0x80;
    test = test >> bitIdx;
    return *(pByteBuffer + byteIdx) & test;
}

void CDataManager::setBit(void *pData, unsigned int idx, bool val, unsigned int nBytes)
{
    if(idx > nBytes * 8)
    {
        qDebug() << idx << nBytes * 8;
        return;
    }

    unsigned int byteIdx = idx / 8;
    unsigned int bitIdx = idx % 8;
    unsigned char *pByteBuffer = (unsigned char*)pData;
    if(val)
        pByteBuffer[byteIdx] |= (0x80 >> bitIdx);
    else
        pByteBuffer[byteIdx] &= ~(0x80 >> bitIdx);
}

void CDataManager::printBinary(unsigned char byte, QDebug &dbg)
{
    unsigned char test = 128;
    for(int i = 0; i < 8; ++i)
    {
        dbg.nospace() << ((byte & test) ? '1' : '0');
        test = test >> 1;

    }
}

void CDataManager::printBuffer(void *pBuffer, unsigned int nBytes)
{
    QDebug dbg = qDebug();
    unsigned char *pByteBuffer = (unsigned char*)pBuffer;
    for(unsigned int i = 0; i < nBytes; ++i)
    {
        printBinary(*(pByteBuffer + i), dbg);
        dbg << " ";
    }
}
