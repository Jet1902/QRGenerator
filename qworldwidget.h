#ifndef QWORLDWIDGET_H
#define QWORLDWIDGET_H

#include <QWidget>
#include <QVector>
#include "cdatamanager.h"

//typedef quint8 qrModule;
enum module {WHITE, BLACK, NA};

class QWorldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QWorldWidget(QWidget *parent = nullptr);

signals:

public slots:
    void drawQR(qrModule* data, QSize size);
    void generateByteArray(QString string);
    void generateServiceModules();

private:
    int m_scale = 10;
    QImage m_QRImage;
    QVector<qrModule> m_qrData;
    QSize m_size;
    CDataManager m_dataManager;

    int at(int i, int j);


    void putData(QVector<qrModule> data);           //Внести исходные данные
    void setSize();                                 //Задать размер в соответствии с таблицей вон та которая внизу вон она видишь? ну вон же блин
    void setSearchPattern(QPoint topLeftModule);    //Нарисовать поисковые узоры
    void setBlankSpaces(int size);                  //Нарисовать пустые места по бокам и сверху и снизу
    void setSynchroStripes(int size);               //Полосы синхронизации
    void setCorrectionPattern(QPoint topLeftModule);//Нарисовать узоры коррекции
    void setVersionPattern(int size, int version);  //Нарисовать код версии
    void setMaskPattern(int size, int corLevel);    //Нарисовать код маски и уровня коррекции
    quint8 mask0(int i, int j, quint8 value);
    void prepare();
    void checkNA();

                                        // Версия равна индексу
                                        //  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22   23   24   25   26   27   28   29   30   31   32   33   34   35   36   37   38   39   40
    int m_correctionPatternsPos[7][40] = {{-1, 18, 22, 26, 30, 34,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6},
                                          {-1, -1, -1, -1, -1, -1, 22, 24, 26, 28, 30, 32, 34, 26, 26, 26, 30, 30, 30, 34, 28, 26,  30,  28,  32,  30,  34,  26,  30,  26,  30,  34,  30,  34,  30,  24,  28,  32,  26,  30},
                                          {-1, -1, -1, -1, -1, -1, 38, 42, 46, 50, 54, 58, 62, 46, 48, 50, 54, 56, 58, 62, 50, 50,  54,  54,  58,  58,  62,  50,  54,  52,  56,  60,  58,  62,  54,  50,  54,  58,  54,  58},
                                          {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 66, 70, 74, 78, 82, 86, 90, 72, 74,  78,  80,  84,  86,  90,  74,  78,  78,  82,  86,  86,  90,  78,  76,  80,  84,  82,  86},
                                          {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 94, 98, 102, 106, 110, 114, 118,  98, 102, 104, 108, 112, 114, 118, 102, 102, 106, 110, 110, 114},
                                          {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,  -1,  -1,  -1,  -1, 122, 126, 130, 134, 138, 142, 146, 126, 128, 132, 136, 138, 142},
                                          {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 150, 154, 158, 162, 166, 170}};



    // QWidget interface
protected:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_FINAL;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_FINAL;
};

#endif // QWORLDWIDGET_H
