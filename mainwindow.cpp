#include "mainwindow.h"
#include <QFileDialog>
#include <QVBoxLayout>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QSpinBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Создание элементов управления вручную
    loadButton = new QPushButton("Load Image", this);
    scaleButton = new QPushButton("Scale Image", this);
    imageLabel = new QLabel(this);
    scaleSlider = new QSlider(Qt::Horizontal, this);
    interpolationMethod = new QComboBox(this);

    // Метка для отображения текущего масштаба
    scaleLabel = new QLabel("Scale: 100%", this);

    // Настройка ползунка
    scaleSlider->setRange(10, 200);
    scaleSlider->setValue(100);  // Начальный коэффициент 100%

    // Добавление методов интерполяции в выпадающий список
    interpolationMethod->addItem("Bilinear");
    interpolationMethod->addItem("Bicubic");

    // Создаем QScrollArea для прокрутки
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);

    // Размещение элементов
    layout->addWidget(loadButton);
    layout->addWidget(scrollArea);
    layout->addWidget(scaleLabel);  // Добавляем метку для масштаба
    layout->addWidget(scaleSlider);
    layout->addWidget(interpolationMethod);
    layout->addWidget(scaleButton);

    // Соединение сигналов и слотов
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(scaleButton, &QPushButton::clicked, this, &MainWindow::scaleImage);
    connect(scaleSlider, &QSlider::valueChanged, this, &MainWindow::updateScaleLabel);

    // Устанавливаем фиксированный размер окна (с небольшим запасом для UI)
    setFixedSize(1000, 800);  // Размер окна, чтобы оно не было слишком маленьким
}

MainWindow::~MainWindow()
{
    // Удаление всех созданных объектов
}

void MainWindow::loadImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.jpg *.bmp *.tiff)");
    if (!filePath.isEmpty()) {
        originalImage = QImage(filePath);
        updateImageLabel(originalImage);
    }
}

void MainWindow::scaleImage()
{
    int scaleFactor = scaleSlider->value();
    int newWidth = originalImage.width() * scaleFactor / 100;
    int newHeight = originalImage.height() * scaleFactor / 100;

    QImage scaledImage;
    if (interpolationMethod->currentIndex() == 0) {
        scaledImage = bilinearInterpolation(originalImage, newWidth, newHeight);
    } else {
        scaledImage = bicubicInterpolation(originalImage, newWidth, newHeight);
    }

    updateImageLabel(scaledImage);
}

void MainWindow::updateScaleLabel(int scaleValue)
{
    // Обновление метки с текущим масштабом
    scaleLabel->setText(QString("Scale: %1%").arg(scaleValue));
}

void MainWindow::updateImageLabel(const QImage& image)
{
    QPixmap pixmap = QPixmap::fromImage(image);
    imageLabel->setPixmap(pixmap);
    imageLabel->resize(pixmap.size());

    // Обновляем размер окна, чтобы оно подстроилось под изображение
    resize(pixmap.size() + QSize(50, 100)); // Добавляем небольшой отступ, чтобы окно не было слишком маленьким
}

QImage MainWindow::bilinearInterpolation(const QImage& image, int newWidth, int newHeight)
{
    QImage result(newWidth, newHeight, image.format());
    double xRatio = static_cast<double>(image.width()) / newWidth;
    double yRatio = static_cast<double>(image.height()) / newHeight;

    for (int i = 0; i < newWidth; ++i) {
        for (int j = 0; j < newHeight; ++j) {
            double x = i * xRatio;
            double y = j * yRatio;
            int x1 = static_cast<int>(x);
            int y1 = static_cast<int>(y);
            int x2 = std::min(x1 + 1, image.width() - 1);
            int y2 = std::min(y1 + 1, image.height() - 1);

            QRgb Q11 = image.pixel(x1, y1);
            QRgb Q21 = image.pixel(x2, y1);
            QRgb Q12 = image.pixel(x1, y2);
            QRgb Q22 = image.pixel(x2, y2);

            int r = (x2 - x) * (y2 - y) * qRed(Q11) + (x - x1) * (y2 - y) * qRed(Q21) + (x2 - x) * (y - y1) * qRed(Q12) + (x - x1) * (y - y1) * qRed(Q22);
            int g = (x2 - x) * (y2 - y) * qGreen(Q11) + (x - x1) * (y2 - y) * qGreen(Q21) + (x2 - x) * (y - y1) * qGreen(Q12) + (x - x1) * (y - y1) * qGreen(Q22);
            int b = (x2 - x) * (y2 - y) * qBlue(Q11) + (x - x1) * (y2 - y) * qBlue(Q21) + (x2 - x) * (y - y1) * qBlue(Q12) + (x - x1) * (y - y1) * qBlue(Q22);

            result.setPixel(i, j, qRgb(r, g, b));
        }
    }
    return result;
}

QImage MainWindow::bicubicInterpolation(const QImage& image, int newWidth, int newHeight)
{
    QImage result(newWidth, newHeight, image.format());
    double xRatio = static_cast<double>(image.width()) / newWidth;
    double yRatio = static_cast<double>(image.height()) / newHeight;

    auto cubicInterpolate = [](double v0, double v1, double v2, double v3, double x) {
        double a0 = v3 - v2 - v0 + v1;
        double a1 = v0 - v1 - a0;
        double a2 = v2 - v0;
        double a3 = v1;
        return a0 * x * x * x + a1 * x * x + a2 * x + a3;
    };

    for (int i = 0; i < newWidth; ++i) {
        for (int j = 0; j < newHeight; ++j) {
            double x = i * xRatio;
            double y = j * yRatio;
            int x1 = static_cast<int>(x);
            int y1 = static_cast<int>(y);

            QRgb pixels[4][4];
            for (int dx = -1; dx < 3; ++dx) {
                for (int dy = -1; dy < 3; ++dy) {
                    int px = std::min(std::max(x1 + dx, 0), image.width() - 1);
                    int py = std::min(std::max(y1 + dy, 0), image.height() - 1);
                    pixels[dx + 1][dy + 1] = image.pixel(px, py);
                }
            }

            double r[4], g[4], b[4];
            for (int row = 0; row < 4; ++row) {
                r[row] = cubicInterpolate(qRed(pixels[row][0]), qRed(pixels[row][1]), qRed(pixels[row][2]), qRed(pixels[row][3]), x - x1);
                g[row] = cubicInterpolate(qGreen(pixels[row][0]), qGreen(pixels[row][1]), qGreen(pixels[row][2]), qGreen(pixels[row][3]), x - x1);
                b[row] = cubicInterpolate(qBlue(pixels[row][0]), qBlue(pixels[row][1]), qBlue(pixels[row][2]), qBlue(pixels[row][3]), x - x1);
            }

            // Нормализуем значения, чтобы они не выходили за пределы диапазона [0, 255]
            int r_value = std::min(std::max(static_cast<int>(cubicInterpolate(r[0], r[1], r[2], r[3], y - y1)), 0), 255);
            int g_value = std::min(std::max(static_cast<int>(cubicInterpolate(g[0], g[1], g[2], g[3], y - y1)), 0), 255);
            int b_value = std::min(std::max(static_cast<int>(cubicInterpolate(b[0], b[1], b[2], b[3], y - y1)), 0), 255);

            result.setPixel(i, j, qRgb(r_value, g_value, b_value));
        }
    }

    return result;
}
