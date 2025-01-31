#include "qworldwidget.h"

#include <QPainter>
#include <QDebug>
#include <QResizeEvent>

QWorldWidget::QWorldWidget(QWidget *parent) : QWidget(parent)
{

}

void QWorldWidget::drawQR(qrModule *data, QSize size)
{
    m_qrData.resize(size.width() * size.height());
    m_size = size;
    memcpy(m_qrData.data(), data, sizeof(qrModule) * size.width() * size.height());
    update();
}

void QWorldWidget::generateByteArray(QString string)
{
    QVector<qrModule> rawData;
    rawData = m_dataManager.generateByteArray(string);
    prepare();
    generateServiceModules();
    checkNA();
    qDebug() << rawData.size();
    putData(rawData);
    update();
}

void QWorldWidget::prepare()
{
    setSize();
    int size = m_size.width();
    m_qrData.resize(size * size);

    // Заполнение неопределенными значениями
    for(int i = 0; i < size; ++i)
        for(int j = 0; j < size; ++j)
            m_qrData[i * size + j] = NA;
}

void QWorldWidget::generateServiceModules()
{
    int size = m_size.width();
    int version = m_dataManager.getVersion();
    int corLevel = m_dataManager.getCorLevel();

    // Отступы
    setBlankSpaces(size);

    // Поисковые узоры
    setSearchPattern({3, 3});
    setSearchPattern({size - 12, 3});
    setSearchPattern({3, size - 12});

    // Полосы синхронизации
    setSynchroStripes(size);

    // Выравнивающие узоры
    if(version > 0)
    {
        int lastPatternPos;
        for(int i = 6; i >= 0; --i)
            if(m_correctionPatternsPos[i][version] != -1)
            {
                lastPatternPos = i;
                break;
            }

        for(int i = 0; i < 7; ++i)
            for(int j = 0; j < 7; ++j)
            {
                if(m_correctionPatternsPos[i][version] == -1 || m_correctionPatternsPos[j][version] == -1)
                    continue;
                if(version > 5 && ((i == 0 && j == 0) || (i == 0 && j == lastPatternPos) || (i == lastPatternPos && j == 0)))
                    continue;
                setCorrectionPattern({m_correctionPatternsPos[i][version] + 2, m_correctionPatternsPos[j][version] + 2});
            }
    }

    //Код версии
    if(version > 5)
        setVersionPattern(size, version);

    //Код маски и уровня коррекции
    setMaskPattern(size, corLevel);
}

int QWorldWidget::at(int i, int j)
{
    return i * m_size.width() + j;
}

void QWorldWidget::putData(QVector<qrModule> data)
{
    int size = m_size.width();
//    qDebug() << data;


//    for(int i = 0; i < data.size(); ++i)
//        data[i] = 0;

//    int j = size - 5;
//    int dataIdx = 0;
    int nStrip = 0;
    int nColumn = 0;
    int Y = 0; // Смещение в одной полоске вертикальное

    qDebug() << data.size();

    for(int iBit = 0; iBit < data.size();/* ++iBit*/)
    {
        for(Y = 0; Y < size - 4; ++Y)
        {
            for( nColumn = 0; nColumn < 2; ++nColumn)
            {
                int xQr = size - 5 - nStrip * 2 - nColumn;
                if(xQr <= 10) //Если мы на вертикальной полосе синхронизации и левее, то еще на одну влево
                    --xQr;
                int yQr;
                if(nStrip % 2) //Если полоска нечетная, то сверху вниз
                    yQr = Y;
                else
                    yQr = size - 5 - Y;

//                qDebug() << xQr << yQr;

                if(xQr < 0 || yQr < 0)
                {
//                    qDebug() << "We're out of top left corner!!!" << iBit ;
                    return;
                }
                if(m_qrData[at(yQr, xQr)] == NA)
                {
                    if(iBit == data.size())
                        continue;
                    m_qrData[at(yQr, xQr)] = mask0(xQr - 4, yQr - 4, data[iBit]);
                    ++iBit;
//                    qDebug() << "       writing!";
                }
                else
                {
//                    qDebug() << "       skipping...";
                    continue;
                }

            }
        }
        ++nStrip;
//        qDebug() << "Strip!\n";
    }

    for(int i = 0; i < size; ++i)
        for(int j = 0; j < size; ++j)
            if(m_qrData[at(i, j)] == NA)
                m_qrData[at(i, j)] = mask0(j - 4, i - 4, WHITE);
}

