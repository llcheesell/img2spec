#pragma once

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <vector>

namespace img2spec {

struct FrequencyGuide {
    double frequencyHz;
    QString label;
};

class ImagePreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit ImagePreviewWidget(QWidget* parent = nullptr);
    ~ImagePreviewWidget();

    void setPixmap(const QPixmap& pixmap);
    void setFrequencyGuides(const std::vector<FrequencyGuide>& guides);
    void clearImage();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateScaledPixmap();

    QPixmap originalPixmap_;
    QPixmap scaledPixmap_;
    std::vector<FrequencyGuide> frequencyGuides_;
};

} // namespace img2spec
