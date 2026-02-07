#include "app/ImagePreviewWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <cmath>

namespace img2spec {

ImagePreviewWidget::ImagePreviewWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setStyleSheet("background-color: #2b2b2b;");
}

ImagePreviewWidget::~ImagePreviewWidget() {}

void ImagePreviewWidget::setPixmap(const QPixmap& pixmap) {
    originalPixmap_ = pixmap;
    updateScaledPixmap();
    update();
}

void ImagePreviewWidget::setFrequencyGuides(const std::vector<FrequencyGuide>& guides) {
    frequencyGuides_ = guides;
    update();
}

void ImagePreviewWidget::setPlaybackPosition(double positionSec, double durationSec) {
    playbackPositionSec_ = positionSec;
    playbackDurationSec_ = durationSec;
    update();
}

void ImagePreviewWidget::clearImage() {
    originalPixmap_ = QPixmap();
    scaledPixmap_ = QPixmap();
    frequencyGuides_.clear();
    playbackPositionSec_ = 0.0;
    playbackDurationSec_ = 0.0;
    update();
}

void ImagePreviewWidget::updateScaledPixmap() {
    if (originalPixmap_.isNull()) {
        scaledPixmap_ = QPixmap();
        return;
    }

    // Scale pixmap to fit widget while maintaining aspect ratio
    scaledPixmap_ = originalPixmap_.scaled(
        size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );
}

void ImagePreviewWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    painter.fillRect(rect(), QColor(43, 43, 43));

    if (scaledPixmap_.isNull()) {
        // Draw "No image loaded" text
        painter.setPen(QColor(136, 136, 136));
        painter.drawText(rect(), Qt::AlignCenter, "No image loaded\nDrag & drop or click 'Open Image'");
        return;
    }

    // Center the scaled pixmap
    const int x = (width() - scaledPixmap_.width()) / 2;
    const int y = (height() - scaledPixmap_.height()) / 2;
    painter.drawPixmap(x, y, scaledPixmap_);

    // Draw frequency guides
    if (!frequencyGuides_.empty() && !scaledPixmap_.isNull()) {
        const int imageTop = y;
        const int imageBottom = y + scaledPixmap_.height();
        const int imageHeight = scaledPixmap_.height();
        const int imageLeft = x;
        const int imageRight = x + scaledPixmap_.width();

        painter.setPen(QPen(QColor(255, 200, 0, 200), 2, Qt::DashLine));
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);

        for (const auto& guide : frequencyGuides_) {
            // Calculate Y position (0 = top = high freq, 1 = bottom = low freq)
            const int lineY = imageTop + static_cast<int>(guide.frequencyHz * imageHeight);

            // Draw horizontal line across the image
            painter.drawLine(imageLeft, lineY, imageRight, lineY);

            // Draw label with background
            const QString labelText = guide.label;
            QFontMetrics fm(font);
            const int textWidth = fm.horizontalAdvance(labelText);
            const int textHeight = fm.height();
            const int padding = 4;

            // Position label on the left side
            const int labelX = imageLeft + 10;
            const int labelY = lineY - textHeight / 2;

            // Draw background rectangle
            painter.fillRect(
                labelX - padding,
                labelY - padding,
                textWidth + 2 * padding,
                textHeight + 2 * padding,
                QColor(0, 0, 0, 180)
            );

            // Draw text
            painter.setPen(QColor(255, 200, 0));
            painter.drawText(labelX, labelY + fm.ascent(), labelText);

            // Restore line pen
            painter.setPen(QPen(QColor(255, 200, 0, 200), 2, Qt::DashLine));
        }
    }

    // Playback position (playhead)
    if (playbackDurationSec_ > 0 && !scaledPixmap_.isNull()) {
        const int imageTop = y;
        const int imageBottom = y + scaledPixmap_.height();
        const int imageLeft = x;
        const int imageWidth = scaledPixmap_.width();
        const double t = std::min(1.0, std::max(0.0, playbackPositionSec_ / playbackDurationSec_));
        const int headX = imageLeft + static_cast<int>(t * imageWidth);
        painter.setPen(QPen(QColor(0, 200, 255), 3, Qt::SolidLine));
        painter.drawLine(headX, imageTop, headX, imageBottom);
    }
}

void ImagePreviewWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateScaledPixmap();
}

} // namespace img2spec