void QWorldWidget::setSize()
{
    if(m_dataManager.getVersion() == 0)
    {
        m_size = QSize(29, 29);
        return;
    }
    for(int i = 6; i >= 0; --i)
        if(m_correctionPatternsPos[i][m_dataManager.getVersion()] != -1)
        {
            m_size = QSize(m_correctionPatternsPos[i][m_dataManager.getVersion()] + 15, m_correctionPatternsPos[i][m_dataManager.getVersion()] + 15);
            return;
        }
}

void QWorldWidget::setSearchPattern(QPoint topLeftModule)
{
    int size = m_size.width();
    for(int i = 0; i < 9; ++i)
        for(int j = 0; j < 9; ++j)
        {
            bool innerWhite = (i == 2) || (i == 6) || (j == 2) || (j == 6);
            bool outerWhite = (i == 0) || (i == 8) || (j == 0) || (j == 8);
            bool outerBlack = (j == 1) || (j == 7) || (i == 1) || (i == 7);

            if((innerWhite && !outerBlack) || outerWhite)
                m_qrData[at(i + topLeftModule.x(), j + topLeftModule.y())] = WHITE;
            else
                m_qrData[at(i + topLeftModule.x(), j + topLeftModule.y())] = BLACK;
        }
}

void QWorldWidget::setBlankSpaces(int size)
{
    for(int i = 0; i < size; ++i)
        for(int j = 0; j < 4; ++j)
        {
            m_qrData[at(i, j)] = WHITE;
            m_qrData[at(j, i)] = WHITE;
            m_qrData[at(i, size - j -1)] = WHITE;
            m_qrData[at(size - j - 1, i)] = WHITE;
        }
}

void QWorldWidget::setSynchroStripes(int size)
{
    for(int i = 0; i < size - 22; ++i)
    {
        m_qrData[at(10, 11 + i)] = (i % 2 == 0 ? WHITE : BLACK);
        m_qrData[at(11 + i, 10)] = (i % 2 == 0 ? WHITE : BLACK);
    }
}

void QWorldWidget::setCorrectionPattern(QPoint topLeftModule)
{
    for(int i = 0; i < 5; ++i)
        for(int j = 0; j < 5; ++j)
        {
            bool innerWhite = (i == 1) || (i == 3) || (j == 1) || (j == 3);
            bool outerBlack = (j == 0) || (j == 4) || (i == 0) || (i == 4);

            if(innerWhite && !outerBlack)
                m_qrData[at(i + topLeftModule.x(), j + topLeftModule.y())] = WHITE;
            else
                m_qrData[at(i + topLeftModule.x(), j + topLeftModule.y())] = BLACK;
        }

}

