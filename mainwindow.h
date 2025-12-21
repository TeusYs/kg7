#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadImage();
    void scaleImage();
    void updateScaleLabel(int scaleValue); // Обновление метки с масштабом

private:
    QLabel *imageLabel;            // QLabel для отображения изображения
    QLabel *scaleLabel;            // QLabel для отображения текущего масштаба
    QSlider *scaleSlider;          // Ползунок для масштаба
    QPushButton *loadButton;       // Кнопка для загрузки изображения
    QPushButton *scaleButton;      // Кнопка для масштабирования изображения
    QComboBox *interpolationMethod;// Выпадающий список для выбора метода интерполяции

    QImage originalImage;
    enum class InterpolationMethod { Bilinear, Bicubic };
    InterpolationMethod currentInterpolationMethod_ = InterpolationMethod::Bilinear;

    void updateImageLabel(const QImage& image);
    QImage bilinearInterpolation(const QImage& image, int newWidth, int newHeight);
    QImage bicubicInterpolation(const QImage& image, int newWidth, int newHeight);
};

#endif // MAINWINDOW_H