void QWorldWidget::setVersionPattern(int size, int version)
{
    qrModule versionCode[34][18] = {{0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0},
                               {0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0},
                               {1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                               {1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
                               {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0},
                               {0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0},
                               {1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0},
                               {1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0},
                               {0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0},
                               {0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0},
                               {1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0},
                               {1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0},
                               {0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0},
                               {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0},
                               {1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0},
                               {1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0},
                               {0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0},
                               {0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0},
                               {1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0},
                               {1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0},
                               {0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0},
                               {0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0},
                               {1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0},
                               {1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
                               {0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0},
                               {1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1},
                               {0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1},
                               {0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1},
                               {1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1},
                               {1, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                               {0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1},
                               {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1},
                               {1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1},
                               {1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1}};

    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 6; ++j)
        {
            m_qrData[at(size - 15 + i, 4 + j)] = versionCode[version - 6][i * 6 + j] == 0 ? WHITE : BLACK;
            m_qrData[at(4 + j, size - 15 + i)] = versionCode[version - 6][i * 6 + j] == 0 ? WHITE : BLACK;
        }
}

void QWorldWidget::setMaskPattern(int size, int corLevel)
{
    qrModule maskCorLevelCodeL0[15] = {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0};
    qrModule maskCorLevelCodeM0[15] = {1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0};
    qrModule maskCorLevelCodeQ0[15] = {0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1};
    qrModule maskCorLevelCodeH0[15] = {0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1};
    qrModule maskCorLevelCodeM6[15] = {1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1};
    qrModule maskCorLevelCodeM2[15] = {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0};

    qrModule *pMaskCorLevelCode;
    switch (corLevel) {
        case L:
            pMaskCorLevelCode = maskCorLevelCodeL0;
            break;
        case M:
            pMaskCorLevelCode = maskCorLevelCodeM2;
            break;
        case Q:
            pMaskCorLevelCode = maskCorLevelCodeQ0;
            break;
        case H:
            pMaskCorLevelCode = maskCorLevelCodeH0;
            break;
    }

    for(int i = 0; i < 7; ++i)
    {
        m_qrData[at(size - 5 - i, 12)] = *(pMaskCorLevelCode + i) == 0 ? WHITE : BLACK;

        if(m_qrData[at(12, 4 + i)] == NA)                                                           //Не занято ли полосами синхронизации
            m_qrData[at(12, 4 + i)] = *(pMaskCorLevelCode + i) == 0 ? WHITE : BLACK;
        else
            m_qrData[at(12, 4 + (i + 1))] = *(pMaskCorLevelCode + i) == 0 ? WHITE : BLACK;
    }

    for(int i = 0; i < 8; ++i)
        {
            m_qrData[at(12, size - 12 + i)] = *(pMaskCorLevelCode + i + 7) == 0 ? WHITE : BLACK;

            if(m_qrData[at(12 - i, 12)] == NA)                                                       //Не занято ли полосами синхронизации
                m_qrData[at(12 - i, 12)] = *(pMaskCorLevelCode + i + 7) == 0 ? WHITE : BLACK;
            else
                m_qrData[at(12 - (i + 1), 12)] = *(pMaskCorLevelCode + i + 7) == 0 ? WHITE : BLACK;
        }

    //Всегда черный модуль
    m_qrData[at(size - 12, 12)] = BLACK;
}

quint8 QWorldWidget::mask0(int x, int y, quint8 value)
{
//    if((i + j) % 2)
//    if(((i*j) % 2 + (i*j) % 3) % 2)
    if(x % 3)
        return value;
    else
        return value == BLACK ? WHITE : BLACK;
}

void QWorldWidget::checkNA()
{
    int iterator = 0;
    for(int i = 0; i < m_qrData.size(); ++i)
        if(m_qrData[i] == NA)
            iterator++;
    qDebug() << iterator;
}


void QWorldWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    for(int i = 0; i < m_qrData.size(); ++i)
    {
        int line = i / m_size.width();
        int col = i % m_size.width();

        QRgb color;
        if(m_qrData[i] == NA)
            color = 0xffff0000;
        else
            color = m_qrData[i] ? 0xff000000 : 0xffffffff;
        for (int x = 0; x < m_scale; ++x)
            for (int y = 0; y < m_scale; ++y)
                m_QRImage.setPixel(col * m_scale + x, line * m_scale + y, color);
    }
    painter.drawImage(QPoint(), m_QRImage);
}


void QWorldWidget::resizeEvent(QResizeEvent *event)
{
    m_QRImage = QImage(event->size(), QImage::Format_RGBA8888);
    qrModule data[] = {1, 0, 1, 0, 1,
                       0, 1, 0, 1, 0,
                       1, 0, 1, 0, 1};
    drawQR(data, QSize(5, 3));
    qDebug() << "!";
}
